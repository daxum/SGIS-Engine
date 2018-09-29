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

#include "BoxPhysicsObject.hpp"

BoxPhysicsObject::BoxPhysicsObject(const Aabb<float>& aabb, const glm::vec3& position, float mass, float friction) :
	PhysicsObject() {

	shape = new btBoxShape(btVector3(aabb.xLength() / 2.0f, aabb.yLength() / 2.0f, aabb.zLength() / 2.0f));
	state = new btDefaultMotionState();

	btVector3 localInertia(0.0f, 0.0f, 0.0f);

	if (mass != 0.0f) {
		shape->calculateLocalInertia(mass, localInertia);
	}

	btTransform initialTransform;
	initialTransform.setIdentity();
	initialTransform.setOrigin(btVector3(position.x, position.y, position.z));
	state->setWorldTransform(initialTransform);

	btRigidBody::btRigidBodyConstructionInfo info(mass, state, shape, localInertia);
	info.m_friction = friction;

	body = new btRigidBody(info);
}
