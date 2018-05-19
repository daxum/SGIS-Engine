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
#include "AxisAlignedBB.hpp"
#include "Object.hpp"
#include "PhysicsComponent.hpp"
#include "Engine.hpp"
#include "RenderComponentManager.hpp"

RenderComponent::RenderComponent(std::string model, glm::vec3 color, glm::vec3 renderScale) :
	Component(RENDER_COMPONENT_NAME),
	model(Engine::instance->getModelManager().getModel(model)),
	textModel(false),
	color(color),
	scale(renderScale) {

}

RenderComponent::RenderComponent(const std::u32string& text, const std::string& font, const std::string& shader, glm::vec3 color, glm::vec3 renderScale) :
	Component(RENDER_COMPONENT_NAME),
	model(Engine::instance->getFontManager().createTextModel(font, text, shader)),
	textModel(true),
	color(color),
	scale(renderScale) {

}

RenderComponent::~RenderComponent() {
	if (textModel) {
		Engine::instance->getFontManager().freeTextModel(model);
	}
}

glm::vec3 RenderComponent::getTranslation() {
	return lockParent()->getPhysics()->getTranslation();
}

glm::quat RenderComponent::getRotation() {
	return  lockParent()->getPhysics()->getRotation();
}

glm::vec3 RenderComponent::getScale() {
	return scale;
}

glm::vec3 RenderComponent::getColor() {
	return color;
}

void RenderComponent::setModel(const Model& newModel) {
	if (textModel) {
		Engine::instance->getFontManager().freeTextModel(model);
	}

	textModel = false;

	std::string oldShader = model.shader;
	model = newModel;

	if (manager) {
		manager->reloadComponent(shared_from_this(), oldShader);
	}
}

void RenderComponent::setTextModel(const std::u32string& text, const std::string& font, const std::string& shader) {
	if (textModel) {
		Engine::instance->getFontManager().freeTextModel(model);
	}

	textModel = true;

	std::string oldShader = model.shader;
	model = Engine::instance->getFontManager().createTextModel(font, text, shader);

	if (manager) {
		manager->reloadComponent(shared_from_this(), oldShader);
	}
}

void RenderComponent::setTextModel(const std::u32string& text) {
	if (!textModel) {
		throw std::runtime_error("setTextModel with no font/shader for previous non-text model!");
	}

	setTextModel(text, model.texture, model.shader);
}
