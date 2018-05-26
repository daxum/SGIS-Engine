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

#include "Object.hpp"

//Messy, but unfortunately necessary. Static class variables are weird.
ObjectPhysicsInterface Object::defaultInterface;

Object::Object() : physics(nullptr) {

}

void Object::addComponent(std::shared_ptr<Component> component) {
	if (components.count(component->name)) {
		throw std::runtime_error("Duplicate component of type " + component->name + " added!");
	}
	component->setParent(shared_from_this());
	components.insert({component->name, component});
}

ObjectPhysicsInterface* Object::getPhysics() {
	if (!hasPhysics()) {
		return &defaultInterface;
	}

	return physics;
}

bool Object::hasPhysics() {
	return physics != nullptr;
}

void Object::setPhysics(ObjectPhysicsInterface* phys) {
	physics = phys;
}
