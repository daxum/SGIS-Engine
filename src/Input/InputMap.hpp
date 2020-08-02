/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018, 2020
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

#include <array>

#include <glm/glm.hpp>

#include "Events/EventListener.hpp"
#include "KeyList.hpp"

class InputMap : public EventListener {
public:
	/**
	 * Default constructor.
	 */
	InputMap();

	/**
	 * Updates the key map and mouse parameters based on the provided event.
	 * @param event The event to process.
	 */
	bool onEvent(const std::shared_ptr<const Event> event) override;

	/**
	 * Checks the key map for whether the given key is pressed.
	 * @param key The key to check state for.
	 * @return Whether the key is pressed.
	 */
	bool isKeyPressed(Key::KeyEnum key) const { return keyMap.at(key); }

	/**
	 * @return The mouse position.
	 */
	glm::vec2 getMousePos() const { return mousePos; }

private:
	//Stores which keys are currently pressed.
	std::array<bool, Key::NUM_KEYS> keyMap;

	//The current position of the mouse.
	glm::vec2 mousePos;

	/**
	 * Syncs the input map with the window interface.
	 */
	void syncInputState();
};
