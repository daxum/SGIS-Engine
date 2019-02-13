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

//Various shapes provided by the physics engine
enum class PhysicsShape {
	//A plane that extends infinitely in all directions. As they are now,
	//these work as floors and walls, but not roofs.
	//All objects below the plane will be pushed above it.
	PLANE,
	//A three-dimensional square.
	BOX,
	//A cylinder with half-spheres at the ends
	CAPSULE,
	//A sphere
	SPHERE
};

//TODO: expose all of btRigidBodyConstructionInfo.
struct PhysicsInfo {
	//The shape for the physics object.
	PhysicsShape shape;
	//The bounding box for the object. Some objects (such as capsules) extend beyond their bounding box.
	//For planes, the min of the box is a point on the plane and (max - min) represents the normal vector.
	Aabb<float> box;
	//The starting position of the object. Note that for planes, this is an additional shift to the one for the box.
	glm::vec3 pos;
	//The mass of the object. 0 indicates a (permanently) static object.
	float mass;
};

//A wrapper for bullet physics objects for easier deletion.
//Only meant to be subclassed from within the engine.
class PhysicsObject {
public:
	/**
	 * Constructs the object for use in the physics engine.
	 * @param createInfo The information needed to create the object.
	 */
	PhysicsObject(const PhysicsInfo& createInfo);

	/**
	 * Creates a static concave triangle bvh from the given mesh, offset
	 * by the given position.
	 * @param modelName The name of the model to generate the object from.
	 * @param pos The offset for the object.
	 */
	PhysicsObject(const std::string& modelName, const glm::vec3& pos = glm::vec3(0.0, 0.0, 0.0));

	/**
	 * Deletes the objects.
	 */
	~PhysicsObject() {
		delete body;
		delete shape;
		delete state;

		if (mesh != nullptr) {
			delete mesh;
		}
	}

	/**
	 * Returns the physics body, only to be called from the PhysicsComponent.
	 */
	btRigidBody* getBody() { return body; }

	/**
	 * Returns the motion state, mostly used for rendering.
	 */
	btMotionState* getMotionState() { return state; }

	/**
	 * Gets the mass the object started with.
	 * @return A random floating point number which may or may not
	 * reflect the value originally passed in, largely dependent on the
	 * season and current phase of the moon.
	 */
	 float getInitialMass() const { return startingMass; }

private:
	btRigidBody* body;
	btCollisionShape* shape;
	btMotionState* state;
	//Only used for static mesh objects.
	btTriangleMesh* mesh;
	//Stored for switching between kinematic/dynamic/static.
	float startingMass;

	/**
	 * Creates a plane collision object from the creation info.
	 * @param createInfo The creation info for the physics object.
	 * @return A plane shape.
	 */
	btStaticPlaneShape* createPlaneObject(const PhysicsInfo& createInfo);

	/**
	 * Creates a box collision object.
	 * @param createInfo The creation info for the physics object.
	 * @return The new box shape.
	 */
	btBoxShape* createBoxObject(const PhysicsInfo& createInfo) {
		Aabb<float> box = createInfo.box;

		return new btBoxShape(btVector3(box.xLength() / 2.0f, box.yLength() / 2.0f, box.zLength() / 2.0f));
	}

	/**
	 * Creates a capsule. The ends are always at the top and bottom for now.
	 * @param createInfo The physics object creation info.
	 * @return A new capsule shape.
	 */
	btCapsuleShape* createCapsuleObject(const PhysicsInfo& createInfo) {
		Aabb<float> box = createInfo.box;

		return new btCapsuleShape(box.xLength() / 2.0f, box.yLength() / 2.0f);
	}

	/**
	 * Creates a sphere. This assumes that the bounding box is a cube.
	 * @param createInfo The physics object creation info.
	 * @return A new sphere shape.
	 */
	btSphereShape* createSphereObject(const PhysicsInfo& createInfo) {
		Aabb<float> box = createInfo.box;

		return new btSphereShape(box.xLength() / 2.0f);
	}
};
