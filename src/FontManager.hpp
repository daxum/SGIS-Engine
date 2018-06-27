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
#include "RenderingEngine.hpp"
#include "AxisAlignedBB.hpp"

//Stores all the fonts used by the game.
class FontManager {
public:
	/**
	 * Creates a font. Uses same pointer-reference shenanigans as ModelLoader.
	 * @param renderer The rendering engine to use.
	 */
	FontManager(std::shared_ptr<RenderingEngine>& renderer) : renderer(renderer) {}

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
	 * @return a model for the given text.
	 */
	Model createTextModel(const std::string& fontName, const std::u32string& text, const std::string& shader);

private:
	//The rendering engine.
	std::shared_ptr<RenderingEngine>& renderer;
	//A map of fonts.
	std::unordered_map<std::string, Font> fontMap;
};
