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
#include <vector>

#include <tbb/concurrent_queue.h>

#include "Object.hpp"
#include "Input/InputMap.hpp"
#include "Components/ComponentManager.hpp"
#include "Events/EventQueue.hpp"

class DisplayEngine;
class RenderManager;
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
	std::shared_ptr<RenderManager> getRenderData() { return renderManager; }
	std::shared_ptr<const RenderManager> getRenderData() const { return renderManager; }

	/**
	 * Constructs and adds a component manager to the screen.
	 * @param args The arguments to the manager's constructor.
	 */
	template<typename T, class... Args>
	void addComponentManager(Args&&... args) {
		addComponentManager(std::make_shared<T>(std::forward<Args>(args)...));
	}

	/**
	 * Creates and adds the given manager to the list of managers for this screen. Some things to note:
	 *  - Only one manager for each type should exist. Duplicates might work, but won't do anything useful.
	 *  - Managers will be updated in the order they are added.
	 *  - Objects that existed before the addition of the manager will not (currently) be added to it.
	 *  - Render component managers will be automatically added to the screen's render data. Adding more render managers will
	 *      overwrite old ones, and the old ones will become functionally useless.
	 * @param manager The manager to add.
	 */
	template<typename T>
	void addComponentManager(std::shared_ptr<T> manager);

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
	template<typename T = Camera> std::shared_ptr<T> getCamera();
	template<typename T = Camera> std::shared_ptr<const T> getCamera() const;

	/**
	 * Sets the camera for this screen.
	 */
	void setCamera(std::shared_ptr<Camera> newCamera);

	template<typename T, class... Args>
	void setState(Args&&... args) {
		setState(std::make_shared<T>(std::forward<Args>(args)...));
	}
	/**
	 * Sets the state for the screen.
	 * @param newState The new screen state.
	 */
	template<typename T>
	void setState(std::shared_ptr<T> newState) {
		static_assert(std::is_base_of<ScreenState, T>::value, "Screen::setState called with incorrect type!");
		state = newState;
	}

	/**
	 * Returns the previously set state for the screen, or null if none
	 * was set.
	 * @return The state for the screen.
	 */
	template<typename T = ScreenState> std::shared_ptr<T> getState() { return std::static_pointer_cast<T>(state); }
	template<typename T = ScreenState> std::shared_ptr<const T> getState() const { return std::static_pointer_cast<T>(state); }

	/**
	 * Pauses / unpauses the screen.
	 */
	void setPaused(bool p) { paused = p; }

	/**
	 * Whether the mouse should be hidden when this screen has focus.
	 */
	bool mouseHidden() { return hideMouse; }

	/**
	 * Retreives the input map for this screen.
	 * @return The input map, for getting the mouse/keyboard/etc. state.
	 */
	std::shared_ptr<InputMap> getInputMap() { return inputMap; }

	/**
	 * Gets the event queue for this screen, either for sending events or
	 * for subscribing it to another queue.
	 * @return The event queue for this screen.
	 */
	std::shared_ptr<EventQueue> getEventQueue() { return eventQueue; }

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
	template<typename T = ComponentManager> std::shared_ptr<T> getManager(const std::string& name);
	template<typename T = ComponentManager> std::shared_ptr<const T> getManager(const std::string& name) const;

protected:
	//The display engine that manages this screen.
	DisplayEngine& display;
	//Handles the input for this screen.
	std::shared_ptr<InputMap> inputMap;
	//Handles events passed into the screen.
	std::shared_ptr<EventQueue> eventQueue;
	//The rendering manager for this screen.
	std::shared_ptr<RenderManager> renderManager;
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

template<typename T>
void Screen::addComponentManager(std::shared_ptr<T> manager) {
	static_assert(std::is_base_of<ComponentManager, T>::value, "Attempt to add component manager which isn't a ComponentManager!");

	//Rendering managers are set as the screens render data.
	if (std::is_same<RenderManager, T>::value) {
		//You would think that this would work without the double cast, due to the if statement, but no.
		renderManager = std::static_pointer_cast<RenderManager>(std::static_pointer_cast<void>(manager));
	}

	//Subscribe manager to events if needed. It never needs to be unsubscribed, because it can't
	//be removed and managers have the same lifetime as the input handler.
	if (manager->receiveEvents) {
		eventQueue->addListener(manager);
	}

	managers.push_back(manager);
	manager->setScreen(this);
}

template<typename T>
std::shared_ptr<T> Screen::getCamera() {
	static_assert(std::is_base_of<Camera, T>::value, "Attempt to get non-camera camera!");
	return std::static_pointer_cast<T>(camera);
}

template<typename T>
std::shared_ptr<const T> Screen::getCamera() const {
	static_assert(std::is_base_of<Camera, T>::value, "Attempt to get non-camera camera!");
	return std::static_pointer_cast<const T>(camera);
}

template<typename T>
std::shared_ptr<T> Screen::getManager(const std::string& name) {
	static_assert(std::is_base_of<ComponentManager, T>::value, "Attempt to get component manager which isn't a manager!");

	for (std::shared_ptr<ComponentManager> compM : managers) {
		if (compM->name == name) {
			return std::static_pointer_cast<T>(compM);
		}
	}

	return std::shared_ptr<T>();
}

template<typename T>
std::shared_ptr<const T> Screen::getManager(const std::string& name) const {
	static_assert(std::is_base_of<ComponentManager, T>::value, "Attempt to get component manager which isn't a manager!");

	for (std::shared_ptr<ComponentManager> compM : managers) {
		if (compM->name == name) {
			return std::static_pointer_cast<T>(compM);
		}
	}

	return std::shared_ptr<const T>();
}
