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

//Careful about include order here.
#include <GLFW/glfw3.h>

#include "WindowSystemInterface.hpp"
#include "DisplayEngine.hpp"

//This class does not handle the initializing / destroying of glfw,
//just the interface for callbacks and events.
class GlfwInterface : public WindowSystemInterface {
public:
	/**
	 * Creates the glfw interface.
	 * @param display The display engine, used for resize callbacks.
	 * @param renderer Needed to set viewport and other stuff on resizes.
	 */
	GlfwInterface(DisplayEngine& display, RenderingEngine* renderer);

	/**
	 * Registers callbacks.
	 * @param newWindow The window to register callbacks for.
	 *     This will be stored in this object as well.
	 */
	void init(GLFWwindow* newWindow);

	/**
	 * Indicates whether the window was closed by the user, and the
	 * game should stop.
	 * @return Whether to terminate the game due to a closed window.
	 */
	bool windowClosed() const override { return glfwWindowShouldClose(window); }

	/**
	 * Polls for mouse / keyboard / etc events.
	 */
	void pollEvents() const override { glfwPollEvents(); }

	/**
	 * Captures / uncaptures the mouse.
	 * @param capture Whether to capture the mouse or not.
 	 */
	void captureMouse(bool capture) const override;

	/**
	 * Gets the window's width, in pixels.
	 */
	float getWindowWidth() const override { return width; }

	/**
	 * Gets the window's height, in pixels.
	 */
	float getWindowHeight() const override { return height; }

	/**
	 * Gets the mouse position from the system.
	 */
	glm::vec2 queryMousePos() const override;

	/**
	 * Gets the internal window object.
	 */
	GLFWwindow* getWindow() { return window; }

private:
	//Reference to the engine's display engine
	DisplayEngine& display;
	//Pointer to parent rendering engine, used for some callbacks
	RenderingEngine* renderer;

	//The window
	GLFWwindow* window;

	//The window's width
	float width;
	//The window's height
	float height;

	/**
	 * Callback.
	 * @param error The glfw error code.
	 * @param description A description of the error.
	 */
	static void glfwError(int error, const char* description);

	/**
	 * Callback.
	 * @param window The window provided to the callback by glfw. Can be null.
	 * @param nWidth The new viewport width.
	 * @param nHeight The new viewport height.
	 */
	static void setViewport(GLFWwindow* window, int nWidth, int nHeight);

	/**
	 * Callback.
	 * @param window The window for the key press.
	 * @param key The key that was pressed.
	 * @param scancode The platform-specific key that was pressed.
	 * @param action What the key did (press, repeat, or release).
	 * @param mods Whether things like the shift key were pressed.
	 */
	static void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods);

	/**
	 * Callback.
	 * @param window The window the mouse was moved in.
	 * @param x The new x position of the mouse.
	 * @param y The new y position of the mouse.
	 */
	static void mouseMove(GLFWwindow* window, double x, double y);

	/**
	 * Callback.
	 * @param window The window the mouse was clicked on.
	 * @param button The button that was clicked.
	 * @param action Whether the click was a press or release.
	 * @param mods Whether things like shift or control were pressed.
	 */
	static void mouseClick(GLFWwindow* window, int button, int action, int mods);

	/**
	 * Callback.
	 * @param window The window the mouse was scrolled on.
	 * @param x The x offset.
	 * @param y The y offset.
	 */
	static void mouseScroll(GLFWwindow* window, double x, double y);
};
