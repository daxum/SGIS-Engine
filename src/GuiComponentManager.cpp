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
#include "Engine.hpp"
#include "PhysicsComponentManager.hpp"
#include "ExtraMath.hpp"
#include "Camera.hpp"

bool GuiComponentManager::onEvent(const InputHandler* handler, const std::shared_ptr<const InputEvent> event) {
	if (event->type == EventType::KEY) {
		std::shared_ptr<const KeyEvent> keyEvent = std::static_pointer_cast<const KeyEvent>(event);

		for (std::shared_ptr<Component> comp : components) {
			std::shared_ptr<GuiComponent> element = std::static_pointer_cast<GuiComponent>(comp);

			if (element->onKeyPress(screen, keyEvent->key, keyEvent->action)) {
				return true;
			}
		}
	}
	//Mouse click events require raytracing, and therefore a physics component manager.
	else if (screen->getManager(PHYSICS_COMPONENT_NAME)) {
		if (event->type == EventType::MOUSE_CLICK) {
			return handleMouseClick(handler, std::static_pointer_cast<const MouseClickEvent>(event));
		}
		else if (event->type == EventType::MOUSE_MOVE) {
			return handleMouseMove(std::static_pointer_cast<const MouseMoveEvent>(event));
		}
	}

	return false;
}

bool GuiComponentManager::handleMouseClick(const InputHandler* handler, const std::shared_ptr<const MouseClickEvent> event) {
	std::shared_ptr<GuiComponent> element = getUnderMouse(handler->getMousePos());

	if (element) {
		element->onMouseClick(screen, event->button, event->action);
		return true;
	}

	return false;
}

bool GuiComponentManager::handleMouseMove(const std::shared_ptr<const MouseMoveEvent> event) {
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

	//Return whether the mouse was over anything.
	return !element;
}

std::shared_ptr<GuiComponent> GuiComponentManager::getUnderMouse(const glm::vec2& mousePos) {
	const std::shared_ptr<const RenderingEngine> renderer = Engine::instance->getRenderer();

	glm::mat4 projection = screen->getCamera()->getProjection();
	glm::mat4 view = screen->getCamera()->getView();
	float width = renderer->getWindowWidth();
	float height = renderer->getWindowHeight();

	const auto nearFarPlanes = screen->getCamera()->getNearFar();
	float near = nearFarPlanes.first;
	float far = nearFarPlanes.second;

	//Position of mouse on near and far plane.
	std::pair<glm::vec3, glm::vec3> nearFar = ExMath::screenToWorld(mousePos, projection, view, width, height, near, far);

	PhysicsComponent* hit = std::static_pointer_cast<PhysicsComponentManager>(screen->getManager(PHYSICS_COMPONENT_NAME))->raytraceSingle(nearFar.first, nearFar.second);

	if (hit != nullptr) {
		return hit->getParent()->getComponent<GuiComponent>(GUI_COMPONENT_NAME);
	}

	return std::shared_ptr<GuiComponent>();
}

void GuiComponentManager::onComponentRemove(std::shared_ptr<Component> comp) {
	//Cast shouldn't be neccessary, but just for completeness.
	if (comp == std::static_pointer_cast<Component>(currentHovered)) {
		currentHovered = std::shared_ptr<GuiComponent>();
	}
}
