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
#include <memory>

#include "Vertex.hpp"

enum class BufferUsage {
	//Reference counted buffer located in gpu memory, if possible. Meshes remain in
	//the buffer even if nothing references them, for possible reuse later.
	//They are only evicted if memory runs out.
	DEDICATED_LAZY,
	//Similar to above, except unused meshes are immediately invalidated and
	//have to be reuploaded if used again.
	DEDICATED_SINGLE,
	//This is just a stream buffer.
	STREAM
};

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

struct RenderBufferData {
	virtual ~RenderBufferData() {}
};

class VertexBuffer {
public:
	/**
	 * Constructor.
	 * @param vertexFormat The layout of a single vertex in the buffer.
	 * @param size The size of the buffer.
	 * @param usage Where the buffer is stored, and how meshes stored in it should be deleted.
	 * @param renderData A renderer-specific pointer to buffer data (VkBuffer, GLuint, etc).
	 */
	VertexBuffer(const std::vector<VertexElement>& vertexFormat, size_t size, BufferUsage usage, std::shared_ptr<RenderBufferData> renderData);

	/**
	 * Creates a vertex with this buffer as its parent. The created vertex
	 * should not be used after the buffer is destroyed.
	 * @return The created vertex.
	 */
	Vertex getVertex() {
		return Vertex(this, vertexSize);
	}

	/**
	 * Retrieves the stored renderData.
	 * @return The renderData.
	 */
	std::shared_ptr<RenderBufferData> getRenderData() { return renderData; }

	/**
	 * Gets the BufferUsage provided in the constructor.
	 * @return The usage for the buffer.
	 */
	BufferUsage getUsage() { return usage; }

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

	/**
	 * Gets the size of one vertex in the buffer.
	 * @return The size of a single vertex.
	 */
	size_t getVertexSize() { return vertexSize; }

private:
	//Vertex element map.
	std::unordered_map<std::string, VertexElementData> vertexElements;
	//Size of one vertex using this buffer's format.
	size_t vertexSize;
	//Size of the vertex buffer.
	size_t bufferSize;
	//How the buffer is used.
	BufferUsage usage;
	//Renderer specific data.
	std::shared_ptr<RenderBufferData> renderData;
};
