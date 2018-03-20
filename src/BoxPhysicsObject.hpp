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
 *****************************************************************************/

#pragma once

#include "PhysicsObject.hpp"

class BoxPhysicsObject : public PhysicsObject {
public:
	BoxPhysicsObject(const AxisAlignedBB& aabb, const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f), float mass = 1.0f, float friction = 0.5f);

	AxisAlignedBB& getBoundingBox() { return box; }

private:
	AxisAlignedBB box;
};
