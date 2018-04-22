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

#include "InputListener.hpp"

class Object;

//The below strings are the names of the engine-provided components.
const std::string RENDER_COMPONENT_NAME = "render";
const std::string AI_COMPONENT_NAME = "ai";
const std::string PHYSICS_COMPONENT_NAME = "physics";
const std::string UPDATE_COMPONENT_NAME = "update";
const std::string GUI_COMPONENT_NAME = "gui";

//A "piece" of an object. Used to implement rendering, physics, and other stuff.
class Component : public InputListener {
public:
	const std::string name;
	const bool receiveEvents;

	/**
	 * Creates a component with the provided parent object
	 * @param parent The owner of the component
	 * @param name The name of the component, from the list above or user-defined values.
	 *     Used to add the component to managers.
	 * @param events Whether to subscribe the component to the input event handler (for key presses and such).
	 *     IMPORTANT: If there isn't a component manager for the component's name when it is added to a screen,
	 *     it WILL NOT be subscribed to any events.
	 */
	Component(Object& parent, std::string name, bool events = false) : name(name), receiveEvents(events), parent(parent) {}

	virtual ~Component() {}

	/**
	 * See InputListener.hpp.
	 */
	virtual bool onEvent(const InputHandler* handler, const std::shared_ptr<InputEvent> event) { return false; }

protected:
	//The parent object.
	Object& parent;
};
