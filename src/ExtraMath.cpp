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

#include "ExtraMath.hpp"

namespace {
	//Psuedo-square time to avoid chunking when threads all initialize at once.
	thread_local std::mt19937 engine(ExMath::getTimeMillis() * ExMath::getTimeMillis());
}

float ExMath::randomFloat(float min, float max) {
	return std::uniform_real_distribution<float>(min, max)(engine);
}

bool ExMath::randomBool(double weight) {
	return std::bernoulli_distribution(weight)(engine);
}

int ExMath::randomInt(int min, int max) {
	return std::uniform_int_distribution<int>(min, max)(engine);
}

int ExMath::randomBinomialInt(int min, int max, int average) {
	return std::binomial_distribution<int>(max - min, (double)(average - min) / (max - min))(engine) + min;
}

float ExMath::randomDistribution(float mean, float stddev) {
	return std::normal_distribution<float>(mean, stddev)(engine);
}

double ExMath::getTimeMillis() {
	std::chrono::duration<double, std::ratio<1, 1000>> time = std::chrono::steady_clock::now().time_since_epoch();
	return time.count();
}

std::pair<glm::vec3, glm::vec3> ExMath::screenToWorld(
	glm::vec2 screenPos,
	const glm::mat4& projection,
	const glm::mat4& view,
	const float screenWidth,
	const float screenHeight,
	const float nearPlane,
	const float farPlane) {

	glm::mat4 viewProjI = glm::inverse(projection * view);

	screenPos.x = (screenPos.x / screenWidth - 0.5f) * 2.0f;
	screenPos.y = -(screenPos.y / screenHeight - 0.5f) * 2.0f;

	float wNear = nearPlane;
	float wFar = farPlane;

	glm::vec4 nearPos(screenPos * wNear, -wNear, wNear);
	nearPos = viewProjI * nearPos;

	glm::vec4 farPos(screenPos * wFar, wFar, wFar);
	farPos = viewProjI * farPos;

	return {nearPos, farPos};
}
