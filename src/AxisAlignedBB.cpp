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

#include "AxisAlignedBB.hpp"
#include "ExtraMath.hpp"

AxisAlignedBB::AxisAlignedBB(glm::vec3 min, glm::vec3 max) :
	min(min),
	max(max) {

}

bool AxisAlignedBB::intersects(const AxisAlignedBB& other) const {
	return min.x < other.max.x && max.x > other.min.x &&
		   min.y < other.max.y && max.y > other.min.y &&
		   min.z < other.max.z && max.z > other.min.z;
}

bool AxisAlignedBB::contains(const AxisAlignedBB& other) const {
	return min.x <= other.min.x && max.x >= other.max.x &&
		   min.y <= other.min.y && max.y >= other.max.y &&
		   min.z <= other.min.z && max.z >= other.max.z;
}

glm::vec3 AxisAlignedBB::getCenter() const {
	return glm::vec3((min.x + max.x) / 2.0f,
					 (min.y + max.y) / 2.0f,
					 (min.z + max.z) / 2.0f);
}

float AxisAlignedBB::getArea() const {
	return (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
}

float AxisAlignedBB::xLength() const {
	return max.x - min.x;
}

float AxisAlignedBB::yLength() const {
	return max.y - min.y;
}

float AxisAlignedBB::zLength() const {
	return max.z - min.z;
}

void AxisAlignedBB::translate(glm::vec3 dist) {
	min += dist;
	max += dist;
}

void AxisAlignedBB::scale(glm::vec3 amount) {
	glm::vec3 center = getCenter();

	glm::vec3 diff = (max - center) * amount - (max - center);

	min -= diff;
	max += diff;
}

void AxisAlignedBB::scaleAll(float amount) {
	scale(glm::vec3(amount, amount, amount));
}

AxisAlignedBB AxisAlignedBB::interpolate(const AxisAlignedBB& start, const AxisAlignedBB& finish, float percent) {
	return AxisAlignedBB(glm::vec3(ExMath::interpolate(start.min.x, finish.min.x, percent),
								   ExMath::interpolate(start.min.y, finish.min.y, percent),
								   ExMath::interpolate(start.min.z, finish.min.z, percent)),
						 glm::vec3(ExMath::interpolate(start.max.x, finish.max.x, percent),
								   ExMath::interpolate(start.max.y, finish.max.y, percent),
								   ExMath::interpolate(start.max.z, finish.max.z, percent)));
}
