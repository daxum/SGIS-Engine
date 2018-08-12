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

#include "VkPipelineCreateObject.hpp"

VkPipelineCreateObject::VkPipelineCreateObject(VkObjectHandler& objectHandler, const std::vector<VkPipelineShaderStageCreateInfo>& moduleInfos, RenderPass renderPass, const VertexBuffer& buffer) :
	objectHandler(objectHandler),
	renderPass(renderPass),
	buffer(buffer),
	moduleInfos(moduleInfos),
	bindingDescription(),
	vertexInputInfo(),
	assemblyCreateInfo(),
	rasterizeCreateInfo(),
	sampleCreateInfo(),
	blendAttach(),
	blendStateCreateInfo() {

	fillPersistentStructs();
}

VkPipelineCreateObject::VkPipelineCreateObject(const VkPipelineCreateObject& other) :
	objectHandler(other.objectHandler),
	renderPass(other.renderPass),
	buffer(other.buffer),
	moduleInfos(other.moduleInfos),
	bindingDescription(),
	vertexInputInfo(),
	assemblyCreateInfo(),
	rasterizeCreateInfo(),
	sampleCreateInfo(),
	blendAttach(),
	blendStateCreateInfo() {

	fillPersistentStructs();
}

VkPipeline VkPipelineCreateObject::createPipeline(VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout) const {
	const VkExtent2D& swapchainExtent = objectHandler.getSwapchainExtent();

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

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = moduleInfos.size();
	pipelineCreateInfo.pStages = moduleInfos.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &assemblyCreateInfo;
	pipelineCreateInfo.pViewportState = &viewStateCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizeCreateInfo;
	pipelineCreateInfo.pMultisampleState = &sampleCreateInfo;
	pipelineCreateInfo.pDepthStencilState = nullptr; //TODO
	pipelineCreateInfo.pColorBlendState = &blendStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = objectHandler.getRenderPass();
	pipelineCreateInfo.subpass = 0;

	VkPipeline graphicsPipeline;

	if (vkCreateGraphicsPipelines(objectHandler.getDevice(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	return graphicsPipeline;
}

void VkPipelineCreateObject::fillPersistentStructs() {
	//Might need multiple of these later
	bindingDescription.binding = 0;
	bindingDescription.stride = buffer.getVertexSize();
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attributeDescriptions = getVertexAttributeDescription(buffer);

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	assemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	assemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

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
	sampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	sampleCreateInfo.sampleShadingEnable = VK_FALSE;
	sampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	//No depth / stencil yet.

	blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttach.blendEnable = (renderPass == RenderPass::TRANSLUCENT) ? VK_TRUE : VK_FALSE;
	blendAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttach.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttach.alphaBlendOp = VK_BLEND_OP_ADD;

	blendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateCreateInfo.logicOpEnable = VK_FALSE;
	blendStateCreateInfo.attachmentCount = 1;
	blendStateCreateInfo.pAttachments = &blendAttach;
}

std::vector<VkVertexInputAttributeDescription> VkPipelineCreateObject::getVertexAttributeDescription(const VertexBuffer& buffer) const {
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
