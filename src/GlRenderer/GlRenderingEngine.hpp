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
#include <array>

#include <glm/glm.hpp>

#include "RenderingEngine.hpp"
#include "CombinedGl.h"
#include "GlShader.hpp"
#include "GlMemoryManager.hpp"
#include "GlTextureLoader.hpp"
#include "Model.hpp"
#include "Logger.hpp"
#include "MatrixStack.hpp"
#include "ModelManager.hpp"
#include "RenderComponent.hpp"
#include "GlfwInterface.hpp"

//An implementation of RenderingEngine that uses the OpenGL graphics api.
class GlRenderingEngine : public RenderingEngine {
public:
	/**
	 * Constructs a GlRenderingEngine and initializes small parts of
	 * glfw - it just calls the init function and sets the error callback.
	 * @param modelManager A reference to the object that stores model data.
	 * @param display A reference to the engine's display engine, used for callbacks.
	 * @param rendererLog The logger config for the rendering engine.
	 * @param loaderLog The logger config for the misc. loaders (texture, shader, model, etc).
	 * @throw runtime_error if glfw initialization failed.
	 */
	GlRenderingEngine(DisplayEngine& display, const LogConfig& rendererLog, const LogConfig& loaderLog);

	/**
	 * Destroys the window and terminates glfw
	 */
	~GlRenderingEngine();

	/**
	 * Initializes OpenGL. A window is created, functions are loaded,
	 * callbacks are registered, and state defaults are set.
	 * @throw runtime_error if initialization failed.
	 */
	void init() override;

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
	 * Swaps the buffers and clears for the next frame.
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
	 * Renders the passed in objects.
	 * @param objects The objects to render.
	 * @param sortedObjects A sorted map of maps of maps of sets of all the possible objects to render.
	 * @param camera The camera.
	 * @param state User-supplied screen state.
	 */
	void renderObjects(const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) override;

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
	 * Renders the object onto the screen.
	 * @param shader The shader used for rendering.
	 * @param data The object to render.
	 */
	void renderObject(MatrixStack& matStack, std::shared_ptr<GlShader> shader, std::shared_ptr<RenderComponent> data, std::shared_ptr<ScreenState> state);

	/**
	 * Renders all the objects in objects. Transparency stuff is set up before this function is called.
	 * @param pass The current rendering pass.
	 * @param visibleObjects A set of objects visible on the screen.
	 * @param objects A container of the objects to render. See RenderComponentManager.hpp for what it actually is.
	 * @param camera The camera to use when rendering.
	 * @param state The screen state, passed to shaders when setting uniforms.
	 */
	void renderTransparencyPass(RenderPass pass, const tbb::concurrent_unordered_set<RenderComponent*>& visibleObjects, const RenderComponentManager::RenderPassList& objects, MatrixStack& matStack, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state);
};
