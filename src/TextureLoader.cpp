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

#include <algorithm>

#include "TextureLoader.hpp"
#include "Engine.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ft2build.h"
#include FT_FREETYPE_H

namespace {
	unsigned char missingData[16] = {
		0x00, 0x00, 0x00, 0xFF,
		0xC8, 0x00, 0xAA, 0xFF,
		0xC8, 0x00, 0xAA, 0xFF,
		0x00, 0x00, 0x00, 0xFF
	};

	//Used to free image data from smart pointers.
	struct ImageDeleter {
		void operator()(unsigned char* p) {
			stbi_image_free(p);
		}
	};

	//Used for missing data.
	struct NonDeleter {
		void operator()(unsigned char* p) {}
	};

	//Used for temporary font storage when creating textures.
	struct CharData {
		CharData(const GlyphData* g, unsigned char* buf, size_t bufferSize) :
			//Don't look
			glyph(const_cast<GlyphData*>(g)),
			buffer(new unsigned char[bufferSize]) {

			memcpy(buffer.get(), buf, bufferSize);
		}

		GlyphData* glyph;
		std::shared_ptr<unsigned char> buffer;
	};

	//Sorts glyphs by decreasing height.
	bool glyphSorter(const GlyphData* a, const GlyphData* b) {
		return a->size.y > b->size.y;
	}
}

TextureData TextureLoader::loadFromDisk(std::string filename) {
	TextureData texData = {};

	logger.debug("Loading \"" + filename + "\".");

	//Channels is currently unused, but will be set with the amount of channels the image originally had
	//if it's ever needed.
	int channels = 0;
	texData.channels = 4;
	unsigned char* imageData = stbi_load(filename.c_str(), &texData.width, &texData.height, &channels, 4);

	if (imageData == nullptr) {
		logger.error("Couldn't load texture \"" + filename + "\" - file doesn't exist.");
		texData.loadSuccess = false;
		texData.width = 2;
		texData.height = 2;
		texData.data.reset(missingData, NonDeleter());
	}
	else {
		texData.loadSuccess = true;
		texData.data.reset(imageData, ImageDeleter());
	}

	//If width and height are not equal, or they are not powers of two, the texture is malformed.
	//Report a warning if this occurs.
	if ((texData.width & (texData.width - 1)) || texData.width != texData.height) {
		logger.warn("Malformed texture \"" + filename + "\" has dimensions " + std::to_string(texData.width) + " x " +
					std::to_string(texData.height) + ". Dimensions must be equal powers of two.");
	}

	logger.debug("Loaded " + std::to_string(texData.width) + " x " + std::to_string(texData.height) + " texture \"" + filename + "\".");

	return texData;
}

