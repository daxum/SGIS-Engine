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

#include "Renderer/RenderingEngine.hpp"
#include "Display/DisplayEngine.hpp"
#include "Renderer/GlfwInterface.hpp"
#include "VkObjectHandler.hpp"
#include "VkMemoryManager.hpp"
#include "VkShader.hpp"
#include "VkRenderObjects.hpp"

class VkRenderingEngine : public RenderingEngine {
public:
	/**
	 * Initializes a vulkan rendering engine.
	 * @param display A reference to the display engine, for input callbacks.
	 * @param renderLog Configuration for the rendering logger.
	 */
	VkRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog);

	/**
	 * Destroys all the vulkan stuff.
	 */
	~VkRenderingEngine();

	/**
	 * Initializes vulkan.
	 */
	void init() override;

	/**
	 * Gets the memory manager.
	 * @return the memory manager.
	 */
	RendererMemoryManager* getMemoryManager() override { return &memoryManager; }

	/**
	 * Finishes initialization, uploads stuff to the gpu.
	 */
	void finishLoad() override;

	/**
	 * Called at the very start of a frame. Begins the command buffer
	 * and executes any pending transfers.
	 */
	void beginFrame() override;

	/**
	 * Called when the window size has changed and the viewport needs to be updated.
	 * @param width The new window width.
	 * @param height The new window height.
	 */
	void setViewport(int width, int height) override;

	/**
	 * Gets the interface to the window, provides things like window size.
	 * @return The interface to the window system.
	 */
	const WindowSystemInterface& getWindowInterface() const override { return interface; }

protected:
	/**
	 * Called when drawing is done and the results can be displayed on the screen.
	 * Also sets up the pipeline for the next frame.
	 */
	void apiPresent() override;

	/**
	 * Renders the visible objects, using the sorted map.
	 * @param sortedObjects All objects, sorted by buffer, then shader, then model.
	 * @param screen The screen to render.
	 */
	void renderObjects(RenderManager::RenderPassList sortedObjects, const Screen* screen) override;

private:
	//Interface with the window system.
	GlfwInterface interface;
	//Handles all internal vulkan objects.
	VkObjectHandler objectHandler;
	//Handles rendering related vulkan objects, such as the swapchain.
	VkRenderObjects swapObjects;
	//Shader map.
	std::unordered_map<std::string, std::shared_ptr<VkShader>> shaderMap;
	//Vulkan memory manager, handles buffers and such.
	VkMemoryManager memoryManager;
	//Command buffers used to render stuff.
	std::array<VkCommandBuffer, MAX_ACTIVE_FRAMES> commandBuffers;
	//Current swapchain image index.
	uint32_t currentImageIndex;

	//Rendering semaphores, one for each frame.
	std::array<VkSemaphore, MAX_ACTIVE_FRAMES> imageAvailable;
	std::array<VkSemaphore, MAX_ACTIVE_FRAMES> renderFinished;
	std::array<VkFence, MAX_ACTIVE_FRAMES> renderFences;

	/**
	 * Renders a single transparency pass.
	 * @param pass The current rendering pass.
	 * @param objects A container of the objects to render. See RenderComponentManager.hpp for what it actually is.
	 * @param camera The camera for the current screen.
	 * @param screenState The state of the current screen.
	 * @return Whether something was drawn, used to shut up the validation layers until a better solution is found.
	 */
	bool renderTransparencyPass(RenderPass pass, RenderManager::RenderPassList sortedObjects, const Camera* camera, const ScreenState* screenState);

	/**
	 * Sets the push constant values for the provided object.
	 * @param shader The shader the object uses, contains push constant offsets and usage flags.
	 * @param comp The object to set push constants for.
	 * @param camera The current camera, for view transforms.
	 */
	void setPushConstants(const std::shared_ptr<const VkShader>& shader, const RenderComponent* comp, const Camera* camera);
};
