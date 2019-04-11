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

#include <vector>
#include <iostream>
#include <unordered_map>

#include "../src/Models/Vertex.hpp"

bool checkHash(const Vertex& v1, const Vertex& v2) {
	if (std::hash<Vertex>()(v1) != std::hash<Vertex>()(v2)) {
		std::cout << "Hash functions don't match!\n";
		return false;
	}

	std::cout << "Hash test passed.\n";
	return true;
}

int main(int argc, char** argv) {
	std::vector<VertexFormat::Element> vertexFormat = {
		{VERTEX_ELEMENT_POSITION, VertexFormat::ElementType::VEC3},
		{VERTEX_ELEMENT_NORMAL, VertexFormat::ElementType::VEC3},
		{VERTEX_ELEMENT_TEXTURE, VertexFormat::ElementType::VEC2}
	};

	VertexFormat format(vertexFormat);

	//Check vertex hash function
	Vertex vert1(&format);
	Vertex vert2(&format);

	vert1.setVec3(VERTEX_ELEMENT_POSITION, glm::vec3(0.1, 2.3, 4.5));
	vert1.setVec3(VERTEX_ELEMENT_NORMAL, glm::vec3(6.7, 8.9, 10.11));
	vert1.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(12.13, 14.15));

	vert2.setVec3(VERTEX_ELEMENT_POSITION, glm::vec3(0.1, 2.3, 4.5));
	vert2.setVec3(VERTEX_ELEMENT_NORMAL, glm::vec3(6.7, 8.9, 10.11));
	vert2.setVec2(VERTEX_ELEMENT_TEXTURE, glm::vec2(12.13, 14.15));

	if (!checkHash(vert1, vert2)) return 0;

	//Check copy constructor and hash again
	vert2 = vert1;

	if (!checkHash(vert1, vert2)) return 0;

	//Check equal
	if (vert1 != vert2) {
		std::cout << "Equal check failed!\n";
		return 0;
	}

	std::cout << "Equal test passed.\n";

	//Check values are what they were set to
	glm::vec3 expectPos(0.1, 2.3, 4.5);
	glm::vec3 expectNor(6.7, 8.9, 10.11);
	glm::vec2 expectTex(12.13, 14.15);

	if (vert1.getVec3(VERTEX_ELEMENT_POSITION) != expectPos ||
		vert1.getVec3(VERTEX_ELEMENT_NORMAL) != expectNor ||
		vert1.getVec2(VERTEX_ELEMENT_TEXTURE) != expectTex) {
			std::cout << "Incorrect stored values!\n";
			return 0;
	}

	//Insert / retrieve from map
	try {
		std::unordered_map<Vertex, size_t> testMap;
		testMap.insert({vert1, 5});
		size_t val = testMap.at(vert1);

		std::cout << "Map test passed! Result: " << val << "\n";
	}
	catch (const std::out_of_range& e) {
		std::cout << "Map test failed! Reason: " << e.what() << "\n";
		return 0;
	}

	std::cout << "All tests passed.\n";

	return 0;
}
