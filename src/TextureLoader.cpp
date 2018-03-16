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

#include "TextureLoader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
