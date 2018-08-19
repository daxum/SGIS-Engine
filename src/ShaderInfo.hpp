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
#include <stdexcept>

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

constexpr uint32_t uniformSize(const UniformType type) {
	switch (type) {
		case UniformType::FLOAT: return sizeof(float);
		case UniformType::VEC2: return 2 * sizeof(float);
		case UniformType::VEC3: return 3 * sizeof(float);
		case UniformType::VEC4: return 4 * sizeof(float);
		case UniformType::MAT3: return 9 * sizeof(float);
		case UniformType::MAT4: return 16 * sizeof(float);
		default: throw std::runtime_error("Uniform size missing!");
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
	//This has special meaning for samplers - for static models,
	//it should be texture_*, where '*' is the index into the model's
	//texture array. For dynamic models, it is the name of the texture
	//to use, which will be static for all models using the descriptor set.
	std::string name;
	//Where the uniform's value comes from.
	UniformProviderType provider;
	//The shader stages that use the uniform.
	std::bitset<2> shaderStages;
};

//Types of uniform set, restricts where values can be pulled from.
enum class UniformSetType {
	//Model uniforms, can only use UniformProviderType::MATERIAL.
	//The dynamic / static determines the type of uniform buffer
	//the data is stored in, as well as the management of the
	//descriptor sets.
	MODEL_STATIC,
	MODEL_DYNAMIC,
	//Per-screen uniforms, allows use of CAMERA_* and SCREEN_* provider types.
	//Samplers are not allowed for PER_SCREEN uniform sets.
	PER_SCREEN,
	//Per-object uniforms, allows only OBJECT_* uniform providers.
	//Samplers are not allowed in PER_OBJECT uniform sets.
	PER_OBJECT
};

//A set of uniforms that can be used in a shader.
//Order in the uniforms vector determines bindings -
//The first non-sampler uniform will determine the
//uniform buffer's binding, and each sampler will
//be assigned to the next binding, in order.
struct UniformSet {
	//The type of uniform set, restricts which provider types are allowed.
	UniformSetType setType;
	//The maximum allowed users of this uniform set. Determines uniform
	//buffer sizes and, for MODEL_STATIC set types, the number of available
	//descriptor sets.
	size_t maxUsers;
	//The uniforms in the set.
	std::vector<UniformDescription> uniforms;
};

//A set of push constants. Be very careful with size here - the
//minimum required in vulkan is 128 bytes. These are probably
//best used with things that change frequently.
struct PushConstantSet {
	//Careful with order here - try to group same shader stages together.
	std::vector<UniformDescription> pushConstants;
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
	//In general, from the values in UniformSetType, prefer PER_SCREEN, then
	//MODEL_*, then PER_OBJECT.
	std::vector<std::string> uniformSets;
	//All push constant values used in the shader.
	PushConstantSet pushConstants;
};
