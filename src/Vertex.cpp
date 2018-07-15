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

#include "Vertex.hpp"

Vertex::Vertex(const std::vector<VertexElement>& elements) :
	elements(elements),
	size(0),
	vertexData(nullptr) {

	size_t totalSize = 0;

	for (const VertexElement& element : elements) {
		totalSize += element.size;
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

size_t Vertex::getElementIndex(const std::string& name) {
	for (size_t i = 0; i < elements.size(); i++) {
		if (elements.at(i).name == name) {
			return i;
		}
	}

	return std::numeric_limits<size_t>::max();
}
