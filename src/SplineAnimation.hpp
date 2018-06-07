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

#include <vector>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class SplineAnimation {
public:
	/**
	 * Creates an animation along the given spline curve that completes in
	 * the given amount of time.
	 * @param frames The control points for the curve. Must have at least 4 points.
	 * @param time The time until the animation completes. Usually dependent
	 *     on EngineConfig::timestep, has no predefined units.
	 * @param matrix The matrix used to calculate a point along the curve.
	 */
	SplineAnimation(const std::vector<std::pair<glm::vec3, glm::quat>>& frames, const float time, const glm::mat4& matrix = B);

	/**
	 * Gets the location at the given time.
	 * @param time The current time along the animation. Will be wrapped to
	 *     be between 0 and max time.
	 * @return A pair with the position as the first component and the rotation
	 *     as the second.
	 */
	std::pair<glm::vec3, glm::quat> getLocation(float time);

	//Default matrix for bezier curve.
	static const glm::mat4 B;
	//Catmull-Rom spline.
	static const glm::mat4 catmullRom;

private:
	//Control points for the curve.
	std::vector<std::pair<glm::vec3, glm::quat>> controlPoints;
	//Animation time.
	float maxTime;
	//Lookup table from time to location along curve using index into controlPoints and percent (arc-length paramaterization).
	std::vector<std::tuple<float, size_t, float>> posLookup;
	//Matrix used in the spline curve.
	glm::mat4 matrix;

	/**
	 * Gets the position at the given index and percent.
	 * @param index The start index in the control points.
	 * @param percent The percent along the line segment, always between 0 and 1.
	 * @return The position at the point.
	 */
	glm::vec3 getPos(size_t index, float percent);

	/**
	 * Same as above, but gets the rotation instead.
	 * @param index The start index of the control points.
	 * @param percent The percent along the line segment.
	 * @return The rotation at the given point.
	 */
	glm::quat getRot(size_t index, float percent);

	/**
	 * Converts a quaternion to a vec4.
	 */
	glm::vec4 quatToVec4(glm::quat q) {
		return glm::vec4(q.x, q.y, q.z, q.w);
	}

	/**
	 * Converts a vec4 to a quaternion.
	 */
	glm::quat vec4ToQuat(glm::vec4 vec) {
		return glm::quat(vec.w, vec.x, vec.y, vec.z);
	}
};
