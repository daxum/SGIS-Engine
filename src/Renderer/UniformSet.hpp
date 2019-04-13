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
#include <bitset>

//Types of uniform set, restricts where values can be pulled from.
enum class UniformSetType {
	//Material uniforms, can only use UniformProviderType::MATERIAL.
	MATERIAL,
	//Per-screen uniforms, allows use of CAMERA_* and SCREEN_* provider types.
	//Samplers are not allowed for PER_SCREEN uniform sets (for now).
	PER_SCREEN,
	//Per-object uniforms, allows only OBJECT_* uniform providers.
	//Samplers are not allowed in PER_OBJECT uniform sets.
	PER_OBJECT
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
	//Retrieved from the object's material.
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

/**
 * Gets the size of a uniform type. Not all uniform types have sizes, so
 * don't call this with samplers or similar.
 * @param tyoe The type to get the size of.
 * @return The size of the type, in bytes.
 */
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
	//The name of the uniform, only used when loading materials.
	//For OpenGL, the name should match the name in the shader
	//(Disregard above if I bothered to move to 4.3).
	std::string name;
	//Where the uniform's value comes from.
	UniformProviderType provider;
	//The shader stages that use the uniform.
	std::bitset<32> shaderStages;
};

typedef std::vector<UniformDescription> UniformList;

class UniformSet {
public:
	/**
	 * Creates a uniform set using the provided uniform list. Also validates
	 * the given list, to make sure the uniforms have allowed providers.
	 * @param type The type of uniform set, used for validation.
	 * @param uniforms A list of uniforms in the set.
	 */
	UniformSet(UniformSetType type, size_t maxUsers, const UniformList& uniforms) :
		type(type),
		maxUsers(maxUsers) {

		for (UniformDescription uniform : uniforms) {
			//Pretty much everything here is intended to fallthrough.
			switch (uniform.type) {
				case UniformType::FLOAT:
				case UniformType::VEC2:
				case UniformType::VEC3:
				case UniformType::VEC4:
				case UniformType::MAT3:
				case UniformType::MAT4:
					bufferedUniforms.push_back(uniform); break;
				case UniformType::SAMPLER_2D:
				case UniformType::SAMPLER_CUBE:
					otherUniforms.push_back(uniform); break;
				default: throw std::runtime_error("Missing uniform type in UniformSet!");
			}
		}
	}

	/**
	 * Gets the type of the uniform set.
	 * @return The type of the set.
	 */
	UniformSetType getType() const { return type; }

	/**
	 * Gets the maximum allowed users for this uniform set.
	 * @return The max allowed users.
	 */
	size_t getMaxUsers() const { return maxUsers; }

	/**
	 * Gets all the buffered uniforms within the set.
	 * @return The buffered uniforms.
	 */
	const UniformList& getBufferedUniforms() const { return bufferedUniforms; }

	/**
	 * Gets all non-buffered uniforms within the set, such as samplers.
	 * @return The non-buffered uniforms in the set.
	 */
	const UniformList& getNonBufferedUniforms() const { return otherUniforms; }

private:
	//The type of the set. Used for validation of element providers.
	UniformSetType type;
	//The maximum allowed users of the uniform set.
	size_t maxUsers;
	//List of uniforms stored in an uniform buffer.
	UniformList bufferedUniforms;
	//List of uniforms not stored in an uniform buffer (like samplers).
	UniformList otherUniforms;
};
