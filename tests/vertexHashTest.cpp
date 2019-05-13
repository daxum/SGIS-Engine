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

#include <iostream>
#include <vector>
#include <unordered_set>

#include "../src/ExtraMath.hpp"
#include "../src/Models/Vertex.hpp"

static constexpr size_t numVertices = 10000000;
static const VertexFormat format({
	{VERTEX_ELEMENT_POSITION, VertexFormat::ElementType::VEC3},
	{VERTEX_ELEMENT_NORMAL, VertexFormat::ElementType::VEC3},
	{VERTEX_ELEMENT_TEXTURE, VertexFormat::ElementType::VEC2}
});

int main(int argc, char** argv) {
	std::vector<Vertex> vertices(numVertices, Vertex(&format));

	//Randomly fill out vertices
	for (Vertex& v : vertices) {
		v.setVec3(VERTEX_ELEMENT_POSITION, {ExMath::randomFloat(-1.0f, 1.0f), ExMath::randomFloat(-1.0f, 1.0f), ExMath::randomFloat(-1.0f, 1.0f)});
		v.setVec3(VERTEX_ELEMENT_NORMAL, {ExMath::randomFloat(-1.0f, 1.0f), ExMath::randomFloat(-1.0f, 1.0f), ExMath::randomFloat(-1.0f, 1.0f)});
		v.setVec2(VERTEX_ELEMENT_TEXTURE, {ExMath::randomFloat(0.0f, 1.0f), ExMath::randomFloat(0.0f, 1.0f)});
	}

	//Test hash performance
	std::vector<uint64_t> vertHash(numVertices, 0);

	double hashStart = ExMath::getTimeMillis();

	for (size_t i = 0; i < numVertices; i++) {
		vertHash.at(i) = std::hash<Vertex>()(vertices.at(i));
	}

	double hashEnd = ExMath::getTimeMillis();

	//Check collision rate
	size_t collisions = 0;
	std::unordered_set<uint64_t> hashes;

	for (const uint64_t h : vertHash) {
		collisions += hashes.count(h);
		hashes.insert(h);
	}

	std::cout << "Testing " << numVertices << " vertices: Hashed in " << (hashEnd - hashStart) << "ms, with " << collisions << " collisions\n";

	return 0;
}
