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
#include <string>

#include <glm/glm.hpp>

//Some vertex element names, to prevent mistyping.
const std::string VERTEX_ELEMENT_POSITION = "pos";
const std::string VERTEX_ELEMENT_NORMAL = "nor";
const std::string VERTEX_ELEMENT_TEXTURE = "tex";

class VertexFormat {
public:
	//Types for the elements in the format.
	enum class ElementType {
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		UINT32,
	};

	//A single element of the format.
	struct Element {
		//Name of the element.
		std::string name;
		//Type of the vertex element.
		ElementType type;
	};

	/**
	 * Gets the size of a vertex type, in bytes.
	 * @param type The type to get the size for.
	 * @return The size of the type.
	 */
	static constexpr size_t typeSize(const ElementType type) {
		switch(type) {
			case ElementType::FLOAT: return sizeof(float);
			case ElementType::VEC2: return sizeof(glm::vec2);
			case ElementType::VEC3: return sizeof(glm::vec3);
			case ElementType::VEC4: return sizeof(glm::vec4);
			case ElementType::UINT32: return sizeof(uint32_t);
			default: return 0;
		}
	}

	/**
	 * Creates a vertex format with the given layout.
	 * @param layout A description of the per-vertex data.
	 */
	VertexFormat(const std::vector<Element>& layout);

	/**
	 * Checks whether the type of the given name matches the provided type.
	 * @param name The name to get the type for.
	 * @param type The type to check.
	 * @return whether the types matched.
	 * @throw std::out_of_range if the name is not part of the buffer.
	 */
	bool checkType(const std::string& name, ElementType type) const {
		return getElement(name).type == type;
	}

	/**
	 * Returns whether the format has the given element.
	 * @param name The name of the element.
	 * @return Whether the format contains the element.
	 */
	bool hasElement(const std::string& name) const;

	/**
	 * Gets the offset (in bytes) into the vertex for the given name.
	 * @param name The name to get the offset for.
	 * @return The offset into the vertex for the element with the given name.
	 * @throw std::out_of_range if the name is not part of the buffer.
	 */
	size_t getElementOffset(const std::string& name) const {
		return getElement(name).offset;
	}

	/**
	 * Gets the size (in bytes) of the element with the given name.
	 * @param name The name to get the size of.
	 * @return The size of the given element.
	 * @throw std::out_of_range if the name is not part of the format.
	 */
	size_t getElementSize(const std::string& name) const {
		return getElement(name).size;
	}

	/**
	 * Gets the total size of a vertex that uses the format, in bytes.
	 * @return The size of a single vertex which uses the format.
	 */
	size_t getVertexSize() const { return vertexSize; }

private:
	//Internal representation of vertex elements.
	struct ElementData {
		//Name of the element.
		std::string name;
		//Type of the element.
		ElementType type;
		//offset (in bytes) from the start of the vertex.
		size_t offset;
		//Size of the element.
		size_t size;
	};

	//Vertex format vector.
	std::vector<ElementData> format;
	//Size of one vertex using this format.
	size_t vertexSize;

	/**
	 * Finds the element with the given name in the format.
	 * @param name The name of the element.
	 * @return The element with the given name.
	 * @throw std::out_of_range if the element isn't present.
	 */
	ElementData& getElement(const std::string& name);
	const ElementData& getElement(const std::string& name) const;
};

