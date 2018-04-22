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

#include "GuiComponentManager.hpp"
#include "GuiComponent.hpp"

bool GuiComponentManager::onEvent(const InputHandler* handler, const std::shared_ptr<InputEvent> event) {
	//Mouse stuff is really annoying, so for now everything is done with keypresses.

	if (event->type == EventType::KEY) {
		std::shared_ptr<KeyEvent> keyEvent = std::static_pointer_cast<KeyEvent>(event);

		for (std::shared_ptr<Component> comp : components) {
			std::shared_ptr<GuiComponent> element = std::static_pointer_cast<GuiComponent>(comp);

			if (element->onKeyPress(parent, keyEvent->key, keyEvent->action)) {
				return true;
			}
		}
	}

	return false;
}
