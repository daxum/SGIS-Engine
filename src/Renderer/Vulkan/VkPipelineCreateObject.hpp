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

#include "VkObjectHandler.hpp"
#include "VkRenderObjects.hpp"
#include "Renderer/ShaderInfo.hpp"

class VkPipelineCreateObject {
public:
	/**
	 * Creates a pipeline creator.
	 * @param objectHandler The vulkan object handler.
	 * @param renderObjects The swapchain handler, for the extent.
	 * @param moduleInfos An array of the shader modules used in the pipeline.
	 * @param renderPass The render pass the pipeline is in - this is for transparency, not VkRenderPass.
	 * @param format The format of the vertex input data.
	 */
	VkPipelineCreateObject(VkObjectHandler& objectHandler, VkRenderObjects& renderObjects, const std::vector<VkPipelineShaderStageCreateInfo>& moduleInfos, RenderPass renderPass, const VertexFormat& format);

	/**
	 * Copy constructor.
	 */
	VkPipelineCreateObject(const VkPipelineCreateObject& other);

	/**
	 * Creates a graphics pipeline using the provided cache and layout.
	 * @param pipelineCache The cache to use to hopefully speed up pipeline creation.
	 * @param pipelineLayout The layout of the pipeline to create.
	 */
	VkPipeline createPipeline(VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout) const;

	/**
	 * Gets the render pass for the pipeline.
	 * @return The render pass.
	 */
	RenderPass getRenderPass() const { return renderPass; }

private:
	//Vulkan object handler.
	VkObjectHandler& objectHandler;
	//Render object handler.
	VkRenderObjects& renderObjects;
	//The render pass for the pipeline, determines blend state.
	RenderPass renderPass;
	//Vertex format object.
	const VertexFormat& buffer;

	//Structures needed to create a pipeline, and that shouldn't change whenever
	//the pipeline needs to be recreated.
	std::vector<VkPipelineShaderStageCreateInfo> moduleInfos;
	VkVertexInputBindingDescription bindingDescription;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo assemblyCreateInfo;
	VkPipelineRasterizationStateCreateInfo rasterizeCreateInfo;
	VkPipelineMultisampleStateCreateInfo sampleCreateInfo;
	VkPipelineDepthStencilStateCreateInfo depthInfo;
	VkPipelineColorBlendAttachmentState blendAttach;
	VkPipelineColorBlendStateCreateInfo blendStateCreateInfo;

	/**
	 * Fills in the persistent structs above. This helps avoid bad pointers
	 * when copy constructed.
	 */
	void fillPersistentStructs();

	/**
	 * Creates attribute descriptions for the passed in vertex format.
	 * @param format The format of the vertex data.
	 * @return A vector of attribute description structures.
	 */
	std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescription(const VertexFormat& format) const;

	/**
	 * Converts the type to a VkFormat.
	 * @param type The type to convert.
	 * @return The corresponding format.
	 */
	static constexpr VkFormat formatFromVertexType(const VertexFormat::ElementType type) {
		switch (type) {
			case VertexFormat::ElementType::FLOAT: return VK_FORMAT_R32_SFLOAT;
			case VertexFormat::ElementType::VEC2: return VK_FORMAT_R32G32_SFLOAT;
			case VertexFormat::ElementType::VEC3: return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexFormat::ElementType::VEC4: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case VertexFormat::ElementType::UINT32: return VK_FORMAT_R32_UINT;
			default: return VK_FORMAT_UNDEFINED;
		}
	}
};
