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

#include <stdexcept>

#include "Vertex.hpp"
#include "VertexBuffer.hpp"

Vertex::Vertex(VertexBuffer* parentBuffer, size_t vertexSize) :
	buffer(parentBuffer),
	size(vertexSize),
	vertexData(new unsigned char[vertexSize]) {

}

Vertex::Vertex(const Vertex& v) :
	buffer(v.buffer),
	size(v.size),
	vertexData(new unsigned char[v.size]) {

}

Vertex::Vertex(Vertex&& v) noexcept :
	buffer(std::exchange(v.buffer, nullptr)),
	size(std::exchange(v.size, 0)),
	vertexData(std::exchange(v.vertexData, nullptr)) {

}

Vertex::~Vertex() {
	if (vertexData) {
		delete[] vertexData;
	}
}

void Vertex::setData(const std::string& name, VertexElementType expectedType, const void* data) {
	if (!buffer->checkType(name, expectedType)) {
		throw std::runtime_error("Type for vertex element \"" + name + "\" doesn't match!");
	}

	size_t offset = buffer->getElementOffset(name);
	size_t dataSize = buffer->getElementSize(name);

	memcpy(vertexData + offset, data, dataSize);
}

void* Vertex::getData(const std::string& name, VertexElementType expectedType) {
	if (!buffer->checkType(name, expectedType)) {
		throw std::runtime_error("Type for vertex element \"" + name + "\" doesn't match!");
	}

	return vertexData + buffer->getElementOffset(name);
}
