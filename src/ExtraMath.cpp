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

#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <tuple>

#include "ExtraMath.hpp"

namespace {
	//Psuedo-square time to avoid chunking when threads all initialize at once.
	thread_local std::mt19937 engine(ExMath::getTimeMillis() * ExMath::getTimeMillis());
}

glm::vec3 ExMath::bilinear3D(std::tuple<glm::vec3, glm::vec3, glm::vec3, glm::vec3> corners, float xWeight, float yWeight) {
	return (1.0f - yWeight) * ((1.0f - xWeight) * std::get<0>(corners) + xWeight * std::get<1>(corners)) +
			yWeight * ((1.0f - xWeight) * std::get<2>(corners) + xWeight * std::get<3>(corners));
}

float ExMath::randomFloat(float min, float max) {
	return std::uniform_real_distribution<float>(min, max)(engine);
}

bool ExMath::randomBool() {
	return std::uniform_int_distribution<unsigned char>(0, 1)(engine);
}

int ExMath::randomInt(int min, int max) {
	return std::uniform_int_distribution<int>(min, max)(engine);// % (max - min + 1) + min;
}

double ExMath::getTimeMillis() {
	std::chrono::duration<double, std::ratio<1, 1000>> time = std::chrono::steady_clock::now().time_since_epoch();
	return time.count();
}

float ExMath::minMagnitude(float val1, float val2) {
	if (std::min(std::abs(val1), std::abs(val2)) == std::abs(val1)) {
		return val1;
	}

	return val2;
}

float ExMath::maxMagnitude(float val1, float val2) {
	if (minMagnitude(val1, val2) == val2) {
		return val1;
	}

	return val2;
}

std::pair<glm::vec3, glm::vec3> ExMath::screenToWorld(glm::vec2 screenPos, glm::mat4 projection, glm::mat4 view, float screenWidth, float screenHeight, float nearPlane, float farPlane) {
	glm::mat4 viewI = glm::inverse(view);
	glm::mat4 projI = glm::inverse(projection);

	screenPos.x = (screenPos.x / screenWidth - 0.5f) * 2.0f;
	screenPos.y = -(screenPos.y / screenHeight - 0.5f) * 2.0f;

	float wNear = nearPlane;
	float wFar = farPlane;

	glm::vec4 nearPos(screenPos * wNear, -wNear, wNear);
	nearPos = viewI * projI * nearPos;

	glm::vec4 farPos(screenPos * wFar, wFar, wFar);
	farPos = viewI * projI * farPos;

	return {nearPos, farPos};
}
