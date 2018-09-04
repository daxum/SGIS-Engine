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

#include <unordered_set>
#include <vulkan/vulkan.h>

#include "VkObjectHandler.hpp"
#include "Engine.hpp"
#include "VkExtensionFunctionLoader.hpp"
#include "ExtraMath.hpp"
#include "VkPDPD.hpp"

namespace {
	const std::vector<std::string> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
};

VkObjectHandler::VkObjectHandler(Logger& logger) :
	logger(logger),
	graphicsQueueIndex(0),
	presentQueueIndex(0),
	transferQueueIndex(0) {

}

void VkObjectHandler::init(GLFWwindow* window) {
	createInstance();

	size_t numFailed = loadInstanceExtensionFunctions(instance);
	if (numFailed > 0) {
		ENGINE_LOG_WARN(logger, "Failed to load " + std::to_string(numFailed) + " vulkan extension functions");
	}
	else {
		ENGINE_LOG_INFO(logger, "Loaded all vulkan extension functions");
	}

	setDebugCallback();

	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}

	setPhysicalDevice();
	createLogicalDevice();
	createCommandPools();
}

void VkObjectHandler::deinit() {
	vkDestroyCommandPool(device, commandPool, nullptr);

	if (hasUniqueTransfer()) {
		vkDestroyCommandPool(device, transferCommandPool, nullptr);
	}

	vkDestroyDevice(device, nullptr);
	vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
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

	ENGINE_LOG_DEBUG(logger, "Found " + std::to_string(extensionCount) + " vulkan instance extensions:");

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	for (const VkExtensionProperties& extension : extensions) {
		ENGINE_LOG_DEBUG(logger, std::string("\t") + extension.extensionName + " " + std::to_string(extension.specVersion));
	}

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	ENGINE_LOG_DEBUG(logger, "Required glfw extensions:");

	std::vector<const char*> extensionNames;

	for (uint32_t i = 0; i < glfwExtensionCount; i++) {
		extensionNames.push_back(glfwExtensions[i]);

		ENGINE_LOG_DEBUG(logger, std::string("\t") + glfwExtensions[i]);
	}

	extensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	//Validation layers
	std::vector<const char*> layerCStr;

	if (!config.renderer.validationLayers.empty()) {
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		ENGINE_LOG_DEBUG(logger, "Found " + std::to_string(layerCount) + " validation layers:");

		std::vector<VkLayerProperties> layers(layerCount);
		enabledLayerNames.reserve(layerCount);

		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

		for (const VkLayerProperties& layer : layers) {
			bool enabled = false;

			for (const std::string& name : config.renderer.validationLayers) {
				if (name == layer.layerName) {
					enabledLayerNames.push_back(layer.layerName);
					enabled = true;
					break;
				}
			}

			ENGINE_LOG_DEBUG(logger, std::string("\t") + layer.layerName + " " + std::to_string(layer.specVersion) + " - " + std::to_string(layer.implementationVersion) + ":");
			ENGINE_LOG_DEBUG(logger, std::string("\t\tEnabled: ") + (enabled ? "Yes" : "No"));
			ENGINE_LOG_DEBUG(logger, std::string("\t\t") + layer.description);
		}

		if (enabledLayerNames.size() != config.renderer.validationLayers.size()) {
			ENGINE_LOG_WARN(logger, "Not all validation layers loaded!");
		}

		layerCStr.reserve(enabledLayerNames.size());

		for (const std::string& s : enabledLayerNames) {
			layerCStr.push_back(s.c_str());
		}
	}

	//Create instance

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = extensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
	instanceCreateInfo.enabledLayerCount = layerCStr.size();
	instanceCreateInfo.ppEnabledLayerNames = layerCStr.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan instance!");
	}

	ENGINE_LOG_DEBUG(logger, "Created vulkan instance");
}

void VkObjectHandler::setDebugCallback() {
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = VkObjectHandler::debugCallback;
	callbackCreateInfo.pUserData = this;

	if (vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback) != VK_SUCCESS) {
		ENGINE_LOG_WARN(logger, "Couldn't create debug report callback");
	}
}

