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

#include <stdexcept>
#include <array>

#include <glm/glm.hpp>

namespace ExMath {
	//Pi, to the ninth digit.
	constexpr static const float PI = 3.141592654f;

	/**
	 * Clamps the first value to be between the second and third values.
	 * The object passed to this function should implement operator<.
	 * Note: std::clamp is defined in the c++17 standard. If this project ever
	 * uses that, this should be removed.
	 * @param value The value to be clamped
	 * @param minimum The lower bound.
	 * @param maximum The upper bound.
	 * @return The clamped value.
	 * @throw logic_error if minimum is greater than maximum.
	 */
	template <typename T>
	constexpr const T& clamp(const T& value, const T& minimum, const T& maximum) {
		if (maximum < minimum) {
			throw std::logic_error("Maximum not greater than minimum!");
		}

		return (value > maximum) ? maximum : ((value < minimum) ? minimum : value);
	}

	/**
	 * Linearly interpolates between start and finish by percent.
	 * @param start The value to start interpolating at.
	 * @param finish The value to finish interpolating at.
	 * @param percent The percent to interpolate.
	 * @return The interpolated value.
	 */
	template<typename T>
	constexpr T interpolate(const T& start, const T& finish, const float percent) {
		return (1.0f - percent) * start + percent * finish;
	}

	/**
	 * Bilinear interpolation between four points.
	 * @param corners The four corners to interpolate between. The order is
	 *     top left, top right, bottom left, bottom right.
	 * @param xWeight, yWeight The x and y percents for the interpolation, in that order.
	 * @return A point that is the result of the bilinear interpolation by the
	 *     given percent of the four corners.
	 */
	template<typename T>
	constexpr T bilinearInterpolate(const std::array<T, 4>& corners, const float xWeight, const float yWeight)  {
		return (1.0f - yWeight) * interpolate(corners.at(0), corners.at(1), xWeight) +
				yWeight * interpolate(corners.at(2), corners.at(3), xWeight);
	}

	/**
	 * Interpolates in three dimensions.
	 * @param corners The eight corners of the cube to interpolate within.
	 *     Goes from min square to max, square corners have the same
	 *     order as in bilinear interpolation.
	 * @param xWeight, yWeight, zWeight The percents to interpolate by.
	 * @return The interpolated value.
	 */
	template<typename T>
	constexpr T trilinearInterpolate(const std::array<T, 8>& corners, const float xWeight, const float yWeight, const float zWeight) {
		std::array<T, 4> top = {corners.at(0), corners.at(1), corners.at(2), corners.at(3)};
		std::array<T, 4> bottom = {corners.at(4), corners.at(5), corners.at(6), corners.at(7)};

		return (1.0f - zWeight) * bilinearInterpolate(top, xWeight, yWeight) +
				zWeight * bilinearInterpolate(bottom, xWeight, yWeight);
	}

	/**
	 * Generates a random floating point number between min and max.
	 * @param min The minimum possible value for the random number.
	 * @param max The maximum possible value for the random number.
	 * @return A random number between min and max.
	 */
	float randomFloat(float min, float max);

	/**
	 * Generates a random boolean value.
	 * @return A random boolean.
	 */
	bool randomBool();

	/**
	 * Generates a random integer value between min and max.
	 * @param min The minimum possible value.
	 * @param max The maximum possible value.
	 * @return A random integer between min and max.
	 */
	int randomInt(int min, int max);

	/**
	 * Gets the current time, in milliseconds.
	 * @return The current time.
	 */
	double getTimeMillis();

	/**
	 * Finds which of two numbers has the lower magnitude.
	 * The minimum magnitude is the lesser of the two numbers'
	 * absolute values.
	 * @param val1 The first number.
	 * @param val2 The second number.
	 * @return The number closest to zero.
	 */
	template<typename T>
	constexpr const T& minMagnitude(const T& val1, const T& val2) {
		if (std::min(std::abs(val1), std::abs(val2)) == std::abs(val1)) {
			return val1;
		}

		return val2;
	}

	/**
	 * Finds which of two numbers has the greater magnitude.
	 * @param val1 The first number.
	 * @param val2 The second number.
	 * @return The number farthest from zero.
	 */
	template<typename T>
	constexpr const T& maxMagnitude(const T& val1, const T& val2) {
		if (minMagnitude(val1, val2) == val2) {
			return val1;
		}

		return val2;
	}

	/**
	 * Converts screen coordinates to world coordinates.
	 * This might go better in RenderingEngine?
	 * @param screenPos The position on the screen.
	 * @param projection The projection matrix from camera to clip space.
	 * @param view The view matrix from world to camera space.
	 * @param screenWidth The width of the screen, in pixels.
	 * @param screenHeight The height of the screen, in pixels.
	 * @param nearPlane The near plane's distance from the camera.
	 * @param farPlane The far plane's distance from the camera.
	 * @return The point projected onto the near plane (first) and the far plane (second).
	 */
	std::pair<glm::vec3, glm::vec3> screenToWorld(
		glm::vec2 screenPos,
		const glm::mat4& projection,
		const glm::mat4& view,
		const float screenWidth,
		const float screenHeight,
		const float nearPlane,
		const float farPlane
	);

	/**
	 * Rounds initVal to the next highest multiple of roundVal.
	 * @param initVal The number to round.
	 * @param roundVal The number to round initVal to a multiple of.
	 * @return The value of initVal rounded to the next highest multiple
	 *     of roundVal.
	 */
	template<typename T>
	constexpr T roundToVal(const T& initVal, const T& roundVal) {
		//% doesn't work with floating point types.
		if (std::is_floating_point<T>::value) {
			if (std::trunc(initVal / roundVal) == initVal / roundVal) {
				return initVal;
			}
		}
		else {
			if ((initVal % roundVal) == 0) {
				return initVal;
			}
		}

		return (initVal / roundVal + 1) * roundVal;
	}
}
