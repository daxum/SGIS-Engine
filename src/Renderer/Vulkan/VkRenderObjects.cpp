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

#include "VkRenderObjects.hpp"
#include "WindowSystemInterface.hpp"
#include "ExtraMath.hpp"
#include "Engine.hpp"

VkRenderObjects::VkRenderObjects(VkObjectHandler& vkObjects) :
	vkObjects(vkObjects),
	swapchain(VK_NULL_HANDLE),
	renderPass(VK_NULL_HANDLE),
	swapchainImages(),
	imageViews(),
	depthView(VK_NULL_HANDLE),
	framebuffers(),
	swapchainImageFormat(VK_FORMAT_UNDEFINED),
	swapchainExtent{0, 0} {

}

void VkRenderObjects::init(VkMemoryManager& memoryManager) {
	createSwapchain();
	createImageViews();
	createRenderPass();

	depthView = memoryManager.createDepthBuffer(swapchainExtent);

	createFramebuffers();
}

void VkRenderObjects::deinit() {
	for (VkFramebuffer framebuffer : framebuffers) {
		vkDestroyFramebuffer(vkObjects.getDevice(), framebuffer, nullptr);
	}

	vkDestroyRenderPass(vkObjects.getDevice(), renderPass, nullptr);

	for (VkImageView view : imageViews) {
		vkDestroyImageView(vkObjects.getDevice(), view, nullptr);
	}

	vkDestroySwapchainKHR(vkObjects.getDevice(), swapchain, nullptr);
}

void VkRenderObjects::reinit(VkMemoryManager& memoryManager) {
	deinit();
	init(memoryManager);
}

void VkRenderObjects::createSwapchain() {
	SwapchainSupportInfo swapInfo = getSwapChainSupport(vkObjects.getPhysicalDevice());

	VkSurfaceFormatKHR format = chooseBestFormat(swapInfo.formats);
	VkExtent2D extent = getSwapExtent(swapInfo.capabilities);
	uint32_t imageCount = std::max(3u, swapInfo.capabilities.minImageCount);

	if (swapInfo.capabilities.maxImageCount != 0) {
		imageCount = std::min(imageCount, swapInfo.capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = vkObjects.getWindowSurface();
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = format.format;
	swapchainCreateInfo.imageColorSpace = format.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = swapInfo.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	uint32_t queueFamilyIndices[] = { vkObjects.getPresentQueueIndex(), vkObjects.getGraphicsQueueIndex() };

	if (vkObjects.hasUniquePresent()) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	if (vkCreateSwapchainKHR(vkObjects.getDevice(), &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(vkObjects.getDevice(), swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vkObjects.getDevice(), swapchain, &imageCount, swapchainImages.data());

	swapchainImageFormat = format.format;
	swapchainExtent = extent;
}

SwapchainSupportInfo VkRenderObjects::getSwapChainSupport(VkPhysicalDevice physDevice) {
	SwapchainSupportInfo info;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, vkObjects.getWindowSurface(), &info.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, vkObjects.getWindowSurface(), &formatCount, nullptr);

	if (formatCount != 0) {
		info.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, vkObjects.getWindowSurface(), &formatCount, info.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, vkObjects.getWindowSurface(), &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		info.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, vkObjects.getWindowSurface(), &presentModeCount, info.presentModes.data());
	}

	return info;
}

VkSurfaceFormatKHR VkRenderObjects::chooseBestFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	if (formats.size() == 1 && formats.at(0).format == VK_FORMAT_UNDEFINED) {
		return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	for (const VkSurfaceFormatKHR& format : formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return formats.at(0);
}

VkExtent2D VkRenderObjects::getSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != 0xFFFFFFFF) {
		return capabilities.currentExtent;
	}

	const WindowSystemInterface& windowInterface = Engine::instance->getWindowInterface();

	VkExtent2D extent = { (uint32_t) windowInterface.getWindowWidth(), (uint32_t) windowInterface.getWindowHeight() };

	ExMath::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	ExMath::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}

void VkRenderObjects::createImageViews() {
	imageViews.resize(swapchainImages.size());

	for (size_t i = 0; i < imageViews.size(); i++) {
		VkImageViewCreateInfo viewCreateInfo = {};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = swapchainImages.at(i);
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = swapchainImageFormat;
		viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vkObjects.getDevice(), &viewCreateInfo, nullptr, &imageViews.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

void VkRenderObjects::createRenderPass() {
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	//TODO: fall back to VK_FORMAT_D16_UNORM if not available?
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference attachRef = {};
	attachRef.attachment = 0;
	attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthRef = {};
	depthRef.attachment = 1;
	depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachRef;
	subpass.pDepthStencilAttachment = &depthRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderCreateInfo = {};
	renderCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderCreateInfo.attachmentCount = attachments.size();
	renderCreateInfo.pAttachments = attachments.data();
	renderCreateInfo.subpassCount = 1;
	renderCreateInfo.pSubpasses = &subpass;
	renderCreateInfo.dependencyCount = 1;
	renderCreateInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(vkObjects.getDevice(), &renderCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void VkRenderObjects::createFramebuffers() {
	framebuffers.resize(imageViews.size());

	for (size_t i = 0; i < imageViews.size(); i++) {
		VkImageView attachments[] = {
			imageViews.at(i),
			depthView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = swapchainExtent.width;
		framebufferCreateInfo.height = swapchainExtent.height;
		framebufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(vkObjects.getDevice(), &framebufferCreateInfo, nullptr, &framebuffers.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}
