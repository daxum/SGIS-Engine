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

#include "Component.hpp"
#include "Screen.hpp"
#include "SequentialSet.hpp"
#include "InputListener.hpp"

class ComponentManager : public InputListener {
public:
	//The name of the components this manager manages (AIComponents would have name AI_COMPONENT_NAME, for example)
	const std::string name;
	const bool receiveEvents;

	/**
	 * Creates a manager for components with the given name. Name should
	 * be unique for all managers in the same screen.
	 * @param name The name of this ComponentManager.
	 * @param events Whether to subscribe the component manager to input events.
	 */
	ComponentManager(std::string name, bool events = false) : name(name), receiveEvents(events) {}

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

	/**
	 * See InputListener.hpp.
	 */
	virtual bool onEvent(const InputHandler* handler, const std::shared_ptr<const InputEvent> event) { return false; }

protected:
	//Stores all the components added to this manager.
	SequentialSet<Component> components;

	/**
	 * Called immediately after a component is added to the manager's
	 * internal list.
	 * @param comp The component that was added.
	 */
	virtual void onComponentAdd(std::shared_ptr<Component> comp) {}

	/**
	 * Called immediately after a component is removed from the manager's
	 * internal list.
	 * @param comp The component that was removed.
	 */
	virtual void onComponentRemove(std::shared_ptr<Component> comp) {}
};
