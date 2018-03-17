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
#include "MatrixStack.hpp"
#include "ModelManager.hpp"
#include "RenderComponent.hpp"

//An implementation of RenderingEngine that uses the OpenGL graphics api.
class GlRenderingEngine : public RenderingEngine {
public:
	/**
	 * Constructs a GlRenderingEngine and initializes small parts of
	 * glfw - it just calls the init function and sets the error callback.
	 * @param modelManager A reference to the object that stores model data.
	 * @param rendererLog The logger config for the rendering engine.
	 * @param loaderLog The logger config for the misc. loaders (texture, shader, model, etc).
	 * @throw runtime_error if glfw initialization failed.
	 */
	GlRenderingEngine(ModelManager& modelManager, const LogConfig& rendererLog, const LogConfig& loaderLog);

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
	 * @param display The display to set the input callbacks to.
	 * @throw runtime_error if initialization failed.
	 */
	void init(int windowWidth, int windowHeight, std::string windowTitle, DisplayEngine* display);

	/**
	 * Adds a mesh to the rendering engine. The opengl version
	 * adds it to its memory manager.
	 * @param data The model data to add.
	 * @param type The type of the mesh to add.
	 * @return the data needed to render the mesh.
	 */
	MeshRenderData addMesh(ModelData& data, MeshType type);

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
	 * @param data The data to render.
	 * @param camera The camera.
	 */
	void render(std::shared_ptr<RenderComponentManager> data, Camera& camera);

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

	/**
	 * Another glfw callback.
	 * @param window The window for the key press.
	 * @param key The key that was pressed.
	 * @param scancode The platform-specific key that was pressed.
	 * @param action What the key did (press, repeat, or release).
	 * @param mods Whether things like the shift key were pressed.
	 */
	static void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	//Projection matrix.
	glm::mat4 projection;
	//The general rendering logger
	Logger logger;
	//The loader logger
	Logger loaderLogger;
	//A map to store texture data
	std::unordered_map<std::string, GLuint> textureMap;
	//A map to store the shaders used by the engine
	std::unordered_map<std::string, std::shared_ptr<GlShader>> shaderMap;
	//The object that stores all the models.
	ModelManager& modelManager;
	//The window created by glfw
	GLFWwindow* window;

	//The memory manager, for buffer management and such.
	GlMemoryManager memoryManager;

	/**
	 * Renders the object onto the screen.
	 * @param shader The shader used for rendering.
	 * @param data The object to render.
	 */
	void renderObject(MatrixStack& matStack, std::shared_ptr<GlShader> shader, std::shared_ptr<RenderComponent> data);
};
