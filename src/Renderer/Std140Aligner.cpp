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

size_t Std140Aligner::getAlignedSize(const UniformSet& set) {
	size_t currentSize = 0;

	for (const UniformDescription& uniform : set.getBufferedUniforms()) {
		if (uniform.count) {
			//Handle arrays
			currentSize = ExMath::roundToVal(ExMath::roundToVal<uint32_t>(currentSize, baseAlignment(uniform.type)), baseAlignment(UniformType::VEC4));
			size_t arrSize = ExMath::roundToVal(alignedSize(uniform.type), baseAlignment(UniformType::VEC4)) * uniform.count;

			currentSize += arrSize;
		}
		else {
			//Increase size to account for padding
			currentSize = ExMath::roundToVal<uint32_t>(currentSize, baseAlignment(uniform.type));
			currentSize += alignedSize(uniform.type);
		}
	}

	return currentSize;
}

Std140Aligner::Std140Aligner(const UniformList& uniforms) :
	uniformData(nullptr),
	dataSize(0) {

	uint32_t currentOffset = 0;

	for (const UniformDescription& uniform : uniforms) {
		//Round the uniform's offset to the next multiple of its base alignment
		uint32_t offset = ExMath::roundToVal(currentOffset, baseAlignment(uniform.type));
		//Matrix types have a larger size due to padding
		uint32_t size = alignedSize(uniform.type);

		if (uniform.count) {
			//Arrays have their base alignment and aligned size rounded up to that of vec4
			offset = ExMath::roundToVal(offset, baseAlignment(UniformType::VEC4));
			size = ExMath::roundToVal(size, baseAlignment(UniformType::VEC4));

			//This implicitly handles the padding on the end of the array
			size *= uniform.count;
		}

		UniformData data = {};
		data.type = uniform.type;
		data.count = uniform.count;
		data.offset = offset;
		data.size = size;

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

Std140Aligner::Std140Aligner(Std140Aligner&& other) :
	uniformMap(std::move(other.uniformMap)),
	uniformData(std::exchange(other.uniformData, nullptr)),
	dataSize(std::exchange(other.dataSize, 0)) {

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
	memcpy(&uniformData[data.offset], &value, sizeof(glm::mat4));
}

void Std140Aligner::setMat3Array(const std::string& name, size_t start, size_t count, const glm::mat3* value) {
	checkType(name, UniformType::MAT3);

	const UniformData& data = uniformMap.at(name);
	unsigned char* writeStart = uniformData + data.offset + start * alignedSize(UniformType::MAT3);

	//All a mat3 really is is an array of vec3s with padding
	for (size_t i = 0; i < count * 3; i++) {
		unsigned char* writePtr = writeStart + i * baseAlignment(UniformType::VEC4);
		const unsigned char* readPtr = ((const unsigned char*)value) + i * sizeof(glm::vec3);
		memcpy(writePtr, readPtr, sizeof(glm::vec3));
	}
}

void Std140Aligner::setMat4Array(const std::string& name, size_t start, size_t count, const glm::mat4* value) {
	checkType(name, UniformType::MAT4);

	const UniformData& data = uniformMap.at(name);
	memcpy(&uniformData[data.offset + start * sizeof(glm::mat4)], value, count * sizeof(glm::mat4));
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
