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
#include <GLFW/glfw3.h>

#include "VkObjectHandler.hpp"
#include "Engine.hpp"
#include "VkExtensionFunctionLoader.hpp"
#include "ExtraMath.hpp"

namespace {
	//TODO: These things should go in the engine config

	const std::vector<std::string> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	const std::vector<std::string> enableLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};
};

VkObjectHandler::VkObjectHandler(Logger& logger, GLFWwindow* window) :
	logger(logger) {

	createInstance();

	size_t numFailed = loadInstanceExtensionFunctions(instance);
	if (numFailed > 0) {
		logger.warn("Failed to load " + std::to_string(numFailed) + " vulkan extension functions");
	}
	else {
		logger.info("Loaded all vulkan extension functions");
	}

	setDebugCallback();
	createSurface(window);
	setPhysicalDevice();
	createLogicalDevice();
	createSwapchain();
}

VkObjectHandler::~VkObjectHandler() {
	vkDestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroySwapchainKHR(device, swapchain, nullptr);
	vkDestroyDevice(device, nullptr);
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

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	logger.debug("Found " + std::to_string(layerCount) + " validation layers:");

	std::vector<VkLayerProperties> layers(layerCount);
	enabledLayerNames.reserve(layerCount);

	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	for (const VkLayerProperties& layer : layers) {
		bool enabled = false;

		for (const std::string& name : enableLayers) {
			if (name == layer.layerName) {
				enabledLayerNames.push_back(layer.layerName);
				enabled = true;
				break;
			}
		}

		logger.debug(std::string("\t") + layer.layerName + " " + std::to_string(layer.specVersion) + " - " + std::to_string(layer.implementationVersion) + ":");
		logger.debug(std::string("\t\tEnabled: ") + (enabled ? "Yes" : "No"));
		logger.debug(std::string("\t\t") + layer.description);
	}

	if (enabledLayerNames.size() != enableLayers.size()) {
		logger.warn("Not all validation layers loaded!");
	}

	//Create instance

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = extensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
	instanceCreateInfo.enabledLayerCount = enabledLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan instance!");
	}

	logger.debug("Created vulkan instance");
}

void VkObjectHandler::createSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
}

void VkObjectHandler::setDebugCallback() {
	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = VkObjectHandler::debugCallback;
	callbackCreateInfo.pUserData = this;

	if (vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback) != VK_SUCCESS) {
		logger.warn("Couldn't create debug report callback");
	}
}

void VkObjectHandler::setPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		logger.error("No devices supporting vulkan, and yet vulkan is installed?");
		throw std::runtime_error("No vulkan supporting devices found");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	//Remove unsuitable devices
	removeInsufficientDevices(devices);

	if (devices.empty()) {
		throw std::runtime_error("No suitable device found!");
	}

	//Take first dgpu found, if doesn't exist, just take first gpu.
	bool deviceSet = false;

	for (const VkPhysicalDevice& physDevice : devices) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physDevice, &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			deviceSet = true;
			physicalDevice = physDevice;
		}
	}

	if (!deviceSet) {
		physicalDevice = devices.at(0);
	}
}

void VkObjectHandler::removeInsufficientDevices(std::vector<VkPhysicalDevice>& devices) {
	for (size_t i = devices.size(); i > 0; i--) {
		VkPhysicalDevice physDevice = devices.at(i - 1);

		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceProperties(physDevice, &properties);
		vkGetPhysicalDeviceFeatures(physDevice, &features);

		if (!findQueueFamilies(physDevice).isComplete() || !deviceHasAllExtensions(physDevice, requiredExtensions)) {
			devices.erase(devices.begin() + (i - 1));
			continue;
		}

		//Only do this if required extensions are found (VK_KHR_swapchain in particular)
		SwapchainSupportDetails details = querySwapChainSupport(physDevice);

		if (details.formats.empty() || details.presentModes.empty()) {
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
		if (queueFamilies.at(i).queueCount > 0 && queueFamilies.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			out.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, surface, &presentSupport);

		if (queueFamilies.at(i).queueCount > 0 && presentSupport) {
			out.presentFamily = i;
		}

		if (out.isComplete()) {
			break;
		}
	}

	return out;
}

void VkObjectHandler::createLogicalDevice() {
	QueueFamilyIndices queueIndices = findQueueFamilies(physicalDevice);
	float queuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::unordered_set<int> uniqueQueueFamilies = { queueIndices.graphicsFamily, queueIndices.presentFamily };

	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures usedDeviceFeatures = {};

	std::vector<const char*> deviceExtensions;
	deviceExtensions.reserve(requiredExtensions.size());

	for (const std::string& s : requiredExtensions) {
		deviceExtensions.push_back(s.c_str());
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
	deviceCreateInfo.pEnabledFeatures = &usedDeviceFeatures;
	deviceCreateInfo.enabledLayerCount = enabledLayerNames.size();
	deviceCreateInfo.ppEnabledLayerNames = enabledLayerNames.data();
	deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device");
	}

	vkGetDeviceQueue(device, queueIndices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueIndices.presentFamily, 0, &presentQueue);
}

SwapchainSupportDetails VkObjectHandler::querySwapChainSupport(VkPhysicalDevice physDevice) {
	SwapchainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

void VkObjectHandler::createSwapchain() {
	SwapchainSupportDetails details = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR format = chooseBestFormat(details.formats);
	VkExtent2D extent = getSwapExtent(details.capabilities);
	uint32_t imageCount = std::max(3u, details.capabilities.minImageCount);

	if (details.capabilities.maxImageCount != 0) {
		imageCount = std::min(imageCount, details.capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.imageFormat = format.format;
	swapchainCreateInfo.imageColorSpace = format.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = details.capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { (uint32_t) indices.presentFamily, (uint32_t) indices.graphicsFamily };

	if (indices.graphicsFamily != indices.presentFamily) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!");
	}

	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

	swapchainImageFormat = format.format;
	swapchainExtent = extent;

	logger.debug("Created swapchain with " + std::to_string(imageCount) + " images");
}

VkSurfaceFormatKHR VkObjectHandler::chooseBestFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
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

VkExtent2D VkObjectHandler::getSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	const WindowSystemInterface& windowInterface = Engine::instance->getWindowInterface();

	VkExtent2D extent = { (uint32_t) windowInterface.getWindowWidth(), (uint32_t) windowInterface.getWindowHeight() };

	ExMath::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	ExMath::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
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

	return VK_FALSE;
}
