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

bool GuiComponentManager::onEvent(const InputHandler* handler, const std::shared_ptr<InputEvent> event) {
	if (event->type == EventType::KEY) {
		std::shared_ptr<KeyEvent> keyEvent = std::static_pointer_cast<KeyEvent>(event);

		for (std::shared_ptr<Component> comp : components) {
			std::shared_ptr<GuiComponent> element = std::static_pointer_cast<GuiComponent>(comp);

			if (element->onKeyPress(parent, keyEvent->key, keyEvent->action)) {
				return true;
			}
		}
	}
	//Mouse click events require raytracing, and therefore a physics component manager.
	else if (event->type == EventType::MOUSE_CLICK && parent->getManager(PHYSICS_COMPONENT_NAME)) {
		return handleMouseClick(handler, std::static_pointer_cast<MouseClickEvent>(event));
	}

	return false;
}

bool GuiComponentManager::handleMouseClick(const InputHandler* handler, const std::shared_ptr<MouseClickEvent> event) {
	const std::shared_ptr<const RenderingEngine> renderer = Engine::instance->getRenderer();
	const RenderConfig renderConf = Engine::instance->getConfig().renderer;

	glm::vec2 mousePos = handler->getMousePos();
	glm::mat4 viewI = glm::inverse(parent->getCamera()->getView());
	glm::mat4 projI = glm::inverse(renderer->getProjection());

	mousePos.x = (mousePos.x / renderer->getWindowWidth() - 0.5f) * 2.0f;
	mousePos.y = -(mousePos.y / renderer->getWindowHeight() - 0.5f) * 2.0f;

	float wNear = renderConf.nearPlane;
	float wFar = renderConf.farPlane;

	glm::vec4 nearPos(mousePos * wNear, -wNear, wNear);
	nearPos = viewI * projI * nearPos;

	glm::vec4 farPos(mousePos * wFar, wFar, wFar);
	farPos = viewI * projI * farPos;

	PhysicsComponent* hit = std::static_pointer_cast<PhysicsComponentManager>(parent->getManager(PHYSICS_COMPONENT_NAME))->raytraceSingle(nearPos, farPos);

	if (hit != nullptr) {
		std::shared_ptr<GuiComponent> element = hit->getParent()->getComponent<GuiComponent>(GUI_COMPONENT_NAME);

		if (element) {
			element->onMouseClick(parent, event->button, event->action);
			return true;
		}
	}

	return false;
}
