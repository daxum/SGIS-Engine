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
#include <unordered_map>

#include <glm/glm.hpp>

struct GlyphData {
	//Position in texture, from top left.
	glm::ivec2 pos;

	//Position of corners in texture, from 0 to 1
	glm::vec4 fPos;

	//Width and height of glyph.
	glm::ivec2 size;

	//Bearing x and y, offset from baseline.
	glm::ivec2 bearing;

	//Distance to next character.
	int advance;
};

class Font {
public:
	/**
	 * Creates a font with the given texture.
	 * @param tex The bitmap containing the font's characters.
	 * @param spaceWidth The width of a space.
	 * @param size The font size.
	 */
	Font(std::string tex, int spaceWidth, size_t size) : texture(tex), spaceWidth(spaceWidth), size(size) {}

	/**
	 * Adds a character to the font. Should only be called during font
	 * loading, unless the new character is also present in the texture.
	 * @param glyph The new character to add.
	 * @param data The data about how to render the glyph.
	 */
	void addGlyph(char32_t glyph, GlyphData& data) { glyphs.insert({glyph, data}); }

	/**
	 * Try to guess what this does.
	 */
	int getSpaceWidth() const { return spaceWidth; }

	/**
	 * Gets the texture data for the given character.
	 * @param glyph The character to get texture data for.
	 * @return The texture data for the character.
	 * @throw Whatever map throws when an element isn't present, whenever
	 *     the character isn't present.
	 */
	const GlyphData& getChar(char32_t glyph) const { return glyphs.at(glyph); }

	/**
	 * Returns the font's texture.
	 */
	const std::string& getTexture() const { return texture; }

	/**
	 * Returns the size of the font.
	 */
	const size_t getSize() const { return size; }

private:
	std::unordered_map<char32_t, GlyphData> glyphs;

	std::string texture;

	int spaceWidth;

	size_t size;
};
