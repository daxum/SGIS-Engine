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
#include <unordered_map>

#include <glm/glm.hpp>

#include "ShaderInfo.hpp"

//Information on how uniforms are stored in the memory buffer.
struct UniformData {
	//Type of the uniform.
	UniformType type;
	//Offset into the buffer, taking alignment into account.
	uint32_t offset;
	//Size of the aligned object. This is different than the size
	//of the type in some cases.
	uint32_t size;
};

//Aligns a uniform block to the std140 layout for use in uniform buffers.
//Rules taken from OpenGL 4.6 core profile specification, section 7.6.2.2.
class Std140Aligner {
public:
	/**
	 * Finds the aligned size of the given uniform set, similarly to what happens in the constructor.
	 * @param set The uniform set to find the size of.
	 * @return The aligned size of the uniform set.
	 */
	static size_t getAlignedSize(const UniformSet& set);

	/**
	 * Constructs the aligned memory region.
	 * @param uniforms The uniforms that will be stored. Samplers and
	 *     similar aren't allowed, for obvious reasons.
	 */
	Std140Aligner(const std::vector<UniformDescription>& uniforms);

	/**
	 * Turns out models need to be copied, and not having this function
	 * causes error messages longer than the terminal buffer.
	 * @param other The aligner to copy.
	 */
	Std140Aligner(const Std140Aligner& other);

	/**
	 * Just a destructor.
	 */
	~Std140Aligner() {
		if (uniformData != nullptr) {
			delete[] uniformData;
		}
	}

	/**
	 * Implement these later if needed, this is to prevent
	 * uniformData from being shallow copied.
	 */
	Std140Aligner(Std140Aligner&&) = delete;
	Std140Aligner& operator=(const Std140Aligner&) = delete;
	Std140Aligner& operator=(Std140Aligner&&) = delete;

	/**
	 * Setters for uniform values. These functions check the uniform's type,
	 * align the provided object, and then copy the aligned data into the
	 * memory buffer.
	 * @param name The name of the uniform to set.
	 * @param value The value to set the uniform to.
	 */
	void setFloat(const std::string& name, const float value);
	void setVec2(const std::string& name, const glm::vec2& value);
	void setVec3(const std::string& name, const glm::vec3& value);
	void setVec4(const std::string& name, const glm::vec4& value);
	void setMat3(const std::string& name, const glm::mat3& value);
	void setMat4(const std::string& name, const glm::mat4& value);

	/**
	 * Returns the aligned uniform data along with its size.
	 * @return A pair of the uniform data and its size.
	 */
	const std::pair<const unsigned char*, size_t> getData() const { return {uniformData, dataSize}; }

	/**
	 * Checks whether a uniform is present with the given name and type.
	 * @param name The name of the uniform.
	 * @param type The expected type of the uniform.
	 * @return Whether the uniform with the given name exists and has the specified type.
	 */
	bool hasUniform(const std::string& name, const UniformType type) const { return uniformMap.count(name) && uniformMap.at(name).type == type; }

private:
	//Map of uniforms, for fast retrieval.
	std::unordered_map<std::string, UniformData> uniformMap;
	//Raw aligned data for the uniforms.
	unsigned char* uniformData;
	//Size of uniformData.
	size_t dataSize;

	/**
	 * Checks whether the type of the uniform with the given name matches
	 * the provided type.
	 * @param name The name of the uniform to check.
	 * @param type The expected type of the uniform.
	 */
	void checkType(const std::string& name, const UniformType type) const {
		if (uniformMap.at(name).type != type) {
			throw std::runtime_error("Invalid type in Std140Aligner!");
		}
	}

	/**
	 * Calculates the size of the data type once it has been properly aligned.
	 * This is mainly useful for arrays and matrices, because the stride is a
	 * bit odd.
	 * @param type The type to get the size of.
	 * @return The size of the type. Note that this does not necessarily determine
	 *     the offset of the next element, due to alignment restrictions.
	 */
	static constexpr uint32_t alignedSize(const UniformType type) {
		switch (type) {
			case UniformType::FLOAT: return sizeof(float);
			case UniformType::VEC2: return 2 * sizeof(float);
			case UniformType::VEC3: return 3 * sizeof(float);
			case UniformType::VEC4: return 4 * sizeof(float);
			//Calculated by: roundToVal(alignedSize(UniformType::VEC3), alignedSize(UniformType::VEC4))
			case UniformType::MAT3: return 3 * alignedSize(UniformType::VEC4);
			case UniformType::MAT4: return 4 * alignedSize(UniformType::VEC4);
			default: throw std::runtime_error("Invalid uniform type provided to alignedSize!");
		}
	}

	/**
	 * Calculates the base alignment of the provided type. When used with
	 * alignedSize, this should help reduce the complexity of aligning
	 * elements, especially in cases like floats following vec3s.
	 * Still might not be a good idea to have vec3s in opengl shaders,
	 * though, because apparently some drivers handle that wrong.
	 * @param type The type to get the base alignment for.
	 * @return The base alignment for the type.
	 */
	static constexpr uint32_t baseAlignment(const UniformType type) {
		switch (type) {
			case UniformType::FLOAT: return sizeof(float);
			case UniformType::VEC2: return 2 * sizeof(float);
			case UniformType::VEC3: return 4 * sizeof(float);
			case UniformType::VEC4: return 4 * sizeof(float);
			//Calculated by: roundToVal(baseAlignment(UniformType::VEC3), baseAlignment(UniformType::VEC4))
			case UniformType::MAT3: return baseAlignment(UniformType::VEC4);
			case UniformType::MAT4: return baseAlignment(UniformType::VEC4);
			default: throw std::runtime_error("Invalid uniform type provided to baseAlignment!");
		}
	}
};
