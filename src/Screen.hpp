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

#include <unordered_set>

#include <tbb/concurrent_queue.h>

#include "KeyList.hpp"
#include "Object.hpp"
#include "ModelManager.hpp"
#include "InputHandler.hpp"

class DisplayEngine;
class ComponentManager;
class RenderComponentManager;
class Camera;

//State for a screen.
struct ScreenState {
	virtual ~ScreenState() {}

	/**
	 * Called whenever a shader uses a uniform provider type of SCREEN_STATE.
	 * Gets a pointer to the value to be passed into the shader.
	 * @param name The name of the value to retrieve.
	 * @return A pointer to the requested value, nullptr if the value doesn't
	 *     exist.
	 */
	virtual const void* getRenderValue(const std::string& name) const = 0;
};

class Screen {
public:
	/**
	 * Constructor
	 */
	Screen(DisplayEngine& display, bool hideMouse);

	/**
	 * Updates all the component managers from first added to last.
	 */
	void update();

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
	 * This function is threadsafe.
	 * @param object The object to add.
	 */
	void addObject(std::shared_ptr<Object> object);

	/**
	 * Queues an object for removal at the end of the tick.
	 * This function is threadsafe.
	 * @param object The object to remove.
	 */
	void removeObject(std::shared_ptr<Object> object);

	/**
	 * Returns the camera associated with this screen.
	 */
	std::shared_ptr<Camera> getCamera() { return camera; }

	/**
	 * Sets the camera for this screen.
	 */
	void setCamera(std::shared_ptr<Camera> newCamera);

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
	 * Retreives the input handler for this screen.
	 * @return The input handler, for getting the mouse/keyboard/etc. state
	 *     and subscribing/removing input listeners.
	 */
	InputHandler& getInputHandler() { return inputHandler; }

	/**
	 * Returns the display engine for this screen, for modifying the screen stack.
	 * Be careful about popping multiple times per update!
	 */
	DisplayEngine& getDisplay() { return display; }

	/**
	 * Gets the component manager with the given name if it is present,
	 * or null otherwise.
	 * @param name The name of the component manager.
	 */
	std::shared_ptr<ComponentManager> getManager(std::string name);

protected:
	//The display engine that manages this screen.
	DisplayEngine& display;

	//Handles the input for this screen.
	InputHandler inputHandler;

	//The rendering manager for this screen.
	std::shared_ptr<RenderComponentManager> renderManager;

	//Just the camera
	std::shared_ptr<Camera> camera;

	//The various managers for the components in this screen.
	std::vector<std::shared_ptr<ComponentManager>> managers;

	//All objects that have been added to the screen.
	std::unordered_set<std::shared_ptr<Object>> objects;

	//Objects to be removed at the end of the update.
	tbb::concurrent_queue<std::shared_ptr<Object>> removalList;

	//Objects to be added after an update.
	tbb::concurrent_queue<std::shared_ptr<Object>> additionList;

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
