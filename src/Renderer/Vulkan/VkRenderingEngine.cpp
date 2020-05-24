/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include "VkRenderingEngine.hpp"
#include "Engine.hpp"
#include "VkShaderLoader.hpp"
#include "VkShader.hpp"
#include "VkTextureLoader.hpp"

namespace {
	const glm::mat4 projectionCorrection = {
		{1.0, 0.0, 0.0, 0.0},
		{0.0, -1.0, 0.0, 0.0},
		{0.0, 0.0, 0.5, 0.0},
		{0.0, 0.0, 0.5, 1.0}
	};
}

VkRenderingEngine::VkRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog) :
	RenderingEngine(std::make_shared<VkTextureLoader>(objectHandler, memoryManager),
					std::make_shared<VkShaderLoader>(objectHandler, swapObjects, &memoryManager, shaderMap),
					&memoryManager,
					rendererLog),
	interface(display, this),
	objectHandler(logger),
	swapObjects(objectHandler),
	memoryManager(rendererLog, objectHandler),
	currentImageIndex(0) {

	if (!glfwInit()) {
		throw std::runtime_error("Couldn't initialize glfw");
	}
}

VkRenderingEngine::~VkRenderingEngine() {
	//Don't destroy things while rendering.
	vkDeviceWaitIdle(objectHandler.getDevice());

	for (size_t i = 0; i < MAX_ACTIVE_FRAMES; i++) {
		vkDestroySemaphore(objectHandler.getDevice(), imageAvailable.at(i), nullptr);
		vkDestroySemaphore(objectHandler.getDevice(), renderFinished.at(i), nullptr);
		vkDestroyFence(objectHandler.getDevice(), renderFences.at(i), nullptr);
	}

	shaderMap.clear();
	memoryManager.deinit();
	shaderLoader.reset();
	swapObjects.deinit();
	objectHandler.deinit();

	GLFWwindow* window = interface.getWindow();

	if (window != nullptr) {
		glfwDestroyWindow(window);
	}

	glfwTerminate();

	ENGINE_LOG_INFO(logger, "Destroyed Vulkan rendering engine.");
}

void VkRenderingEngine::init() {
	//Initialize window

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	const int initWidth = Engine::instance->getConfig().renderer.windowWidth;
	const int initHeight = Engine::instance->getConfig().renderer.windowHeight;
	const std::string& title = Engine::instance->getConfig().renderer.windowTitle;

	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, title.c_str(), nullptr, nullptr);

	if (window == nullptr) {
		throw std::runtime_error("Failed to create window and context");
	}

	ENGINE_LOG_INFO(logger, "Created window");

	//Register callbacks

	interface.init(window);

	//Create vulkan objects and memory manager

	objectHandler.init(window);
	memoryManager.init();
	swapObjects.init(memoryManager);

	//Allocate command buffers

	VkCommandBufferAllocateInfo bufferAllocInfo = {};
	bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocInfo.commandPool = objectHandler.getCommandPool();
	bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferAllocInfo.commandBufferCount = commandBuffers.size();

	if (vkAllocateCommandBuffers(objectHandler.getDevice(), &bufferAllocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	//Create semaphores

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_ACTIVE_FRAMES; i++) {
		if (vkCreateSemaphore(objectHandler.getDevice(), &semaphoreInfo, nullptr, &imageAvailable.at(i)) != VK_SUCCESS ||
			vkCreateSemaphore(objectHandler.getDevice(), &semaphoreInfo, nullptr, &renderFinished.at(i)) != VK_SUCCESS ||
			vkCreateFence(objectHandler.getDevice(), &fenceInfo, nullptr, &renderFences.at(i)) != VK_SUCCESS) {

			throw std::runtime_error("Failed to create semaphores or fences!");
		}
	}
}

void VkRenderingEngine::finishLoad() {
	/** TODO: Something might go here. At some point. Maybe. **/
}

