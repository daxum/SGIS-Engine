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
#include "TextureLoader.hpp"
#include "ShaderLoader.hpp"

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
	 * @throw runtime_error if initialization failed.
	 */
	RenderingEngine(std::shared_ptr<TextureLoader> tl, std::shared_ptr<ShaderLoader> sl) :
		texLoader(tl), shaderLoader(sl) {}

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
	 * @param windowWidth The width of the created window.
	 * @param windowHeight The height of the created window.
	 * @param windowTitle The title of the created window.
	 * @throw runtime_error if initialization failed.
	 */
	virtual void init(int windowWidth, int windowHeight, std::string windowTitle) = 0;

	/**
	 * To be called to put the engine in a renderable state - uploads models to the gpu,
	 * and similar things.
	 */
	virtual void finishLoad() = 0;

	/**
	 * Loads any default shaders for the game engine, from the folder
	 * specified by path. See EngineConfig.hpp for the format of path.
	 * @param path The path to the folder containing the default shaders.
	 */
	 virtual void loadDefaultShaders(std::string path) = 0;

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
	 * Returns the api-specific model loader for this rendering engine.
	 * Not currently implemented.
	 */
	void getModelLoader() {}

	/**
	 * Renders the passed in object (Not yet implemented).
	 * @param partialTicks The time between game updates, in milliseconds.
	 *     Used for interpolation.
	 */
	virtual void render(float partialTicks) = 0;

	/**
	 * Indicates whether the window was closed by the user, and the
	 * game should stop.
	 * @return Whether to terminate the game due to a closed window.
	 */
	virtual bool windowClosed() = 0;

	/**
	 * Called from the engine to poll for events. This will usually
	 * call glfwPollEvents().
	 */
	virtual void pollEvents() = 0;

protected:
	std::shared_ptr<TextureLoader> texLoader;
	std::shared_ptr<ShaderLoader> shaderLoader;
};
