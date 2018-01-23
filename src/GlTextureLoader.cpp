#include <stdexcept>
#include "GlTextureLoader.hpp"

void GlTextureLoader::loadTexture(std::string name, std::string filename, Filter minFilter, Filter magFilter, bool mipmap ) {
	if (textureMap.count(name) != 0) {
		//Duplicate texture, skip it.
		//TODO: warning message
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
		//TODO: warning
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texData.width, texData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data.get());

	//Set filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter == Filter::LINEAR ? GL_LINEAR : GL_NEAREST);

	//Generate mipmaps
	if (mipmap) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	textureMap.insert(std::make_pair(name, texture));

	glBindTexture(GL_TEXTURE_2D, 0);
}
