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

#include "PhysicsObject.hpp"

PhysicsObject::PhysicsObject(const PhysicsInfo& createInfo) :
	body(nullptr),
	shape(nullptr),
	state(nullptr),
	startingMass(createInfo.mass) {

	switch (createInfo.shape) {
		case PhysicsShape::PLANE: shape = createPlaneObject(createInfo); break;
		case PhysicsShape::BOX: shape = createBoxObject(createInfo); break;
		case PhysicsShape::CAPSULE: shape = createCapsuleObject(createInfo); break;
		default: throw std::runtime_error("Missing physics shape!");
	}

	state = new btDefaultMotionState();

	btVector3 localInertia(0.0f, 0.0f, 0.0f);

	if (createInfo.mass != 0.0f) {
		shape->calculateLocalInertia(createInfo.mass, localInertia);
	}

	glm::vec3 origin = createInfo.pos;

	btTransform initialTransform;
	initialTransform.setIdentity();
	initialTransform.setOrigin(btVector3(origin.x, origin.y, origin.z));
	state->setWorldTransform(initialTransform);

	btRigidBody::btRigidBodyConstructionInfo info(createInfo.mass, state, shape, localInertia);

	body = new btRigidBody(info);
}

btStaticPlaneShape* PhysicsObject::createPlaneObject(const PhysicsInfo& createInfo) {
	glm::vec3 normal(createInfo.box.max - createInfo.box.min);
	glm::normalize(normal);

	//Point on the plane
	glm::vec3 point = createInfo.box.min;

	//'d' from the plane equation: ax + by + cz + d = 0. Normally distance uses
	//absolute value and divides by length, but here that is fortunately not
	//necessary.
	float offset = glm::dot(normal, point);

	return new btStaticPlaneShape(btVector3(normal.x, normal.y, normal.z), offset);
}
