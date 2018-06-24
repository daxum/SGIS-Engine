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

VkRenderingEngine::VkRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog, const LogConfig& loaderLog) :
	RenderingEngine(/** TODO **/ nullptr, nullptr, rendererLog, loaderLog),
	interface(display, this),
	objectHandler(nullptr) {

	if (!glfwInit()) {
		throw std::runtime_error("Couldn't initialize glfw");
	}
}

VkRenderingEngine::~VkRenderingEngine() {
	//Cleans up vulkan
	delete objectHandler;

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

	if (objectHandler != nullptr) {
		throw std::runtime_error("Init called more than once in VkRenderingEngine!");
	}

	objectHandler = new VkObjectHandler(logger, window);
}

void VkRenderingEngine::finishLoad() {
	/** TODO **/
}

void VkRenderingEngine::render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) {
	/** TODO **/
}

void VkRenderingEngine::clearBuffers() {
	/** TODO **/
}

void VkRenderingEngine::present() {
	/** TODO **/
}

void VkRenderingEngine::setViewport(int width, int height) {
	/** TODO **/
}
