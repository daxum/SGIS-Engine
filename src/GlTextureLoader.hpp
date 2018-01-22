#pragma once

#include <string>
#include <unordered_map>
#include "TextureLoader.hpp"
#include "CombinedGl.h"

class GlTextureLoader : public TextureLoader {
public:
	/**
	 * Creates a GlTextureLoader, ready for loading textures.
	 * @param texMap The map where loaded textures are stored.
	 */
	GlTextureLoader(std::unordered_map<std::string, GLuint>& texMap) : textureMap(texMap) {}

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
	void loadTexture(std::string name, std::string filename, Filter minFilter, Filter magFilter, bool mipmap);

private:
	//The map where loaded textures are stored
	std::unordered_map<std::string, GLuint>& textureMap;
};
