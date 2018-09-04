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

#include "Std140Aligner.hpp"
#include "ExtraMath.hpp"

size_t Std140Aligner::getAlignedSize(const UniformSet& set) {
	size_t currentSize = 0;

	for (const UniformDescription& uniform : set.uniforms) {
		if (isSampler(uniform.type)) {
			continue;
		}

		//Increase size to account for padding
		currentSize = ExMath::roundToVal(currentSize, baseAlignment(uniform.type));
		currentSize += alignedSize(uniform.type);
	}

	return currentSize;
}

Std140Aligner::Std140Aligner(const std::vector<UniformDescription>& uniforms) :
	uniformData(nullptr),
	dataSize(0) {

	uint32_t currentOffset = 0;

	for (const UniformDescription& uniform : uniforms) {
		UniformData data = {};
		data.type = uniform.type;
		//Round the uniform's offset to the next multiple of its base alignment
		data.offset = ExMath::roundToVal(currentOffset, baseAlignment(uniform.type));
		//Array and matrix types have a larger size due to padding
		data.size = alignedSize(uniform.type);

		currentOffset = data.offset + data.size;

		uniformMap.insert({uniform.name, data});
	}

	uniformData = new unsigned char[currentOffset];
	dataSize = currentOffset;
}

Std140Aligner::Std140Aligner(const Std140Aligner& other) :
	uniformMap(other.uniformMap),
	uniformData(new unsigned char[other.dataSize]),
	dataSize(other.dataSize) {

	memcpy(uniformData, other.uniformData, dataSize);
}

void Std140Aligner::setMat3(const std::string& name, const glm::mat3& value) {
	checkType(name, UniformType::MAT3);

	const UniformData& data = uniformMap.at(name);
	uint32_t arrayOffset = 0;

	for (size_t i = 0; i < 3; i++) {
		memcpy(&uniformData[data.offset + arrayOffset], &value[i], sizeof(glm::vec3));
		arrayOffset += baseAlignment(UniformType::VEC3);
	}
}

void Std140Aligner::setMat4(const std::string& name, const glm::mat4& value) {
	checkType(name, UniformType::MAT4);

	const UniformData& data = uniformMap.at(name);
	uint32_t arrayOffset = 0;

	for (size_t i = 0; i < 4; i++) {
		memcpy(&uniformData[data.offset + arrayOffset], &value[i], sizeof(glm::vec4));
		arrayOffset += baseAlignment(UniformType::VEC4);
	}
}

glm::mat3 Std140Aligner::getMat3(const std::string& name) const {
	checkType(name, UniformType::MAT3);

	const UniformData& data = uniformMap.at(name);
	uint32_t arrayOffset = 0;
	glm::mat3 out;

	for (size_t i = 0; i < 3; i++) {
		memcpy(&out[i], &uniformData[data.offset + arrayOffset], sizeof(glm::vec3));
		arrayOffset += baseAlignment(UniformType::VEC3);
	}

	return out;
}

glm::mat4 Std140Aligner::getMat4(const std::string& name) const {
	checkType(name, UniformType::MAT4);

	const UniformData& data = uniformMap.at(name);
	uint32_t arrayOffset = 0;
	glm::mat4 out;

	for (size_t i = 0; i < 4; i++) {
		memcpy(&out[i], &uniformData[data.offset + arrayOffset], sizeof(glm::vec4));
		arrayOffset += baseAlignment(UniformType::VEC4);
	}

	return out;
}
