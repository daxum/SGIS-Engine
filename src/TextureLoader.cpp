#include "TextureLoader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Used to free image data from smart pointers.
struct ImageDeleter {
	void operator()(unsigned char* p) {
		stbi_image_free(p);
	}
};

TextureData TextureLoader::loadFromDisk(std::string filename) {
	TextureData texData = {};

	//Channels is currently unused, but will be set with the amount of channels the image originally had
	//if it's ever needed.
	int channels = 0;
	texData.channels = 4;
	unsigned char* imageData = stbi_load(filename.c_str(), &texData.width, &texData.height, &channels, 4);

	if (imageData == nullptr) {
		//TODO: missing data
		texData.loadSuccess = false;
	}
	else {
		texData.loadSuccess = true;
		texData.data.reset(imageData, ImageDeleter());
	}

	return texData;
}
