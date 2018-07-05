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

#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Logger.hpp"

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

//Handles creation and destruction of all vulkan objects.
class VkObjectHandler {
public:
	/**
	 * Sets logger.
	 */
	VkObjectHandler(Logger& logger);

	/**
	 *Destroys all active objects.
	 */
	~VkObjectHandler();

	/**
	 * Initializes all objects.
	 */
	void init(GLFWwindow* window);


	VkDevice getDevice() { return device; }
	const VkExtent2D& getSwapchainExtent() const {return swapchainExtent; }
	VkRenderPass getRenderPass() { return renderPass; }

private:
	//The logger
	Logger& logger;

	//Vulkan Objects
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkDebugReportCallbackEXT callback;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkRenderPass renderPass;

	//Swapchain stuff
	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExtent;

	//List of enabled validation layers
	std::vector<std::string> enabledLayerNames;

	/**
	 * Creates the instance object.
	 */
	void createInstance();

	/**
	 * Creates the window surface for rendering.
	 * @param window The window to create the surface for.
	 */
	void createSurface(GLFWwindow* window);

	/**
	 * Sets the debug callback.
	 */
	void setDebugCallback();

	/**
	 * Gets the best physical device pointer from the instance.
	 */
	void setPhysicalDevice();

	/**
	 * Removes all devices that don't meet requirements from the list.
	 * @param devices A list of devices to check.
	 */
	void removeInsufficientDevices(std::vector<VkPhysicalDevice>& devices);

	/**
	 * Checks whether the device supports the given extensions.
	 * @param physDevice The device to check.
	 * @param extensions The extensions to verify the presence of.
	 * @return whether all the extensions were present.
	 */
	bool deviceHasAllExtensions(VkPhysicalDevice physDevice, const std::vector<std::string>& extensions);

	/**
	 * Finds all necessary queue families for the given device.
	 */
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice);

	/**
	 * Creates the logical device from the physical device.
	 */
	void createLogicalDevice();

	/**
	 * Gets swapchain support for the given device.
	 * @param physDevice the device to query support for.
	 */
	SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice physDevice);

	/**
	 * Creates the swap chain object.
	 */
	void createSwapchain();

	/**
	 * Creates image views for each of the swap chain's images.
	 */
	void createImageViews();

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
	 * Creates the render pass. This might belong somewhere else?
	 */
	void createRenderPass();

	/**
	 * Creates the framebuffers.
	 */
	void createFramebuffers();

	/**
	 * Vulkan debug callback.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* mesg, void* usrData);
};
