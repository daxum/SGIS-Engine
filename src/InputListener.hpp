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

#include "InputEvent.hpp"

class InputListener {
public:
	/**
	 * Called from the event handler when an event happens.
	 * @param event The event.
	 * @return Whether the event should be removed from the event queue.
	 *     Will not prevent it from being sent to other listeners on the
	 *     same screen, but will prevent it from being sent to screens
	 *     below the current one.
	 */
	virtual bool onEvent(const std::shared_ptr<InputEvent> event) = 0;
};
