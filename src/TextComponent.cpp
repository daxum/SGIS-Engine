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

#include "TextComponent.hpp"
#include "RenderComponent.hpp"
#include "Engine.hpp"

std::u32string TextComponent::convToU32(const std::string& s) {
	std::u32string out;
	out.reserve(s.size());

	for (const char& c : s) {
		out.push_back((char32_t)c);
	}

	return out;
}

TextComponent::TextComponent(const std::u32string& text, const std::string& font, const std::string& material, const std::string& buffer, const std::string format, glm::vec3 scale) :
	Component(TEXT_COMPONENT_NAME),
	logger(Engine::instance->getConfig().componentLog),
	meshInfo({font, text, buffer, format}),
	material(material),
	textModel(Engine::instance->getFontManager().createTextModel(meshInfo, material)),
	initScale(scale) {

}

void TextComponent::onParentSet() {
	std::shared_ptr<RenderComponent> render = lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME);

	if (!render) {
		lockParent()->addComponent(std::make_shared<RenderComponent>(textModel, initScale));
	}
	else {
		render->setModel(textModel);
		render->setScale(initScale);
	}
}

Aabb<float> TextComponent::getTextBox() {
	Aabb<float> textBox = textModel.mesh->getBox();

	textBox.translate(-textBox.getCenter());
	textBox.scale(lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)->getScale());

	return textBox;
}

void TextComponent::fitToBox(const glm::vec2& box, bool preserveAspect) {
	if (!lockParent() || !lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)) {
		ENGINE_LOG_WARN(logger, "Attempt to call fitToBox on TextComponent before parent set!");
		return;
	}

	const Aabb<float>& textBox = getTextBox();

	float xScale = box.x / textBox.xLength();
	float yScale = box.y / textBox.yLength();

	if (preserveAspect) {
		const float scale = std::min(xScale, yScale);

		xScale = scale;
		yScale = scale;
	}

	std::shared_ptr<RenderComponent> render = lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME);

	glm::vec3 adjustedScale = render->getScale();
	adjustedScale.x *= xScale;
	adjustedScale.y *= yScale;

	render->setScale(adjustedScale);
}
