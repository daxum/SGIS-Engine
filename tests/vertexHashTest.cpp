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