void TextureLoader::loadFont(const std::string& name, const std::vector<std::string>& filenames, const std::u32string& characters, size_t size) {
	FT_Library library;
	FT_Init_FreeType(&library);

	std::vector<FT_Face> faces(filenames.size());

	//Open all fonts.
	for (size_t i = 0; i < filenames.size(); i++) {
		if (FT_New_Face(library, filenames.at(i).c_str(), 0, &faces.at(i))) {
			throw std::runtime_error("Couldn't open file \"" + filenames.at(i) + "\", or the filetype is not supported.");
		}

		FT_Set_Pixel_Sizes(faces.at(i), 0, size);
	}

	std::vector<GlyphData*> glyphs;
	std::vector<CharData> chars;

	if (FT_Load_Char(faces.at(0), U' ', FT_LOAD_RENDER)) {
		throw std::runtime_error("Missing space in font " + filenames.at(0) + "!");
	}

	Font& font = Engine::instance->getFontManager().addFont(name, faces.at(0)->glyph->advance.x >> 6, size);

	//Load characters from fonts.
	for (char32_t character : characters) {
		for (const FT_Face& face : faces) {
			//Maybe this char is in the next font...
			if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
				continue;
			}

			size_t bitmapSize = face->glyph->bitmap.width * face->glyph->bitmap.rows;

			//Create glyph data
			GlyphData data;
			data.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
			data.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
			data.advance = face->glyph->advance.x >> 6;

			//Add to font
			font.addGlyph(character, data);

			//Add buffer for later adding to texture
			chars.emplace_back(&font.getChar(character), face->glyph->bitmap.buffer, bitmapSize);
			glyphs.push_back(chars.back().glyph);

			break;
		}
	}

	//Sort glyph vector by height
	std::sort(glyphs.begin(), glyphs.end(), glyphSorter);

	//Try to fit glyphs in smallest texture possible
	unsigned int texSize = 2;

	logger.debug("Trying texture of size " + std::to_string(texSize));

	while (!tryPositionGlyphs(glyphs, texSize)) {
		texSize = texSize << 1u;

		//Couldn't even fit on the biggest of textures. What kind of font is this?!
		if (texSize == 0) {
			throw std::runtime_error("Couldn't fit font \"" + name + "\" on any texture.");
		}

		logger.debug("Trying texture of size " + std::to_string(texSize));
	}

	logger.debug("Using " + std::to_string(texSize) + " x " + std::to_string(texSize) + " texture for font \"" + name + "\"");

	//Create texture
	TextureData fontTexture;
	fontTexture.data.reset(new unsigned char[texSize * texSize]);
	fontTexture.width = texSize;
	fontTexture.height = texSize;
	fontTexture.channels = 1;
	fontTexture.loadSuccess = true;

	memset(fontTexture.data.get(), 0, texSize * texSize);

	//Fill texture
	for (const CharData& data : chars) {
		const glm::ivec2 pos = data.glyph->pos;
		const int width = data.glyph->size.x;
		const int height = data.glyph->size.y;

		if (pos.x + width > (int)texSize || pos.y + height > (int)texSize) {
			logger.fatal("Memory corruption imminent!");
			logger.fatal("pos: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")");
			logger.fatal("size: (" + std::to_string(width) + ", " + std::to_string(height) + ")");
			throw std::runtime_error("Font loading failed!");
		}

		//Copy texture one row at a time
		for (int i = pos.y; i < pos.y + height; i++) {
			memcpy(&fontTexture.data.get()[i * texSize] + pos.x, &data.buffer.get()[(i - pos.y) * width], width);
		}
	}

	//Upload texture
	addFontTexture(name, fontTexture);

	//Free freetype
	for (FT_Face& face : faces) {
		FT_Done_Face(face);
	}

	FT_Done_FreeType(library);
}

bool TextureLoader::tryPositionGlyphs(std::vector<GlyphData*>& glyphs, const unsigned int texSize) {
	if (glyphs.empty()) {
		return true;
	}

	unsigned int nextHeight = glyphs.at(0)->size.y;
	unsigned int currentHeight = 0;
	unsigned int currentPos = 0;

	for (GlyphData* glyph : glyphs) {
		//TODO: calculate spacing based on font size.
		glm::vec2 size(glyph->size.x + 4, glyph->size.y + 4);

		//If glyph is too long, go to next row.
		if (currentPos + size.x > texSize) {
			currentPos = 0;
			currentHeight = nextHeight;
			//Glyphs are sorted by height, so just increment by this one's height.
			nextHeight += size.y;
		}

		//If past end of texture, return false (this requires the glyphs to be sorted by height).
		if (size.y + currentHeight > texSize) {
			return false;
		}

		//Set glyph x and y.
		glyph->pos.x = currentPos;
		glyph->pos.y = currentHeight;

		//Set tex coords
		glyph->fPos.x = currentPos / (float)texSize;
		glyph->fPos.y = currentHeight / (float)texSize;
		glyph->fPos.z = (currentPos + glyph->size.x) / (float)texSize;
		glyph->fPos.w = (currentHeight + glyph->size.y) / (float)texSize;

		currentPos += size.x;
	}

	//Made it through all the glyphs, so they fit.
	return true;
}
