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

class GuiComponent;

class GuiComponentManager : public ComponentManager {
public:
	/**
	 * Constructor.
	 */
	GuiComponentManager() : ComponentManager(GUI_COMPONENT_NAME, true) {}

	/**
	 * Does nothing at the moment, might change later to allow gui components to update themselves.
	 * @param screen The parent screen of this gui.
	 */
	void update() {}

	/**
	 * See InputListener.hpp.
	 */
	bool onEvent(const InputHandler* handler, const std::shared_ptr<const InputEvent> event);

private:
	//The component the mouse is currently over.
	std::shared_ptr<GuiComponent> currentHovered;

	/**
	 * Handles a mouse click.
	 */
	bool handleMouseClick(const InputHandler* handler, const std::shared_ptr<const MouseClickEvent> event);

	/**
	 * Handles mouse movement.
	 */
	bool handleMouseMove(const std::shared_ptr<const MouseMoveEvent> event);

	/**
	 * Performs a raytrace for the given mouse position and returns the object hit, if any.
	 * @param mousePos The current position of the mouse.
	 * @return The object the mouse is hovering over, nullptr if not present.
	 */
	std::shared_ptr<GuiComponent> getUnderMouse(const glm::vec2& mousePos);

	/**
	 * Sets the current hovered component to nullptr if neccessary.
	 * @param comp The component that was removed.
	 */
	void onComponentRemove(std::shared_ptr<Component> comp);
};
