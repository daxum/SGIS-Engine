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

#include <stdexcept>

#include "SplineAnimation.hpp"
#include "ExtraMath.hpp"

//Move to header + constexpr for c++14?
const glm::mat4 SplineAnimation::B = {
	0.0,  1.0,  0.0,  0.0,
	-0.5,  0.0,  0.5,  0.0,
	1.0, -2.5,  2.0, -0.5,
	-0.5,  1.5, -1.5,  0.5
};

const glm::mat4 SplineAnimation::catmullRom = {
	0.0, 2.0, 0.0, 0.0,
	-1.0, 0.0, 1.0, 0.0,
	2.0, -5.0, 4.0, -1.0,
	-1.0, 3.0, -3.0, 1.0
};

SplineAnimation::SplineAnimation(const std::vector<std::pair<glm::vec3, glm::quat>>& frames, const float time, const glm::mat4& matrix) :
	controlPoints(frames),
	maxTime(time),
	matrix(matrix) {

	if (controlPoints.size() < 4) {
		throw std::runtime_error("Not enough control points in animation");
	}

	glm::quat prevQuat = controlPoints.front().second;

	//Fix rotations to go the short way around
	for (auto& point : controlPoints) {
		if (glm::dot(prevQuat, point.second) < 0.0) {
			point.second = -point.second;
		}

		prevQuat = point.second;
	}

	//1 / (amount of subdivisions per line segment)
	constexpr const float subdivisionLength = 0.1f;

	//Length of the subsections - index, percent, then length
	std::vector<std::tuple<size_t, float, float>> lengths;
	//Total arc length
	float arcLength = 0.0f;

	//Number of line segments in curve
	const size_t lineSegments = controlPoints.size() - 3;

	//Calculate individual line segment lengths and total arc length
	for (size_t i = 0; i < lineSegments; i++) {
		for (float j = 0.0f; j < 1.0f; j += subdivisionLength) {
			float length = glm::length(getPos(i, std::min(j + subdivisionLength, 1.0f)) - getPos(i, j));

			//This line might be problematic in the future. If so, sum the lengths merge-sort style after this instead
			arcLength += length;

			lengths.emplace_back(i, j, length);
		}
	}

	//Build lookup table using %length -> %time
	float lengthSum = 0.0f;

	for (const auto& segment : lengths) {
		lengthSum += std::get<2>(segment);

		const float lengthPercent = lengthSum / arcLength;
		const float time = lengthPercent * maxTime;

		posLookup.emplace_back(time, std::get<0>(segment), std::get<1>(segment));
	}
}

std::pair<glm::vec3, glm::quat> SplineAnimation::getLocation(float time) {
	time = std::fmod(time, maxTime);

	//TODO: binary search
	for (size_t i = 0; i < posLookup.size(); i++) {
		if (std::get<0>(posLookup.at(i)) < time) {
			continue;
		}

		//Get times
		float tMin = 0.0f;
		const float tMax = std::get<0>(posLookup.at(i));

		if (i != 0) {
			tMin = std::get<0>(posLookup.at(i - 1));
		}

		//Index + percent
		const float combinedLoc = std::get<1>(posLookup.at(i)) + std::get<2>(posLookup.at(i));

		float nextCombinedLoc = controlPoints.size() - 3;

		//If not last index, set normally, else use last segment (size - 3) as the next index (max index - 1 with 1.0 percent)
		if (i != posLookup.size() - 1) {
			nextCombinedLoc = std::get<1>(posLookup.at(i + 1)) + std::get<2>(posLookup.at(i + 1));
		}

		float timePercent = (time - tMin) / (tMax - tMin);

		float fIndex = 0;
		float percent = std::modf(ExMath::interpolate(combinedLoc, nextCombinedLoc, timePercent), &fIndex);
		size_t index = (size_t)fIndex;

		//Fix percent for last point
		if (index == posLookup.size()) {
			index--;
			percent = 1.0f;
		}

		return {getPos(index, percent), getRot(index, percent)};
	}

	throw std::runtime_error("SplineAnimation: Couldn't find time in lookup table");
}

glm::vec3 SplineAnimation::getPos(size_t index, float percent) {
	glm::mat4 points;
	points[0] = glm::vec4(controlPoints.at(index).first, 0.0);
	points[1] = glm::vec4(controlPoints.at(index + 1).first, 0.0);
	points[2] = glm::vec4(controlPoints.at(index + 2).first, 0.0);
	points[3] = glm::vec4(controlPoints.at(index + 3).first, 0.0);

	glm::vec4 vec(1.0, percent, percent*percent, percent*percent*percent);

	return points * (matrix * vec);
}

glm::quat SplineAnimation::getRot(size_t index, float percent) {
	glm::mat4 points;
	points[0] = quatToVec4(controlPoints.at(index).second);
	points[1] = quatToVec4(controlPoints.at(index + 1).second);
	points[2] = quatToVec4(controlPoints.at(index + 2).second);
	points[3] = quatToVec4(controlPoints.at(index + 3).second);

	glm::vec4 vec(1.0, percent, percent*percent, percent*percent*percent);

	return glm::normalize(vec4ToQuat(points * (matrix * vec)));
}
