/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2020
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

#include "PhysicsGhostObject.hpp"

PhysicsGhostObject::PhysicsGhostObject(const PhysicsGhostInfo& info, PhysicsComponent* parent) :
	ghost(nullptr),
	shape(nullptr),
	posOffset(info.pos),
	parent(parent) {

	ghost = new btGhostObject;

	switch (info.shape) {
		case PhysicsShape::PLANE: throw std::runtime_error("Plane not supported for ghosts!");
		case PhysicsShape::BOX: shape = createBoxObject(info); break;
		case PhysicsShape::CAPSULE: shape = createCapsuleObject(info); break;
		case PhysicsShape::SPHERE: shape = createSphereObject(info); break;
		default: throw std::runtime_error("Missing physics shape!");
	}

	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(info.pos.x, info.pos.y, info.pos.z));

	ghost->setWorldTransform(trans);

	ghost->setCollisionShape(shape);
	ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

std::vector<PhysicsComponent*> PhysicsGhostObject::getCollisions() {
	uint64_t numCollisions = ghost->getNumOverlappingObjects();

	std::vector<PhysicsComponent*> out;

	for (uint64_t i = 0; i < numCollisions; i++) {
		PhysicsComponent* comp = static_cast<PhysicsComponent*>(ghost->getOverlappingObject(i)->getUserPointer());

		if (comp != nullptr && comp != parent) {
			out.push_back(comp);
		}
	}

	return out;
}
