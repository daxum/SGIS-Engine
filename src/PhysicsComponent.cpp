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
#include "ExtraMath.hpp"

PhysicsComponent::PhysicsComponent(std::shared_ptr<PhysicsObject> physics, std::shared_ptr<CollisionHandler> collHandler) :
	Component(PHYSICS_COMPONENT_NAME),
	physics(physics),
	collider(collHandler),
	currentMode(PhysicsControlMode::DYNAMIC),
	linearBrakes(false),
	angularBrakes(false),
	velocity(0.0, 0.0, 0.0),
	angularVelocity(0.0, 0.0, 0.0),
	acceleration(1.2f),
	rotAccel(1.2f) {

	physics->getBody()->setUserPointer(this);

	if (collider) {
		collider->parent = this;
	}

	//Close enough...
	if (physics->getBody()->getInvMass() == 0.0f) {
		currentMode = PhysicsControlMode::STATIC;
	}
}

void PhysicsComponent::setControlMode(PhysicsControlMode mode) {
	currentMode = mode;

	switch (mode) {
		case PhysicsControlMode::DYNAMIC: {
			if (physics->getInitialMass() == 0.0f) {
				throw std::runtime_error("Attempt to set zero-mass object to dynamic!");
			}

			btRigidBody* body = physics->getBody();
			body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(ACTIVE_TAG);
			body->setMassProps(physics->getInitialMass(), body->getLocalInertia());
		}; break;
		case PhysicsControlMode::KINEMATIC: {
			btRigidBody* body = physics->getBody();
			body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setActivationState(DISABLE_DEACTIVATION);
			body->setMassProps(0.0f, body->getLocalInertia());
		}; break;
		case PhysicsControlMode::STATIC: {
			btRigidBody* body = physics->getBody();
			body->setCollisionFlags(body->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
			body->setMassProps(0.0f, body->getLocalInertia());
		}; break;
		default: throw std::runtime_error("Somehow static, dynamic, and kinematic weren't enough!");
	}
}

void PhysicsComponent::onParentSet() {
	lockParent()->setPhysics(this);
}

void PhysicsComponent::update() {
	switch (currentMode) {
		case PhysicsControlMode::DYNAMIC: {
			btRigidBody* body = physics->getBody();

			btVector3 force = getAdjustedForce(velocity, body->getLinearVelocity(), acceleration, body->getLinearDamping(), linearBrakes);
			btVector3 torque = getAdjustedForce(angularVelocity, body->getAngularVelocity(), rotAccel, body->getAngularDamping(), angularBrakes);

			body->applyCentralForce(force);
			body->applyTorque(torque);
		}; break;
		case PhysicsControlMode::KINEMATIC: {
			ObjectPhysicsInterface* provider = lockParent()->getPhysics();

			glm::vec3 pos = provider->getTranslation();
			glm::quat rot = provider->getRotation();

			physics->getMotionState()->setWorldTransform(btTransform(btQuaternion(rot.x, rot.y, rot.z, rot.w), btVector3(pos.x, pos.y, pos.z)));
		}; break;
		case PhysicsControlMode::STATIC: break; //Noop
		default: throw std::runtime_error("Missing physics control mode");
	}
}

glm::vec3 PhysicsComponent::getTranslation() const {
	btTransform transform;
	physics->getMotionState()->getWorldTransform(transform);
	btVector3 trans = transform.getOrigin();

	return glm::vec3(trans.x(), trans.y(), trans.z());
}


glm::quat PhysicsComponent::getRotation() const {
	btTransform trans;
	physics->getMotionState()->getWorldTransform(trans);

	btQuaternion btRot = trans.getRotation();

	return glm::quat(btRot.w(), btRot.x(), btRot.y(), btRot.z());
}

glm::vec3 PhysicsComponent::getFront() {
	btTransform trans;
	physics->getMotionState()->getWorldTransform(trans);

	btVector3 front(0.0, 0.0, -1.0);
	front = trans.getBasis() * front;

	return glm::normalize(glm::vec3(front.x(), front.y(), front.z()));
}

void PhysicsComponent::setVelocity(glm::vec3 v) {
	//TODO: Make threshold configurable?
	if (glm::distance(v, glm::vec3(velocity.x(), velocity.y(), velocity.z())) > 0.01f) {
		physics->getBody()->activate(true);
		velocity = btVector3(v.x, v.y, v.z);
	}
}

glm::vec3 PhysicsComponent::getVelocity() {
	const btVector3& velocity = getBody()->getBody()->getLinearVelocity();
	return glm::vec3(velocity.x(), velocity.y(), velocity.z());
}

void PhysicsComponent::applyImpulse(glm::vec3 impulse) {
	if (currentMode == PhysicsControlMode::DYNAMIC) {
		physics->getBody()->activate(true);
		physics->getBody()->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
	}
}

void PhysicsComponent::rotate(glm::vec3 amount) {
	physics->getBody()->activate(true);
	physics->getBody()->applyTorque(btVector3(amount.x, amount.y, amount.z));
}

void PhysicsComponent::setRotation(glm::vec3 amount) {
	physics->getBody()->activate(true);
	angularVelocity = btVector3(amount.x, amount.y, amount.z);
}

void PhysicsComponent::velocityReduction(bool enable) {
	linearBrakes = enable;
}

void PhysicsComponent::rotationReduction(bool enable) {
	angularBrakes = enable;
}

void PhysicsComponent::setAcceleration(float accel) {
	acceleration = accel;
}

void PhysicsComponent::setRotationalAcceleration(float accel) {
	rotAccel = accel;
}

void PhysicsComponent::onCollide(Screen* screen, PhysicsComponent* other) {
	if (collider) {
		collider->handleCollision(screen, other);
	}
}

btVector3 PhysicsComponent::getAdjustedForce(btVector3 target, btVector3 current, float acceleration, float damping, bool brakes) {
	btVector3 newVel = target;

	//Don't slow the object down along an axis if the target velocity is slower than its current speed.
	if (!brakes) {
		btVector3 absSum = current + target;
		absSum = btVector3(std::abs(absSum.x()), std::abs(absSum.y()), std::abs(absSum.z()));

		//If the sum of the magnitudes is greater than the longest vector, they must be pointing in the same direction along that axis.
		if (absSum.x() >= std::max(std::abs(target.x()), std::abs(current.x()))) {
			newVel.setX(ExMath::maxMagnitude(target.x(), current.x()));
		}
		if (absSum.y() >= std::max(std::abs(target.y()), std::abs(current.y()))) {
			newVel.setY(ExMath::maxMagnitude(target.y(), current.y()));
		}
		if (absSum.z() >= std::max(std::abs(target.z()), std::abs(current.z()))) {
			newVel.setZ(ExMath::maxMagnitude(target.z(), current.z()));
		}
	}

	btVector3 velocityDiff = current - newVel;
	return -acceleration * velocityDiff - damping * velocityDiff;
}
