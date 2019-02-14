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

class VertexBuffer;

//Some vertex element names, to prevent mistyping.
const std::string VERTEX_ELEMENT_POSITION = "pos";
const std::string VERTEX_ELEMENT_NORMAL = "nor";
const std::string VERTEX_ELEMENT_TEXTURE = "tex";

enum class VertexElementType {
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	UINT32,
};

constexpr size_t sizeFromVertexType(const VertexElementType type) {
	switch(type) {
		case VertexElementType::FLOAT: return sizeof(float);
		case VertexElementType::VEC2: return sizeof(glm::vec2);
		case VertexElementType::VEC3: return sizeof(glm::vec3);
		case VertexElementType::VEC4: return sizeof(glm::vec4);
		case VertexElementType::UINT32: return sizeof(uint32_t);
		default: return 0;
	}
}

class Vertex {
public:
	/**
	 * Constructor.
	 * @param parentBuffer The buffer that owns this vertex - determines vertex format.
	 * @param vertexSize The size of one vertex in the parent buffer.
	 */
	Vertex(VertexBuffer* parentBuffer, size_t vertexSize);

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
	 * @throw std::out_of_range if the name doesn't exist or std::runtime_error if the type
	 *     for the name doesn't match the function's type.
	 */
	void setFloat(const std::string& name, float value) { setData(name, VertexElementType::FLOAT, &value); }
	void setVec2(const std::string& name, const glm::vec2& value) { setData(name, VertexElementType::VEC2, &value); }
	void setVec3(const std::string& name, const glm::vec3& value) { setData(name, VertexElementType::VEC3, &value); }
	void setVec4(const std::string& name, const glm::vec4& value) { setData(name, VertexElementType::VEC4, &value); }
	void setUint32(const std::string& name, uint32_t value) { setData(name, VertexElementType::UINT32, &value); }

	/**
	 * "Getting" functions, gets the value of the name in the data buffer.
	 * If the name was never set, these have undefined behaviour!
	 * @param name The name to get.
	 * @return The value stored at the name.
	 * @throw std::out_of_range if the name doesn't exist or std::runtime_error if the type
	 *     for the name doesn't match the function's type.
	 */
	float getFloat(const std::string& name) { return *(float*) getData(name, VertexElementType::FLOAT); }
	glm::vec2 getVec2(const std::string& name) { return *(glm::vec2*) getData(name, VertexElementType::VEC2); }
	glm::vec3 getVec3(const std::string& name) { return *(glm::vec3*) getData(name, VertexElementType::VEC3); }
	glm::vec4 getVec4(const std::string& name) { return *(glm::vec4*) getData(name, VertexElementType::VEC4); }
	uint32_t getUint32(const std::string& name) { return *(uint32_t*) getData(name, VertexElementType::UINT32); }

	/**
	 * Returns the vertex data for copying into a buffer.
	 */
	const unsigned char* getData() const { return vertexData; }

	/**
	 * Returns the size of the vertex, in bytes.
	 */
	size_t getSize() const { return size; }

	/**
	 * Returns whether the two vertices are equal.
	 */
	friend bool operator==(const Vertex& v1, const Vertex& v2) {
		//memcmp returns zero if blocks match.
		return v1.buffer == v2.buffer && v1.size == v2.size && !memcmp(v1.vertexData, v2.vertexData, v1.size);
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

			buffer = v.buffer;
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
			buffer = std::exchange(v.buffer, nullptr);
		}

		return *this;
	}

private:
	//The parent buffer, determines vertex format.
	VertexBuffer* buffer;
	//Size of vertex, in bytes. Also size of below memory block.
	size_t size;
	//The data for this vertex. Originally this was going to use template metaprogramming
	//to auto-generate a vertex struct for each format, but that turned out to be too
	//impractical, so now we're throwing safety to the wind and doing it this way instead.
	//Don't forget to free!
	unsigned char* vertexData;

	/**
	 * Helper function for the set* functions above. Directly copies the data
	 * into the data buffer.
	 * @param name The element's name.
	 * @param expected type The expected type of the element stored under name.
	 * @param data The data to copy.
	 * @throw std::out_of_range if name not found or std::runtime_error if name's type doesn't match expectedType.
	 */
	void setData(const std::string& name, VertexElementType expectedType, const void* data);

	/**
	 * Helper function for the get* functions. Gets the stored value in the data buffer.
	 * @param name The elements name.
	 * @param expectedType The type of the retrieved element, checked against the vertex format.
	 * @return The data stored at the given name.
	 * @throw std::out_of_range if name not found, std::runtime_error if name has the wrong type.
	 */
	void* getData(const std::string& name, VertexElementType expectedType);
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(const Vertex& v) const {
			const unsigned char* vertexData = v.getData();
			size_t vertexSize = v.getSize();

			size_t blocks = vertexSize >> 3;
			size_t extra = vertexSize - (blocks << 3);

			size_t hashedVal = 0;

			for (size_t i = 0; i < blocks; i++) {
				hashedVal ^= ((const uint64_t*)vertexData)[i] << 1;
			}

			if (extra) {
				uint64_t extraData = 0;
				memcpy(&extraData, &((const uint64_t*)vertexData)[blocks], extra);

				hashedVal ^= extraData << 1;
			}

			return hashedVal;
		}
	};
}
