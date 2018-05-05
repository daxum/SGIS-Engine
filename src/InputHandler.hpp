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

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "InputListener.hpp"

class InputHandler {
public:
	/**
	 * Adds the given listener to the listener list, so that
	 * it will be notified of events.
	 * @param listener The object to add.
	 */
	void addListener(std::shared_ptr<InputListener> listener);

	/**
	 * Removes the listener from the listener list.
	 * @param listener The listener to remove.
	 */
	void removeListener(std::shared_ptr<InputListener> listener);

	/**
	 * Passes the events in the set to the event listeners in no defined order,
	 * and then removes any handled events, except key release events.
	 * Also updates the mouse position / distance and the key map.
	 * @param events The list of events that happened this tick.
	 */
	void update(std::vector<std::shared_ptr<InputEvent>>& events);

	/**
	 * Checks the key map for whether the given key is pressed.
	 * @param key The key to check state for.
	 * @return Whether the key is pressed.
	 */
	bool isKeyPressed(Key key);

	/**
	 * @return The mouse position.
	 */
	glm::vec2 getMousePos() const;

	/**
	 * @return The distance the mouse moved in the last tick.
	 */
	glm::vec2 getMouseDist() const;

private:
	//List of subscribed listeners
	std::unordered_set<std::shared_ptr<InputListener>> listeners;

	//Stores which keys are currently pressed.
	std::unordered_map<Key, bool> keyMap;

	//The current position of the mouse.
	glm::vec2 mousePos;

	//The amount the mouse has moved since the last tick.
	glm::vec2 mouseDistance;
};
