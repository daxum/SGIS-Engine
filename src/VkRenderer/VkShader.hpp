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

#include "ShaderInterface.hpp"

class VkShader : public ShaderInterface {
public:
	//The pipeline representing the shader, used by the rendering engine.
	const VkPipeline pipeline;

	/**
	 * Constructor.
	 * @param device The logical device this shader is owned by.
	 * @param pipeline The pipeline object created by the shader loader.
	 * @param pipelineLayout The pipeline layout object.
	 */
	VkShader(VkDevice device, VkPipeline pipeline, VkPipelineLayout pipelineLayout);

	/**
	 * Destroys the pipeline and pipeline layout.
	 */
	~VkShader();

	/**
	 * Binds the shader for use in rendering.
	 */
	void bind() override;

	/**
	 * Sets a uniform variable.
	 * @param type The type of the uniform.
	 * @param name The name of the uniform in the shader.
	 * @param data The value to set the uniform data to.
	 */
	void setUniform(UniformType type, const std::string& name, const void* data) override;

	/**
	 * Sets the texture at the given index to the provided name.
	 * @param name The name of the texture to bind.
	 * @param index The texture unit to bind the texture to.
	 */
	void setTexture(const std::string& name, unsigned int index) override;

private:
	//Used for deleting.
	VkDevice device;
	VkPipelineLayout pipelineLayout;
};
