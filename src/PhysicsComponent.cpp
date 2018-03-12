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

#include "PhysicsComponent.hpp"

void PhysicsComponent::update() {
	btRigidBody* body = physics->getBody();

	btVector3 velocityDiff = body->getLinearVelocity() - velocity;
	btVector3 force = -acceleration * velocityDiff - body->getLinearDamping() * velocityDiff;
	body->applyCentralForce(force);
}

glm::vec3 PhysicsComponent::getTranslation() {
	btTransform transform;
	physics->getMotionState()->getWorldTransform(transform);
	btVector3 trans = transform.getOrigin();

	return glm::vec3(trans.x(), trans.y(), trans.z());
}


glm::vec3 PhysicsComponent::getRotation() {
	glm::vec3 rotation;

	btTransform trans;
	physics->getMotionState()->getWorldTransform(trans);

	//What even is this...
	trans.getRotation().getEulerZYX(rotation.z, rotation.y, rotation.x);

	return rotation;
}

void PhysicsComponent::setVelocity(glm::vec3 v) {
	physics->getBody()->activate(true);
	velocity = btVector3(v.x, v.y, v.z);
}

void PhysicsComponent::setAcceleration(float accel) {
	acceleration = accel;
}
