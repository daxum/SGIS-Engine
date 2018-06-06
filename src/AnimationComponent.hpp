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

#include "Component.hpp"
#include "SplineAnimation.hpp"
#include "ObjectPhysicsInterface.hpp"

class AnimationComponent : public Component, ObjectPhysicsInterface {
public:
	/**
	 * Constructor.
	 * @param frames The key frames for the animation.
	 * @param time The time to complete the animation, in EngineConfig::timestep units.
	 * @param matrix A matrix for the spline curve, defaults to a bezier curve.
	 */
	AnimationComponent(const std::vector<std::pair<glm::vec3, glm::quat>>& frames, const float time, const glm::mat4& matrix = SplineAnimation::B);

	/**
	 * Just increments the time.
	 */
	void update() { time++; }

	/**
	 * Sets the animation component as the physics provider.
	 */
	void onParentSet();

	/**
	 * Gets the translation of the object from the animation.
	 * @return The translation of the object.
	 */
	glm::vec3 getTranslation();

	/**
	 * Gets the rotation of the object.
	 * @return The rotation.
	 */
	glm::quat getRotation();

private:
	//The spline curve that defines this animation.
	SplineAnimation animation;
	//Current animation time, in update ticks.
	size_t time;
};
