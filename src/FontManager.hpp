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

#include <unordered_map>
#include <memory>

#include "Font.hpp"
#include "ModelManager.hpp"
#include "AxisAlignedBB.hpp"

//Stores all the fonts used by the game.
class FontManager {
public:
	/**
	 * Creates a font manager.
	 * @param modelManager The model manager to load text models to.
	 */
	FontManager(ModelManager& modelManager) : modelManager(modelManager) {}

	/**
	 * Creates an empty font with the given name.
	 * @param name The texture name of the font.
	 * @param spaceWidth The width of a space.
	 * @param size The font size.
	 * @return A new font with the given name.
	 */
	Font& addFont(const std::string& name, int spaceWidth, size_t size);

	/**
	 * Creates a model for the given text using the provided font.
	 * @param fontName The name of the font to use.
	 * @param text The text to generate a model for.
	 * @param shader The shader to use to render the text.
	 * @param buffer The buffer to upload the text model to.
	 * @param uniformSet The set of uniforms for the model to use.
	 * @return a reference to a model for the given text.
	 */
	std::shared_ptr<const ModelRef> createTextModel(const std::string& fontName, const std::u32string& text, const std::string& shader, const std::string& buffer, const std::string& uniformSet);

private:
	//The model manager.
	ModelManager& modelManager;
	//A map of fonts.
	std::unordered_map<std::string, Font> fontMap;

	/**
	 * Creates a mesh object for the given text and adds it to the model manager.
	 * @param fontName The font to use.
	 * @param text The text to generate a mesh for.
	 * @param buffer The buffer to upload the mesh to.
	 */
	void createTextMesh(const std::string& fontName, const std::u32string& text, const std::string& buffer);

	/**
	 * Generates a unique name for a text mesh based on its creation parameters.
	 * @param fontName The name of the font.
	 * @param text The text the mesh contains.
	 * @param buffer The buffer the mesh is loaded into.
	 * @return a unique name for the mesh.
	 */
	std::string getMeshName(const std::string& fontName, const std::u32string& text, const std::string& buffer) const;
};
