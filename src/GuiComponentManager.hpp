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

#include "ComponentManager.hpp"

class GuiComponentManager : public ComponentManager {
public:
	/**
	 * Constructor.
	 * @param s The screen that owns this component manager.
	 */
	GuiComponentManager(std::shared_ptr<Screen> s) : ComponentManager(GUI_COMPONENT_NAME, true), parent(s) {}

	/**
	 * Does nothing at the moment, might change later to allow gui components to update themselves.
	 * @param screen The parent screen of this gui.
	 */
	void update(Screen* screen) {}

	/**
	 * See InputListener.hpp.
	 */
	bool onEvent(const InputHandler* handler, const std::shared_ptr<InputEvent> event);

private:
	//The screen that owns this manager.
	std::shared_ptr<Screen> parent;
};
