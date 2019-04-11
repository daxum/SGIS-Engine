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

#include "Renderer/ShaderInfo.hpp"
#include "VkPipelineCreateObject.hpp"

struct PushRange {
	//The start of this range in push constant memory.
	uint32_t start;
	//The size of this range.
	uint32_t size;
	//The shader stages this range is used in.
	std::bitset<32> shaderStages;
	//Individual push constant offsets.
	std::vector<uint32_t> pushOffsets;
	//A vector of uniforms corresponding to the offsets.
	std::vector<UniformDescription> pushData;
};

class VkShader {
public:
	/**
	 * Constructor.
	 * @param device The logical device this shader is owned by.
	 * @param pipelineCache The pipeline cache to use when creating pipelines.
	 * @param pipelineLayout The pipeline layout object.
	 * @param pushConstants The push constants used in this shader.
	 * @param pipelineCreator The pipeline object used to recreate this shader.
	 * @param screenSet The screen-level descriptor set, or an empty string if there isn't one.
	 * @param objectSet The object-level descriptor set, or an empty string if there isn't one.
	 */
	VkShader(VkDevice device, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, const PushConstantSet& pushConstants, const VkPipelineCreateObject& pipelineCreator, const std::string& screenSet, const std::string& objectSet);

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
	 * Gets the ranges of push constants used in the shader. This includes memory
	 * layouts and shader stage usage.
	 * @return A vector of push constant ranges.
	 */
	const std::vector<PushRange>& getPushConstantRanges() const { return pushConstantRanges; }

	/**
	 * Gets the descriptor set used for per-screen values, or an empty string if there isn't one.
	 * @return The descriptor set used for per-screen values.
	 */
	const std::string& getPerScreenDescriptor() const { return screenSet; }

	/**
	 * Gets the per-object descriptor set, or an empty string if there isn't one.
	 * @return The per-object descriptor set.
	 */
	const std::string& getPerObjectDescriptor() const { return objectSet; }

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

	//Stores relevent information about push constant ranges.
	std::vector<PushRange> pushConstantRanges;

	//Screen and object descriptor sets, empty string if not present.
	std::string screenSet;
	std::string objectSet;

	/**
	 * Returns the base alignment of the uniform type for
	 * std430 rules, suitable for push constants.
	 * @param type The type of the uniform.
	 * @return The base alignment of the uniform type.
	 */
	static constexpr size_t getPushConstantAligment(const UniformType type) {
		switch (type) {
			case UniformType::FLOAT: return sizeof(float);
			case UniformType::VEC2: return 2 * sizeof(float);
			case UniformType::VEC3: return 4 * sizeof(float);
			case UniformType::VEC4: return 4 * sizeof(float);
			//Don't round to vec4 for std430.
			case UniformType::MAT3: return getPushConstantAligment(UniformType::VEC3);
			case UniformType::MAT4: return getPushConstantAligment(UniformType::VEC4);
			default: throw std::runtime_error("Invalid uniform type in push constant alignment!");
		}
	}
};
