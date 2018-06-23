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
#include "Shader.hpp"
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
	void init();

	/**
	 * Gets the memory manager for this rendering engine, used for uploading
	 * models.
	 * @return The memory manager for this rendering engine.
	 */
	RendererMemoryManager* getMemoryManager();

	/**
	 * Uploads data to gpu and sets some state stuff.
	 */
	void finishLoad();

	/**
	 * Renders the passed in object.
	 * @param data The object to render.
	 * @param camera The camera.
	 */
	void render(std::shared_ptr<RenderComponentManager> data, std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state);

	/**
	 * Clears the depth and stencil buffers.
	 */
	void clearBuffers();

	/**
	 * Swaps the buffers and clears for the next frame.
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
	//The type for the camera view box.
	typedef std::array<std::pair<glm::vec3, glm::vec3>, 6> CameraBox;
	//The general rendering logger
	Logger logger;
	//The loader logger
	Logger loaderLogger;
	//A map to store texture data
	std::unordered_map<std::string, GlTextureData> textureMap;
	//A map to store the shaders used by the engine
	std::unordered_map<std::string, std::shared_ptr<Shader>> shaderMap;
	//Callback handler object
	GlfwInterface interface;

	//The memory manager, for buffer management and such.
	GlMemoryManager memoryManager;

	/**
	 * Renders the object onto the screen.
	 * @param shader The shader used for rendering.
	 * @param data The object to render.
	 */
	void renderObject(MatrixStack& matStack, std::shared_ptr<Shader> shader, std::shared_ptr<RenderComponent> data, std::shared_ptr<ScreenState> state);

	/**
	 * Renders all the objects in objects. Transparency stuff is set up before this function is called.
	 * @param objects A container of the objects to render. See RenderComponentManager.hpp for what it actually is.
	 * @param camera The camera to use when rendering.
	 * @param viewBox The view box of the camera, used for culling.
	 * @param state The screen state, passed to shaders when setting uniforms.
	 * @param blend Whether to enable blend when rendering an object.
	 */
	void renderTransparencyPass(const RenderComponentManager::RenderPassObjects& objects, MatrixStack& matStack, std::shared_ptr<Camera> camera, const CameraBox& viewBox, std::shared_ptr<ScreenState> state, bool blend = false);

	/**
	 * Checks whether the given sphere is in the camera's view.
	 * @param sphere A sphere represented by a position and a radius.
	 * @param camera A deformed box representing the camera's view, where each entry defines a
	 *     plane with a position and a normal.
	 * @return Whether the sphere is intersecting the camera's view.
	 */
	bool checkVisible(const std::pair<glm::vec3, float>& sphere, const CameraBox& camera);

	/**
	 * Gets a deformed collision box of the camera with normals for each face.
	 * @param view The camera's view matrix.
	 * @param projection The camera's projection matrix.
	 * @param nearPlane The near plane.
	 * @param farplane The far plane.
	 * @return a set of pairs of vectors. The first vector contains the center
	 *     position of one of the faces, and the second contains the normal
	 *     for that face.
	 */
	CameraBox getCameraCollisionData(glm::mat4 view, glm::mat4 projection, float nearPlane, float farPlane);
};
