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

#include <memory>
#include <string>
#include <unordered_map>

#include "Component.hpp"
#include "ObjectPhysicsInterface.hpp"

//This will probably be expanded later.
struct ObjectState {};

//An object in a world. Stores rendering, physics, etc.
class Object : public std::enable_shared_from_this<Object> {
public:
	/**
	 * Creates an object.
	 */
	Object();

	/**
	 * Retrieves the component with the requested name, or null if it doesn't exist.
	 * @param name The name of the component.
	 * @return A pointer to the component for this object, will be null if the component isn't found.
	 */
	template <typename T>
	std::shared_ptr<T> getComponent(std::string name) {
		if (components.count(name)) {
			return std::static_pointer_cast<T>(components.at(name));
		}
		else {
			return std::shared_ptr<T>();
		}
	}

	/**
	 * Adds a component to the object. This should usually be done before the object is
	 * added to the world. The object will set itself as the component's parent.
	 * @param component The component to add.
	 */
	void addComponent(std::shared_ptr<Component> component);

	/**
	 * Returns the physics interface for this object. Primarily used by RenderComponents.
	 * Should never be null.
	 * @return The object's physics interface.
	 */
	ObjectPhysicsInterface* getPhysics();

	/**
	 * Returns whether the object has a set physics interface.
	 * @return true if the object has a physics interface, false otherwise.
	 */
	bool hasPhysics();

	/**
	 * Sets the physics interface for the object. This should never really need to be
	 * called outside of the engine except for in rare circumstances.
	 * The set interface will not be deleted by the object.
	 * @param phys The new physics interface for the object.
	 */
	void setPhysics(ObjectPhysicsInterface* phys);

	/**
	 * Sets the state for this object.
	 * @param newState The new state variable, user defined. Can be anything.
	 */
	void setState(std::shared_ptr<ObjectState> newState) { state = newState; }

	/**
	 * Returns the previously set state pointer, or null if none was set.
	 */
	std::shared_ptr<ObjectState> getState() { return state; }

private:
	//Used by objects with no set physics interface. Completely stateless.
	static ObjectPhysicsInterface defaultInterface;

	//The map of components for this object. Component names should be in Component.hpp.
	std::unordered_map<std::string, std::shared_ptr<Component>> components;

	//The physics interface for the object.
	ObjectPhysicsInterface* physics;

	//User-defined object state.
	std::shared_ptr<ObjectState> state;
};
