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

#include <unordered_set>

#include "InputHandler.hpp"

void InputHandler::addListener(std::shared_ptr<InputListener> listener) {
	listeners.insert(listener);
}

void InputHandler::removeListener(std::shared_ptr<InputListener> listener) {
	listeners.erase(listener);
}

void InputHandler::update(std::vector<std::shared_ptr<InputEvent>>& events) {
	std::unordered_set<std::shared_ptr<InputEvent>> toRemove;
	mouseDistance = glm::vec2(0.0, 0.0);

	//Handle all the events.
	for (const std::shared_ptr<InputEvent> event : events) {
		//Key release events don't get removed from the event list.
		bool keyReleaseEvent = false;

		//Update keymap / mouse position / etc.
		switch(event->type) {
			case EventType::KEY: {
				const std::shared_ptr<KeyEvent> keyEvent = std::static_pointer_cast<KeyEvent>(event);

				keyReleaseEvent = keyEvent->action == KeyAction::RELEASE;
				keyMap[keyEvent->key] = keyEvent->action == KeyAction::PRESS || keyEvent->action == KeyAction::REPEAT;
				break;
			}

			case EventType::MOUSE_MOVE: {
				const std::shared_ptr<MouseMoveEvent> mouseEvent = std::static_pointer_cast<MouseMoveEvent>(event);

				glm::vec2 newPos(mouseEvent->x, mouseEvent->y);
				mouseDistance = newPos - mousePos;
				mousePos = newPos;
				break;
			}

			default: break;
		}

		//Send event to listeners.
		bool remove = false;

		for (std::shared_ptr<InputListener> listener : listeners) {
			remove = listener->onEvent(this, event) || remove;
		}

		//Mark handled events for removal, except key release events.
		if (remove && !keyReleaseEvent) {
			toRemove.insert(event);
		}
	}

	//Remove any handled events
	for (size_t i = events.size(); i > 0; i--) {
		if (toRemove.count(events[i - 1])) {
			events.erase(events.begin() + i - 1);
		}
	}
}

bool InputHandler::isKeyPressed(Key key) {
	//Theoretically, operator[] default-initializes missing members for maps, but
	//not sure if it can be trusted to consistently initialize booleans to false.
	//(Old MSVC might initialize to true?)
	if (keyMap.count(key) == 0) {
		keyMap[key] = false;
	}

	return keyMap[key];
}

glm::vec2 InputHandler::getMousePos() const {
	return mousePos;
}

glm::vec2 InputHandler::getMouseDist() const {
	return mouseDistance;
}
