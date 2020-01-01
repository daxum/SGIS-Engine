/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018, 2020
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
#include "PhysicsComponent.hpp"

struct RaytraceResult {
	//The physics component of the object that was hit, nullptr if nothing was hit.
	PhysicsComponent* hitComp;
	//The world position of the hit.
	glm::vec3 hitPos;
	//The normal of the face that was hit.
	glm::vec3 hitNormal;
};

class PhysicsComponentManager : public ComponentManager {
public:
	PhysicsComponentManager();
	~PhysicsComponentManager();

	/**
	 * Updates all physics components.
	 * @param screen The screen that owns this PhysicsComponentManager.
	 */
	void update() override;

	/**
	 * Sets the gravity for the world.
	 * @param x, y, z The force vector to be applied.
	 */
	void setGravity(float x, float y, float z) { world->setGravity(btVector3(x, y, z)); }

	/**
	 * Raytraces through the world and returns the first physics object in the path.
	 * Note that if the ray starts inside an object, that object will be missed.
	 * @param start Where the raytrace starts.
	 * @param end Where the raytrace ends.
	 * @return The first hit, or blank if nothing were hit.
	 */
	RaytraceResult raytraceSingle(glm::vec3 start, glm::vec3 end);

	/**
	 * Raytraces through the world and returns all physics object in the path.
	 * Also see the note on raytraceSingle above.
	 * @param start Where the raytrace starts.
	 * @param end Where the raytrace ends.
	 * @return A list of objects found on the path, with hit information for each.
	 */
	std::vector<RaytraceResult> raytraceAll(glm::vec3 start, glm::vec3 end);

	/**
	 * Raytraces from the mouse position projected into 3d space using the
	 * current camera, and returns the first physics component hit, if any.
	 * @return The object under the mouse, or blank if there wasn't one.
	 */
	RaytraceResult raytraceUnderMouse();

	/**
	 * If using a physics debug renderer, draws a line of the given color at the
	 * specified positions. Does nothing if debug drawing is disabled.
	 * @param from The start of the line.
	 * @param to The end of the line.
	 * @param color The color of the line.
	 */
	void drawDebugLine(glm::vec3 from, glm::vec3 to, glm::vec3 color);

	/**
	 * Returns the physics world, for use in things like debug drawing.
	 * @return The physics world.
	 */
	btCollisionWorld* getWorld() const { return world; }

private:
	/**
	 * Only called from bullet.
	 */
	static void physicsTickCallback(btDynamicsWorld* world, btScalar timeStep);

	//Misc. bullet junk.
	btDiscreteDynamicsWorldMt* world;
	btDefaultCollisionConfiguration* conf;
	btCollisionDispatcherMt* dispatcher;
	btBroadphaseInterface* broadphase;
	btSequentialImpulseConstraintSolverMt* solver;
	btConstraintSolverPoolMt* solverPool;

	/**
	 * Overridden from ComponentManager.
	 */
	void onComponentAdd(std::shared_ptr<Component> comp) override;

	/**
	 * Overridden from ComponentManager.
	 */
	void onComponentRemove(std::shared_ptr<Component> comp) override;

	/**
	 * Called from bullet, used for collision callbacks in physics objects.
	 */
	void tickCallback();
};
