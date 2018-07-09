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

namespace {
	//Temporary, for testing purposes only!
	std::vector<VkCommandBuffer> commandBuffers;
}

VkRenderingEngine::VkRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog, const LogConfig& loaderLog) :
	RenderingEngine(/** TODO **/ std::shared_ptr<TextureLoader>(), std::make_shared<VkShaderLoader>(objectHandler, loaderLogger, shaderMap), rendererLog, loaderLog),
	interface(display, this),
	objectHandler(logger),
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

	//Create vulkan objects

	objectHandler.init(window);

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
	//Very nasty, but temporary, hack.
	commandBuffers = objectHandler.getCommandBuffers(std::static_pointer_cast<VkShader>(shaderMap.at("basic")->getRenderInterface())->pipeline);
	/** TODO **/
}

void VkRenderingEngine::render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	//Crash if a frame takes too long to render - this usually happens because something is wrong with the fences. There are 9 0's in the second number below.
	if (vkWaitForFences(objectHandler.getDevice(), 1, &renderFences.at(currentFrame), VK_TRUE, 20u * 1000000000u) == VK_TIMEOUT) {
		throw std::runtime_error("Fence wait timed out!");
	}

	uint32_t imageIndex = 0;

	VkResult result = vkAcquireNextImageKHR(objectHandler.getDevice(), objectHandler.getSwapchain(), 0xFFFFFFFFFFFFFFFF, imageAvailable.at(currentFrame), VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		//TODO: this is in three places, and needs to be moved somewhere where it can be handled for all models at once.
		vkDeviceWaitIdle(objectHandler.getDevice());
		vkFreeCommandBuffers(objectHandler.getDevice(), objectHandler.getCommandPool(), commandBuffers.size(), commandBuffers.data());
		objectHandler.recreateSwapchain();
		commandBuffers = objectHandler.getCommandBuffers(std::static_pointer_cast<VkShader>(shaderMap.at("basic")->getRenderInterface())->pipeline);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to get image!");
	}

	//Reset fence here because of the return above
	vkResetFences(objectHandler.getDevice(), 1, &renderFences.at(currentFrame));

	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailable.at(currentFrame);
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers.at(imageIndex);
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
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(objectHandler.getPresentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		vkDeviceWaitIdle(objectHandler.getDevice());
		vkFreeCommandBuffers(objectHandler.getDevice(), objectHandler.getCommandPool(), commandBuffers.size(), commandBuffers.data());
		objectHandler.recreateSwapchain();
		commandBuffers = objectHandler.getCommandBuffers(std::static_pointer_cast<VkShader>(shaderMap.at("basic")->getRenderInterface())->pipeline);
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present!");
	}

	currentFrame = (currentFrame + 1) % MAX_ACTIVE_FRAMES;
}

void VkRenderingEngine::clearBuffers() {
	/** TODO **/
}

void VkRenderingEngine::present() {
	/** TODO **/
}

void VkRenderingEngine::setViewport(int width, int height) {
	//Width / height unused, retrieved from window interface in below function.
	vkDeviceWaitIdle(objectHandler.getDevice());
	vkFreeCommandBuffers(objectHandler.getDevice(), objectHandler.getCommandPool(), commandBuffers.size(), commandBuffers.data());
	objectHandler.recreateSwapchain();
	commandBuffers = objectHandler.getCommandBuffers(std::static_pointer_cast<VkShader>(shaderMap.at("basic")->getRenderInterface())->pipeline);
}
