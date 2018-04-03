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
#include "Object.hpp"
#include "ModelManager.hpp"
#include "SequentialSet.hpp"
#include "DefaultCamera.hpp"

class DisplayEngine;
class ComponentManager;
class RenderComponentManager;

//Might be expanded later.
struct ScreenState {};

class Screen {
public:
	/**
	 * Constructor
	 */
	Screen(DisplayEngine& display, bool hideMouse) : display(display), camera(std::make_shared<DefaultCamera>()), paused(false), hideMouse(hideMouse) {}

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
	std::shared_ptr<RenderComponentManager> getRenderData() { return renderManager; }

	/**
	 * Adds the given manager to the list of managers for this screen. Some things to note:
	 *  - Only one manager for each type should exist. Duplicates might work, but won't do anything useful.
	 *  - Managers will be updated in the order they are added.
	 *  - Objects that existed before the addition of the manager will not (currently) be added to it.
	 *  - Render component managers will be automatically added to the screen's render data. Adding more render managers will
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
	 * Same as above, but with regular pointers instead of shared,
	 * for when an object needs to be removed but you don't have
	 * access to the shared pointer.
	 * @param object The object to remove.
	 */
	void removeObject(Object* object);

	/**
	 * Returns the camera associated with this screen.
	 */
	std::shared_ptr<Camera> getCamera() { return camera; }

	/**
	 * Sets the camera for this screen.
	 */
	void setCamera(std::shared_ptr<Camera> newCamera) { camera = newCamera; }

	/**
	 * Sets the state for the screen.
	 * @param newState The new screen state.
	 */
	void setState(std::shared_ptr<ScreenState> newState) { state = newState; }

	/**
	 * Returns the previously set state for the screen, or null if none
	 * was set.
	 * @return The state for the screen.
	 */
	std::shared_ptr<ScreenState> getState() { return state; }

	/**
	 * Pauses / unpauses the screen.
	 */
	void setPaused(bool p) { paused = p; }

	/**
	 * Whether the mouse should be hidden when this screen has focus.
	 */
	bool mouseHidden() { return hideMouse; }

	/**
	 * Returns the mouse position.
	 */
	glm::vec2 getMousePos();

	/**
	 * Returns how far the mouse moved in the last tick.
	 */
	glm::vec2 getMouseDist();

protected:
	//The display engine that manages this screen.
	DisplayEngine& display;

	//The rendering manager for this screen.
	std::shared_ptr<RenderComponentManager> renderManager;

	//Just the camera
	std::shared_ptr<Camera> camera;

	//The various managers for the components in this screen.
	std::vector<std::shared_ptr<ComponentManager>> managers;

	//All objects that have been added to the screen.
	SequentialSet<Object> objects;

	//Objects to be removed at the end of the update.
	std::vector<std::shared_ptr<Object>> removalList;

	//Objects to be added after an update.
	std::vector<std::shared_ptr<Object>> additionList;

	//User-defined state for the screen.
	std::shared_ptr<ScreenState> state;

	//Whether the screen has been paused (all updates stopped, only rendering).
	bool paused;

	//Whether to hide the mouse when this screen has focus.
	bool hideMouse;

	/**
	 * Deletes the provided object from this screen.
	 */
	void deleteObject(std::shared_ptr<Object> object);

	/**
	 * Adds the provided object to the object list.
	 */
	void addObjectToList(std::shared_ptr<Object> object);
};
