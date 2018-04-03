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
	//Skip update if paused
	if (paused) {
		return;
	}

	//Add queued objects
	for (std::shared_ptr<Object> object : additionList) {
		addObjectToList(object);
	}

	additionList.clear();

	//Update components
	for (std::shared_ptr<ComponentManager> manager : managers) {
		manager->update(this);
	}

	//Update camera
	camera->update();

	//Remove queued objects
	for (std::shared_ptr<Object> object : removalList) {
		deleteObject(object);
	}

	removalList.clear();
}

bool Screen::isKeyPressed(Key key) {
	return display.isKeyPressed(key);
}

void Screen::addComponentManager(std::shared_ptr<ComponentManager> manager) {
	if (manager->name == RENDER_COMPONENT_NAME) {
		renderManager = std::static_pointer_cast<RenderComponentManager>(manager);
	}

	managers.push_back(manager);
}

void Screen::addObject(std::shared_ptr<Object> object) {
	additionList.push_back(object);
}

void Screen::removeObject(std::shared_ptr<Object> object) {
	removalList.push_back(object);
}

void Screen::removeObject(Object* object) {
	std::shared_ptr<Object> sharedObject = objects.getShared(object);

	if (sharedObject) {
		removeObject(sharedObject);
	}
}

void Screen::deleteObject(std::shared_ptr<Object> object) {
	objects.erase(object);

	//Remove components
	for (std::shared_ptr<ComponentManager> manager : managers) {
		std::shared_ptr<Component> comp = object->getComponent<Component>(manager->name);

		if (comp) {
			manager->removeComponent(comp);
		}
	}
}

void Screen::addObjectToList(std::shared_ptr<Object> object) {
	//Add to main list
	objects.insert(object);

	//Add any components to managers
	for (std::shared_ptr<ComponentManager> manager : managers) {
		std::shared_ptr<Component> comp = object->getComponent<Component>(manager->name);

		if (comp) {
			manager->addComponent(comp);
		}
	}
}

glm::vec2 Screen::getMousePos() {
	return display.getMousePos();
}

glm::vec2 Screen::getMouseDist() {
	return display.getMouseDist();
}
