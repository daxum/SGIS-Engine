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

#include "VertexFormat.hpp"

VertexFormat::VertexFormat(const std::vector<Element>& layout) :
	vertexSize(0) {

	size_t totalSize = 0;

	for (const Element& element : layout) {
		ElementData data = {
			.name = element.name,
			.type = element.type,
			.offset = totalSize,
			.size = typeSize(element.type),
		};

		totalSize += data.size;

		format.push_back(data);
	}

	vertexSize = totalSize;
}

bool VertexFormat::hasElement(const std::string& name) const {
	for (const ElementData& data : format) {
		if (data.name == name) {
			return true;
		}
	}

	return false;
}

VertexFormat::ElementData& VertexFormat::getElement(const std::string& name) {
	for (ElementData& data : format) {
		if (data.name == name) {
			return data;
		}
	}

	throw std::out_of_range("Vertex element not present in format!");
}

const VertexFormat::ElementData& VertexFormat::getElement(const std::string& name) const {
	for (const ElementData& data : format) {
		if (data.name == name) {
			return data;
		}
	}

	throw std::out_of_range("Vertex element not present in format!");
}
