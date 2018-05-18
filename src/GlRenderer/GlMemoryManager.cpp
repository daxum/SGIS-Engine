#include <stdexcept>
#include <cstring>

#include "GlMemoryManager.hpp"

GlMemoryManager::GlMemoryManager(Logger& logger) :
	logger(logger),
	initialized(false) {

}

GlMemoryManager::~GlMemoryManager() {
	glDeleteVertexArrays(BUFFER_COUNT, vaos);
	glDeleteBuffers(BUFFER_COUNT, vertexBuffers);
	glDeleteBuffers(BUFFER_COUNT, indexBuffers);
}

MeshRenderData GlMemoryManager::addStaticMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
	if (initialized) {
		throw std::runtime_error("Cannot add static meshes after initialization!");
	}

	MeshRenderData data = {};
	data.type = MeshType::STATIC;
	data.indexStart = staticIndices.size() * sizeof(uint32_t);

	size_t indexStartSize = staticIndices.size();

	logger.debug("Adding mesh to static data...");

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

	data.indexCount = staticIndices.size() - indexStartSize;

	return data;
}

MeshRenderData GlMemoryManager::addTextMesh(const std::vector<TextVertex>& vertices, const std::vector<uint32_t>& indices) {
	if (!initialized) {
		throw std::runtime_error("Attempt to add dynamic mesh before initialization!");
	}

	const size_t vertSize = vertices.size() * sizeof(TextVertex);
	const size_t indexSize = indices.size() * sizeof(uint32_t);

	size_t vertexStart = allocateFromList(textAllocList, nextAlloc, vertSize);
	size_t indexStart = allocateFromList(textIndexAllocList, nextIndexAlloc, indexSize);

	//Adjust indices for vertex start.
	uint32_t indexOffset = vertexStart / sizeof(TextVertex);

	std::vector<uint32_t> adjIndices(indices);

	for (uint32_t& i : adjIndices) {
		i += indexOffset;
	}

	//Upload mesh data.
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[MeshType::DYNAMIC_TEXT]);
	void* bufferData = glMapBufferRange(GL_ARRAY_BUFFER, vertexStart, vertSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(bufferData, vertices.data(), vertSize);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffers[MeshType::DYNAMIC_TEXT]);
	void* indexBufferData = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, indexStart, indexSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	memcpy(indexBufferData, adjIndices.data(), indexSize);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	//Add to map.
	textVertMap.insert({indexStart, nextAlloc});
	textIndexMap.insert({indexStart, nextIndexAlloc});

	return {DYNAMIC_TEXT, indexStart, indices.size()};
}

void GlMemoryManager::freeTextMesh(const MeshRenderData& data) {
	DynBufElement vertFreePos = textVertMap.at(data.indexStart);
	DynBufElement indexFreePos = textIndexMap.at(data.indexStart);

	textVertMap.erase(data.indexStart);
	textIndexMap.erase(data.indexStart);

	freeFromList(textAllocList, nextAlloc, vertFreePos);
	freeFromList(textIndexAllocList, nextIndexAlloc, indexFreePos);

	//Invalidating the buffer ranges would go here if supported.
}

void GlMemoryManager::upload() {
	if (initialized) {
		throw std::runtime_error("Init called twice for memory manager!");
	}

	//Static mesh data
	logger.debug("Uploading static mesh data...");
	//Don't ask.
	logger.debug(std::string("Static mesh stats:") +
				 "\n\tVertices:   " + std::to_string(staticVertices.size()) +
				 "\n\tIndices:    " + std::to_string(staticIndices.size()) +
				 "\n\tTotal size: " + std::to_string(staticVertices.size() * sizeof(Vertex) + staticIndices.size() * sizeof(uint32_t)) + " bytes");

	glGenVertexArrays(BUFFER_COUNT, vaos);
	glGenBuffers(BUFFER_COUNT, vertexBuffers);
	glGenBuffers(BUFFER_COUNT, indexBuffers);

	glBindVertexArray(vaos[MeshType::STATIC]);

	logger.debug("Creating " + std::to_string(staticVertices.size() * sizeof(Vertex)) + " byte vertex buffer for static data.");

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[MeshType::STATIC]);
	glBufferData(GL_ARRAY_BUFFER, staticVertices.size() * sizeof(Vertex), staticVertices.data(), GL_STATIC_DRAW);

	logger.debug("Creating " + std::to_string(staticIndices.size() * sizeof(uint32_t)) + " byte index buffer for static data.");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffers[MeshType::STATIC]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, staticIndices.size() * sizeof(uint32_t), staticIndices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));

	//Text buffers

	glBindVertexArray(vaos[MeshType::DYNAMIC_TEXT]);

	logger.debug("Creating " + std::to_string(textBufferSize) + " byte vertex buffer for text data.");

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[MeshType::DYNAMIC_TEXT]);
	glBufferData(GL_ARRAY_BUFFER, textBufferSize, nullptr, GL_DYNAMIC_DRAW);

	logger.debug("Creating " + std::to_string(textIndexBufferSize) + " byte index buffer for text data.");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffers[MeshType::DYNAMIC_TEXT]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndexBufferSize, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*) offsetof(TextVertex, pos));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*) offsetof(TextVertex, tex));

	glBindVertexArray(0);

	//Initialize text buffer free lists
	textAllocList.clear();
	textIndexAllocList.clear();

	textAllocList.push_back({0, textBufferSize, true});
	textIndexAllocList.push_back({0, textIndexBufferSize, true});

	nextAlloc = textAllocList.begin();
	nextIndexAlloc = textIndexAllocList.begin();

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
	if (type >= BUFFER_COUNT) {
		throw std::runtime_error("Invalid meshtype in bindBuffer!");
	}

	glBindVertexArray(vaos[type]);
}

size_t GlMemoryManager::allocateFromList(std::list<AllocInfo>& list, DynBufElement& current, size_t allocSize) {
	DynBufElement start = current;

	//Find free segment of proper size.
	while (current->size < allocSize || !current->free) {
		current++;

		if (current == list.end()) {
			current = list.begin();
		}

		if (current == start) {
			throw std::runtime_error("Failed to allocate " + std::to_string(allocSize) + " bytes from dynamic buffer");
		}
	}

	//Found segment, allocate it.
	size_t pos = current->start;
	current->free = false;

	//Split segment if needed.
	size_t extra = current->size - allocSize;

	if (extra > 0) {
		DynBufElement next = current;
		next++;

		list.insert(next, {current->start + current->size, extra, true});
		current->size -= extra;
	}

	return pos;
}

void GlMemoryManager::freeFromList(std::list<AllocInfo>& list, DynBufElement& current, DynBufElement freePos) {
	if (freePos->free) {
		throw std::runtime_error("Invalid free attempt from dynamic buffer!");
	}

	freePos->free = true;

	//Merge with previous.
	DynBufElement prev = freePos;
	prev--;

	if (freePos != list.begin() && prev->free) {
		DynBufElement next = freePos;
		freePos = prev;

		freePos->size += next->size;
		list.erase(next);

		//Move current past merged element if needed.
		if (current == freePos) {
			current++;

			if (current == list.end()) {
				current = list.begin();
			}
		}
	}

	//Merge with next.
	DynBufElement next = freePos;
	next++;

	if (next != list.end() && next->free) {
		//Move current past to-be-merged element.
		if (current == next) {
			current++;

			if (current == list.end()) {
				current = list.begin();
			}
		}

		//Merge elements.
		freePos->size += next->size;
		list.erase(next);
	}
}
