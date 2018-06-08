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

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <list>

#include "Vertex.hpp"
#include "CombinedGl.h"
#include "Model.hpp"
#include "Logger.hpp"
#include "RendererMemoryManager.hpp"

//Used for allocations in dynamic buffers.
//All values in bytes.
struct AllocInfo {
	size_t start;
	size_t size;
	bool free;
};

class GlMemoryManager : public RendererMemoryManager {
public:
	/**
	 * Initializes the memory manager - this does NOT allocate any
	 * gpu memory.
	 * @param logger The logger with which to log.
	 */
	GlMemoryManager(Logger& logger);

	/**
	 * Frees any allocated gpu memory.
	 */
	~GlMemoryManager();

	/**
	 * Adds a static mesh (vertices + indices) to be uploaded to the gpu at a later time.
	 * @param vertices The vertices for the mesh to be uploaded.
	 * @param indices The vertex indices for the mesh to be uploaded.
	 * @return a MeshData struct that can be used for drawing.
	 * @throw runtime_error if the memory manager has already been initialized.
	 */
	MeshRenderData addStaticMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	/**
	 * Dynamically uploads text model data to the gpu. Must call free later!
	 * @param vertices The vertices for the text model.
	 * @param indices The indices for the text model.
	 * @return model data with which to render the text.
	 */
	MeshRenderData addTextMesh(const std::vector<TextVertex>& vertices, const std::vector<uint32_t>& indices);

	/**
	 * Marks the memory previously occupied by the model data as unused.
	 * @param data The model data to free.
	 */
	void freeTextMesh(const MeshRenderData& data);

	/**
	 * Uploads static data to gpu to prepare for drawing.
	 * @throw runtime_error if the memory manager has already been initialized.
	 */
	void upload();

	/**
	 * Initializes the dynamic buffers.
	 */
	void init();

	/**
	 * Binds the specified buffer for drawing.
	 * @param type The buffer to bind.
	 */
	void bindBuffer(MeshType type);

private:
	//Just a logger
	Logger& logger;
	//Whether init has been called.
	bool initialized;

	//Data for STATIC MeshType meshes.
	std::vector<Vertex> staticVertices;
	std::unordered_map<Vertex, uint32_t> staticUniqueVertices;
	std::vector<uint32_t> staticIndices;

	//Buffers.
	GLuint vaos[BUFFER_COUNT];
	GLuint vertexBuffers[BUFFER_COUNT];
	GLuint indexBuffers[BUFFER_COUNT];

	//Text buffer stuff.
	//Maybe make this configurable later. 8MB is enough for around 130,000 characters.
	const size_t textBufferSize = 8388608;
	//For every 4 vertices, there are 6 indices.
	//The full equation is: textBufferSize * ((indices * indexSize) / (vertices * vertexSize)),
	//which results in textBufferSize * ((6*4) / (4*16)). Simplifying gives the below equation.
	const size_t textIndexBufferSize = (6 * textBufferSize) / sizeof(TextVertex);

	//Text memory management
	typedef std::list<AllocInfo>::iterator DynBufElement;

	//Stores current text buffer allocations.
	std::list<AllocInfo> textAllocList;
	//Same for index buffer
	std::list<AllocInfo> textIndexAllocList;
	//Points to next free segment in text buffer.
	DynBufElement nextAlloc;
	DynBufElement nextIndexAlloc;
	//Maps from index start to free list positions.
	std::unordered_map<size_t, DynBufElement> textVertMap;
	std::unordered_map<size_t, DynBufElement> textIndexMap;

	/**
	 * Allocates a section from the list.
	 * @param list The free list for the memory being allocated.
	 * @param current The current allocation position in the list.
	 * @param allocSize The amount of memory to allocate.
	 * @return The allocated element.
	 * @throw runtime_error if the allocation failed.
	 */
	DynBufElement allocateFromList(std::list<AllocInfo>& list, DynBufElement& current, size_t allocSize);

	/**
	 * Frees a section of the list and merges with adjacent free segments.
	 * Current will be moved past the merged segment if needed, to prevent
	 * immediate reallocation.
	 * @param list The list to free from.
	 * @param current The next segment to be used for allocation.
	 * @param freePos The position in the list to free.
	 */
	void freeFromList(std::list<AllocInfo>& list, DynBufElement& current, DynBufElement freePos);
};
