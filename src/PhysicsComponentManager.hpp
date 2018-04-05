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
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h"
#include "ComponentManager.hpp"

class PhysicsComponentManager : public ComponentManager {
public:
	PhysicsComponentManager();
	~PhysicsComponentManager();

	/**
	 * Updates all physics components.
	 * @param screen The screen that owns this PhysicsComponentManager.
	 */
	void update(Screen* screen);

	/**
	 * Sets the gravity for the world.
	 * @param x, y, z The force vector to be applied.
	 */
	void setGravity(float x, float y, float z) { world->setGravity(btVector3(x, y, z)); }

private:
	/**
	 * Only called from bullet.
	 */
	static void physicsTickCallback(btDynamicsWorld* world, btScalar timeStep);

	//Screen pointer, set during update function for access from collision callback.
	Screen* currentScreen;

	btDiscreteDynamicsWorldMt* world;
	btDefaultCollisionConfiguration* conf;
	btCollisionDispatcherMt* dispatcher;
	btBroadphaseInterface* broadphase;
	btSequentialImpulseConstraintSolverMt* solver;
	btConstraintSolverPoolMt* solverPool;

	/**
	 * Overridden from ComponentManager.
	 */
	void onComponentAdd(std::shared_ptr<Component> comp);

	/**
	 * Overridden from ComponentManager.
	 */
	void onComponentRemove(std::shared_ptr<Component> comp);

	/**
	 * Called from bullet, used for collision callbacks in physics objects.
	 */
	void tickCallback();
};
