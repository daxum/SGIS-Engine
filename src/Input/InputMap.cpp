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

#include <memory>

#include "Engine.hpp"
#include "InputMap.hpp"
#include "InputEvent.hpp"
#include "InputMapSyncEvent.hpp"

InputMap::InputMap() :
	mousePos(0.0, 0.0) {

	for (uint64_t key = 0; key < Key::NUM_KEYS; key++) {
		keyMap.at(key) = false;
	}
}

bool InputMap::onEvent(const std::shared_ptr<const Event> event) {
	switch(event->type) {
		case KeyEvent::EVENT_TYPE: {
			const std::shared_ptr<const KeyEvent> keyEvent = std::static_pointer_cast<const KeyEvent>(event);

			//Ignore repeat here
			if (keyEvent->action == KeyAction::PRESS) {
				keyMap.at(keyEvent->key) = true;
			}
			else if (keyEvent->action == KeyAction::RELEASE) {
				keyMap.at(keyEvent->key) = false;
			}
		}; break;
		case MouseMoveEvent::EVENT_TYPE: {
			const std::shared_ptr<const MouseMoveEvent> mouseEvent = std::static_pointer_cast<const MouseMoveEvent>(event);

			glm::vec2 newPos(mouseEvent->x, mouseEvent->y);
			mousePos = newPos;
		}; break;
		case InputMapSyncEvent::EVENT_TYPE: {
			syncInputState();
		}; break;

		default: break;
	}

	return false;
}

void InputMap::syncInputState() {
	const WindowSystemInterface& window = Engine::instance->getWindowInterface();

	mousePos = window.queryMousePos();

	for (uint64_t i = 0; i < Key::NUM_KEYS; i++) {
		keyMap.at(i) = window.queryKey((Key::KeyEnum)i) == KeyAction::PRESS;
	}
}
