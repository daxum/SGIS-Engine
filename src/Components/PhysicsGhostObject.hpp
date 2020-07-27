/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2020
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

#include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "PhysicsObject.hpp"

class PhysicsComponent;

struct PhysicsGhostInfo {
	//The shape for the ghost.
	PhysicsShape shape;
	//The bounding box for the ghost. Some objects (such as capsules) extend beyond their bounding box.
	//For planes, the min of the box is a point on the plane and (max - min) represents the normal vector.
	Aabb<float> box;
	//The starting position of the ghost, relative to the parent object. Note that for planes, this is an additional shift to the one for the box.
	glm::vec3 pos;
};

class PhysicsGhostObject {
public:
	/**
	 * Constructor for the ghost.
	 * @param info The physics info defining the object.
	 * @param parent The physics component this ghost is a part of.
	 */
	PhysicsGhostObject(const PhysicsGhostInfo& info, PhysicsComponent* parent);

	/**
	 * Destroys the ghost object.
	 */
	~PhysicsGhostObject() {
		delete ghost;
		delete shape;
	}

	/**
	 * Gets the created ghost object, for adding to the physics component.
	 * @return The ghost object.
	 */
	btGhostObject* getObject() { return ghost; }

	/**
	 * Gets the offset of the ghost from its parent object.
	 * @return The relative offset from the parent's center.
	 */
	glm::vec3 getOffset() const { return posOffset; }

	/**
	 * Finds all the physics components that are currently colliding with
	 * this ghost object, excluding the parent object.
	 * @return A list of colliding physics components.
	 */
	std::vector<PhysicsComponent*> getCollisions();

private:
	//The object.
	btGhostObject* ghost;
	//Shape used by the object for collisions.
	btCollisionShape* shape;
	//Relative position of the ghost from the parent object's center.
	glm::vec3 posOffset;
	//The parent of this ghost object.
	PhysicsComponent* parent;

	/**
	 * Creates a box collision object.
	 * @param createInfo The creation info for the physics object.
	 * @return The new box shape.
	 */
	btBoxShape* createBoxObject(const PhysicsGhostInfo& createInfo) {
		Aabb<float> box = createInfo.box;

		return new btBoxShape(btVector3(box.xLength() / 2.0f, box.yLength() / 2.0f, box.zLength() / 2.0f));
	}

	/**
	 * Creates a capsule. The ends are always at the top and bottom for now.
	 * @param createInfo The physics object creation info.
	 * @return A new capsule shape.
	 */
	btCapsuleShape* createCapsuleObject(const PhysicsGhostInfo& createInfo) {
		Aabb<float> box = createInfo.box;

		return new btCapsuleShape(box.xLength() / 2.0f, box.yLength());
	}

	/**
	 * Creates a sphere. This assumes that the bounding box is a cube.
	 * @param createInfo The physics object creation info.
	 * @return A new sphere shape.
	 */
	btSphereShape* createSphereObject(const PhysicsGhostInfo& createInfo) {
		Aabb<float> box = createInfo.box;

		return new btSphereShape(box.xLength() / 2.0f);
	}
};
