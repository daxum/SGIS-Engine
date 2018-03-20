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

#include <limits>

#include "PhysicsObject.hpp"

class PlanePhysicsObject : public PhysicsObject {
public:
	PlanePhysicsObject() : PhysicsObject(), planeBox(glm::vec3(std::numeric_limits<float>::min(), 0.0, std::numeric_limits<float>::min()), glm::vec3(std::numeric_limits<float>::max(), 0.0, std::numeric_limits<float>::max())) {
		shape = new btStaticPlaneShape(btVector3(0.0, 1.0, 0.0), 0.0);
		state = new btDefaultMotionState();

		btRigidBody::btRigidBodyConstructionInfo info(0.0, state, shape);

		body = new btRigidBody(info);
	}

	AxisAlignedBB& getBoundingBox() { return planeBox; }

private:
	AxisAlignedBB planeBox;
};
