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
#include <Vertex.hpp>

#include "CombinedGl.h"

//The type of mesh, affects mesh storage
enum class MeshType {
	//Static meshes will be uploaded to gpu memory, for fast access by the gpu. Their buffers
	//will never be mapped. In addition, their vertices may be combined with the vertices of other
	//static meshes to save memory.
	STATIC
};

//Contains the data needed to draw a mesh, such as offsets in the index buffer
struct MeshData {
	//The type of this mesh - determines which buffer to use
	MeshType type;
	//The starting position (byte offset) in the index buffer
	uint32_t indexStart;
	//The number of indices in the mesh
	uint32_t indexCount;
};

class GlMemoryManager {
public:
	/**
	 * Initializes the memory manager - this does NOT allocate any
	 * gpu memory.
	 */
	GlMemoryManager() {}

	/**
	 * Frees any allocated gpu memory.
	 */
	~GlMemoryManager();

	/**
	 * Adds a mesh (vertices + indices) to be uploaded to the gpu at a later time.
	 * @param vertices The vertices for the mesh to be uploaded.
	 * @param indices The vertex indices for the mesh to be uploaded.
	 * @param type The type of mesh.
	 * @return a MeshData struct that can be used for drawing.
	 * @throw runtime_error if the memory manager has already been initialized.
	 */
	MeshData addMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, MeshType type);

	/**
	 * Uploads data to gpu to prepare for drawing.
	 * @throw runtime_error if the memory manager has already been initialized.
	 */
	void upload();

private:
	//Whether init has been called.
	bool initialized;

	//Data for STATIC MeshType meshes.
	std::vector<Vertex> staticVertices;
	std::unordered_map<Vertex, uint32_t> staticUniqueVertices;
	std::vector<uint32_t> staticIndices;

	//Buffers. If more are added, these will probably become arrays.
	GLuint vao;
	GLuint vertexBuffer;
	GLuint indexBuffer;
};
