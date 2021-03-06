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
#include "Display/Object.hpp"
#include "PhysicsObject.hpp"
#include "Display/ObjectPhysicsInterface.hpp"
#include "Display/Screen.hpp"
#include "PhysicsGhostObject.hpp"

class PhysicsComponent;

//Determines how the physics body is controlled. Defaults to dynamic if
//mass is non-zero, static otherwise. 0 mass objects cannot currently
//be made dynamic.
enum class PhysicsControlMode {
	//Static body, doesn't move.
	STATIC,
	//Kinematic body, reads transform from the object physics provider.
	KINEMATIC,
	//Dynamic body, almost completely controlled by the physics engine.
	DYNAMIC
};

//Allows for user-defined collision responses.
struct CollisionHandler {
	virtual ~CollisionHandler() {}

	//Set by physics component when added.
	PhysicsComponent* parent;

	/**
	 * Handles collision with the provided object. This function MUST
	 * be threadsafe.
	 * @param screen The parent screen, for adding / removing objects and
	 *     modifying screen state.
	 * @param hitObject The object that was hit.
	 */
	virtual void handleCollision(Screen* screen, PhysicsComponent* hitObject) = 0;
};

class PhysicsComponent : public Component, ObjectPhysicsInterface {
public:
	/**
	 * Required from component.
	 * @return The component's name.
	 */
	static const std::string getName() { return PHYSICS_COMPONENT_NAME; }

	/**
	 * Creates a PhysicsComponent.
	 * @param physics The PhysicsObject that defines this object in the physics engine.
	 * @param collHandler An object containing a function to call when this object collides with another one.
	 */
	PhysicsComponent(std::shared_ptr<PhysicsObject> physics, std::shared_ptr<CollisionHandler> collHandler = std::shared_ptr<CollisionHandler>());

	/**
	 * Changes the way the object is animated, between static, dynamic,
	 * and kinematic. Note that if switching between dynamic or static
	 * and kinematic, the object physics provider also needs to be changed.
	 * @param mode The new mode for the object.
	 */
	void setControlMode(PhysicsControlMode mode);

	/**
	 * Called from Component when the parent object is set.
	 */
	void onParentSet() override;

	/**
	 * Returns the physics body associated with this component.
	 */
	std::shared_ptr<PhysicsObject> getBody() { return physics; }

	/**
	 * Adds the provided ghost object to the component. This should be called
	 * before the parent object is added to the screen!
	 * @param ghost The object to be added.
	 * @return A unique identifier the can be used to reference the added ghost.
	 */
	uint64_t addGhost(const PhysicsGhostInfo& info) {
		ghosts.push_back(std::make_shared<PhysicsGhostObject>(info, this));
		return ghosts.size() - 1;
	}

	/**
	 * Fetches all the physics components intersecting with the provided
	 * ghost object.
	 * @param index The value returned from the call to addGhost for the desired ghost object.
	 * @return A list of items colliding with the selected ghost.
	 */
	std::vector<PhysicsComponent*> getGhostCollisions(uint64_t index) {
		return ghosts.at(index)->getCollisions();
	}

	/**
	 * Applies velocity changes and such to the internal object.
	 */
	void update();

	/**
	 * Used by rendering.
	 */
	glm::vec3 getTranslation() const override;

	/**
	 * Also used by rendering.
	 */
	glm::quat getRotation() const override;

	/**
	 * Returns a unit vector representing the direction the object is facing.
	 */
	glm::vec3 getFront();

	/**
	 * Sets the target velocity of the object, mostly called from the ai.
	 */
	void setVelocity(glm::vec3 v);

	/**
	 * Returns the current velocity of the object.
	 */
	glm::vec3 getVelocity();

	/**
	 * Applies a single-time force to the object.
	 */
	void applyImpulse(glm::vec3 impulse);

	/**
	 * Directly applies torque to the underlying physics object.
	 */
	void rotate(glm::vec3 amount);

	/**
	 * Sets the object's angular velocity to the provided amount.
	 */
	void setRotation(glm::vec3 amount);

	/**
	 * If this is on, the objects velocity will gradually be changed to the set velocity.
	 * If this is off, the same happens, but the object's velocity will not be slowed for components
	 * that have the same direction as the set velocity.
	 * Defaults to on.
	 */
	void velocityReduction(bool enable);

	/**
	 * Same as above, but for angular velocity.
	 * Defaults to off.
	 */
	void rotationReduction(bool enable);

	/**
	 * Sets the acceleration, determines how quickly the object reaches the set velocity.
	 */
	void setAcceleration(float accel);

	/**
	 * Sets rotational acceleration.
	 */
	void setRotationalAcceleration(float accel);

	/**
	 * Sets the damping on the linear velocity of the object.
	 * @param amount The new damping value.
	 */
	void setLinearDamping(float amount) { physics->getBody()->setDamping(amount, physics->getBody()->getAngularDamping()); }

	/**
	 * Called by the physics component manager when this object collides with another.
	 * @param screen A screen, can be used for removing / adding other objects as a result of a collision.
	 * @param other The object that was collided with.
	 */
	void onCollide(Screen* screen, PhysicsComponent* other);

	/**
	 * Returns the parent object, mainly for removal from screen and manipulating state.
	 */
	std::shared_ptr<Object> getParent() { return lockParent(); }

	/**
	 * Gets the components ghost object array. For use by PhysicsManager.
	 * @return The array of ghost objects for this component.
	 */
	std::vector<std::shared_ptr<PhysicsGhostObject>>& getGhosts() { return ghosts; }

private:
	//Physics object used by this physics component.
	std::shared_ptr<PhysicsObject> physics;
	//Collision handler to be called when the component collides with another.
	std::shared_ptr<CollisionHandler> collider;
	//List of ghost objects associated to this component.
	std::vector<std::shared_ptr<PhysicsGhostObject>> ghosts;

	PhysicsControlMode currentMode;
	bool linearBrakes;
	bool angularBrakes;
	btVector3 velocity;
	btVector3 angularVelocity;
	float acceleration;
	float rotAccel;

	btVector3 getAdjustedForce(btVector3 target, btVector3 current, float acceleration, float damping, bool brakes);
};
