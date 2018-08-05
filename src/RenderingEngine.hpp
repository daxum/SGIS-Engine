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

#include <memory>
#include <string>
#include <array>

#include <glm/glm.hpp>
#include <tbb/concurrent_unordered_set.h>

#include "TextureLoader.hpp"
#include "ShaderLoader.hpp"
#include "ModelLoader.hpp"
#include "Model.hpp"
#include "RenderComponentManager.hpp"
#include "RendererMemoryManager.hpp"
#include "Screen.hpp"
#include "WindowSystemInterface.hpp"

//A generic rendering engine. Provides the base interfaces, like resource loading
//and rendering, but leaves the implementation to api-specific subclasses, like
//a GlRenderingEngine, or a VulkanRenderingEngine (or maybe even a DxRenderingEngine.
//Unlikely, though). For the purpose of optimization, most functions should probably
//be as vague as possible - so they would be like "render all this stuff, I don't
//care how", and then the rendering engine would figure it out. To facilitate this,
//things like world objects and menus should NOT contain rendering code, but rather
//information on how to render them. Hopefully this strategy won't be too restrictive.
class RenderingEngine {
public:
	/**
	 * Constructs the base rendering engine and initializes some parts of
	 * the rendering api.
	 * @param tl The texture loader this engine should use.
	 * @param sl The shader loader this engine should use.
	 * @param rendererLog The logger config for the rendering engine.
	 * @param loaderLog The logger config for the misc. loaders (texture, shader, model, etc).
	 * @throw runtime_error if initialization failed.
	 */
	RenderingEngine(std::shared_ptr<TextureLoader> tl, std::shared_ptr<ShaderLoader> sl, const LogConfig& rendererLog, const LogConfig& loaderLog) :
		texLoader(tl), shaderLoader(sl), logger(rendererLog.type, rendererLog.mask, rendererLog.outputFile),
		loaderLogger(loaderLog.type, loaderLog.mask, loaderLog.outputFile) {}

	/**
	 * Destroys any api-agnostic resources the engine might
	 * have created.
	 */
	virtual ~RenderingEngine() {}

	/**
	 * Initialize the rendering engine. Initialization usually consists of:
	 *  - creating a window
	 *  - allocating device resources
	 *  - creating swapchains, framebuffers, and other nonsense
	 *  - registering callbacks
	 * @throw runtime_error if initialization failed.
	 */
	virtual void init() = 0;

	/**
	 * Gets the memory manager for this rendering engine, used for uploading
	 * models.
	 * @return The memory manager for this rendering engine.
	 */
	virtual RendererMemoryManager* getMemoryManager() = 0;

	/**
	 * To be called to put the engine in a renderable state - uploads models to the gpu,
	 * and similar things.
	 */
	virtual void finishLoad() = 0;

	/**
	 * Gets the texture loader for this rendering engine, which is
	 * used to transfer textures from the disk to the GPU. Some loaders
	 * might also have the option of storing loaded textures in RAM.
	 * @return The texture loader for this rendering engine.
	 */
	std::shared_ptr<TextureLoader> getTextureLoader() { return texLoader; }

	/**
	 * Gets the shader loader for this rendering engine, which is used
	 * to load shader programs and sometimes to set up the rending pipeline.
	 * @return The shader loader for this rendering engine.
	 */
	std::shared_ptr<ShaderLoader> getShaderLoader() { return shaderLoader; }

	/**
	 * Called at the very start of a frame. Does any work needed to set
	 * up the engine for the frame.
	 */
	virtual void beginFrame() = 0;

	/**
	 * Renders the passed in object. This function performs view culling if needed and
	 * passed all visible renderComponents to the underlying graphics rendering api
	 * @param data The stuff to render.
	 * @param camera The camera to use.
	 * @param state User-set screen state, passed to shader when setting uniforms.
	 */
	void render(std::shared_ptr<RenderComponentManager> renderManager, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state);

	/**
	 * Called when drawing is done and the results can be displayed on the screen.
	 * Also sets up the pipeline for the next frame.
	 */
	virtual void present() = 0;

	/**
	 * Called when the window size has changed and the viewport needs to be updated.
	 * @param width The new window width.
	 * @param height The new window height.
	 */
	virtual void setViewport(int width, int height) = 0;

	/**
	 * Gets the interface to the window, provides things like window size.
	 * @return The interface to the window system.
	 */
	virtual const WindowSystemInterface& getWindowInterface() const = 0;

protected:
	//The texture loader.
	std::shared_ptr<TextureLoader> texLoader;
	//The shader loader.
	std::shared_ptr<ShaderLoader> shaderLoader;
	//The general rendering logger
	Logger logger;
	//The loader logger
	Logger loaderLogger;

	/**
	 * Renders the visible objects, using the sorted map.
	 * The depth and stencil buffers should be cleared before or after this function
	 * so different screens don't effect each other's rendering.
	 * @param objects A set of objects that have been determined to be visible.
	 * @param sortedObjects All objects, sorted by buffer, then shader, then model.
	 * @param camera The current camera.
	 * @param state User-provided screen state.
	 */
	virtual void renderObjects(const tbb::concurrent_unordered_set<RenderComponent*>& objects, RenderComponentManager::RenderPassList sortedObjects, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) = 0;

private:
	/**
	 * Checks whether the given object is visible from the camera.
	 * @param cameraBox The box of the camera, in camera coordinates. Goes
	 *     {top left, top right, bottom left, bottom right}.
	 * @param viewMat The view matrix, used to transform object position into camera space.
	 * @param object The object to check.
	 * @param nearDist The distance for the near plane.
	 * @param farDist The distance for the far plane.
	 * @return Whether the object can be seen from the camera.
	 */
	bool checkVisible(const std::array<std::pair<glm::vec2, glm::vec2>, 4>& cameraBox, const glm::mat4& viewMat, RenderComponent* object, float nearDist, float farDist);
};
