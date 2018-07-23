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

#include "RenderComponent.hpp"
#include "Engine.hpp"
#include "RenderComponentManager.hpp"

RenderComponent::RenderComponent(std::string model, glm::vec3 color, glm::vec3 renderScale) :
	Component(RENDER_COMPONENT_NAME),
	model(Engine::instance->getModel(model)),
	color(color),
	scale(renderScale),
	manager(nullptr) {

}

RenderComponent::RenderComponent(std::shared_ptr<ModelRef> model, glm::vec3 color, glm::vec3 renderScale) :
	Component(RENDER_COMPONENT_NAME),
	model(model),
	color(color),
	scale(renderScale),
	manager(nullptr) {

}

glm::vec3 RenderComponent::getTranslation() {
	return lockParent()->getPhysics()->getTranslation();
}

glm::quat RenderComponent::getRotation() {
	return  lockParent()->getPhysics()->getRotation();
}

void RenderComponent::setModel(std::shared_ptr<ModelRef> newModel) {
	std::shared_ptr<ModelRef> oldModel = model;
	model = newModel;

	if (manager) {
		//Get shared pointer from parent because not stored here (shared_from_this was having problems for some reason).
		manager->reloadComponent(lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME), oldModel);
	}
}
