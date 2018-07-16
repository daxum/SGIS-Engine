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

#include "Vertex.hpp"

//External representation of vertex elements, supplied to constructor.
struct VertexElement {
	//Name of the element.
	std::string name;
	//Type of the vertex element.
	VertexElementType type;
};

//Internal representation of vertex elements.
struct VertexElementData {
	//Type of the element.
	VertexElementType type;
	//offset (in bytes) from the start of the vertex.
	size_t offset;
	//Size of the element.
	size_t size;
};

class VertexBuffer {
public:
	/**
	 * Constructor.
	 * @param vertexFormat The layout of a single vertex in the buffer.
	 */
	VertexBuffer(const std::vector<VertexElement>& vertexFormat);

	/**
	 * Creates a vertex with this buffer as its parent. The created vertex
	 * should not be used after the buffer is destroyed.
	 * @return The created vertex.
	 */
	Vertex getVertex() {
		return Vertex(this, vertexSize);
	}

	/**
	 * Checks whether the type of the given name matches the provided type.
	 * @param name The name to get the type for.
	 * @param type The type to check.
	 * @return whether the types matched.
	 * @throw std::out_of_range if the name is not part of the buffer.
	 */
	bool checkType(const std::string& name, VertexElementType type) {
		return vertexElements.at(name).type == type;
	}

	/**
	 * Gets the offset (in bytes) into the vertex for the given name.
	 * @param name The name to get the offset for.
	 * @return The offset into the vertex for the element with the given name.
	 * @throw std::out_of_range if the name is not part of the buffer.
	 */
	size_t getElementOffset(const std::string& name) {
		return vertexElements.at(name).offset;
	}

	/**
	 * Gets the size (in bytes) of the element with the given name.
	 * @param name The name to get the size of.
	 * @return The size of the given element.
	 * @throw std::out_of_range if the name is not part of the buffer.
	 */
	size_t getElementSize(const std::string& name) {
		return vertexElements.at(name).size;
	}

private:
	//Vertex element map.
	std::unordered_map<std::string, VertexElementData> vertexElements;
	//Size of one vertex using this buffer's format.
	size_t vertexSize;
};
