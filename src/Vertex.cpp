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

Vertex::Vertex(const Vertex& v) :
	format(v.format),
	vertexData(new unsigned char[v.getSize()]) {

	memcpy(vertexData, v.vertexData, v.getSize());
}

void Vertex::setDataVal(const std::string& name, VertexFormat::ElementType expectedType, const void* data) {
	if (!format->checkType(name, expectedType)) {
		throw std::runtime_error("Type for vertex element \"" + name + "\" doesn't match!");
	}

	size_t offset = format->getElementOffset(name);
	size_t dataSize = format->getElementSize(name);

	memcpy(vertexData + offset, data, dataSize);
}

void* Vertex::getDataVal(const std::string& name, VertexFormat::ElementType expectedType) {
	if (!format->checkType(name, expectedType)) {
		throw std::runtime_error("Type for vertex element \"" + name + "\" doesn't match!");
	}

	return vertexData + format->getElementOffset(name);
}
