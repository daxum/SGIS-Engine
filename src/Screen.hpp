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

#include "KeyList.hpp"
#include "ScreenRenderData.hpp"
#include "Object.hpp"
#include "ModelManager.hpp"

class DisplayEngine;
class ComponentManager;

class Screen {
public:
	/**
	 * Constructor
	 */
	Screen(DisplayEngine& display) : display(display) {}

	/**
	 * Updates all the component managers from first added to last.
	 */
	void update();

	/**
	 * Called whenever a key is pressed. Useless right now, but will be updated for an event system later.
	 * @param key The key that was pressed
	 * @return Whether to stop passing the event to screens below this one.
	 */
	bool onKeyPressed(Key key) { return false; }

	/**
	 * Returns whether the given key is pressed. Will most likely be
	 * expanded into an event system later so that ai and similar can tell when
	 * a key is first pressed without constantly polling everything.
	 * @param key The key to query.
	 * @return Whether the key is pressed.
	 */
	bool isKeyPressed(Key key);

	/**
	 * Gets all the information required to render the screen (models for objects, post-processing steps, etc).
	 * @return Rendering data that can be used by RenderingEngine to produce a picture.
	 */
	ScreenRenderData& getRenderData() { return renderData; }

	/**
	 * Adds the given manager to the list of managers for this screen. Some things to note:
	 *  - Only one manager for each type should exist. Duplicates might work, but won't do anything useful.
	 *  - Managers will be updated in the order they are added.
	 *  - Objects that existed before the addition of the manager will not (currently) be added to it.
	 *  - Render components will be automatically added to the screen's render data. Adding more render components will
	 *      overwrite old ones, and the old ones will become functionally useless.
	 * @param manager The manager to add.
	 */
	void addComponentManager(std::shared_ptr<ComponentManager> manager);

	/**
	 * Queues an object and its components to be added to the screen.
	 * @param object The object to add.
	 */
	void addObject(std::shared_ptr<Object> object);

	/**
	 * Queues an object for removal at the end of the tick.
	 * @param object The object to remove.
	 */
	void removeObject(std::shared_ptr<Object> object);

	/**
	 * Retrieves the model manager from the display
	 */
	ModelManager& getModelManager();

protected:
	//The display engine that manages this screen.
	DisplayEngine& display;

	//Information on how to render this screen.
	ScreenRenderData renderData;

	//The various managers for the components in this screen.
	std::vector<std::shared_ptr<ComponentManager>> managers;

	//All objects that have been added to the screen.
	std::vector<std::shared_ptr<Object>> objects;

	//Objects to be removed at the end of the update.
	std::vector<std::shared_ptr<Object>> removalList;

	//Objects to be added after an update.
	std::vector<std::shared_ptr<Object>> additionList;

	//The indices of the objects in the object vector, for fast removal.
	std::unordered_map<std::shared_ptr<Object>, size_t> objectIndices;

	/**
	 * Deletes the provided object from this screen.
	 */
	void deleteObject(std::shared_ptr<Object> object);

	/**
	 * Adds the provided object to the object list.
	 */
	void addObjectToList(std::shared_ptr<Object> object);
};