void VkRenderingEngine::beginFrame() {
	memoryManager.executeTransfers();

	//Crash if a frame takes too long to render - this usually happens because something is wrong with the fences.
	if (vkWaitForFences(objectHandler.getDevice(), 1, &renderFences.at(currentFrame), VK_TRUE, 20u * 1'000'000'000u) == VK_TIMEOUT) {
		throw std::runtime_error("Fence wait timed out!");
	}

	currentImageIndex = 0;

	VkResult result = vkAcquireNextImageKHR(objectHandler.getDevice(), swapObjects.getSwapchain(), 0xFFFFFFFFFFFFFFFF, imageAvailable.at(currentFrame), VK_NULL_HANDLE, &currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		vkDeviceWaitIdle(objectHandler.getDevice());
		swapObjects.reinit(memoryManager);
		std::static_pointer_cast<VkShaderLoader>(shaderLoader)->reloadShaders();
		//Eventually this'll work
		beginFrame();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to get image!");
	}

	//Reset fence here because of the return above
	vkResetFences(objectHandler.getDevice(), 1, &renderFences.at(currentFrame));

	//Begin command buffer for this frame
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffers.at(currentFrame), &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to start recording command buffer!");
	}

	//Render pass, move elsewhere later
	VkClearValue clearColor = {};
	clearColor.color = {{0.0f, 0.2f, 0.5f, 1.0f}};

	VkClearValue clearDepth = {};
	clearDepth.depthStencil = {1.0f, 0};

	VkClearValue clearValues[] = { clearColor, clearDepth };

	VkRenderPassBeginInfo passBeginInfo = {};
	passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passBeginInfo.renderPass = swapObjects.getRenderPass();
	passBeginInfo.framebuffer = swapObjects.getFramebuffer(currentImageIndex);
	passBeginInfo.renderArea.offset = {0, 0};
	passBeginInfo.renderArea.extent = swapObjects.getSwapchainExtent();
	passBeginInfo.clearValueCount = 2;
	passBeginInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(commandBuffers.at(currentFrame), &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VkRenderingEngine::setViewport(int width, int height) {
	//Width / height unused, retrieved from window interface in below function.
	vkDeviceWaitIdle(objectHandler.getDevice());
	swapObjects.reinit(memoryManager);
	std::static_pointer_cast<VkShaderLoader>(shaderLoader)->reloadShaders();
}

void VkRenderingEngine::apiPresent() {
	vkCmdEndRenderPass(commandBuffers.at(currentFrame));

	if (vkEndCommandBuffer(commandBuffers.at(currentFrame)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.");
	}

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable.at(currentFrame);
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers.at(currentFrame);
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinished.at(currentFrame);

	if (vkQueueSubmit(objectHandler.getGraphicsQueue(), 1, &submitInfo, renderFences.at(currentFrame)) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit command buffer.");
	}

	VkSwapchainKHR swapchain = swapObjects.getSwapchain();

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderFinished.at(currentFrame);
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &currentImageIndex;

	VkResult result = vkQueuePresentKHR(objectHandler.getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		vkDeviceWaitIdle(objectHandler.getDevice());
		swapObjects.reinit(memoryManager);
		std::static_pointer_cast<VkShaderLoader>(shaderLoader)->reloadShaders();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present!");
	}
}

void VkRenderingEngine::renderObjects(RenderManager::RenderPassList sortedObjects, const Screen* screen) {
	const Camera* camera = screen->getCamera().get();
	const ScreenState* state = screen->getState().get();

	bool drewSomething = false;

	drewSomething |= renderTransparencyPass(RenderPass::OPAQUE, sortedObjects, camera, state);
	drewSomething |= renderTransparencyPass(RenderPass::TRANSPARENT, sortedObjects, camera, state);
	drewSomething |= renderTransparencyPass(RenderPass::TRANSLUCENT, sortedObjects, camera, state);

	if (drewSomething) {
		//TODO: generate render passes at engine initialization to render this unnecessary
		VkClearAttachment depthClear = {};
		depthClear.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthClear.clearValue.depthStencil = {1.0f, 0};

		VkClearRect clearRect = {};
		clearRect.rect.extent = swapObjects.getSwapchainExtent();
		clearRect.layerCount = 1;

		vkCmdClearAttachments(commandBuffers.at(currentFrame), 1, &depthClear, 1, &clearRect);
	}
}

bool VkRenderingEngine::renderTransparencyPass(RenderPass pass, RenderManager::RenderPassList sortedObjects, const Camera* camera, const ScreenState* screenState) {
	//TODO: this needs to be made threadable, and just rewritten in general - it's currently just a direct port of the GlRenderingEngine's loop.
	//Also, each loop should probably be its own function, this is getting ridiculous.

	bool drewSomething = false;

	//Per-buffer loop
	for (const auto& shaderObjectMap : sortedObjects) {
		bool bufferBound = false;

		//Per-shader loop
		for (const auto& modelMap : shaderObjectMap.second) {
			const std::string& shaderName = modelMap.first;
			const std::shared_ptr<VkShader> shader = shaderMap.at(shaderName);
			bool shaderBound = false;

			//Skip these objects if their shader isn't in the current render pass
			if (shader->getRenderPass() != pass) {
				continue;
			}

			bool screenSetBound = false;
			const std::string& screenSetName = shader->getPerScreenDescriptor();

			//Per-model loop
			for (const auto& objectSet : modelMap.second) {
				const Material* material = objectSet.first;

				bool materialSetBound = false;

				//Per-object loop
				for (const RenderComponent* comp : objectSet.second) {
					if (!comp->isVisible()) {
						continue;
					}

					//Set shader / buffer if needed
					if (!shaderBound) {
						vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shader->getPipeline());
						shaderBound = true;
					}

					if (!bufferBound) {
						const VkDeviceSize zero = 0;

						//For now, assume that vertex buffers are always paired with the same index buffers
						const Mesh::BufferInfo& buffers = comp->getModel().mesh->getBufferInfo();

						VkBuffer vertexBuffer = ((const VkBufferContainer*) buffers.vertex)->getBuffer();
						VkBuffer indexBuffer = ((const VkBufferContainer*) buffers.index)->getBuffer();

						vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), 0, 1, &vertexBuffer, &zero);
						vkCmdBindIndexBuffer(commandBuffers.at(currentFrame), indexBuffer, 0, VK_INDEX_TYPE_UINT32);

						bufferBound = true;
					}

					//Bind descriptor sets if needed

					std::array<VkDescriptorSet, 3> bindSets;
					std::array<uint32_t, 3> bindOffsets;
					size_t numSets = 0;
					size_t numOffsets = 0;
					//Which set to start binding at - don't rebind already bound sets.
					size_t startSet = 0;

					//Screen set
					if (!screenSetName.empty()) {
						if (!screenSetBound) {
							Std140Aligner& screenAligner = memoryManager.getDescriptorAligner(screenSetName);

							setPerScreenUniforms(memoryManager.getUniformSet(screenSetName), screenAligner, screenState, camera, projectionCorrection);

							bindSets.at(numSets) = memoryManager.getDescriptorSet(screenSetName);
							bindOffsets.at(numOffsets) = memoryManager.writePerFrameUniforms(screenAligner, currentFrame);

							screenSetBound = true;
							numSets++;
							numOffsets++;
						}
						else {
							//Screen set already bound, start binding at model set
							startSet++;
						}
					}

					//Model set
					if (!materialSetBound) {
						bindSets.at(numSets) = memoryManager.getDescriptorSet(material->name);
						numSets++;

						if (material->hasBufferedUniforms) {
							bindOffsets.at(numOffsets) = material->uniformOffset;
							numOffsets++;
						}

						materialSetBound = true;
					}
					else {
						//Model set will never be bound without a screen set, so this is perfectly safe
						startSet++;
					}

					//Object set
					if (!shader->getPerObjectDescriptor().empty()) {
						const std::string& objectDescriptor = shader->getPerObjectDescriptor();

						Std140Aligner& objectAligner = memoryManager.getDescriptorAligner(objectDescriptor);

						setPerObjectUniforms(memoryManager.getUniformSet(objectDescriptor), objectAligner, comp, camera);

						bindSets.at(numSets) = memoryManager.getDescriptorSet(objectDescriptor);
						bindOffsets.at(numOffsets) = memoryManager.writePerFrameUniforms(objectAligner, currentFrame);

						numSets++;
						numOffsets++;
					}

					if (numSets > 0) {
						vkCmdBindDescriptorSets(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shader->getPipelineLayout(), startSet, numSets, bindSets.data(), numOffsets, bindOffsets.data());
					}

					setPushConstants(shader, comp, camera);

					const std::tuple<uintptr_t, uint32_t, int32_t> meshInfo = comp->getModel().mesh->getRenderInfo();

					vkCmdDrawIndexed(commandBuffers.at(currentFrame), std::get<1>(meshInfo), 1, std::get<0>(meshInfo), std::get<2>(meshInfo), 0);
					drewSomething = true;
				}
			}
		}
	}

	return drewSomething;
}

