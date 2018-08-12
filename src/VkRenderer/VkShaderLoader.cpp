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
	vkObjects(vkObjects) {

}

VkShaderLoader::~VkShaderLoader() {
	for (const auto& layout : descriptorLayouts) {
		vkDestroyDescriptorSetLayout(vkObjects.getDevice(), layout.second, nullptr);
	}
}

void VkShaderLoader::loadShader(std::string name, const ShaderInfo& info) {
	logger.debug("Loading shader \"" + name + "\"");

	VkDevice device = vkObjects.getDevice();
	const VkExtent2D& swapchainExtent = vkObjects.getSwapchainExtent();

	const VkShaderModule vertShader = createShaderModule(info.vertex);
	const VkShaderModule fragShader = createShaderModule(info.fragment);

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

	VkPipelineShaderStageCreateInfo infos[] = { vertCreateInfo, fragCreateInfo };

	const VertexBuffer& buffer = memoryManager->getBuffer(info.buffer);

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = buffer.getVertexSize();
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = getVertexAttributeDescription(buffer);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo = {};
	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapchainExtent.width;
	viewport.height = (float) swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = swapchainExtent;

	VkPipelineViewportStateCreateInfo viewStateCreateInfo = {};
	viewStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewStateCreateInfo.viewportCount = 1;
	viewStateCreateInfo.pViewports = &viewport;
	viewStateCreateInfo.scissorCount = 1;
	viewStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizeCreateInfo = {};
	rasterizeCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizeCreateInfo.depthClampEnable = VK_FALSE;
	rasterizeCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizeCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizeCreateInfo.lineWidth = 1.0f;
	//TODO: these two don't look right.
	rasterizeCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizeCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizeCreateInfo.depthBiasEnable = VK_FALSE;

	//Enable later?
	VkPipelineMultisampleStateCreateInfo sampleCreateInfo = {};
	sampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	sampleCreateInfo.sampleShadingEnable = VK_FALSE;
	sampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//No depth / stencil yet.

	VkPipelineColorBlendAttachmentState blendAttach = {};
	blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttach.blendEnable = (info.pass == RenderPass::TRANSLUCENT) ? VK_TRUE : VK_FALSE;
	blendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttach.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttach.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo blendStateCreateInfo = {};
	blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateCreateInfo.logicOpEnable = VK_FALSE;
	blendStateCreateInfo.attachmentCount = 1;
	blendStateCreateInfo.pAttachments = &blendAttach;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicCreateInfo = {};
	dynamicCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicCreateInfo.dynamicStateCount = 2;
	dynamicCreateInfo.pDynamicStates = dynamicStates;

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

	if (vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout for shader \"" + name + "\"");
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = infos;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &assemblyCreateInfo;
	pipelineCreateInfo.pViewportState = &viewStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizeCreateInfo;
	pipelineCreateInfo.pMultisampleState = &sampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr; //TODO
	pipelineCreateInfo.pColorBlendState = &blendStateCreateInfo;
	pipelineCreateInfo.pDynamicState = &dynamicCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = vkObjects.getRenderPass();
	pipelineCreateInfo.subpass = 0;

	VkPipeline graphicsPipeline;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline for shader \"" + name + "\"!");
	}

	//Add to shader map
	shaderMap.insert({name, std::make_shared<VkShader>(device, graphicsPipeline, pipelineLayout, info.pushConstants)});

	vkDestroyShaderModule(device, vertShader, nullptr);
	vkDestroyShaderModule(device, fragShader, nullptr);

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

std::vector<VkVertexInputAttributeDescription> VkShaderLoader::getVertexAttributeDescription(const VertexBuffer& buffer) const {
	std::vector<VkVertexInputAttributeDescription> descriptions;
	const std::vector<VertexElement>& bufferFormat = buffer.getVertexFormat();

	descriptions.reserve(bufferFormat.size());
	uint32_t location = 0;

	for (const VertexElement& element : bufferFormat) {
		VkVertexInputAttributeDescription descr = {};
		descr.binding = 0;
		descr.location = location;
		descr.format = formatFromVertexType(element.type);
		descr.offset = buffer.getElementOffset(element.name);

		descriptions.push_back(descr);
		location++;
	}

	return descriptions;
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
