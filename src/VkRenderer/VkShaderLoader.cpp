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

#include <fstream>

#include "VkShaderLoader.hpp"
#include "VkShader.hpp"

VkShaderLoader::VkShaderLoader(VkObjectHandler& vkObjects, VkMemoryManager* memoryManager, Logger& logger, std::unordered_map<std::string, std::shared_ptr<VkShader>>& shaderMap) :
	ShaderLoader(logger, memoryManager),
	shaderMap(shaderMap),
	vkObjects(vkObjects),
	pipelineCache(VK_NULL_HANDLE) {

}

VkShaderLoader::~VkShaderLoader() {
	for (const auto& layout : descriptorLayouts) {
		vkDestroyDescriptorSetLayout(vkObjects.getDevice(), layout.second, nullptr);
	}

	for (const auto& module : loadedModules) {
		vkDestroyShaderModule(vkObjects.getDevice(), module.second, nullptr);
	}

	if (pipelineCache != VK_NULL_HANDLE) {
		vkDestroyPipelineCache(vkObjects.getDevice(), pipelineCache, nullptr);
	}
}

void VkShaderLoader::loadShader(std::string name, const ShaderInfo& info) {
	logger.debug("Loading shader \"" + name + "\"");

	//Create pipeline cache if this is the first shader loaded
	if (pipelineCache == VK_NULL_HANDLE) {
		VkPipelineCacheCreateInfo cacheCreateInfo = {};
		cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		//TODO: load from disk?

		if (vkCreatePipelineCache(vkObjects.getDevice(), &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline cache!");
		}
	}

	//Create shader modules, or used cached ones if they were already loaded.
	VkShaderModule vertShader;
	VkShaderModule fragShader;

	if (loadedModules.count(info.vertex)) {
		vertShader = loadedModules.at(info.vertex);
	}
	else {
		vertShader = createShaderModule(info.vertex);
		loadedModules.insert({info.vertex, vertShader});
	}

	if (loadedModules.count(info.fragment)) {
		fragShader = loadedModules.at(info.fragment);
	}
	else {
		fragShader = createShaderModule(info.fragment);
		loadedModules.insert({info.fragment, fragShader});
	}

	VkPipelineShaderStageCreateInfo vertCreateInfo = {};
	vertCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertCreateInfo.module = vertShader;
	vertCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragCreateInfo = {};
	fragCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragCreateInfo.module = fragShader;
	fragCreateInfo.pName = "main";

	const VertexBuffer& buffer = memoryManager->getBuffer(info.buffer);

	//Create pipeline creator
	VkPipelineCreateObject pipelineCreator(vkObjects, {vertCreateInfo, fragCreateInfo}, info, buffer);

	//Create pipeline layout
	std::vector<VkDescriptorSetLayout> layouts;

	for (const std::string& set : info.uniformSets) {
		layouts.push_back(descriptorLayouts.at(set));
	}

	std::vector<VkPushConstantRange> pushRanges = convertToRanges(info.pushConstants);

	//TODO: Move into shader object
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.setLayoutCount = layouts.size();
	layoutCreateInfo.pSetLayouts = layouts.data();
	layoutCreateInfo.pushConstantRangeCount = pushRanges.size();
	layoutCreateInfo.pPushConstantRanges = pushRanges.data();

	VkPipelineLayout pipelineLayout;

	if (vkCreatePipelineLayout(vkObjects.getDevice(), &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout for shader \"" + name + "\"");
	}

	//Create shader and add to shader map
	shaderMap.insert({name, std::make_shared<VkShader>(vkObjects.getDevice(), pipelineCache, pipelineLayout, info.pushConstants, pipelineCreator)});

	logger.debug("Loaded shader \"" + name + "\"");
}

void VkShaderLoader::addUniformSet(const UniformSet& set, const std::string& name) {
	std::bitset<2> uboUseStages;
	uint32_t nextBinding = 0;
	bool hasUbo = false;

	//Set stages the uniform buffer is used in
	for (const UniformDescription& descr : set.uniforms) {
		if (!isSampler(descr.type)) {
			uboUseStages |= descr.shaderStages;
		}
	}

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	//Create descriptor set layout
	for (const UniformDescription& descr : set.uniforms) {
		if (!isSampler(descr.type)) {
			//Only add the uniform buffer once, at the location of the first buffered variable
			if (hasUbo) {
				continue;
			}

			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = nextBinding;
			binding.descriptorType = descriptorTypeFromBuffer(set.bufferType);
			binding.descriptorCount = 1;
			binding.stageFlags = uboUseStages.to_ulong();

			bindings.push_back(binding);
			nextBinding++;
			hasUbo = true;
		}
		else {
			VkDescriptorSetLayoutBinding binding = {};
			binding.binding = nextBinding;
			binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			binding.descriptorCount = 1;
			binding.stageFlags = descr.shaderStages.to_ulong();

			bindings.push_back(binding);
			nextBinding++;
		}
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = bindings.size();
	layoutCreateInfo.pBindings = bindings.data();

	VkDescriptorSetLayout setLayout;

	if (vkCreateDescriptorSetLayout(vkObjects.getDevice(), &layoutCreateInfo, nullptr, &setLayout) != VK_SUCCESS) {
		throw std::runtime_error("Could not create descriptor set layout");
	}

	descriptorLayouts.insert({name, setLayout});
}

VkShaderModule VkShaderLoader::createShaderModule(const std::string& filename) {
	std::vector<char> byteCode = loadFromDisk(filename);
	VkShaderModule module;

	VkShaderModuleCreateInfo moduleCreateInfo = {};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = byteCode.size();
	moduleCreateInfo.pCode = (const uint32_t*) byteCode.data();

	if (vkCreateShaderModule(vkObjects.getDevice(), &moduleCreateInfo, nullptr, &module) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module for file \"" + filename + "\"!");
	}

	return module;
}

std::vector<char> VkShaderLoader::loadFromDisk(const std::string& filename) {
	std::vector<char> fileData;

	try {
		std::ifstream inFile(filename, std::ios::ate | std::ios::binary);
		inFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		size_t fileSize = inFile.tellg();
		fileData.resize(fileSize);

		inFile.seekg(0);
		inFile.read(fileData.data(), fileSize);
	}
	catch (const std::ifstream::failure& e) {
		throw std::runtime_error("Failed to read file \"" + filename + "\"!");
	}

	return fileData;
}

std::vector<VkPushConstantRange> VkShaderLoader::convertToRanges(const PushConstantSet& pushSet) {
	std::vector<VkPushConstantRange> ranges;

	if (pushSet.pushConstants.empty()) {
		return ranges;
	}

	std::bitset<2> currentShaderStages = pushSet.pushConstants.front().shaderStages;

	VkPushConstantRange currentRange = {};
	currentRange.stageFlags = currentShaderStages.to_ulong();

	for (const UniformDescription& uniform : pushSet.pushConstants) {
		if (uniform.shaderStages != currentShaderStages) {
			ranges.push_back(currentRange);

			currentShaderStages = uniform.shaderStages;

			currentRange.stageFlags = currentShaderStages.to_ulong();
			currentRange.offset += currentRange.size;
			currentRange.size = 0;
		}

		currentRange.size += uniformSize(uniform.type);
	}

	ranges.push_back(currentRange);

	return ranges;
}
