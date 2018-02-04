#include <stdexcept>
#include "GlMemoryManager.hpp"

GlMemoryManager::~GlMemoryManager() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexBuffer);
}
#include <iostream>
MeshData GlMemoryManager::addMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshType type) {
	if (initialized) {
		throw std::runtime_error("Cannot add meshes after initialization!");
	}

	MeshData data = {};
	data.type = type;
	data.indexStart = staticIndices.size() * sizeof(uint32_t);

	uint32_t indexStartSize = staticIndices.size();

	switch (type) {
		case MeshType::STATIC:
			//STATIC: all static meshes are combined into one unmodifiable buffer in gpu memory
			logger.info("Adding mesh to static data...");

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

	data.indexCount = staticIndices.size() - indexStartSize;

	return data;
}

void GlMemoryManager::upload() {
	if (initialized) {
		throw std::runtime_error("Init called twice for memory manager!");
	}

	//Static mesh data
	logger.info("Uploading static mesh data...");
	//Don't ask.
	logger.debug(std::string("Static mesh stats:") +
				 "\n\tVertices:   " + std::to_string(staticVertices.size()) +
				 "\n\tIndices:    " + std::to_string(staticIndices.size()) +
				 "\n\tTotal size: " + std::to_string(staticVertices.size() * sizeof(Vertex) + staticIndices.size() * sizeof(uint32_t)) + " bytes");

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);

	glBindVertexArray(vao);

	logger.debug("Creating " + std::to_string(staticVertices.size() * sizeof(Vertex)) + " byte vertex buffer for static data.");

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, staticVertices.size() * sizeof(Vertex), staticVertices.data(), GL_STATIC_DRAW);

	logger.debug("Creating " + std::to_string(staticIndices.size() * sizeof(uint32_t)) + " byte index buffer for static data.");

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
	logger.info("Deleting mesh caches... (will save around " + std::to_string(staticVertices.size() * sizeof(Vertex) +
				staticIndices.size() * sizeof(uint32_t) + staticUniqueVertices.size() * sizeof(Vertex) +
				staticUniqueVertices.size() * sizeof(uint32_t)) + " bytes)");

	std::vector<Vertex>().swap(staticVertices);
	std::vector<uint32_t>().swap(staticIndices);
	std::unordered_map<Vertex, uint32_t>().swap(staticUniqueVertices);

	initialized = true;
}

void GlMemoryManager::bindBuffer(MeshType type) {
	switch (type) {
		case MeshType::STATIC: glBindVertexArray(vao); break;
		default: throw std::runtime_error("Missing type in GlMemoryManager::bindBuffer!");
	}
}
