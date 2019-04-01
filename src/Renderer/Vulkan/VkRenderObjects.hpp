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

#pragma once

#include <vulkan/vulkan.h>

#include "VkObjectHandler.hpp"
#include "VkMemoryManager.hpp"

struct SwapchainSupportInfo {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

//These names get worse every time...
class VkRenderObjects {
public:
	VkRenderObjects(VkObjectHandler& vkObjects);

	/**
	 * Initializes the swapchain and related objects.
	 * @param memoryManager The memory manager used to create the depth buffer.
	 */
	void init(VkMemoryManager& memoryManager);

	/**
	 * Destroys the swapchain.
	 */
	void deinit();

	/**
	 * Reinitializes the swapchain, which is necessary on events like
	 * window resizes. This does mostly the same thing as calling deinit
	 * then init.
	 * @param The memory manager used to create the depth buffer.
	 */
	void reinit(VkMemoryManager& memoryManager);

	/**
	 * Misc. getters for swapchain related stuff.
	 */
	const VkExtent2D& getSwapchainExtent() const {return swapchainExtent; }
	VkRenderPass getRenderPass() { return renderPass; }
	VkSwapchainKHR getSwapchain() { return swapchain; }
	VkFramebuffer getFramebuffer(size_t index) { return framebuffers.at(index); }

private:
	//Object handler, for device / physical device.
	VkObjectHandler& vkObjects;

	//Created swapchain.
	VkSwapchainKHR swapchain;
	//Render pass.
	VkRenderPass renderPass;
	//Images for the swapchain.
	std::vector<VkImage> swapchainImages;
	//Views for swapchain images.
	std::vector<VkImageView> imageViews;
	//Image view for the depth buffer.
	VkImageView depthView;
	//Framebuffers.
	std::vector<VkFramebuffer> framebuffers;
	//Format of the swapchain images.
	VkFormat swapchainImageFormat;
	//Size of the framebuffers.
	VkExtent2D swapchainExtent;

	/**
	 * Creates the swapchain object.
	 */
	void createSwapchain();

	/**
	 * Gets swapchain support for the given device.
	 * @param physDevice the device to query support for.
	 * @return information about the swapchain support for the given device.
	 */
	SwapchainSupportInfo getSwapChainSupport(VkPhysicalDevice physDevice);

	/**
	 * Takes the best format from the list.
	 * @param formats A list of the available formats.
	 * @return The best format in the list.
	 */
	VkSurfaceFormatKHR chooseBestFormat(const std::vector<VkSurfaceFormatKHR>& formats);

	/**
	 * Gets the swap chain resolution.
	 */
	VkExtent2D getSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/**
	 * Creates image views for each of the swap chain's images.
	 */
	void createImageViews();

	/**
	 * Creates the render pass. This might belong somewhere else?
	 */
	void createRenderPass();

	/**
	 * Creates the framebuffers.
	 */
	void createFramebuffers();
};
