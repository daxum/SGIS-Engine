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
#include "Object.hpp"
#include "PhysicsObject.hpp"
#include "ObjectPhysicsInterface.hpp"
#include "Screen.hpp"

class PhysicsComponent;

//Allows for user-defined collision responses.
struct CollisionHandler {
	//Set by physics component when added.
	PhysicsComponent* parent;

	virtual void handleCollision(Screen* screen, PhysicsComponent* hitObject) = 0;
};

class PhysicsComponent : public Component, ObjectPhysicsInterface {
public:
	/**
	 * Creates a PhysicsComponent with the provided object as its parent.
	 * @param object The parent of this component.
	 * @param physics The PhysicsObject that defines this object in the physics engine.
	 * @param collHandler An object containing a function to call when this object collides with another one.
	 * @param collMask A mask to prevent collision callbacks from being called.
	 */
	PhysicsComponent(Object& object, std::shared_ptr<PhysicsObject> physics, std::shared_ptr<CollisionHandler> collHandler = std::shared_ptr<CollisionHandler>());

	/**
	 * Returns the physics body associated with this component.
	 */
	std::shared_ptr<PhysicsObject> getBody() { return physics; }

	/**
	 * Applies velocity changes and such to the internal object.
	 */
	void update();

	/**
	 * Used by rendering.
	 */
	glm::vec3 getTranslation();

	/**
	 * Also used by rendering.
	 */
	glm::vec3 getRotation();

	/**
	 * Returns a unit vector representing the direction the object is facing.
	 */
	glm::vec3 getFront();

	/**
	 * Sets the target velocity of the object, mostly called from the ai.
	 */
	void setVelocity(glm::vec3 v);

	/**
	 * If this is on, the objects velocity will gradually be changed to the set velocity.
	 * If this is off, the same happens, but the object's velocity will not be slowed for components
	 * that have the same direction as the set velocity.
	 * Defaults to on.
	 */
	void velocityReduction(bool enable);

	/**
	 * Sets the acceleration, determines how quickly the object reaches the set velocity.
	 */
	void setAcceleration(float accel);

	/**
	 * Called by the physics component manager when this object collides with another.
	 * @param screen A screen, can be used for removing / adding other objects as a result of a collision.
	 * @param other The object that was collided with.
	 */
	void onCollide(Screen* screen, PhysicsComponent* other);

	/**
	 * Returns the parent object, mainly for removal from screen and manipulating state.
	 */
	Object* getParent() { return &parent; }

private:
	std::shared_ptr<PhysicsObject> physics;
	std::shared_ptr<CollisionHandler> collider;

	bool brakes;
	btVector3 velocity;
	float acceleration;
};
