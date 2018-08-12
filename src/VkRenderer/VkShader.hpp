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

class VkShader {
public:
	//The pipeline representing the shader, used by the rendering engine.
	const VkPipeline pipeline;
	//Push constants used in the shader.
	const PushConstantSet pushConstants;

	/**
	 * Constructor.
	 * @param device The logical device this shader is owned by.
	 * @param pipeline The pipeline object created by the shader loader.
	 * @param pipelineLayout The pipeline layout object.
	 * @param pushConstants The push constants used in this shader.
	 */
	VkShader(VkDevice device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, const PushConstantSet& pushConstants);

	/**
	 * Destroys the pipeline and pipeline layout.
	 */
	~VkShader();

private:
	//Used for deleting.
	VkDevice device;
	VkPipelineLayout pipelineLayout;
};
