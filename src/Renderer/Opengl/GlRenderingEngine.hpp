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

#include <unordered_map>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "Renderer/RenderingEngine.hpp"
#include "CombinedGl.h"
#include "GlShader.hpp"
#include "GlMemoryManager.hpp"
#include "GlTextureLoader.hpp"
#include "Logger.hpp"
#include "Components/RenderComponent.hpp"
#include "Renderer/GlfwInterface.hpp"

//An implementation of RenderingEngine that uses the OpenGL graphics api.
class GlRenderingEngine : public RenderingEngine {
public:
	/**
	 * Constructs a GlRenderingEngine and initializes small parts of
	 * glfw - it just calls the init function and sets the error callback.
	 * @param display A reference to the engine's display engine, used for callbacks.
	 * @param rendererLog The logger config for the rendering engine.
	 * @throw runtime_error if glfw initialization failed.
	 */
	GlRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog);

	/**
	 * Destroys the window and terminates glfw
	 */
	virtual ~GlRenderingEngine();

	/**
	 * Initializes OpenGL. A window is created, functions are loaded,
	 * callbacks are registered, and state defaults are set.
	 * @throw runtime_error if initialization failed.
	 */
	virtual void init() override;

	/**
	 * Gets the memory manager for this rendering engine, used for uploading
	 * models.
	 * @return The memory manager for this rendering engine.
	 */
	RendererMemoryManager* getMemoryManager() override { return &memoryManager; }

	/**
	 * This used to do stuff.
	 */
	void finishLoad() override {}

	/**
	 * Nothing needs to happen here.
	 */
	void beginFrame() override {}

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
	 * Swaps the buffers and clears for the next frame.
	 */
	void apiPresent() override;

	/**
	 * Renders the passed in objects.
	 * @param sortedObjects A sorted map of maps of maps of sets of all the possible objects to render.
	 * @param screen The screen to render.
	 */
	void renderObjects(RenderManager::RenderPassList sortedObjects, const Screen* screen) override;

private:
	//A map to store texture data
	std::unordered_map<std::string, GlTextureData> textureMap;
	//A map to store the shaders used by the engine
	std::unordered_map<std::string, std::shared_ptr<GlShader>> shaderMap;
	//Callback handler object
	GlfwInterface interface;
	//The memory manager, for buffer management and such.
	GlMemoryManager memoryManager;

	/**
	 * Renders all the objects in objects. Transparency stuff is set up before this function is called.
	 * @param pass The current rendering pass.
	 * @param objects A container of the objects to render. See RenderComponentManager.hpp for what it actually is.
	 * @param camera The camera to use when rendering.
	 * @param state The screen state, passed to shaders when setting uniforms.
	 */
	void renderTransparencyPass(RenderPass pass, const RenderManager::RenderPassList& objects, const Camera* camera, const ScreenState* state);

	/**
	 * Emulates push constants from Vulkan. Really, this just sets uniform locations in the provided
	 * program.
	 * @param shader The shader to set uniforms in.
	 * @param comp The render component for the object to set constants for.
	 * @param camera The camera of the screen currently being rendered.
	 */
	void setPushConstants(const GlShader* shader, const RenderComponent* comp, const Camera* camera);
};
