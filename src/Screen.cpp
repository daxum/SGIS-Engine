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
#include "Components/ComponentManager.hpp"
#include "Components/RenderManager.hpp"
#include "DefaultCamera.hpp"

Screen::Screen(DisplayEngine& display, bool hideMouse) :
	display(display),
	inputMap(std::make_shared<InputMap>()),
	eventQueue(std::make_shared<EventQueue>()),
	camera(std::make_shared<DefaultCamera>()),
	paused(false),
	hideMouse(hideMouse) {

	camera->setProjection();
	eventQueue->addListener(inputMap);
}

void Screen::update() {
	//Skip update if paused
	if (paused) {
		return;
	}

	//Add queued objects
	std::shared_ptr<Object> toAdd;

	while (!additionList.empty()) {
		additionList.try_pop(toAdd);
		addObjectToList(toAdd);
	}

	//Update components
	for (std::shared_ptr<ComponentManager> manager : managers) {
		manager->update();
	}

	//Update camera
	camera->update();

	//Remove queued objects
	std::shared_ptr<Object> toRemove;

	while (!removalList.empty()) {
		removalList.try_pop(toRemove);
		deleteObject(toRemove);
	}
}

void Screen::addObject(std::shared_ptr<Object> object) {
	additionList.push(object);
}

void Screen::removeObject(std::shared_ptr<Object> object) {
	removalList.push(object);
}

void Screen::setCamera(std::shared_ptr<Camera> newCamera) {
	camera = newCamera;
	camera->setProjection();
}

void Screen::deleteObject(std::shared_ptr<Object> object) {
	objects.erase(object);

	//Remove components
	for (std::shared_ptr<ComponentManager> manager : managers) {
		std::shared_ptr<Component> comp = object->getComponent<Component>(manager->name);

		if (comp) {
			manager->removeComponent(comp);

			//Unsubscribe to prevent leakage.
			if (comp->receiveEvents) {
				eventQueue->removeListener(comp);
			}
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

			//Subscribe to events if needed
			if (comp->receiveEvents) {
				eventQueue->addListener(comp);
			}
		}
	}
}
