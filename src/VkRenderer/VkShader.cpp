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

#include "VkShader.hpp"
#include "ExtraMath.hpp"

VkShader::VkShader(VkDevice device, VkPipelineCache pipelineCache, VkPipelineLayout pipelineLayout, const PushConstantSet& pushConstants, const VkPipelineCreateObject& pipelineCreator, const std::string& screenSet, const std::string& objectSet) :
	device(device),
	pipelineLayout(pipelineLayout),
	pipelineCache(pipelineCache),
	pipeline(pipelineCreator.createPipeline(pipelineCache, pipelineLayout)),
	pipelineCreator(pipelineCreator),
	screenSet(screenSet),
	objectSet(objectSet) {

	uint32_t offset = 0;
	PushRange currentRange = {};

	if (!pushConstants.pushConstants.empty()) {
		currentRange.shaderStages = pushConstants.pushConstants.front().shaderStages;
	}

	for (const UniformDescription& uniform : pushConstants.pushConstants) {
		//Shader stages differ, so start a new range
		if (uniform.shaderStages != currentRange.shaderStages) {
			//Set the size - this should not take into account the alignment of the next element
			currentRange.size = offset - currentRange.start;

			pushConstantRanges.push_back(currentRange);

			//Reset range structure for the next range.
			currentRange.start += currentRange.size;
			currentRange.shaderStages = uniform.shaderStages;
			currentRange.pushOffsets.clear();
			currentRange.pushData.clear();
		}

		ExMath::roundToVal(offset, getPushConstantAligment(uniform.type));

		currentRange.pushOffsets.push_back(offset);
		currentRange.pushData.push_back(uniform);

		//mat3 has size of mat4 due to vec3's alignment
		offset += uniform.type == UniformType::MAT3 ? uniformSize(UniformType::MAT4) : uniformSize(uniform.type);
	}

	//Add last push range
	currentRange.size = offset - currentRange.start;
	pushConstantRanges.push_back(currentRange);
}

VkShader::~VkShader() {
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
