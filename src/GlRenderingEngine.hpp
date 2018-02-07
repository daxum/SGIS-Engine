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

#include "RenderingEngine.hpp"
#include "CombinedGl.h"
#include "GlShader.hpp"
#include "GlMemoryManager.hpp"
#include "Model.hpp"
#include "Logger.hpp"
#include "Camera.hpp"

//An implementation of RenderingEngine that uses the OpenGL graphics api.
class GlRenderingEngine : public RenderingEngine {
public:
	/**
	 * Constructs a GlRenderingEngine and initializes small parts of
	 * glfw - it just calls the init function and sets the error callback.
	 * @param rendererLog The logger config for the rendering engine.
	 * @param loaderLog The logger config for the misc. loaders (texture, shader, model, etc).
	 * @throw runtime_error if glfw initialization failed.
	 */
	GlRenderingEngine(const LogConfig& rendererLog, const LogConfig& loaderLog);

	/**
	 * Destroys the window and terminates glfw
	 */
	~GlRenderingEngine();

	/**
	 * Initializes OpenGL. A window is created, functions are loaded,
	 * callbacks are registered, and state defaults are set.
	 * @param windowWidth The width of the created window.
	 * @param windowHeight The height of the created window.
	 * @param windowTitle The title of the created window.
	 * @throw runtime_error if initialization failed.
	 */
	void init(int windowWidth, int windowHeight, std::string windowTitle);

	/**
	 * Uploads data to gpu and sets some state stuff.
	 */
	void finishLoad();

	/**
	 * Loads all default glsl shaders from the provided folder.
	 * @param path The path to the folder containing the default shaders.
	 */
	void loadDefaultShaders(std::string path);

	/**
	 * Renders the passed in object. This clears the color and depth buffers,
	 * draws the object(s), and presents the image.
	 * @param partialTicks The time since the last game update.
	 */
	void render(float partialTicks);

	/**
	 * Clears the depth and stencil buffers.
	 */
	virtual void clearBuffers();

	/**
	 * Swaps the buffers and clears for the next frame.
	 */
	virtual void present();

	/**
	 * Indicates whether the window was closed by the user, and the
	 * game should stop.
	 * @return Whether to terminate the game due to a closed window.
	 */
	bool windowClosed();

	/**
	 * Just calls glfwPollEvents() and returns.
	 */
	void pollEvents();

	/**
	 * Sets the projection matrix. Only intended to be called from setViewport.
	 * @param width The width of the window.
	 * @param height The height of the window.
	 */
	void setProjection(int width, int height);

	/**
	 * Sets the window viewport with OpenGL. This is a callback
	 * function for glfw, and should only be called directly once,
	 * at the end of OpenGL initialization.
	 * @param window The window provided to the callback by glfw. Can be null.
	 * @param width The new viewport width.
	 * @param height The new viewport height.
	 */
	static void setViewport(GLFWwindow* window, int width, int height);

	/**
	 * The error callback for glfw. Never call this directly.
	 * @param error The glfw error code.
	 * @param description A description of the error.
	 */
	static void glfwError(int error, const char* description);

private:
	//Projection matrix.
	glm::mat4 projection;
	//Just the camera. Does camera things.
	Camera camera;
	//The general rendering logger
	Logger logger;
	//The loader logger
	Logger loaderLogger;
	//A map to store texture data
	std::unordered_map<std::string, GLuint> textureMap;
	//A map to store the shaders used by the engine
	std::unordered_map<std::string, std::shared_ptr<GlShader>> shaderMap;
	//A map to store loaded model information
	std::unordered_map<std::string, Model> modelMap;
	//The window created by glfw
	GLFWwindow* window;

	//The memory manager, for buffer management and such.
	GlMemoryManager memoryManager;
};
