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
#include "ExtraMath.hpp"

namespace {
	std::mt19937 engine(ExMath::getTimeMillis());
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	std::uniform_int_distribution<int> intDistribution;
}

float ExMath::interpolate(float start, float finish, float percent) {
		return (finish - start) * percent + start;
	}

float ExMath::randomFloat(float min, float max) {
	return interpolate(min, max, distribution(engine));
}

bool ExMath::randomBool() {
	return (bool)(intDistribution(engine) & 1);
}

int ExMath::randomInt(int min, int max) {
	return intDistribution(engine) % (max - min + 1) + min;
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
