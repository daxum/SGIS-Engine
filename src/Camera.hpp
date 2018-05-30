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

class Camera {
public:
	virtual ~Camera() {}

	/**
	 * Calculates a view matrix to use in rendering.
	 * @return the view matrix.
	 */
	virtual glm::mat4 getView() = 0;

	/**
	 * Fetches a projection matrix.
	 * @return the projection matrix.
	 */
	virtual glm::mat4 getProjection() = 0;

	/**
	 * Called whenever the window changes to reset the projection matrix.
	 */
	virtual void setProjection() {}

	/**
	 * Retrieves the near and far plane.
	 * @return A pair with the near plane as the first element and the far plane as the second.
	 */
	virtual std::pair<float, float> getNearFar() = 0;

	/**
	 * Updates the camera.
	 */
	virtual void update() = 0;
};
