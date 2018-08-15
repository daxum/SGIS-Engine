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

Std140Aligner::Std140Aligner(const std::vector<UniformDescription>& uniforms) :
	uniformData(nullptr) {

	uint32_t currentOffset = 0;

	for (const UniformDescription& uniform : uniforms) {
		UniformData data = {};
		data.type = uniform.type;
		//Round the uniform's offset to the next multiple of its base alignment
		data.offset = roundToVal(currentOffset, baseAlignment(uniform.type));
		//Array and matrix types have a larger size due to padding
		data.size = alignedSize(uniform.type);

		currentOffset = data.offset + data.size;

		uniformMap.insert({uniform.name, data});
	}

	uniformData = new unsigned char[currentOffset];
}

void Std140Aligner::setFloat(const std::string& name, const float value) {
	checkType(name, UniformType::FLOAT);

	const UniformData& data = uniformMap.at(name);
	memcpy(&uniformData[data.offset], &value, sizeof(float));
}

void Std140Aligner::setVec2(const std::string& name, const glm::vec2& value) {
	checkType(name, UniformType::VEC2);

	const UniformData& data = uniformMap.at(name);
	memcpy(&uniformData[data.offset], &value, sizeof(glm::vec2));
}

void Std140Aligner::setVec3(const std::string& name, const glm::vec3& value) {
	checkType(name, UniformType::VEC3);

	const UniformData& data = uniformMap.at(name);
	memcpy(&uniformData[data.offset], &value, sizeof(glm::vec3));
}

void Std140Aligner::setVec4(const std::string& name, const glm::vec4& value) {
	checkType(name, UniformType::VEC4);

	const UniformData& data = uniformMap.at(name);
	mempcpy(&uniformData[data.offset], &value, sizeof(glm::vec4));
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
