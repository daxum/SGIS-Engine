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
#include <iostream>
#include <string>

struct AxisAlignedBB {
	glm::vec3 min;
	glm::vec3 max;

	AxisAlignedBB() : min(0.0, 0.0, 0.0), max(0.0, 0.0, 0.0) {}
	AxisAlignedBB(glm::vec3 min, glm::vec3 max);
	AxisAlignedBB(const AxisAlignedBB& box1, const AxisAlignedBB& box2);

	bool intersects(const AxisAlignedBB& other) const;
	bool contains(const AxisAlignedBB& other) const;
	bool formsBoxWith(const AxisAlignedBB& other) const;

	glm::vec3 getCenter() const;
	float getArea() const { return xLength() * yLength() * zLength(); }
	float xLength() const { return max.x - min.x; }
	float yLength() const { return max.y - min.y; }
	float zLength() const { return max.z - min.z; }

	void translate(glm::vec3 dist) { min += dist; max += dist; }
	void scale(glm::vec3 amount);
	void scaleAll(float amount) { scale({amount, amount, amount}); }

	std::string toString() const;

	static AxisAlignedBB interpolate(const AxisAlignedBB& start, const AxisAlignedBB& finish, float percent);
};

std::ostream& operator<<(std::ostream& out, const AxisAlignedBB& box);
