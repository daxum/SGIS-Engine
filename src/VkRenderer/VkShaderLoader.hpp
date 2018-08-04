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

#include <vector>

#include "ShaderLoader.hpp"
#include "VkObjectHandler.hpp"
#include "VkMemoryManager.hpp"

class VkShaderLoader : public ShaderLoader {
public:
	/**
	 * Constructor
	 */
	VkShaderLoader(VkObjectHandler& vkObjects, VkMemoryManager* memoryManager, Logger& logger, std::unordered_map<std::string, std::shared_ptr<Shader>>& shaderMap);

	/**
	 * Loads the shaders from disk and constructs a program object from them.
	 * @param name A name given to the loaded shader.
	 * @param info Information about the shader to be loaded.
	 */
	void loadShader(std::string name, const ShaderInfo& info) override;

private:
	//Map loaded shaders are added to.
	std::unordered_map<std::string, std::shared_ptr<Shader>>& shaderMap;
	//Object handling all vulkan objects.
	VkObjectHandler& vkObjects;

	/**
	 * Creates attribute descriptions for the passed in vertex format.
	 * @param bufferFormat The format of the buffers that will be used with this shader.
	 * @return A vector of attribute description structures.
	 */
	std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescription(const VertexBuffer& buffer) const;

	/**
	 * Loads shader bytecode from disk and creates a shader module for it.
	 * @param filename The file containing the bytecode.
	 * @return A shader module for the shader.
	 */
	VkShaderModule createShaderModule(const std::string& filename);

	/**
	 * Loads the given file from disk as binary data.
	 * @param filename The file to load.
	 * @return The binary data the file contained.
	 */
	std::vector<char> loadFromDisk(const std::string& filename);

	/**
	 * Converts the type to a VkFormat.
	 * @param type The type to convert.
	 * @return The corresponding format.
	 */
	static VkFormat formatFromVertexType(const VertexElementType& type) {
		switch (type) {
			case VertexElementType::FLOAT: return VK_FORMAT_R32_SFLOAT;
			case VertexElementType::VEC2: return VK_FORMAT_R32G32_SFLOAT;
			case VertexElementType::VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexElementType::VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			default: return VK_FORMAT_UNDEFINED;
		}
	}
};

