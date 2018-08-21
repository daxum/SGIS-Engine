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

#include <vulkan/vulkan.h>

#include "ShaderInfo.hpp"
#include "VkPipelineCreateObject.hpp"

class VkShader {
public:
	//Push constants used in the shader.
	const PushConstantSet pushConstants;

	/**
	 * Constructor.
	 * @param device The logical device this shader is owned by.
	 * @param pipelineCache The pipeline cache to use when creating pipelines.
	 * @param pipelineLayout The pipeline layout object.
	 * @param pushConstants The push constants used in this shader.
	 * @param pipelineCreator The pipeline object used to recreate this shader.
	 */
	VkShader(VkDevice device, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, const PushConstantSet& pushConstants, const VkPipelineCreateObject& pipelineCreator);

	/**
	 * Destroys the pipeline and pipeline layout.
	 */
	~VkShader();

	/**
	 * Gets the shader's pipeline.
	 * @return The shader's pipeline.
	 */
	VkPipeline getPipeline() const { return pipeline; }

	/**
	 * Returns the shader's pipeline layout.
	 * @return The pipeline layout.
	 */
	VkPipelineLayout getPipelineLayout() const { return pipelineLayout; }

	/**
	 * Gets the offsets for the push constants used in the shader.
	 * @return A vector of push constant offsets, in the same order
	 *     as the vector in pushConstants.
	 */
	const std::vector<uint32_t>& getPushConstantOffsets() const { return pushOffsets; }

	/**
	 * Gets the render pass for the shader, which is determined by its pipeline.
	 * @return The render pass the shader is in.
	 */
	RenderPass getRenderPass() const { return pipelineCreator.getRenderPass(); }

	/**
	 * Recreates the shader's pipeline and frees the old pipeline.
	 */
	void reload() {
		vkDestroyPipeline(device, pipeline, nullptr);
		pipeline = pipelineCreator.createPipeline(pipelineCache, pipelineLayout);
	}

private:
	//Used for deleting.
	VkDevice device;
	VkPipelineLayout pipelineLayout;

	//Pipeline cache object.
	VkPipelineCache pipelineCache;
	//The pipeline representing the shader, used by the rendering engine.
	VkPipeline pipeline;
	//Object used to create this shader's pipeline, used to reload the shader.
	VkPipelineCreateObject pipelineCreator;

	//Offsets for the different push constants.
	std::vector<uint32_t> pushOffsets;
};
