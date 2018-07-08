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

#include <array>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "RenderingEngine.hpp"
#include "DisplayEngine.hpp"
#include "GlfwInterface.hpp"
#include "VkObjectHandler.hpp"

class VkRenderingEngine : public RenderingEngine {
public:
	/**
	 * Initializes a vulkan rendering engine.
	 * @param display A reference to the display engine, for input callbacks.
	 * @param renderLog Configuration for the rendering logger.
	 * @param loaderLog Configuration for the loader logger.
	 */
	VkRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog, const LogConfig& loaderLog);

	/**
	 * Destroys all the vulkan stuff.
	 */
	~VkRenderingEngine();

	/**
	 * Initializes vulkan.
	 */
	void init();

	/**
	 * Gets the memory manager.
	 * @return the memory manager.
	 */
	RendererMemoryManager* getMemoryManager() { return nullptr; /** TODO **/ }

	/**
	 * Finishes initialization, uploads stuff to the gpu.
	 */
	void finishLoad();

	/**
	 * Renders the passed in object.
	 * @param data The stuff to render.
	 * @param camera The camera to use.
	 * @param state User-set screen state, passed to shader when setting uniforms.
	 */
	void render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state);

	/**
	 * Called clearBuffers for lack of a better name. Clears the depth and stencil
	 * buffers, but not the color buffer.
	 */
	void clearBuffers();

	/**
	 * Called when drawing is done and the results can be displayed on the screen.
	 * Also sets up the pipeline for the next frame.
	 */
	void present();

	/**
	 * Called when the window size has changed and the viewport needs to be updated.
	 * @param width The new window width.
	 * @param height The new window height.
	 */
	void setViewport(int width, int height);

	/**
	 * Gets the interface to the window, provides things like window size.
	 * @return The interface to the window system.
	 */
	const WindowSystemInterface& getWindowInterface() const { return interface; }

private:
	constexpr static size_t MAX_ACTIVE_FRAMES = 2;

	//Interface with the window system.
	GlfwInterface interface;
	//Handles all internal vulkan objects.
	VkObjectHandler objectHandler;
	//Shader map
	std::unordered_map<std::string, std::shared_ptr<Shader>> shaderMap;

	//Rendering semaphores, one for each frame
	std::array<VkSemaphore, MAX_ACTIVE_FRAMES> imageAvailable;
	std::array<VkSemaphore, MAX_ACTIVE_FRAMES> renderFinished;
	std::array<VkFence, MAX_ACTIVE_FRAMES> renderFences;

	//The current frame being rendered, always between 0 and MAX_ACTIVE_FRAMES
	size_t currentFrame;

};
