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

#include "TextureLoader.hpp"
#include "ShaderLoader.hpp"
#include "RenderComponentManager.hpp"
#include "RendererMemoryManager.hpp"
#include "Screen.hpp"
#include "WindowSystemInterface.hpp"
#include "Camera.hpp"
#include "RenderInitializer.hpp"

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
	//Max number of queued frames to be rendered.
	constexpr static size_t MAX_ACTIVE_FRAMES = 2;

	/**
	 * Constructs the base rendering engine and initializes some parts of
	 * the rendering api.
	 * @param tl The texture loader this engine should use.
	 * @param sl The shader loader this engine should use.
	 * @param memManager The renderer memory manager used in the render initializer.
	 * @param rendererLog The logger config for the rendering engine.
	 * @throw runtime_error if initialization failed.
	 */
	RenderingEngine(std::shared_ptr<TextureLoader> tl, std::shared_ptr<ShaderLoader> sl, RendererMemoryManager* memManager, const LogConfig& rendererLog) :
		texLoader(tl), shaderLoader(sl), renderInit(memManager), logger(rendererLog) {}

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
	 * Gets the renderer initializer for this rendering engine, which is used
	 * to add vertex buffers and descriptor sets before everything else is loaded.
	 * @return The render initializer for this rendering engine.
	 */
	RenderInitializer& getRenderInitializer() { return renderInit; }

	/**
	 * Called at the very start of a frame. Does any work needed to set
	 * up the engine for the frame.
	 */
	virtual void beginFrame() = 0;

	/**
	 * Renders the passed in object. This function performs view culling if needed and
	 * passed all visible renderComponents to the underlying graphics rendering api
	 * @param screen The screen to render.
	 */
	void render(const Screen* screen);

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
	//Renderer initializer.
	RenderInitializer renderInit;
	//The general rendering logger.
	Logger logger;

	/**
	 * Renders the visible objects, using the sorted map.
	 * The depth and stencil buffers should be cleared before or after this function
	 * so different screens don't effect each other's rendering.
	 * @param sortedObjects All objects, sorted by buffer, then shader, then model.
	 * @param screen The screen being rendered.
	 */
	virtual void renderObjects(RenderComponentManager::RenderPassList sortedObjects, const Screen* screen) = 0;

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
	bool checkVisible(const std::array<std::pair<glm::vec2, glm::vec2>, 4>& cameraBox, const glm::mat4& viewMat, const RenderComponent* object, float nearDist, float farDist);
};
