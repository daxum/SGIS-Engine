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

class AIComponent : public Component {
public:
	/**
	 * Creates an AIComponent.
	 * @param events Whether to subscribe to the input event handler.
	 */
	AIComponent(bool events = false) : Component(events) {}

	virtual ~AIComponent() {}

	/**
	 * Updates this component using the provided world. It is important to
	 * note that to allow threading, the world should be treated as read-only
	 * at all times, and only ai-specific parts of the parent object should be
	 * modified in this function. Similarly, ai parts of other objects should not
	 * be modified (or even read) here. This will become more explicit later.
	 * @param world The world the parent object is in.
	 */
	virtual void update(Screen* screen) = 0;
};