void VkObjectHandler::setPhysicalDevice() {
	const EngineConfig& config = Engine::instance->getConfig();

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		ENGINE_LOG_ERROR(logger, "No devices supporting vulkan, and yet vulkan is installed?");
		throw std::runtime_error("No vulkan supporting devices found");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	//Force the physical device to the one in the config if requested
	if (config.renderer.deviceOverride && config.renderer.forceIndex < devices.size()) {
		physicalDevice = devices.at(config.renderer.forceIndex);
	}
	else {
		//Tried to force device, but failed due to invalid index
		if (config.renderer.deviceOverride && config.renderer.forceIndex >= devices.size()) {
			ENGINE_LOG_WARN(logger, "Failed to force physical device - invalid device index " + std::to_string(config.renderer.forceIndex) + " (max " + std::to_string(devices.size() - 1) + ")");
		}

		//Remove unsuitable devices
		removeInsufficientDevices(devices);

		if (devices.empty()) {
			throw std::runtime_error("No suitable device found!");
		}

		ENGINE_LOG_DEBUG(logger, std::to_string(devices.size()) + " physical devices found");

		//Take first dgpu found, if doesn't exist, just take first gpu.
		bool deviceSet = false;

		for (const VkPhysicalDevice& physDevice : devices) {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(physDevice, &properties);

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				deviceSet = true;
				physicalDevice = physDevice;
				break;
			}
		}

		if (!deviceSet) {
			physicalDevice = devices.at(0);
		}
	}

	//Set queue families and physical device properties
	QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
	graphicsQueueIndex = queueIndices.graphicsFamily;
	presentQueueIndex = queueIndices.presentFamily;
	transferQueueIndex = queueIndices.transferFamily;

	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

	logPhysicalDeviceProperties();
	logPhysicalDeviceFeatures();
}

void VkObjectHandler::removeInsufficientDevices(std::vector<VkPhysicalDevice>& devices) {
	for (size_t i = devices.size(); i > 0; i--) {
		VkPhysicalDevice physDevice = devices.at(i - 1);

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(physDevice, &properties);
		vkGetPhysicalDeviceFeatures(physDevice, &features);

		if (!findQueueFamilies(physDevice).foundFamilies.all() || !deviceHasAllExtensions(physDevice, requiredExtensions)) {
			devices.erase(devices.begin() + (i - 1));
			continue;
		}

		//Only do this if required extensions are found (VK_KHR_swapchain in particular)
		uint32_t formatCount = 0;

		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);

		if (formatCount == 0) {
			devices.erase(devices.begin() + (i - 1));
		}
	}
}

bool VkObjectHandler::deviceHasAllExtensions(VkPhysicalDevice device, const std::vector<std::string>& extensions) {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::unordered_set<std::string> unavailableExtensions(extensions.begin(), extensions.end());

	for (const VkExtensionProperties& property : availableExtensions) {
		unavailableExtensions.erase(property.extensionName);
	}

	return unavailableExtensions.empty();
}

QueueFamilyIndices VkObjectHandler::findQueueFamilies(VkPhysicalDevice physDevice) {
	QueueFamilyIndices out;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilies.data());

	for (size_t i = 0; i < queueFamilies.size(); i++) {
		//Why would this happen...?
		if (queueFamilies.at(i).queueCount == 0) {
			continue;
		}

		if (queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			out.graphicsFamily = i;
			out.foundFamilies.set(0);
		}
		else if (queueFamilies.at(i).queueFlags & VK_QUEUE_TRANSFER_BIT) {
			//Prefer only transfer to transfer + compute
			if (!out.foundFamilies.test(1) || (queueFamilies.at(out.transferFamily).queueFlags & VK_QUEUE_COMPUTE_BIT)) {
				out.transferFamily = i;
				out.foundFamilies.set(1);
			}
		}

		if (!out.foundFamilies.test(2) || !out.foundFamilies.test(0) || out.presentFamily != out.graphicsFamily) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, surface, &presentSupport);

			if (presentSupport) {
				out.presentFamily = i;
				out.foundFamilies.set(2);
			}
		}

		if (out.foundFamilies.all()) {
			break;
		}
	}

	if (!out.foundFamilies.test(1)) {
		out.transferFamily = out.graphicsFamily;
	}

	return out;
}

