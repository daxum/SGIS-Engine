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

#include <glm/glm.hpp>

//An interface into the underlying windowing system,
//for things like callbacks and queries. This allows
//for better sharing of code between different
//rendering engines.
class WindowSystemInterface {
public:
	/**
	 * Initializes the interface. This should do things like register callbacks.
	 * @param arg A pointer to an object or objects needed to initialize the interface,
	 *     determined by subclass. For example, the glfw interface takes a GLFWwindow*.
	 */
	virtual void init(void* arg) = 0;

	/**
	 * Indicates whether the window was closed by the user, and the
	 * game should stop.
	 * @return Whether to terminate the game due to a closed window.
	 */
	virtual bool windowClosed() const = 0;

	/**
	 * Polls for mouse / keyboard / etc events.
	 */
	virtual void pollEvents() const = 0;

	/**
	 * Captures / uncaptures the mouse.
	 * @param capture Whether to capture the mouse or not.
 	 */
	virtual void captureMouse(bool capture) const = 0;

	/**
	 * Gets the window's width, in pixels.
	 */
	virtual float getWindowWidth() const = 0;

	/**
	 * Gets the window's height, in pixels.
	 */
	virtual float getWindowHeight() const = 0;

	/**
	 * Gets the mouse position from the system.
	 */
	virtual glm::vec2 queryMousePos() const = 0;
};
