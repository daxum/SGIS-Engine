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

#include <stdexcept>
#include "GlTextureLoader.hpp"

GlTextureLoader::GlTextureLoader(Logger& logger, std::unordered_map<std::string, GlTextureData>& texMap) :
	TextureLoader(logger),
	textureMap(texMap) {

}

void GlTextureLoader::loadTexture(const std::string& name, const std::string& filename, Filter minFilter, Filter magFilter, bool mipmap ) {
	if (textureMap.count(name) != 0) {
		//Duplicate texture, skip it.
		logger.warn("Attempted to load duplicate texture \"" + filename + "\".");
		return;
	}

	//Create texture object
	GLuint texture = 0;
	glGenTextures(1, &texture);

	if (texture == 0) {
		throw std::runtime_error("glGenTextures() returned 0 - could not allocate texture.");
	}

	//Bind and upload texture data
	glBindTexture(GL_TEXTURE_2D, texture);

	TextureData texData = loadFromDisk(filename);

	//Don't abort if image loading failed - the missing texture should be perfectly usable
	if (!texData.loadSuccess) {
		logger.warn("Using missing texture data for \"" + filename + "\".");
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texData.width, texData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data.get());

	//Set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);

	//Generate mipmaps
	if (mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	textureMap.insert(std::make_pair(name, GlTextureData{TextureType::TEX_2D, texture}));

	glBindTexture(GL_TEXTURE_2D, 0);

	logger.debug("Uploaded texture \"" + name + "\".");
}

void GlTextureLoader::loadCubeMap(const std::string& name, const std::vector<std::string>& filenames, Filter minFilter, Filter magFilter, bool mipmap) {
	if (filenames.size() != 6) {
		throw std::runtime_error("Bad cubemap size");
	}

	if (textureMap.count(name) != 0) {
		throw std::runtime_error("Attempted to load duplicate texture \"" + name + "\"");
	}

	GLuint cubeMap = 0;
	glGenTextures(1, &cubeMap);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	for (unsigned int i = 0; i < filenames.size(); i++) {
		TextureData texData = loadFromDisk(filenames.at(i));

		if (!texData.loadSuccess) {
			throw std::runtime_error("Failed to load cubemap texture " + filenames.at(i));
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, texData.width, texData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data.get());
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);

	if (mipmap) {
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	textureMap.insert(std::make_pair(name, GlTextureData{TextureType::CUBEMAP, cubeMap}));

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	logger.debug("Uploaded cubemap texture \"" + name + "\"");
}

//This and loadTexture should be merged eventually.
void GlTextureLoader::addFontTexture(const std::string textureName, const TextureData& data) {
	if (textureMap.count(textureName) != 0) {
		//Duplicate texture, skip it.
		logger.warn("Attempted to add duplicate texture \"" + textureName + "\".");
		return;
	}

	//Create texture object
	GLuint texture = 0;
	glGenTextures(1, &texture);

	if (texture == 0) {
		throw std::runtime_error("glGenTextures() returned 0 - could not allocate texture.");
	}

	//Bind and upload texture data
	glBindTexture(GL_TEXTURE_2D, texture);

	//The font texture should always be 4-byte aligned, but it's always good to be safe.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, data.width, data.height, 0, GL_RED, GL_UNSIGNED_BYTE, data.data.get());
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//Set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	textureMap.insert(std::make_pair(textureName, GlTextureData{TextureType::TEX_2D, texture}));

	glBindTexture(GL_TEXTURE_2D, 0);

	logger.debug("Uploaded texture \"" + textureName + "\".");
}