void VkObjectHandler::createLogicalDevice() {
	float queuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::unordered_set<uint32_t> uniqueQueueFamilies = { graphicsQueueIndex, presentQueueIndex, transferQueueIndex };

	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures usedDeviceFeatures = {};
	usedDeviceFeatures.samplerAnisotropy = physicalDeviceFeatures.samplerAnisotropy;

	std::vector<const char*> deviceExtensions;
	deviceExtensions.reserve(requiredExtensions.size());

	for (const std::string& s : requiredExtensions) {
		deviceExtensions.push_back(s.c_str());
	}

	std::vector<const char*> layerCStr;
	layerCStr.reserve(enabledLayerNames.size());

	for (const std::string& s : enabledLayerNames) {
		layerCStr.push_back(s.c_str());
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pEnabledFeatures = &usedDeviceFeatures;
	deviceCreateInfo.enabledLayerCount = layerCStr.size();
	deviceCreateInfo.ppEnabledLayerNames = layerCStr.data();
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);
	vkGetDeviceQueue(device, transferQueueIndex, 0, &transferQueue);
}

void VkObjectHandler::createCommandPools() {
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = graphicsQueueIndex;

	if (vkCreateCommandPool(device, &poolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}

	if (hasUniqueTransfer()) {
		VkCommandPoolCreateInfo transferPoolCreateInfo = {};
		transferPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		transferPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		transferPoolCreateInfo.queueFamilyIndex = transferQueueIndex;

		if (vkCreateCommandPool(device, &transferPoolCreateInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create transfer command pool!");
		}
	}
	else {
		transferCommandPool = commandPool;
	}
}

void VkObjectHandler::logPhysicalDeviceProperties() {
	ENGINE_LOG_INFO(logger, std::string("Physical device properties:"));
	ENGINE_LOG_INFO(logger, std::string("\tApi version: ") +
										std::to_string(VK_VERSION_MAJOR(physicalDeviceProperties.apiVersion)) + "." +
										std::to_string(VK_VERSION_MINOR(physicalDeviceProperties.apiVersion)) + "." +
										std::to_string(VK_VERSION_PATCH(physicalDeviceProperties.apiVersion)));
	ENGINE_LOG_INFO(logger, std::string("\tDriver version: ") +
										std::to_string(VK_VERSION_MAJOR(physicalDeviceProperties.driverVersion)) + "." +
										std::to_string(VK_VERSION_MINOR(physicalDeviceProperties.driverVersion)) + "." +
										std::to_string(VK_VERSION_PATCH(physicalDeviceProperties.driverVersion)));
	ENGINE_LOG_INFO(logger, std::string("\tVendor: ") + getPciVendorName(physicalDeviceProperties.vendorID));
	ENGINE_LOG_INFO(logger, std::string("\tDevice: ") + physicalDeviceProperties.deviceName);
	ENGINE_LOG_INFO(logger, std::string("\tDevice type: ") + getDeviceTypeName(physicalDeviceProperties.deviceType));
	ENGINE_LOG_DEBUG(logger, std::string("\tDevice id: ") + std::to_string(physicalDeviceProperties.deviceID));
}

void VkObjectHandler::logPhysicalDeviceFeatures() {
	ENGINE_LOG_INFO(logger, "Feature availability:");
	ENGINE_LOG_INFO(logger, "\tAnisotropic filtering: " + std::string(physicalDeviceFeatures.samplerAnisotropy ? "Yes" : "No"));
	ENGINE_LOG_INFO(logger, "\tMax Anisotropy: " + std::to_string(physicalDeviceProperties.limits.maxSamplerAnisotropy));
}

VKAPI_ATTR VkBool32 VKAPI_CALL VkObjectHandler::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* mesg, void* usrData) {
	std::string message("Message from layer \"");
	message = message + layerPrefix + "\": " + mesg;

	VkObjectHandler* objHandler = (VkObjectHandler*) usrData;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		ENGINE_LOG_ERROR(objHandler->logger, message);
	}
	else if (flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) {
		ENGINE_LOG_WARN(objHandler->logger, message);
	}

	return VK_FALSE;
}
