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
#include "Models/ModelManager.hpp"
#include "AxisAlignedBB.hpp"

struct TextMeshInfo {
	//The font the text uses.
	std::string font;
	//The text in the mesh.
	std::u32string text;
	//The vertex buffer to store the mesh in.
	std::string vertexBuffer;
	//The index buffer to store the mesh in.
	std::string indexBuffer;
	//The format of the text vertices.
	std::string format;
};

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
	 * @param meshInfo The info used to create a mesh for the text.
	 * @param material The material the text uses.
	 * @return A model for the given text.
	 */
	Model createTextModel(const TextMeshInfo& meshInfo, const std::string& material);

private:
	//The model manager.
	ModelManager& modelManager;
	//A map of fonts.
	std::unordered_map<std::string, Font> fontMap;

	/**
	 * Creates a mesh object for the given text and adds it to the model manager.
	 * @param meshInfo The info used to create the mesh.
	 */
	void createTextMesh(const TextMeshInfo& meshInfo);

	/**
	 * Generates a unique name for a text mesh based on its creation parameters.
	 * @param meshInfo The info that uniquely identifies this text mesh.
	 * @return a unique name for the mesh.
	 */
	std::string getMeshName(const TextMeshInfo& meshInfo) const;
};
