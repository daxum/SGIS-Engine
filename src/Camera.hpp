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

#include "Object.hpp"

class Camera {
public:
	/**
	 * Creates a camera at the given position looking in the specific direction
	 * @param startPos The camera's starting position
	 * @param startLook The place where the camera is looking
	 * @param startup Which direction is "up" - should almost always be (0, 1, 0)
	 */
	Camera(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 startLook = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f));

	/**
	 * Moves the camera to the given position
	 * @param x The x coordinate to move to
	 * @param y The y coordinate to move to
	 * @param z The z coordinate to move to
	 */
	void move(float x, float y, float z);

	/**
	 * Turns the camera to look at the given location
	 * @param x The x coordinate to look at
	 * @param y The y coordinate to look at
	 * @param z The z coordinate to look at
	 */
	void lookAt(float x, float y, float z);

	/**
	 * Calculates a view matrix to use in rendering
	 * @return the view matrix
	 */
	glm::mat4 getView();

	/**
	 * Updates the camera. This currently moves it to track the object it is following.
	 */
	void update();

	/**
	 * Sets the object for the camera to track. If the object doesn't have a physics component,
	 * this won't do anything.
	 */
	void setTarget(std::shared_ptr<Object> object);

private:
	//These three vectors define a camera - pos is position,
	//look is where the camera is looking, up is the up vector.
	glm::vec3 pos;
	glm::vec3 look;
	glm::vec3 up;

	//The object to follow.
	std::shared_ptr<Object> target;

	//The velocity of the camera.
	glm::vec3 velocity;
};
