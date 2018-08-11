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

#include <string>
#include <bitset>
#include <vector>

//Render passes for the engine.
//Each shader is part of one render pass.
enum class RenderPass {
	OPAQUE,
	TRANSPARENT,
	TRANSLUCENT
};

//Used to determine where a shader uniform value comes from.
enum class UniformProviderType {
	//Projection matrix from the camera.
	CAMERA_PROJECTION,
	//View matrix from the camera.
	CAMERA_VIEW,
	//Retrieved from the screen state.
	SCREEN_STATE,
	//Model-view matrix for the rendered object.
	OBJECT_MODEL_VIEW,
	//Transform of the rendered object, separate from the view matrix.
	OBJECT_TRANSFORM,
	//Retrieved from the object state.
	OBJECT_STATE,
	//Retrieved from the object's model.
	MATERIAL
};

//Types of uniforms currently supported.
enum class UniformType {
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	MAT3,
	MAT4,
	SAMPLER_2D,
	SAMPLER_CUBE
};

constexpr bool isSampler(const UniformType type) {
	switch (type) {
		case UniformType::FLOAT:
		case UniformType::VEC2:
		case UniformType::VEC3:
		case UniformType::VEC4:
		case UniformType::MAT3:
		case UniformType::MAT4:
			return false;
		case UniformType::SAMPLER_2D:
		case UniformType::SAMPLER_CUBE:
			return true;
		default:
			throw std::runtime_error("Missing uniform type!");
	}
}

//Shader stages. This should match VkShaderStageFlagBits.
enum UniformShaderUsage {
	USE_VERTEX_SHADER = 0x00000001,
	USE_FRAGMENT_SHADER = 0x00000010
};

//Description of a single uniform value.
struct UniformDescription {
	//Type of the uniform.
	UniformType type;
	//The name of the uniform, primarily used to retrieve values.
	std::string name;
	//Where the uniform's value comes from.
	UniformProviderType provider;
	//The shader stages that use the uniform.
	std::bitset<2> shaderStages;
};

//Uniform buffers to pull set values from.
enum class UniformBufferType {
	//No uniform buffer used, for sets of samplers.
	NO_BUFFER,
	//Static model buffer, for all models loaded at startup.
	STATIC_MODEL,
	//Dynamic model buffer, for dynamic models (like text).
	DYNAMIC_MODEL,
	//Per frame buffer, for screen and camera state.
	PER_FRAME,
	//Per object buffer. Avoid this one if possible, prefer
	//push constants instead.
	PER_OBJECT
};

//A set of uniforms that can be used in a shader.
//Order in the uniforms vector determines bindings -
//The first non-sampler uniform will determine the
//uniform buffer's binding, and each sampler will
//be assigned to the next binding, in order.
//Push constants work in a similar fashion with regards
//to their offsets.
struct UniformSet {
	//Whether this uniform set will have its data stored in a buffer
	//or be pushed during rendering.
	bool pushConstantSet;
	//Which buffer to pull non-sampler uniform values from. This is
	//ignored for push constants.
	UniformBufferType bufferType;
	//The uniforms in the set. Try to group push constants with
	//same shader stages together!
	std::vector<UniformDescription> uniforms;
};

struct ShaderInfo {
	//Path to vertex shader.
	std::string vertex;
	//Path to fragment shader.
	std::string fragment;
	//The render pass the shader is part of.
	RenderPass pass;
	//The vertex buffer this shader takes its format from. This does not
	//restrict the shader to use only this buffer, other buffers with
	//the same format will also work.
	std::string buffer;
	//Names of all the uniform sets used in the shader. Order is very important
	//here, as it affects which uniforms need to be rebound when the shader changes.
	//In general, from the values in UniformBufferType, prefer PER_FRAME, then
	//*_MODEL, then PER_OBJECT. In addition, there can only be one push constant
	//set per shader.
	std::vector<std::string> uniformSets;
};
