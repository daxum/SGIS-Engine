#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "RenderingEngine.hpp"
#include "CombinedGl.h"

//An implementation of RenderingEngine that uses the OpenGL graphics api.
class GlRenderingEngine : public RenderingEngine {
public:
	/**
	 * Constructs a GlRenderingEngine and initializes small parts of
	 * glfw - it just calls the init function and sets the error callback.
	 * @throw runtime_error if glfw initialization failed.
	 */
	GlRenderingEngine();

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
	//A map to store texture data
	std::unordered_map<std::string, GLuint> textureMap;
	//The window created by glfw
	GLFWwindow* window;
};
