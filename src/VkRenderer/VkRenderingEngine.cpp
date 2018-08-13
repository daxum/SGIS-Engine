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
#include "VkRenderInitializer.hpp"

VkRenderingEngine::VkRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog, const LogConfig& loaderLog) :
	RenderingEngine(/** TODO **/ std::shared_ptr<TextureLoader>(),
					std::make_shared<VkShaderLoader>(objectHandler, &memoryManager, loaderLogger, shaderMap),
					std::make_shared<VkRenderInitializer>(objectHandler, &memoryManager),
					rendererLog,
					loaderLog),
	interface(display, this),
	objectHandler(logger),
	memoryManager(rendererLog, objectHandler),
	currentImageIndex(0),
	currentFrame(0) {

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
	objectHandler.deinit();

	GLFWwindow* window = interface.getWindow();

	if (window != nullptr) {
		glfwDestroyWindow(window);
	}

	glfwTerminate();

	logger.info("Destroyed Vulkan rendering engine.");
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

	logger.info("Created window");

	//Register callbacks

	interface.init(window);

	//Create vulkan objects and memory manager

	objectHandler.init(window);
	memoryManager.init();

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

	VkResult result = vkAcquireNextImageKHR(objectHandler.getDevice(), objectHandler.getSwapchain(), 0xFFFFFFFFFFFFFFFF, imageAvailable.at(currentFrame), VK_NULL_HANDLE, &currentImageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		vkDeviceWaitIdle(objectHandler.getDevice());
		objectHandler.recreateSwapchain();
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
}

void VkRenderingEngine::present() {
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

	VkSwapchainKHR swapchain = objectHandler.getSwapchain();

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
		objectHandler.recreateSwapchain();
		std::static_pointer_cast<VkShaderLoader>(shaderLoader)->reloadShaders();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present!");
	}

	currentFrame = (currentFrame + 1) % MAX_ACTIVE_FRAMES;
}

void VkRenderingEngine::setViewport(int width, int height) {
	//Width / height unused, retrieved from window interface in below function.
	vkDeviceWaitIdle(objectHandler.getDevice());
	objectHandler.recreateSwapchain();
	std::static_pointer_cast<VkShaderLoader>(shaderLoader)->reloadShaders();
}

void VkRenderingEngine::renderObjects(const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	//TODO: this still needs fixing
	VkClearValue clearColor = {0.0f, 0.2f, 0.5f, 1.0f};

	VkRenderPassBeginInfo passBeginInfo = {};
	passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passBeginInfo.renderPass = objectHandler.getRenderPass();
	passBeginInfo.framebuffer = objectHandler.getFramebuffer(currentImageIndex);
	passBeginInfo.renderArea.offset = {0, 0};
	passBeginInfo.renderArea.extent = objectHandler.getSwapchainExtent();
	passBeginInfo.clearValueCount = 1;
	passBeginInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffers.at(currentFrame), &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	renderTransparencyPass(RenderPass::OPAQUE, objects, sortedObjects);
	renderTransparencyPass(RenderPass::TRANSPARENT, objects, sortedObjects);
	renderTransparencyPass(RenderPass::TRANSLUCENT, objects, sortedObjects);

	vkCmdEndRenderPass(commandBuffers.at(currentFrame));
}

void VkRenderingEngine::renderTransparencyPass(RenderPass pass, const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects) {
	//TODO: this needs to be made threadable, and just rewritten in general - it's currently just a direct port of the GlRenderingEngine's loop.

	std::string currentBuffer = "";
	std::string currentShader = "";

	for (const auto& shaderObjectMap : sortedObjects) {
		const std::string& buffer = shaderObjectMap.first;

		for (const auto& modelMap : shaderObjectMap.second) {
			const std::string& shaderName = modelMap.first;
			const std::shared_ptr<VkShader> shader = shaderMap.at(shaderName);

			//Skip these objects if their shader isn't in the current render pass
			if (shader->getRenderPass() != pass) {
				continue;
			}

			for (const auto& objectSet : modelMap.second) {
				for (const std::shared_ptr<RenderComponent>& comp : objectSet.second) {
					if (objects.count(comp.get())) {
						//Set shader / buffer if needed
						if (currentShader != shaderName) {
							vkCmdBindPipeline(commandBuffers.at(currentFrame), VK_PIPELINE_BIND_POINT_GRAPHICS, shader->getPipeline());
							currentShader = shaderName;
						}

						if (currentBuffer != buffer) {
							const VkDeviceSize zero = 0;
							std::shared_ptr<VkBufferData> bufferData = std::static_pointer_cast<VkBufferData>(memoryManager.getBuffer(buffer).getRenderData());

							vkCmdBindVertexBuffers(commandBuffers.at(currentFrame), 0, 1, &bufferData->vertexBuffer, &zero);
							vkCmdBindIndexBuffer(commandBuffers.at(currentFrame), bufferData->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

							currentBuffer = buffer;
						}

						//TODO: per-object push constants go here

						const VkMeshRenderData& meshRenderData = memoryManager.getMeshRenderData(comp->getModel()->getModel().mesh);

						vkCmdDrawIndexed(commandBuffers.at(currentFrame), meshRenderData.indexCount, 1, meshRenderData.indexStart, 0, 0);
					}
				}
			}
		}
	}
}
