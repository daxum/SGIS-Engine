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

#include <string>

#include <glm/glm.hpp>

#include "Component.hpp"
#include "Models/ModelManager.hpp"
#include "Logger.hpp"
#include "FontManager.hpp"
#include "Engine.hpp"
#include "Object.hpp"
#include "RenderComponent.hpp"

class TextComponent : public Component {
public:
	/**
	 * Converts a normal string to a u32string.
	 * @param s The string to convert.
	 * @return The u32string version of s.
	 */
	static std::u32string convToU32(const std::string& s);

	/**
	 * Required from component.
	 * @return The component's name.
	 */
	static const std::string getName() { return TEXT_COMPONENT_NAME; }

	/**
	 * Creates a text component. If an object is going to be only text,
	 * adding a render component is uneccessary, as the text component will
	 * add one itself (only if one is not already present).
	 * @param meshInfo The info about the mesh for the text.
	 * @param material The material the text uses for rendering.
	 * @param scale The scale of the text. Might need to be rather small for
	 *     large font sizes.
	 */
	TextComponent(const TextMeshInfo& meshInfo, const std::string& material, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0));

	/**
	 * Called from Component, sets the rendering component if one is not
	 * already present.
	 */
	void onParentSet() override;

	/**
	 * Sets the text to be rendered. Regenerates the text mesh.
	 * @param newText The new text to use.
	 */
	void setText(const std::u32string& newText) {
		meshInfo.text = newText;
		reloadModel();
	}

	/**
	 * Sets the font. Careful calling this, as it needs to regenerate the mesh.
	 * @param font The new font.
	 */
	void setFont(const std::string& font) {
		meshInfo.font = font;
		reloadModel();
	}

	/**
	 * Changes the material the text uses.
	 * @param material The new material for the text.
	 */
	void setMaterial(const std::string& newMaterial) {
		material = newMaterial;
		reloadModel();
	}

	/**
	 * Returns the bounding box of the current text model, centered and
	 * taking scale into account.
	 * @return The centered and scaled bounding box of the text.
	 */
	Aabb<float> getTextBox();

	/**
	 * Adjusts the rendering scale so that the text fits inside the given bounding box.
	 * @param box A vector containing the x and y lengths to fit the text within.
	 * @param perserveAspect If true, this will scale the text equally for both the x and y directions.
	 *     If false, it will fit the text inside the box exactly.
	 */
	void fitToBox(const glm::vec2& box, bool preserveAspect = true);

private:
	//Logger for text component.
	Logger logger;

	//Info used to create the text mesh.
	TextMeshInfo meshInfo;
	//Name of the material the text is using.
	std::string material;
	//Model for the text mesh.
	Model textModel;

	//Only used in onParentSet.
	glm::vec3 initScale;

	/**
	 * Regenerates the text model and uploads to the render component.
	 * This will not create a new mesh if only the material changed.
	 */
	void reloadModel() {
		textModel = Engine::instance->getFontManager().createTextModel(meshInfo, material);
		lockParent()->getComponent<RenderComponent>(RENDER_COMPONENT_NAME)->setModel(textModel);
	}
};
