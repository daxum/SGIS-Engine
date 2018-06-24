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

#include "Logger.hpp"

struct QueueFamilyIndices {
	int graphicsFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0;
	}
};

//Handles creation and destruction of all vulkan objects.
class VkObjectHandler {
public:
	/**
	 * Initializes vulkan objects.
	 */
	VkObjectHandler(Logger& logger);

	/**
	 *Destroys all active objects.
	 */
	~VkObjectHandler();

private:
	//The logger
	Logger& logger;

	//Vulkan instance
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkDebugReportCallbackEXT callback;
	VkQueue graphicsQueue;

	//List of enabled validation layers
	std::vector<const char*> enabledLayerNames;

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

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice);

	/**
	 * Creates the logical device from the physical device.
	 */
	void createLogicalDevice();

	/**
	 * Vulkan debug callback.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* mesg, void* usrData);
};
