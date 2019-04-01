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

#include "TextureLoader.hpp"
#include "VkMemoryManager.hpp"

class VkTextureLoader : public TextureLoader {
public:
	/**
	 * Creates the texture loader.
	 * @param vkObjects The object handler to get the device from.
	 * @param memoryManager The memory manager to allocate textures from.
	 */
	VkTextureLoader(VkObjectHandler& vkObjects, VkMemoryManager& memoryManager);

	/**
	 * Destroys stuff.
	 */
	~VkTextureLoader() {}

	/**
	 * Loads a texture and uploads it to the memory manager.
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
	 * Loads a cubemap texture from the textures specified by filenames.
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
	 * Adds a font texture in the same way as loadTexture.
	 * @param textureName The name to store the texture under.
	 * @param data The texture data to store.
	 */
	void addFontTexture(const std::string& textureName, const TextureData& data) override;

private:
	//Object handler, for the device.
	VkObjectHandler& vkObjects;
	//Memory manager to upload textures to.
	VkMemoryManager& memoryManager;

	/**
	 * Creates a sampler and stores it in the memory manager under the given name.
	 * @param imageName The name of the image that uses this sampler.
	 * @param minFilter The filter used when downscaling.
	 * @param magFilter The filter used when upscaling.
	 */
	void addTextureSampler(const std::string& imageName, Filter minFilter, Filter magFilter);

	/**
	 * Converts a filter to a VkFilter.
	 * @param filter The filter.
	 * @return The VkFilter version of the filter.
	 */
	static constexpr VkFilter filterToVk(const Filter filter) {
		switch (filter) {
			case Filter::NEAREST: return VK_FILTER_NEAREST;
			case Filter::LINEAR: return VK_FILTER_LINEAR;
			default: throw std::runtime_error("Missing sampler filter!");
		}
	}
};
