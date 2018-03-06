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
#include <unordered_map>
#include <vector>

#include "Component.hpp"
#include "Screen.hpp"

class ComponentManager {
public:
	//The name of the components this manager manages (AIComponents would have name AI_COMPONENT_NAME, for example)
	const std::string name;

	/**
	 * Creates a manager for components with the given name. Name should
	 * be unique for all managers in the same screen.
	 * @param name The name of this ComponentManager.
	 */
	ComponentManager(std::string name) : name(name) {}

	/**
	 * Virtual destructor
	 */
	virtual ~ComponentManager() {}

	/**
	 * Adds a component to this component manager.
	 * @param comp The component to add.
	 */
	void addComponent(std::shared_ptr<Component> comp);

	/**
	 * Removes a component from this component manager.
	 * @param comp The component to remove.
	 */
	void removeComponent(std::shared_ptr<Component> comp);

	/**
	 * Updates all components managed by this component manager in no specific order.
	 * @param screen The screen that owns this component manager, used to get input and such.
	 */
	virtual void update(Screen* screen) = 0;

protected:
	//Indexes into the component vector for fast removal.
	std::unordered_map<std::shared_ptr<Component>, size_t> entryIndices;

	//Stores all the components managed by this manager.
	std::vector<std::shared_ptr<Component>> components;
};
