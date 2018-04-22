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
#include "ObjectPhysicsInterface.hpp"
#include "Screen.hpp"

class GuiComponent : public Component, ObjectPhysicsInterface {
public:
	/**
	 * Creates a guicomponent for the given object.
	 * @param object The parent of this component.
	 * @param position The position of this component, in world coordinates.
	 */
	GuiComponent(Object& object, glm::vec3 position) : Component(object, GUI_COMPONENT_NAME), pos(position) {
		parent.setPhysics(this);
	}

	/**
	 * Nothing to see here.
	 */
	virtual ~GuiComponent() {}

	/**
	 * Called by the GuiComponentManager when a key is pressed.
	 * @param screen The parent screen.
	 * @param key The key that was pressed.
	 * @param action Whether the action was a press, release, or repeat.
	 * @return Whether the component handled the event.
	 */
	virtual bool onKeyPress(std::shared_ptr<Screen> screen, Key key, KeyAction action) { return false; }

	/**
	 * Gets the translation of the object.
	 * @return The translation of the object.
	 */
	glm::vec3 getTranslation() { return pos; }

protected:
	//The position of the object.
	glm::vec3 pos;
};
