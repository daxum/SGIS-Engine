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
	 * Required from component.
	 * @return The component's name.
	 */
	static const std::string getName() { return GUI_COMPONENT_NAME; }

	/**
	 * Creates a guicomponent for the given object.
	 * @param position The position of this component, in world coordinates. Won't be used if the parent has a physics component.
	 */
	GuiComponent(glm::vec3 position = glm::vec3(0.0, 0.0, 0.0)) : pos(position) {}

	/**
	 * Sets this component as the physics provider if there isn't one already.
	 */
	void onParentSet() override {
		if (!lockParent()->hasPhysics()) {
			lockParent()->setPhysics(this);
		}
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
	virtual bool onKeyPress(Screen* screen, Key key, KeyAction action) { return false; }

	/**
	 * Called when the mouse is clicked when hovering over this component. The object must have a physics component for
	 * this to be called, due to the required ray tracing.
	 * @param screen The parent screen.
	 * @param button The mouse button that was clicked.
	 * @param action Whether the click was a press or release.
	 */
	virtual void onMouseClick(Screen* screen, MouseButton button, MouseAction action) {}

	/**
	 * Called when the scroll wheel is scrolled.
	 * @param screen The parent screen.
	 * @param x The x offset of the wheel, usually 0.
	 * @param y The y offset of the wheel.
	 */
	virtual void onMouseScroll(Screen* screen, float x, float y) {}

	/**
	 * Called when the mouse moves over the component.
	 * @param screen The parent screen.
	 */
	virtual void onHoverStart(Screen* screen) {}

	/**
	 * Called when the mouse moves off of the component.
	 * @param screen The parent screen.
	 */
	virtual void onHoverStop(Screen* screen) {}

	/**
	 * Gets the translation of the object.
	 * @return The translation of the object.
	 */
	glm::vec3 getTranslation() const override { return pos; }

protected:
	//The position of the object.
	glm::vec3 pos;
};
