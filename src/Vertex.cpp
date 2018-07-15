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

#include <limits>
#include <stdexcept>

#include "Vertex.hpp"

Vertex::Vertex(const std::vector<VertexElement>& elements) :
	elements(elements),
	size(0),
	vertexData(nullptr) {

	size_t totalSize = 0;

	for (const VertexElement& element : elements) {
		totalSize += sizeFromVertexType(element.type);
	}

	vertexData = new unsigned char[totalSize];
	size = totalSize;
}

Vertex::Vertex(const Vertex& v) :
	elements(v.elements),
	size(0),
	vertexData(new unsigned char[v.size]) {

	size = v.size;
}

Vertex::Vertex(Vertex&& v) noexcept :
	elements(std::move(v.elements)),
	size(std::exchange(v.size, 0)),
	vertexData(std::exchange(v.vertexData, nullptr)) {

}

Vertex::~Vertex() {
	if (vertexData) {
		delete[] vertexData;
	}
}

void Vertex::setFloat(const std::string& name, float value) {
	if (!checkType(name, VertexElementType::FLOAT)) {
		throw std::runtime_error("Vertex element type doesn't match!");
	}

	setData(name, &value);
}

void Vertex::setVec2(const std::string& name, const glm::vec2& value) {
	if (!checkType(name, VertexElementType::VEC2)) {
		throw std::runtime_error("Vertex element type doesn't match!");
	}

	setData(name, &value);
}

void Vertex::setVec3(const std::string& name, const glm::vec3& value) {
	if (!checkType(name, VertexElementType::VEC3)) {
		throw std::runtime_error("Vertex element type doesn't match!");
	}

	setData(name, &value);
}

void Vertex::setVec4(const std::string& name, const glm::vec4& value) {
	if (!checkType(name, VertexElementType::VEC4)) {
		throw std::runtime_error("Vertex element type doesn't match!");
	}

	setData(name, &value);
}

size_t Vertex::getElementIndex(const std::string& name) {
	for (size_t i = 0; i < elements.size(); i++) {
		if (elements.at(i).name == name) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}

size_t Vertex::getElementOffset(const std::string& name) {
	size_t offset = 0;

	for (const VertexElement& element : elements) {
		if (element.name == name) {
			return offset;
		}

		offset += sizeFromVertexType(element.type);
	}

	return std::numeric_limits<size_t>::max();
}

bool Vertex::checkType(const std::string& name, VertexElementType type) {
	for (const VertexElement& element : elements) {
		if (element.name == name) {
			if (element.type == type) {
				return true;
			}

			return false;
		}
	}

	return false;
}

void Vertex::setData(const std::string& name, const void* data) {
	size_t offset = getElementOffset(name);
	size_t index = getElementIndex(name);

	if (index == std::numeric_limits<size_t>::max() || offset == std::numeric_limits<size_t>::max()) {
		throw std::runtime_error("Vertex element \"" + name + "\" doesn't exist");
	}

	size_t dataSize = sizeFromVertexType(elements.at(index).type);

	memcpy(vertexData + offset, data, dataSize);
}
