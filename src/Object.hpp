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

//An object in a world. Stores rendering, physics, etc.
class Object {
public:
	/**
	 * Creates an object.
	 */
	Object() {}

	/**
	 * Retrieves the component with the requested name, or null if it doesn't exist.
	 * @param name The name of the component.
	 * @return A pointer to the component for this object, will be null if the component isn't found.
	 */
	std::shared_ptr<Component> getComponent(std::string name) {
		return components[name];
	}

	/**
	 * Adds a component to the object. This should usually be done before the object is
	 * added to the world.
	 * @param component The component to add.
	 */
	void addComponent(std::shared_ptr<Component> component) {
		components[component->name] = component;
	}

	/**
	 * Sets the name in this object's state map to the provided value.
	 * @param name The name to store the value under.
	 * @param value The value to store.
	 */
	template<typename T>
	void setState(std::string name, std::shared_ptr<T> value) {
		state[name] = value;
	}

	/**
	 * Ensures that the state with the given name is present.
	 * If it is not, creates it with the given value.
	 * @param name The name to check.
	 * @param defaultValue The value to set it to if it's missing.
	 */
	template<typename T>
	void ensureState(std::string name, std::shared_ptr<T> defaultValue) {
		if (!state[name]) {
			state[name] = defaultValue;
		}
	}

	/**
	 * Retrieves the value in this object's state map for the provided name.
	 * @param name The name to retrieve.
	 * @return The value for the given name, will be null if the name isn't present.
	 */
	template<typename T>
	std::shared_ptr<T> getState(std::string name) {
		return std::static_pointer_cast<T>(state[name]);
	}

private:
	//The map of components for this object. Component names should be in Component.hpp.
	std::unordered_map<std::string, std::shared_ptr<Component>> components;

	//At this point we're just reimplementing subclasses. Oh well.
	//Stores information used by the various components for the object.
	std::unordered_map<std::string, std::shared_ptr<void>> state;
};
