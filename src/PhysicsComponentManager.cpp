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

#include "PhysicsComponentManager.hpp"
#include "PhysicsComponent.hpp"
#include "ExtraMath.hpp"

PhysicsComponentManager::PhysicsComponentManager() :
	ComponentManager(PHYSICS_COMPONENT_NAME),
	conf(new btDefaultCollisionConfiguration()),
	broadphase(new btDbvtBroadphase()),
	solver(new btSequentialImpulseConstraintSolver()) {

	dispatcher = new btCollisionDispatcher(conf);
	world = new  btDiscreteDynamicsWorld(dispatcher, broadphase, solver, nullptr);
}

PhysicsComponentManager::~PhysicsComponentManager() {
	delete world;
	delete solver;
	delete broadphase;
	delete dispatcher;
	delete conf;
}

void PhysicsComponentManager::update(Screen* screen) {
	for (std::shared_ptr<Component> comp : components) {
		std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);
		physics->update();
	}

	world->stepSimulation(1.0f/60.0f, 10);
}

void PhysicsComponentManager::onComponentAdd(std::shared_ptr<Component> comp) {
	std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);

	world->addRigidBody(physics->getBody());
}

void PhysicsComponentManager::onComponentRemove(std::shared_ptr<Component> comp) {
	std::shared_ptr<PhysicsComponent> physics = std::static_pointer_cast<PhysicsComponent>(comp);

	world->removeRigidBody(physics->getBody());
}
