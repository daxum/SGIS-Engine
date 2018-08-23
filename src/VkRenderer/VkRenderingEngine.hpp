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
#include "VkMemoryManager.hpp"
#include "VkShader.hpp"

class VkRenderingEngine : public RenderingEngine {
public:
	//Max number of queued frames to be rendered.
	constexpr static size_t MAX_ACTIVE_FRAMES = 2;

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
	 * Called when drawing is done and the results can be displayed on the screen.
	 * Also sets up the pipeline for the next frame.
	 */
	void present() override;

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
	 * Renders the visible objects, using the sorted map.
	 * @param objects A set of objects that have been determined to be visible.
	 * @param sortedObjects All objects, sorted by buffer, then shader, then model.
	 * @param camera The current camera.
	 * @param state User-provided screen state.
	 */
	void renderObjects(const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) override;

private:
	//Interface with the window system.
	GlfwInterface interface;
	//Handles all internal vulkan objects.
	VkObjectHandler objectHandler;
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

	//The current frame being rendered, always between 0 and MAX_ACTIVE_FRAMES.
	size_t currentFrame;

	/**
	 * Renders a single transparency pass.
	 * @param pass The current rendering pass.
	 * @param visibleObjects A set of objects visible on the screen.
	 * @param objects A container of the objects to render. See RenderComponentManager.hpp for what it actually is.
	 * @param camera The camera for the current screen.
	 * @param screenState The state of the current screen.
	 */
	void renderTransparencyPass(RenderPass pass, const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects, std::shared_ptr<const Camera> camera, std::shared_ptr<const ScreenState> screenState);

	/**
	 * Sets the push constant values for the provided object.
	 * @param shader The shader the object uses, contains push constant offsets and usage flags.
	 * @param comp The object to set push constants for.
	 * @param camera The current camera, for view transforms.
	 */
	void setPushConstants(const std::shared_ptr<const VkShader>& shader, const RenderComponent* comp, const std::shared_ptr<const Camera>& camera);

	/**
	 * Sets the per-screen uniforms for set in the provided aligner using the values obtained from state and camera.
	 * @param set The uniform set containing values to set.
	 * @param aligner The object the values are set in.
	 * @param state The screen state object to obtain SCREEN_STATE values from.
	 * @param camera The camera object to obtain CAMERA_* values from.
	 */
	void setPerScreenUniforms(const UniformSet& set, Std140Aligner& aligner, const ScreenState* state, const Camera* camera);

	/**
	 * Sets the per-object uniforms for the given object.
	 * @param set The set for the given object.
	 * @param aligner The aligner to write the uniforms to.
	 * @param comp The render component for the object.
	 * @param camera The current camera.
	 */
	void setPerObjectUniforms(const UniformSet& set, Std140Aligner& aligner, const RenderComponent* comp, const Camera* camera);

	/**
	 * Sets the value in aligner to the provided value.
	 * @param type The type of the uniform to set.
	 * @param uniformName The name of the uniform to set.
	 * @param value The value to set the uniform to.
	 * @param aligner The aligner to set the value in.
	 */
	static constexpr void setUniformValue(const UniformType type, const std::string& uniformName, const void* value, Std140Aligner& aligner) {
		switch (type) {
			case UniformType::FLOAT: aligner.setFloat(uniformName, *(const float*)value); break;
			case UniformType::VEC2: aligner.setVec2(uniformName, *(const glm::vec2*)value); break;
			case UniformType::VEC3: aligner.setVec3(uniformName, *(const glm::vec3*)value); break;
			case UniformType::VEC4: aligner.setVec4(uniformName, *(const glm::vec4*)value); break;
			case UniformType::MAT3: aligner.setMat3(uniformName, *(const glm::mat3*)value); break;
			case UniformType::MAT4: aligner.setMat4(uniformName, *(const glm::mat4*)value); break;
			default: throw std::runtime_error("Invalid buffered uniform type for uniform \"" + uniformName + "\"!");
		}
	}
};
