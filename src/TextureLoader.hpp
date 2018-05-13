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
#include <memory>
#include <vector>

#include "Logger.hpp"
#include "Font.hpp"

//Returned when loading from disk
struct TextureData {
	//The loaded texture data
	std::shared_ptr<unsigned char> data;
	//The width of the loaded texture
	int width;
	//The height of the loaded texture
	int height;
	//The number of channels in the loaded texture
	int channels;
	//Whether the texture was loaded successfully - if false,
	//the above data will be missing texture data.
	bool loadSuccess;
};

//Texture filtering options
enum class Filter {
	NEAREST,
	LINEAR
};

//A generic texture loader, to go with RenderingEngine.
//Meant to be extended for different apis, but does provide some
//general texture utilities, namely loading the data from disk.
class TextureLoader {
public:
	/**
	 * Creates a texture loader. Just sets the logger.
	 * @param logger The logger to send messages to.
	 */
	TextureLoader(Logger& logger) : logger(logger) {}

	/**
	 * Destroys a texture loader. This doesn't do much right now,
	 * but texture data caching may be implemented later (for streaming
	 * large textures to the gpu, perhaps?), in which case this would
	 * destroy any leftover data.
	 */
	virtual ~TextureLoader() {}

	/**
	 * To be implemented by subclasses. This should load a texture and
	 * give it to the rendering engine. The method of transfer from the loader
	 * to the engine can be anything, but it has to get there.
	 * @param name The name the texture is stored under, used by other components
	 *     to refer to the texture.
	 * @param filename The full filename of the texture to be loaded.
	 * @param minFilter The type of filtering used on the texture when downscaling.
	 * @param magFilter The type of filtering used on the texture when upscaling.
	 * @param mipmap Whether to generate mipmaps for the texture.
	 * @throw runtime_error if the texture could not be created.
	 */
	virtual void loadTexture(const std::string& name, const std::string& filename, Filter minFilter, Filter magFilter, bool mipmap) = 0;

	/**
	 * Loads a font and generates a texture from it. Only adds the specified
	 * characters to the texture.
	 * @param name The name of the font texture.
	 * @param filenames A vector of filenames to load. Each font will be searched in
	 *     order for each character, stopping as soon as it is found.
	 * @param characters The characters to load from the specified file.
	 * @param size The height of each character (in pixels?).
	 */
	void loadFont(const std::string& name, const std::vector<std::string>& filenames, const std::u32string& characters, size_t size);

protected:
	//The logger for the loader.
	Logger& logger;

	/**
	 * Loads the provided texture from disk, and returns a pointer to its data.
	 * @param filename The filename of the texture to load.
	 * @return A filled out TextureData structure created from the provided file name.
	 */
	TextureData loadFromDisk(std::string filename);

	/**
	 * Adds a font to wherever fonts are stored and returns a reference to it.
	 * @param textureName The name of the font's texture.
	 * @return A reference to the stored font.
	 */
	virtual Font& addFont(const std::string& textureName) = 0;

	/**
	 * Adds a font texture in the same way as loadTexture.
	 * @param textureName The name to store the texture under.
	 * @param data The texture data to store. Be careful with the
	 *     number of channels.
	 */
	virtual void addFontTexture(const std::string textureName, const TextureData& data) = 0;

private:
	/**
	 * Sets the glyph's x and y coordinates so that they all fit on a texture of the given
	 * size, without overlaps.
	 * @param glyphs An array of glyphs to position on the texture. Must be sorted by height.
	 * @param texSize The size (width and height) of the texture to arrange the glyphs on.
	 * @return Whether all the glyphs fit on a texture of the given size. If this is false,
	 *     some were not positioned.
	 */
	bool tryPositionGlyphs(std::vector<GlyphData*>& glyphs, const unsigned int texSize);
};
