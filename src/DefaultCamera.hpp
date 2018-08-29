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

#include "Camera.hpp"
#include "Engine.hpp"
#include "ExtraMath.hpp"

class DefaultCamera : public Camera {
public:
	DefaultCamera() : near(0.1f), far(100.0f), pos(0.0, 0.0, 1.0), lookDir(0.0, 0.0, -1.0), up(0.0, 1.0, 0.0) {}

	/**
	 * Calculates a view matrix to use in rendering.
	 * @return the view matrix.
	 */
	const glm::mat4 getView() const override { return view; }

	/**
	 * Creates a projection matrix.
	 * @return the projection matrix.
	 */
	const glm::mat4 getProjection() const override { return projection; }

	/**
	 * Sets the projection matrix.
	 * This will be called before the parent screen is added to the
	 * display, so be careful.
	 */
	void setProjection() override {
		float width = Engine::instance->getWindowInterface().getWindowWidth();
		float height = Engine::instance->getWindowInterface().getWindowHeight();

		projection = glm::perspective(ExMath::PI / 4.0f, width / height, near, far);
	}

	/**
	 * Gets the near and far planes.
	 */
	std::pair<float, float> getNearFar() const override { return {near, far}; }

	/**
	 * Gets the field of view.
	 */
	float getFOV() const override { return ExMath::PI / 4.0f; }

	/**
	 * Updates the camera. Does nothing.
	 */
	void update() override {}

	/**
	 * Sets the pos, look, and up variables, updating the view matrix in the process.
	 */
	void setPos(const glm::vec3& newPos) { pos = newPos; view = glm::lookAt(pos, pos + lookDir, up); }
	void setLook(const glm::vec3& newLook) { lookDir = newLook; view = glm::lookAt(pos, pos + lookDir, up); }
	void setUp(const glm::vec3& newUp) { up = newUp; view = glm::lookAt(pos, pos + lookDir, up); }

	//Near and far plane.
	float near;
	float far;

	//Projection matrix.
	glm::mat4 projection;
	//View matrix.
	glm::mat4 view;

private:
	//Simple camera variables.
	glm::vec3 pos;
	glm::vec3 lookDir;
	glm::vec3 up;
};

