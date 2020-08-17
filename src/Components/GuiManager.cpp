/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018, 2019
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

#include "GuiManager.hpp"
#include "GuiComponent.hpp"
#include "Engine.hpp"
#include "PhysicsManager.hpp"
#include "ExtraMath.hpp"
#include "Display/Camera.hpp"
#include "Display/ScreenChangeEvent.hpp"

bool GuiManager::onEvent(const std::shared_ptr<const Event> event) {
	if (event->type == KeyEvent::EVENT_TYPE) {
		std::shared_ptr<const KeyEvent> keyEvent = std::static_pointer_cast<const KeyEvent>(event);

		for (std::shared_ptr<Component> comp : components) {
			std::shared_ptr<GuiComponent> element = std::static_pointer_cast<GuiComponent>(comp);

			if (element->onKeyPress(screen, keyEvent->key, keyEvent->action)) {
				return true;
			}
		}
	}
	//Mouse click and position events require raytracing, and therefore a physics component manager
	else if (screen->getManager(PHYSICS_COMPONENT_NAME)) {
		if (event->type == MouseClickEvent::EVENT_TYPE) {
			return handleMouseClick(std::static_pointer_cast<const MouseClickEvent>(event));
		}
		else if (event->type == MouseMoveEvent::EVENT_TYPE) {
			handleMouseMove(std::static_pointer_cast<const MouseMoveEvent>(event));
		}
		else if (event->type == ScreenChangeEvent::EVENT_TYPE) {
			glm::vec2 mousePos = Engine::instance->getWindowInterface().queryMousePos();
			handleMouseMove(std::make_shared<MouseMoveEvent>(mousePos.x, mousePos.y));
		}
	}
	else if (event->type == MouseScrollEvent::EVENT_TYPE) {
		std::shared_ptr<const MouseScrollEvent> scrollEvent = std::static_pointer_cast<const MouseScrollEvent>(event);

		for (std::shared_ptr<Component> comp : components) {
			std::shared_ptr<GuiComponent> element = std::static_pointer_cast<GuiComponent>(comp);

			element->onMouseScroll(screen, scrollEvent->x, scrollEvent->y);
		}
	}
	else {
		//TODO: Subscribe GuiComponents to events directly, and send out
		//additional events from GuiManager
		for (std::shared_ptr<Component> comp : components) {
			if (comp->onEvent(event)) {
				return true;
			}
		}
	}

	return false;
}

bool GuiManager::handleMouseClick(const std::shared_ptr<const MouseClickEvent> event) {
	std::shared_ptr<GuiComponent> element = getUnderMouse(screen->getInputMap()->getMousePos());

	if (element) {
		element->onMouseClick(screen, event->button, event->action);
		return true;
	}

	return false;
}

void GuiManager::handleMouseMove(const std::shared_ptr<const MouseMoveEvent> event) {
	std::shared_ptr<GuiComponent> element = getUnderMouse(glm::vec2(event->x, event->y));

	if (element != currentHovered) {
		if (element) {
			element->onHoverStart(screen);
		}

		if (currentHovered) {
			currentHovered->onHoverStop(screen);
		}

		currentHovered = element;
	}
}

std::shared_ptr<GuiComponent> GuiManager::getUnderMouse(const glm::vec2& mousePos) {
	RaytraceResult hit = std::static_pointer_cast<PhysicsManager>(screen->getManager(PHYSICS_COMPONENT_NAME))->raytraceUnderMouse();

	if (hit.hitComp != nullptr) {
		return hit.hitComp->getParent()->getComponent<GuiComponent>(GUI_COMPONENT_NAME);
	}

	return std::shared_ptr<GuiComponent>();
}

void GuiManager::onComponentRemove(std::shared_ptr<Component> comp) {
	//Cast shouldn't be neccessary, but just for completeness
	if (comp == std::static_pointer_cast<Component>(currentHovered)) {
		currentHovered = std::shared_ptr<GuiComponent>();
	}
}
