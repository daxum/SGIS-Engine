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

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex {
	//The x y and z coordinates of the vertex.
	glm::vec3 pos;
	//The normal vector for the vertex.
	glm::vec3 normal;
	//The texture coordinates for the vertex.
	glm::vec2 texCoords;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texCoords == other.texCoords;
	}
};

struct TextVertex {
	glm::vec2 pos;
	glm::vec2 tex;
};

inline std::string vertToString(const Vertex& v) {
	return "(" + std::to_string(v.pos.x) + ", " + std::to_string(v.pos.y) + ", " + std::to_string(v.pos.z) + ") (" +
		   std::to_string(v.normal.x) + ", " + std::to_string(v.normal.y) + ", " + std::to_string(v.normal.z) + ") (" +
		   std::to_string(v.texCoords.x) + ", " + std::to_string(v.texCoords.y) + ")";
}

inline std::string textVertToString(const TextVertex& t) {
	return "(" + std::to_string(t.pos.x) + ", " + std::to_string(t.pos.y) + ") (" +
		   std::to_string(t.tex.x) + ", " + std::to_string(t.tex.y) + ")";
}

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                   (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.texCoords) << 1);
        }
    };
}
