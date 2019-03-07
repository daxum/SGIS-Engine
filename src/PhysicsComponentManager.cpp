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

#include <thread>

#include "LinearMath/btThreads.h"
#include "PhysicsComponentManager.hpp"
#include "PhysicsComponent.hpp"
#include "ExtraMath.hpp"
#include "Engine.hpp"

void PhysicsComponentManager::physicsTickCallback(btDynamicsWorld* world, btScalar timeStep) {
	static_cast<PhysicsComponentManager*>(world->getWorldUserInfo())->tickCallback();
}

PhysicsComponentManager::PhysicsComponentManager() :
	ComponentManager(PHYSICS_COMPONENT_NAME),
	conf(new btDefaultCollisionConfiguration()),
	broadphase(new btDbvtBroadphase()),
	solver(new btSequentialImpulseConstraintSolverMt()),
	//Number of parallel solvers might need tweaking later, depending on other threads needed.
	solverPool(new btConstraintSolverPoolMt(std::max(std::thread::hardware_concurrency(), 1u))) {

	dispatcher = new btCollisionDispatcherMt(conf, 40);
	world = new  btDiscreteDynamicsWorldMt(dispatcher, broadphase, solverPool, solver, conf);
	world->setGravity(btVector3(0.0, -9.80665, 0.0));
	world->setInternalTickCallback(physicsTickCallback, this, false);

	btSetTaskScheduler(btGetTBBTaskScheduler());
}

PhysicsComponentManager::~PhysicsComponentManager() {
	delete world;
	delete solver;
	delete broadphase;
	delete dispatcher;
	delete conf;
	delete solverPool;
}

void PhysicsComponentManager::update() {
	for (std::shared_ptr<Component> comp : components) {
		std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);
		physics->update();
	}

	world->stepSimulation(Engine::instance->getConfig().timestep / 1000.0, 20, Engine::instance->getConfig().physicsTimestep);
}

PhysicsComponent* PhysicsComponentManager::raytraceSingle(glm::vec3 start, glm::vec3 end) {
	btVector3 from(start.x, start.y, start.z);
	btVector3 to(end.x, end.y, end.z);

	if (world->getDebugDrawer()) {
		world->getDebugDrawer()->drawLine(from, to, btVector3(1.0, 1.0, 0.0));
	}

	btCollisionWorld::ClosestRayResultCallback closestResult(from, to);

	world->rayTest(from, to, closestResult);

	if (closestResult.hasHit()) {
		return (PhysicsComponent*) closestResult.m_collisionObject->getUserPointer();
	}

	return nullptr;
}

std::vector<PhysicsComponent*> PhysicsComponentManager::raytraceAll(glm::vec3 start, glm::vec3 end) {
	btVector3 from(start.x, start.y, start.z);
	btVector3 to(end.x, end.y, end.z);

	if (world->getDebugDrawer()) {
		world->getDebugDrawer()->drawLine(from, to, btVector3(1.0, 0.0, 0.0));
	}

	btCollisionWorld::AllHitsRayResultCallback allResults(from, to);

	world->rayTest(from, to, allResults);

	std::vector<PhysicsComponent*> out;

	//Apparently bullet arrays can have negative sizes
	for (size_t i = 0; i < (size_t) allResults.m_collisionObjects.size(); i++) {
		out.push_back((PhysicsComponent*) allResults.m_collisionObjects.at(i)->getUserPointer());
	}

	return out;
}

void PhysicsComponentManager::drawDebugLine(glm::vec3 from, glm::vec3 to, glm::vec3 color) {
	btVector3 start(from.x, from.y, from.z);
	btVector3 end(to.x, to.y, to.z);

	if (world->getDebugDrawer()) {
		world->getDebugDrawer()->drawLine(start, end, btVector3(color.x, color.y, color.z));
	}
}

void PhysicsComponentManager::onComponentAdd(std::shared_ptr<Component> comp) {
	std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);

	//Looks stupid, but works. Oh well.
	world->addRigidBody(physics->getBody()->getBody());
}

void PhysicsComponentManager::onComponentRemove(std::shared_ptr<Component> comp) {
	std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);

	world->removeRigidBody(physics->getBody()->getBody());
}

void PhysicsComponentManager::tickCallback() {
	int manifoldCount = world->getDispatcher()->getNumManifolds();

	Engine::instance->parallelFor(0, manifoldCount, [&](size_t i) {
		btPersistentManifold* manifold = world->getDispatcher()->getManifoldByIndexInternal(i);

		PhysicsComponent* object1 = static_cast<PhysicsComponent*>(manifold->getBody0()->getUserPointer());
		PhysicsComponent* object2 = static_cast<PhysicsComponent*>(manifold->getBody1()->getUserPointer());

		object1->onCollide(screen, object2);
		object2->onCollide(screen, object1);
	});
}
