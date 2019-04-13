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

#include "VertexFormat.hpp"

class Vertex {
public:
	/**
	 * Constructor.
	 * @param format The format of the vertex.
	 */
	Vertex(const VertexFormat* format) :
		format(format),
		vertexData(new unsigned char[format->getVertexSize()]) {}

	/**
	 * Copy constructor.
	 */
	Vertex(const Vertex& v);

	/**
	 * Move constructor.
	 */
	Vertex(Vertex&& v) noexcept  :
		format(std::exchange(v.format, nullptr)),
		vertexData(std::exchange(v.vertexData, nullptr)) {}

	/**
	 * Deletes the vertex data.
	 */
	~Vertex() {
		if (vertexData) {
			delete[] vertexData;
		}
	}

	/**
	 * "Setting" functions, sets the value of the name in the data buffer to the provided value.
	 * @param name The name to set.
	 * @param value The value to set the name to.
	 * @throw std::out_of_range if the name doesn't exist or std::runtime_error if the type
	 *     for the name doesn't match the function's type.
	 */
	void setFloat(const std::string& name, float value) { setDataVal(name, VertexFormat::ElementType::FLOAT, &value); }
	void setVec2(const std::string& name, const glm::vec2& value) { setDataVal(name, VertexFormat::ElementType::VEC2, &value); }
	void setVec3(const std::string& name, const glm::vec3& value) { setDataVal(name, VertexFormat::ElementType::VEC3, &value); }
	void setVec4(const std::string& name, const glm::vec4& value) { setDataVal(name, VertexFormat::ElementType::VEC4, &value); }
	void setUint32(const std::string& name, uint32_t value) { setDataVal(name, VertexFormat::ElementType::UINT32, &value); }

	/**
	 * "Getting" functions, gets the value of the name in the data buffer.
	 * If the name was never set, these have undefined behaviour!
	 * @param name The name to get.
	 * @return The value stored at the name.
	 * @throw std::out_of_range if the name doesn't exist or std::runtime_error if the type
	 *     for the name doesn't match the function's type.
	 */
	float getFloat(const std::string& name) const { return *(const float*) getDataVal(name, VertexFormat::ElementType::FLOAT); }
	glm::vec2 getVec2(const std::string& name) const { return *(const glm::vec2*) getDataVal(name, VertexFormat::ElementType::VEC2); }
	glm::vec3 getVec3(const std::string& name) const { return *(const glm::vec3*) getDataVal(name, VertexFormat::ElementType::VEC3); }
	glm::vec4 getVec4(const std::string& name) const { return *(const glm::vec4*) getDataVal(name, VertexFormat::ElementType::VEC4); }
	uint32_t getUint32(const std::string& name) const { return *(const uint32_t*) getDataVal(name, VertexFormat::ElementType::UINT32); }

	/**
	 * Returns the vertex data for copying into a buffer.
	 */
	const unsigned char* getData() const { return vertexData; }

	/**
	 * Returns the size of the vertex, in bytes.
	 */
	size_t getSize() const { return format->getVertexSize(); }

	/**
	 * Returns whether the two vertices are equal.
	 */
	friend bool operator==(const Vertex& v1, const Vertex& v2) {
		//memcmp returns zero if blocks match.
		//Formats are currently singletons, so pointer comparison here is fine.
		return v1.format == v2.format && !memcmp(v1.vertexData, v2.vertexData, v1.getSize());
	}

	/**
	 * Who knows what this does.
	 */
	friend bool operator!=(const Vertex& v1, const Vertex& v2) {
		return !(v1 == v2);
	}

	/**
	 * Copy assignment.
	 */
	Vertex& operator=(const Vertex& v) {
		if (this != &v) {
			if (getSize() < v.getSize()) {
				delete[] vertexData;
				vertexData = nullptr;
				vertexData = new unsigned char[v.getSize()];
			}

			format = v.format;
			memcpy(vertexData, v.vertexData, v.getSize());
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
			format = std::exchange(v.format, nullptr);
		}

		return *this;
	}

private:
	//The element layout of this vertex.
	const VertexFormat* format;
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
	void setDataVal(const std::string& name, VertexFormat::ElementType expectedType, const void* data);

	/**
	 * Helper function for the get* functions. Gets the stored value in the data buffer.
	 * @param name The elements name.
	 * @param expectedType The type of the retrieved element, checked against the vertex format.
	 * @return The data stored at the given name.
	 * @throw std::out_of_range if name not found, std::runtime_error if name has the wrong type.
	 */
	const void* getDataVal(const std::string& name, VertexFormat::ElementType expectedType) const;
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
