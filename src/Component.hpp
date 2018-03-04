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

class Object;

//The below strings are the names of the engine-provided components.
const std::string RENDER_COMPONENT_NAME = "render";
const std::string AI_COMPONENT_NAME = "ai";
const std::string PHYSICS_COMPONENT_NAME = "physics";
const std::string UPDATE_COMPONENT_NAME = "update";

//A "piece" of an object. Used to implement rendering, physics, and other stuff.
class Component {
public:
	const std::string name;

	/**
	 * Creates a component with the provided parent object
	 * @param parent The owner of the component
	 */
	Component(Object& parent, std::string name) : name(name), parent(parent) {}

	virtual ~Component() {}

protected:
	//The parent object.
	Object& parent;
};
