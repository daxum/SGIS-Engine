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

#include "btBulletDynamicsCommon.h"

#include "AxisAlignedBB.hpp"

//A wrapper for bullet physics objects for easier deletion.
//Only meant to be subclassed from within the engine.
class PhysicsObject {
public:
	/**
	 * Constructs the object with the given bullet stuff.
	 * Doesn't actually set the member variables - Subclasses have
	 * to do that themselves.
	 */
	PhysicsObject() {}

	/**
	 * Deletes the objects.
	 */
	virtual ~PhysicsObject() {
		delete body;
		delete shape;
		delete state;
	}

	/**
	 * Returns the physics body, only to be called from the PhysicsComponent.
	 */
	btRigidBody* getBody() { return body; }

	/**
	 * Returns the motion state, mostly used for rendering.
	 */
	btMotionState* getMotionState() { return state; }

protected:
	btRigidBody* body;
	btCollisionShape* shape;
	btMotionState* state;
};
