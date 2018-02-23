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

#include "World.hpp"
#include "RenderComponent.hpp"
#include "AIComponent.hpp"

World::World(DisplayEngine& display) :
	Screen(display) {

}

void World::update() {
	for (std::shared_ptr<AIComponent> ai : aiObjects) {
		ai->update(this);
	}

	//"Physics engine"
	for (std::shared_ptr<Object> object : objects) {
		object->pos += object->velocity;
		object->velocity *= 0.98f;
	}
}

void World::addObject(std::shared_ptr<Object> object) {
	objects.push_back(object);

	//Temporary fix, need component system for world too.
	std::shared_ptr<RenderComponent> objectRender = object->getComponent<RenderComponent>(RENDER_COMPONENT_NAME);
	std::shared_ptr<AIComponent> objectAI = object->getComponent<AIComponent>(AI_COMPONENT_NAME);

	if (objectRender) {
		renderData.addObject(objectRender);
	}

	if (objectAI) {
		aiObjects.push_back(objectAI);
	}
}

void World::removeObject(std::shared_ptr<Object> object) {
	for (size_t i = 0; i < objects.size(); i++) {
		if (object == objects[i]) {
			objects.erase(objects.begin() + i);

			std::shared_ptr<RenderComponent> objectRender = object->getComponent<RenderComponent>(RENDER_COMPONENT_NAME);
			std::shared_ptr<AIComponent> objectAI = object->getComponent<AIComponent>(AI_COMPONENT_NAME);

			if (objectRender) {
				renderData.removeObject(objectRender);
			}

			if (objectAI) {
				for (size_t j = 0; j < aiObjects.size(); j++) {
					if (objectAI == aiObjects[j]) {
						aiObjects.erase(aiObjects.begin() + j);
						break;
					}
				}
			}

			return;
		}
	}
}

void World::setMap(std::shared_ptr<Map> newMap) {
	//Out with the old...
	if (map) {
		renderData.removeObject(map->getRenderData());
	}

	//And in with the new!
	if (newMap) {
		renderData.addObject(newMap->getRenderData());
	}

	map = newMap;
}
