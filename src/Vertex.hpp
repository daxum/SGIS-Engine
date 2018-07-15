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

#include <string>
#include <vector>
#include <cstring>
#include <utility>

#include <glm/glm.hpp>

enum class VertexElementType {
	FLOAT,
	VEC2,
	VEC3,
	VEC4
};

size_t sizeFromVertexType(VertexElementType type) {
	switch(type) {
		case VertexElementType::FLOAT: return sizeof(float);
		case VertexElementType::VEC2: return sizeof(glm::vec2);
		case VertexElementType::VEC3: return sizeof(glm::vec3);
		case VertexElementType::VEC4: return sizeof(glm::vec4);
		default: return 0;
	}
}

struct VertexElement {
	//Type of the vertex element.
	VertexElementType type;
	//Name of the element.
	std::string name;

	friend bool operator==(const VertexElement& e1, const VertexElement& e2) {
		return e1.type == e2.type && e1.name == e2.name;
	}
};

class Vertex {
public:
	/**
	 * Constructor.
	 * @param elements A list of vertex elements that will be placed in a buffer.
	 */
	Vertex(const std::vector<VertexElement>& elements);

	/**
	 * Copy constructor.
	 */
	Vertex(const Vertex& v);

	/**
	 * Move constructor.
	 */
	Vertex(Vertex&& v) noexcept;

	/**
	 * Deletes the vertex data.
	 */
	~Vertex();

	/**
	 * "Setting" functions, sets the value of the name in the data buffer to the provided value.
	 * @param name The name to set.
	 * @param value The value to set the name to.
	 * @throw std::runtime_error if the name doesn't exist in the element array or if the type
	 *     for the name doesn't match the function's type.
	 */
	void setFloat(const std::string& name, float value);
	void setVec2(const std::string& name, const glm::vec2& value);
	void setVec3(const std::string& name, const glm::vec3& value);
	void setVec4(const std::string& name, const glm::vec4& value);

	/**
	 * Returns the vertex data for copying into a buffer, along with its size.
	 */
	std::pair<const unsigned char*, size_t> getData() const { return {vertexData, size}; }

	/**
	 * Returns whether the two vertices are equal.
	 */
	friend bool operator==(const Vertex& v1, const Vertex& v2) {
		//memcmp returns zero if blocks match.
		return v1.size == v2.size && v1.elements == v2.elements && !memcmp(v1.vertexData, v2.vertexData, v1.size);
	}

	/**
	 * Who knows what this does.
	 */
	friend bool operator!=(const Vertex& v1, const Vertex& v2) {
		return !operator==(v1, v2);
	}

	/**
	 * Copy assignment.
	 */
	Vertex& operator=(const Vertex& v) {
		if (this != &v) {
			if (size != v.size) {
				delete[] vertexData;
				size = 0;
				vertexData = nullptr;
				vertexData = new unsigned char[v.size];
				size = v.size;
			}

			elements = v.elements;
			memcpy(vertexData, v.vertexData, size);
		}

		return *this;
	}

	/**
	 * Move assignment.
	 */
	Vertex& operator=(Vertex&& v) noexcept {
		if (this != &v) {
			delete[] vertexData;
			vertexData = std::exchange(v.vertexData, nullptr);
			size = std::exchange(v.size, 0);
			elements = std::move(v.elements);
		}

		return *this;
	}

private:
	//The vertex format
	std::vector<VertexElement> elements;
	//Size of vertex, in bytes. Also size of below memory block.
	size_t size;
	//The data for this vertex. Originally this was going to use template metaprogramming
	//to auto-generate a vertex struct for each format, but that turned out to be too
	//impractical, so now we're throwing safety to the wind and doing it this way instead.
	//Don't forget to free!
	unsigned char* vertexData;

	/**
	 * Find an element's index in the element array given the name.
	 * @param name The name to find.
	 * @return The index of the element, or the maximum value of size_t if not found.
	 */
	size_t getElementIndex(const std::string& name);

	/**
	 * Gets the offset (in bytes) into the vertexData buffer for the given name.
	 * @param name The name to get the offset for.
	 * @return The offset into the buffer where the data for the name is stored,
	 *     or the maximum value of size_t if not found.
	 */
	size_t getElementOffset(const std::string& name);

	/**
	 * Checks whether the type of the given name matches the provided type.
	 * @param name The name to get the type for.
	 * @param type The type to check.
	 * @return whether the types matched.
	 */
	bool checkType(const std::string& name, VertexElementType type);

	/**
	 * Helper function for the set* functions above. Directly copies the data
	 * into the data buffer.
	 * @param name The element's name.
	 * @param data The data to copy. Size inferred from name's entry in elements vector.
	 * @throw std::runtime_error if name not found.
	 */
	void setData(const std::string& name, const void* data);
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(const Vertex& v) const {
			std::pair<const unsigned char*, size_t> vertexData = v.getData();
			size_t blocks = vertexData.second >> 3;
			size_t extra = vertexData.second - (blocks << 3);

			size_t hashedVal = 0;

			for (size_t i = 0; i < blocks; i++) {
				hashedVal ^= ((const uint64_t*)vertexData.first)[i] << 1;
			}

			if (extra) {
				uint64_t extraData = 0;
				memcpy(&extraData, &((const uint64_t*)vertexData.first)[blocks], extra);

				hashedVal ^= extraData << 1;
			}

			return hashedVal;
		}
	};
}
