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
#include "VkShader.hpp"

class VkShaderLoader : public ShaderLoader {
public:
	/**
	 * Constructor.
	 */
	VkShaderLoader(VkObjectHandler& vkObjects, VkRenderObjects& renderObjects, VkMemoryManager* memoryManager, Logger& logger, std::unordered_map<std::string, std::shared_ptr<VkShader>>& shaderMap);

	/**
	 * Destroys the descriptor set layouts.
	 */
	~VkShaderLoader();

	/**
	 * Loads the shaders from disk and constructs a program object from them.
	 * @param name A name given to the loaded shader.
	 * @param info Information about the shader to be loaded.
	 */
	void loadShader(std::string name, const ShaderInfo& info) override;

	/**
	 * Reloads all shader objects.
	 */
	void reloadShaders() {
		for (auto& shaderPair : shaderMap) {
			shaderPair.second->reload();
		}
	}

private:
	//Map loaded shaders are added to.
	std::unordered_map<std::string, std::shared_ptr<VkShader>>& shaderMap;
	//Loaded shader modules, needed for when window is resized and pipelines
	//need to be recreated.
	std::unordered_map<std::string, VkShaderModule> loadedModules;
	//Object handling all vulkan objects.
	VkObjectHandler& vkObjects;
	//Object handling all swapchain related vulkan objects.
	VkRenderObjects& renderObjects;
	//Pipeline cache. Maybe save this to disk later, but that might interfere
	//with development...
	VkPipelineCache pipelineCache;
	//Memory manager, used to fetch descriptor set layouts.
	VkMemoryManager* memoryManager;

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
	 * Converts a set of push constant uniform descriptions to a set of push
	 * constant ranges, based on the shader stages and variable sizes.
	 * @param pushSet A set of push constant uniforms.
	 * @return A vector of ranges for the provided push constants.
	 */
	static std::vector<VkPushConstantRange> convertToRanges(const PushConstantSet& pushSet);
};

