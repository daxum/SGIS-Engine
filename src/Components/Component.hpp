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

#include <string>
#include <memory>

#include "Events/EventListener.hpp"

class Object;

//The below strings are the names of the engine-provided components.
//Every instantiation of component needs a static member function
//called 'getName' in order to be properly added to objects.
const std::string RENDER_COMPONENT_NAME = "rndr";
const std::string AI_COMPONENT_NAME = "ai";
const std::string PHYSICS_COMPONENT_NAME = "phys";
const std::string UPDATE_COMPONENT_NAME = "updt";
const std::string GUI_COMPONENT_NAME = "gui";
const std::string TEXT_COMPONENT_NAME = "txt";
const std::string ANIMATION_COMPONENT_NAME = "anim";

//A "piece" of an object. Used to implement rendering, physics, and other stuff.
class Component : public EventListener {
public:
	const bool receiveEvents;

	/**
	 * Creates a component.
	 * @param events Whether to subscribe the component to the input event handler (for key presses and such).
	 *     IMPORTANT: If there isn't a component manager for the component's name when it is added to a screen,
	 *     it WILL NOT be subscribed to any events.
	 */
	Component(bool events = false) : receiveEvents(events) {}

	virtual ~Component() {}

	/**
	 * Only intended to be called from Object. Bad things may
	 * or may not happen if this is called more than once.
	 * @param The new parent of this component.
	 */
	void setParent(std::shared_ptr<Object> newParent) { parent = newParent; onParentSet(); }

	/**
	 * Called when the parent object is set.
	 */
	virtual void onParentSet() {}

	/**
	 * See InputListener.hpp.
	 */
	virtual bool onEvent(const std::shared_ptr<const Event> event) override { return false; }

protected:
	//The parent object.
	std::weak_ptr<Object> parent;

	/**
	 * Locks the parent pointer for modification.
	 * @return A shared_ptr to the parent, will be null if
	 *     the parent has been destructed or wasn't set.
	 */
	std::shared_ptr<Object> lockParent() { return parent.lock(); }
	std::shared_ptr<const Object> lockParent() const { return parent.lock(); }
};
