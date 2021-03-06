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

#include "TextureLoader.hpp"
#include "CombinedGl.h"

//Stores information for rendering a texture.
struct GlTextureData {
	GLenum type;
	GLuint id;
};

class GlTextureLoader : public TextureLoader {
public:
	/**
	 * Creates a GlTextureLoader, ready for loading textures.
	 * @param texMap The map where loaded textures are stored.
	 */
	GlTextureLoader(std::unordered_map<std::string, GlTextureData>& texMap);

	/**
	 * Loads a texture from disk and uploads it to the GPU after storing it in the
	 * renderer's texture map.
	 * @param name The name the texture is stored under, used by other components
	 *     to refer to the texture.
	 * @param filename The full filename of the texture to be loaded.
	 * @param minFilter The type of filtering used on the texture when downscaling.
	 * @param magFilter The type of filtering used on the texture when upscaling.
	 * @param mipmap Whether to generate mipmaps for the texture.
	 * @throw runtime_error if the texture could not be created.
	 */
	void loadTexture(const std::string& name, const std::string& filename, Filter minFilter, Filter magFilter, bool mipmap) override;

	/**
	 * Loads a cubemap texture.
	 * @param name The name to store the texture under.
	 * @param filenames An array of six filenames in the order {+x, -x, +y, -y, +z, -z}.
	 *     All textures must be the same size.
	 * @param minFilter The filter to use when downscaling.
	 * @param magFilter The filter to use the upscaling.
	 * @param mipmap Whether to generate mipmaps.
	 */
	void loadCubeMap(const std::string& name, const std::array<std::string, 6>& filenames, Filter minFilter, Filter magFilter, bool mipmap) override;

protected:
	/**
	 * Adds a font texture.
	 * @param textureName The name to store the texture under.
	 * @param data The texture data to store.
	 */
	void addFontTexture(const std::string& textureName, const TextureData& data) override;

private:
	//The map where loaded textures are stored
	std::unordered_map<std::string, GlTextureData>& textureMap;
};