void VkRenderingEngine::setPushConstants(const std::shared_ptr<const VkShader>& shader, const RenderComponent* comp, const Camera* camera) {
	//Need to make this bigger if the minimum size ever changes. Maybe make it 256 (biggest value seen for maxPushConstantsSize) and restrict it dynamically?
	unsigned char pushConstantMem[128];
	const std::vector<PushRange>& pushRanges = shader->getPushConstantRanges();

	for (const PushRange& range : pushRanges) {
		for (size_t i = 0; i < range.pushOffsets.size(); i++) {
			const UniformDescription& uniform = range.pushData.at(i);
			const uint32_t offset = range.pushOffsets.at(i);

			glm::mat4 tempMat;
			const void* pushVal = nullptr;

			switch (uniform.provider) {
				case UniformProviderType::OBJECT_STATE: pushVal = comp->getParentState()->getRenderValue(uniform.name); break;
				case UniformProviderType::OBJECT_TRANSFORM: tempMat = comp->getTransform(); pushVal = &tempMat; break;
				case UniformProviderType::OBJECT_MODEL_VIEW: tempMat = camera->getView() * comp->getTransform(); pushVal = &tempMat; break;
				default: throw std::runtime_error("Invalid push constant provider!");
			}

			//mat3 needs to be handled specially because of the alignment of vec3
			if (uniform.type != UniformType::MAT3) {
				memcpy(&pushConstantMem[offset], pushVal, uniformSize(uniform.type));
			}
			else {
				memcpy(&pushConstantMem[offset], pushVal, uniformSize(UniformType::VEC3));
				memcpy(&pushConstantMem[offset + uniformSize(UniformType::VEC4)], ((unsigned char*)pushVal) + 3 * sizeof(float), uniformSize(UniformType::VEC3));
				memcpy(&pushConstantMem[offset + 2 * uniformSize(UniformType::VEC4)], ((unsigned char*)pushVal) + 6 * sizeof(float), uniformSize(UniformType::VEC3));
			}
		}

		vkCmdPushConstants(commandBuffers.at(currentFrame), shader->getPipelineLayout(), range.shaderStages.to_ulong(), range.start, range.size, &pushConstantMem[range.start]);
	}
}
