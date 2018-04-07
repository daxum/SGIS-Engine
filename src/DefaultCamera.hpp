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

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.hpp"

class DefaultCamera : public Camera {
public:
	/**
	 * Calculates a view matrix to use in rendering.
	 * @return the view matrix.
	 */
	glm::mat4 getView() { return glm::lookAt(glm::vec3(0.0, 0.1, 1.0), glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)); }

	/**
	 * Updates the camera. Does nothing.
	 */
	void update() {}
};
