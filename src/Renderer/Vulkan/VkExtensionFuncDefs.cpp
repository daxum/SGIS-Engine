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

#include <unordered_map>
#include <vulkan/vulkan.h>

//This file contains definitions for extension functions which use dynamically loaded
//function pointers. Not necessarily the best way of doing things, but hopefully good enough.

namespace {
	struct FuncPtrs {
		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
	};

	//Not threadsafe
	std::unordered_map<VkInstance, FuncPtrs> instanceFuncMap;
}

size_t loadInstanceExtensionFunctions(VkInstance instance) {
	size_t failCount = 0;

	FuncPtrs ptrs = {0};
	ptrs.vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (!ptrs.vkCreateDebugReportCallbackEXT) failCount++;
	ptrs.vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (!ptrs.vkDestroyDebugReportCallbackEXT) failCount++;

	instanceFuncMap.insert({instance, ptrs});

	return failCount;
}

void destroyInstanceExtensionFunctions(VkInstance instance) {
	instanceFuncMap.erase(instance);
}

//Extension function definitions

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback) {

	auto func = instanceFuncMap.at(instance).vkCreateDebugReportCallbackEXT;

	if (func) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator) {

	auto func = instanceFuncMap.at(instance).vkDestroyDebugReportCallbackEXT;

	if (func) {
		func(instance, callback, pAllocator);
	}
}
