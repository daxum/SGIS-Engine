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
	template <typename T>
	std::shared_ptr<T> getComponent(std::string name) {
		return std::static_pointer_cast<T>(components[name]);
	}

	/**
	 * Adds a component to the object. This should usually be done before the object is
	 * added to the world.
	 * @param component The component to add.
	 */
	void addComponent(std::shared_ptr<Component> component) {
		components[component->name] = component;
	}

private:
	//The map of components for this object. Component names should be in Component.hpp.
	std::unordered_map<std::string, std::shared_ptr<Component>> components;
};
