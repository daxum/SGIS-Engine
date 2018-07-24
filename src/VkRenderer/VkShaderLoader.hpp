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
};

