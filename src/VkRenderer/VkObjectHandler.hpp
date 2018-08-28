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
#include <bitset>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Logger.hpp"

struct QueueFamilyIndices {
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	uint32_t transferFamily;

	//1 bit - graphics, 2 bit - present, 4 bit - transfer
	std::bitset<3> foundFamilies;
};

//Handles creation and destruction of all vulkan objects.
class VkObjectHandler {
public:
	/**
	 * Sets logger.
	 */
	VkObjectHandler(Logger& logger);

	/**
	 * Initializes all objects.
	 */
	void init(GLFWwindow* window);

	/**
	 * Destroys all objects.
	 */
	void deinit();

	/**
	 * Returns whether the graphics and present queues are separate.
	 * @return true if graphics and present queues are different, false if same.
	 */
	bool hasUniquePresent() const { return presentQueueIndex != graphicsQueueIndex; }

	/**
	 * Returns whether the graphics and transfer queues are separate.
	 * @return true if graphics and transfer queues are different, false if same.
	 */
	bool hasUniqueTransfer() const { return transferQueueIndex != graphicsQueueIndex; }

	/**
	 * Device related getters.
	 */
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const { return physicalDeviceProperties; }
	const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const { return physicalDeviceFeatures; }
	VkDevice getDevice() { return device; }
	VkCommandPool getCommandPool() { return commandPool; }
	VkCommandPool getTransferCommandPool() { return transferCommandPool; }

	/**
	 * Queue getters.
	 */
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentQueue() { return presentQueue; }
	VkQueue getTransferQueue() { return transferQueue; }
	uint32_t getGraphicsQueueIndex() const { return graphicsQueueIndex; }
	uint32_t getPresentQueueIndex() const { return presentQueueIndex; }
	uint32_t getTransferQueueIndex() const { return transferQueueIndex; }

	/**
	 * Gets the surface for the swapchain.
	 * @return The window surface.
	 */
	VkSurfaceKHR getWindowSurface() { return surface; }

private:
	//The logger.
	Logger& logger;

	//Vulkan Objects.
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	VkDevice device;
	VkDebugReportCallbackEXT callback;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VkSurfaceKHR surface;
	VkCommandPool commandPool;
	VkCommandPool transferCommandPool;

	//Queue indices for physical device.
	uint32_t graphicsQueueIndex;
	uint32_t presentQueueIndex;
	uint32_t transferQueueIndex;

	//List of enabled validation layers.
	std::vector<std::string> enabledLayerNames;

	/**
	 * Creates the instance object.
	 */
	void createInstance();

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
	 * Creates all command pools.
	 */
	void createCommandPools();

	/**
	 * Basically dumps the relevant information from the physicalDeviceProperties
	 * object to the log.
	 */
	void logPhysicalDeviceProperties();

	/**
	 * Prints out the availability of relevent device features.
	 */
	void logPhysicalDeviceFeatures();

	/**
	 * Vulkan debug callback.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* mesg, void* usrData);
};
