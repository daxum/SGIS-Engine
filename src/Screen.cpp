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
#include "DefaultCamera.hpp"

Screen::Screen(DisplayEngine& display, bool hideMouse) :
	display(display),
	camera(std::make_shared<DefaultCamera>()),
	paused(false),
	hideMouse(hideMouse) {

	camera->setProjection();
}
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
		manager->update();
	}

	//Update camera
	camera->update();

	//Remove queued objects
	for (std::shared_ptr<Object> object : removalList) {
		deleteObject(object);
	}

	removalList.clear();
}

void Screen::addComponentManager(std::shared_ptr<ComponentManager> manager) {
	//Rendering managers are set as the screens render data
	if (manager->name == RENDER_COMPONENT_NAME) {
		renderManager = std::static_pointer_cast<RenderComponentManager>(manager);
	}

	//Subscribe manager to events if needed. It never needs to be unsubscribed, because it can't
	//be removed and managers have the same lifetime as the input handler.
	if (manager->receiveEvents) {
		inputHandler.addListener(manager);
	}

	managers.push_back(manager);
	manager->setScreen(this);
}

void Screen::addObject(std::shared_ptr<Object> object) {
	additionList.push_back(object);
}

void Screen::removeObject(std::shared_ptr<Object> object) {
	removalList.push_back(object);
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
				inputHandler.removeListener(comp);
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
				inputHandler.addListener(comp);
			}
		}
	}
}

std::shared_ptr<ComponentManager> Screen::getManager(std::string name) {
	for (std::shared_ptr<ComponentManager> compM : managers) {
		if (compM->name == name) {
			return compM;
		}
	}

	return std::shared_ptr<ComponentManager>();
}
