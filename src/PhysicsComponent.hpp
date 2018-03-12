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

class PhysicsComponent : public Component {
public:
	/**
	 * Creates a PhysicsComponent with the provided object as its parent.
	 * @param object The parent of this component.
	 * @param physics The PhysicsObject that defines this object in the physics engine.
	 */
	PhysicsComponent(Object& object, std::shared_ptr<PhysicsObject> physics) : Component(object, PHYSICS_COMPONENT_NAME), physics(physics), acceleration(2.0f) {}

	/**
	 * Returns the physics body associated with this component.
	 * Should only be called from the ComponentManager.
	 */
	btRigidBody* getBody() { return physics->getBody(); }

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
	 * Sets the target velocity of the object, mostly called from the ai.
	 */
	void setVelocity(glm::vec3 v);

	/**
	 * Sets the acceleration, determines how quickly the object reaches the set velocity.
	 */
	void setAcceleration(float accel);

private:
	std::shared_ptr<PhysicsObject> physics;

	btVector3 velocity;
	float acceleration;
};
