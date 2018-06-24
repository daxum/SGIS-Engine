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

#include <GLFW/glfw3.h>

#include "VkObjectHandler.hpp"
#include "Engine.hpp"
#include "VkExtensionFunctionLoader.hpp"

VkObjectHandler::VkObjectHandler(Logger& logger) :
	logger(logger) {

	createInstance();
	loadInstanceExtensionFunctions(instance);
	setDebugCallback();
}

VkObjectHandler::~VkObjectHandler() {
	vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroyInstance(instance, nullptr);

	//This is perfectly safe, the instance isn't accessed in any way.
	destroyInstanceExtensionFunctions(instance);
}

void VkObjectHandler::createInstance() {
	const EngineConfig& config = Engine::instance->getConfig();

	//Create application info

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = config.gameName.c_str();
	appInfo.applicationVersion = config.gameVersion;
	appInfo.pEngineName = "Hello driver, how are you today?";
	appInfo.engineVersion = VK_MAKE_VERSION(Engine::VERSION_MAJOR, Engine::VERSION_MINOR, Engine::VERSION_PATCH);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

	//Get extension information

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	logger.debug("Found " + std::to_string(extensionCount) + " vulkan instance extensions:");

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (const VkExtensionProperties& extension : extensions) {
		logger.debug(std::string("\t") + extension.extensionName + " " + std::to_string(extension.specVersion));
	}

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	logger.debug("Required glfw extensions:");

	std::vector<const char*> extensionNames;

	for (uint32_t i = 0; i < glfwExtensionCount; i++) {
		extensionNames.push_back(glfwExtensions[i]);

		logger.debug(std::string("\t") + glfwExtensions[i]);
	}

	extensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	//Validation layers (TODO: allow turning off later, specify which ones from game)

	const std::vector<std::string> enableLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	logger.debug("Found " + std::to_string(layerCount) + " validation layers:");

	std::vector<VkLayerProperties> layers(layerCount);
	std::vector<const char*> layerNames;
	layerNames.reserve(layerCount);

	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	for (const VkLayerProperties& layer : layers) {
		bool enabled = false;

		for (const std::string& name : enableLayers) {
			if (name == layer.layerName) {
				layerNames.push_back(layer.layerName);
				enabled = true;
				break;
			}
		}

		logger.debug(std::string("\t") + layer.layerName + " " + std::to_string(layer.specVersion) + " - " + std::to_string(layer.implementationVersion) + ":");
		logger.debug(std::string("\t\tEnabled: ") + (enabled ? "Yes" : "No"));
		logger.debug(std::string("\t\t") + layer.description);
	}

	if (layerNames.size() != enableLayers.size()) {
		logger.warn("Not all validation layers loaded!");
	}

	//Create instance

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = extensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
	instanceCreateInfo.enabledLayerCount = layerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = layerNames.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan instance!");
	}

	logger.debug("Created vulkan instance");
}

void VkObjectHandler::setDebugCallback() {
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	callbackCreateInfo.pfnCallback = VkObjectHandler::debugCallback;
	callbackCreateInfo.pUserData = this;

	vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkObjectHandler::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* mesg, void* usrData) {
	std::string message("Message from layer \"");
	message = message + layerPrefix + "\": " + mesg;

	VkObjectHandler* objHandler = (VkObjectHandler*) usrData;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		objHandler->logger.error(message);
	}
	else if (flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
		objHandler->logger.warn(message);
	}
	else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
		objHandler->logger.info(message);
	}
	else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
		objHandler->logger.debug(message);
	}

	return VK_FALSE;
}
