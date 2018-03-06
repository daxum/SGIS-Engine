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

#include "Component.hpp"
#include "AxisAlignedBB.hpp"
#include "Object.hpp"

class PhysicsComponent : public Component {
public:
	/**
	 * Creates a PhysicsComponent with the provided object as its parent.
	 * @param object The parent of this component.
	 * @param box The bounding box of this object.
	 */
	PhysicsComponent(Object& object, AxisAlignedBB box) : Component(object, PHYSICS_COMPONENT_NAME), box(box), velocity(0.0, 0.0, 0.0) {}

	/**
	 * Returns a reference to the box for the object. This SHOULD NOT be stored anywhere
	 * where it might be dereferenced after the object is removed.
	 * @return The box for the object.
	 */
	AxisAlignedBB& getBox() { return box; }

	/**
	 * Returns a reference to the velocity of the object. This also should not be stored anywhere
	 * where it might be dereferenced after the object is removed.
	 * @return The velocity of the object.
	 */
	glm::vec3& getVelocity() { return velocity; }

private:
	AxisAlignedBB box;

	glm::vec3 velocity;
};
