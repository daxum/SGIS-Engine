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
#include "Model.hpp"

class TextComponent : public Component {
public:
	/**
	 * Converts a normal string to a u32string.
	 * @param s The string to convert.
	 * @return The u32string version of s.
	 */
	static std::u32string convToU32(const std::string& s);

	/**
	 * Creates a text component. If an object is going to be only text,
	 * adding a render component is uneccessary, as the text component will
	 * add one itself (only if one is not already present).
	 * @param text The text to render.
	 * @param font The font to use.
	 * @param shader The shader to use to render the text.
	 * @param buffer The buffer for the text model to reside in.
	 * @param scale The scale of the text. Might need to be rather small for
	 *     large font sizes.
	 * @param color The color of the text.
	 */
	TextComponent(const std::u32string& text, const std::string& font, const std::string& shader, const std::string& buffer, glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), glm::vec3 color = glm::vec3(0.0, 0.0, 0.0));

	/**
	 * Called from Component, sets the rendering component if one is not
	 * already present.
	 */
	void onParentSet();

	/**
	 * Sets the text to be rendered. Regenerates the text model.
	 * @param newText The new text to use.
	 */
	void setText(const std::u32string& newText);

	/**
	 * Sets the font. Careful calling this, as it needs to regenerate the model.
	 * @param font The new font.
	 */
	void setFont(const std::string& font);

	/**
	 * Sets the model's shader. Also regenerates the text model.
	 * @param shader The new shader.
	 */
	void setShader(const std::string& shader);

	/**
	 * Returns the bounding box of the current text model, centered and
	 * taking scale into account.
	 * @return The centered and scaled bounding box of the text.
	 */
	AxisAlignedBB getTextBox();

	/**
	 * Adjusts the rendering scale so that the text fits inside the given bounding box.
	 * @param box A vector containing the x and y lengths to fit the text within.
	 * @param perserveAspect If true, this will scale the text equally for both the x and y directions.
	 *     If false, it will fit the text inside the box exactly.
	 */
	void fitToBox(const glm::vec2& box, bool preserveAspect = true);

private:
	std::string currentFont;
	std::u32string currentText;
	std::shared_ptr<ModelRef> textModel;

	//Only used in onParentSet.
	glm::vec3 initScale;
	glm::vec3 initColor;
};
