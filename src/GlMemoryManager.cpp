#include <stdexcept>
#include "GlMemoryManager.hpp"

GlMemoryManager::~GlMemoryManager() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
}

MeshData GlMemoryManager::addMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshType type) {
	if (initialized) {
		throw std::runtime_error("Cannot add meshes after initialization!");
	}

	MeshData data = {};
	data.type = type;
	data.indexStart = indices.size() * sizeof(uint32_t);

	uint32_t indexStartSize = indices.size();

	switch (type) {
		case MeshType::STATIC:
			//STATIC: all static meshes are combined into one unmodifiable buffer in gpu memory
			for (const uint32_t i : indices) {
				const Vertex& vertex = vertices[i];

				//If vertex is unique to all static meshes
				if (staticUniqueVertices.count(vertex) == 0) {
					//Add new vertex and index
					staticUniqueVertices[vertex] = staticVertices.size();
					staticVertices.push_back(vertex);
				}

				//Add index for vertex, index may refer to vertex of another mesh
				staticIndices.push_back(staticUniqueVertices[vertex]);
			}
			break;
		default: throw std::runtime_error("Missing mesh type!");
	}

	data.indexCount = indices.size() - indexStartSize;

	return data;
}

void GlMemoryManager::upload() {
	if (initialized) {
		throw std::runtime_error("Init called twice for memory manager!");
	}

	//Static mesh data

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, staticVertices.size() * sizeof(Vertex), staticVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, staticIndices.size() * sizeof(uint32_t), staticIndices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));

	glBindVertexArray(0);

	//Delete mesh caches
	std::vector<Vertex>().swap(staticVertices);
	std::vector<uint32_t>().swap(staticIndices);
	std::unordered_map<Vertex, uint32_t>().swap(staticUniqueVertices);

	initialized = true;
}
