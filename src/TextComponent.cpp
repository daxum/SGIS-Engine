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

TextComponent::TextComponent(const std::u32string& text, const std::string& font, const std::string& shader, glm::vec3 scale, glm::vec3 color) :
	Component(TEXT_COMPONENT_NAME),
	currentFont(font),
	currentShader(shader),
	currentText(text),
	textModel(Engine::instance->getFontManager().createTextModel(font, text, shader)),
	initScale(scale),
	initColor(color) {

}

TextComponent::~TextComponent() {
	Engine::instance->getFontManager().freeTextModel(textModel);
}

void TextComponent::onParentSet() {
	std::shared_ptr<RenderComponent> render = lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME);

	if (!render) {
		lockParent()->addComponent(std::make_shared<RenderComponent>(textModel, initColor, initScale));
	}
	else {
		render->setModel(textModel);
		render->setScale(initScale);
		render->setColor(initColor);
	}
}

void TextComponent::setText(const std::u32string& newText) {
	currentText = newText;

	Model newModel = Engine::instance->getFontManager().createTextModel(currentFont, currentText, currentShader);
	Engine::instance->getFontManager().freeTextModel(textModel);
	textModel = newModel;

	lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)->setModel(textModel);
}

void TextComponent::setFont(const std::string& font) {
	currentFont = font;

	Model newModel = Engine::instance->getFontManager().createTextModel(currentFont, currentText, currentShader);
	Engine::instance->getFontManager().freeTextModel(textModel);
	textModel = newModel;

	lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)->setModel(textModel);
}

void TextComponent::setShader(const std::string& shader) {
	currentShader = shader;

	//Don't need to regenerate the model in this case.
	textModel.shader = shader;
	lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)->setModel(textModel);
}

AxisAlignedBB TextComponent::getTextBox() {
	AxisAlignedBB textBox = textModel.meshBox;

	textBox.translate(-textBox.getCenter());
	textBox.scale(lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)->getScale());

	return textBox;
}
