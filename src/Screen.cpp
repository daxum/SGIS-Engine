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

#include "Screen.hpp"
#include "DisplayEngine.hpp"
#include "ComponentManager.hpp"
#include "RenderComponentManager.hpp"

void Screen::update() {
	for (std::shared_ptr<ComponentManager> manager : managers) {
		manager->update(this);
		//TODO: object removal needs to be deferred to around here. Basically, in between update steps, or after updating is complete.
	}

	//TEMPORARY "physics engine" for testing. Will move into component manager later.
	for (std::shared_ptr<Object> object : objects) {
		object->pos += object->velocity;
		object->velocity *= 0.98f;
	}
}

bool Screen::isKeyPressed(Key key) {
	return display.isKeyPressed(key);
}

void Screen::addComponentManager(std::shared_ptr<ComponentManager> manager) {
	if (manager->name == RENDER_COMPONENT_NAME) {
		renderData.componentManager = std::static_pointer_cast<RenderComponentManager>(manager);
	}

	managers.push_back(manager);
}

void Screen::addObject(std::shared_ptr<Object> object) {
	//Add to main list
	objectIndices[object] = objects.size();
	objects.push_back(object);

	//Add any components to managers
	for (std::shared_ptr<ComponentManager> manager : managers) {
		std::shared_ptr<Component> comp = object->getComponent(manager->name);

		if (comp) {
			manager->addComponent(comp);
		}
	}
}

void Screen::removeObject(std::shared_ptr<Object> object) {
	if (objectIndices.count(object) == 0) {
		//Object not found.
		return;
	}

	//Switch with last, pop, and clear map.
	objects[objectIndices[object]] = objects.back();
	objects.pop_back();
	objectIndices.erase(object);

	//Remove components
	for (std::shared_ptr<ComponentManager> manager : managers) {
		std::shared_ptr<Component> comp = object->getComponent(manager->name);

		if (comp) {
			manager->removeComponent(comp);
		}
	}
}

void Screen::setMap(std::shared_ptr<Map> newMap) {
	renderData.mapData = newMap->getRenderData();
	map = newMap;
}
