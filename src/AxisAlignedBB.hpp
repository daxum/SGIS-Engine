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

struct AxisAlignedBB {
	glm::vec3 min;
	glm::vec3 max;

	AxisAlignedBB() : min(0.0, 0.0, 0.0), max(0.0, 0.0, 0.0) {}

	AxisAlignedBB(glm::vec3 min, glm::vec3 max);

	bool intersects(const AxisAlignedBB& other) const;
	bool contains(const AxisAlignedBB& other) const;

	glm::vec3 getCenter() const;
	float getArea() const;
	float xLength() const;
	float yLength() const;
	float zLength() const;

	void translate(glm::vec3 dist);
	void scale(glm::vec3 amount);
	void scaleAll(float amount);

	static AxisAlignedBB interpolate(const AxisAlignedBB& start, const AxisAlignedBB& finish, float percent);
};
