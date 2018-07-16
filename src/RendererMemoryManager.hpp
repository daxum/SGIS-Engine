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

#include <vector>
#include <cstdint>

#include "VertexBuffer.hpp"

enum class BufferUsage {
	//Reference counted buffer located in gpu memory, if possible. Meshes remain in
	//the buffer even if nothing references them, for possible reuse later.
	//They are only evicted if memory runs out.
	DEDICATED_LAZY,
	//Similar to above, except unused meshes are immediately invalidated and
	//have to be reuploaded if used again.
	DEDICATED_SINGLE,
	//This is just a stream buffer.
	STREAM
};

//All the information needed to create a vertex buffer.
struct VertexBufferInfo {
	//Format of the buffer (vertex layout).
	std::vector<VertexElement> format;
	//Buffer usage, determines where the buffer is stored and the eviction policy.
	BufferUsage usage;
	//Size of the buffer.
	uint64_t size;
};

//An interface to the rendering engine's memory manager.
class RendererMemoryManager {
public:
	/**
	 * Destructor.
	 */
	virtual ~RendererMemoryManager() {}

	/**
	 * Adds a vertex buffer to the memory manager. Currently also creates an index buffer as well.
	 * @param name The name of the buffer.
	 * @param info Initialization info for the buffer.
	 */
	virtual void addBuffer(const std::string& name, const VertexBufferInfo& info) = 0;

	/**
	 * Adds a mesh to the provided buffer, and creates any resources needed to render it.
	 * @param name The name to store the mesh under.
	 * @param buffer The buffer to add the mesh to.
	 * @param vertexData The data to add to the buffer.
	 * @param dataSize The size of the data to add, in bytes.
	 * @param indices Indices for the mesh to load into an index buffer.
	 * @throw std::runtime_error If the name already exists.
	 */
	virtual void addMesh(const std::string& name, const std::string& buffer, const unsigned char* vertexData, size_t dataSize, const std::vector<uint32_t>& indices) = 0;

	/**
	 * Gets whether the mesh is already present in the given buffer.
	 * @param mesh The name of the mesh to check.
	 * @param buffer The buffer to check for the name.
	 * @return Whether a mesh with the given name exists in the buffer.
	 */
	virtual bool hasMesh(const std::string& mesh, const std::string& buffer) = 0;

	/**
	 * Adds another user for the given mesh.
	 * @param mesh The name of the mesh.
	 * @param buffer The name of the buffer the mesh resides in.
	 */
	virtual void addUser(const std::string& mesh, const std::string& buffer) = 0;

	/**
	 * Removes a user of the given mesh. If the mesh has no users and is not part of
	 * a lazy removal buffer, the memory it used is freed.
	 * @param mesh The mesh to remove a user for.
	 * @param buffer The buffer that contains the mesh.
	 */
	virtual void removeUser(const std::string& mesh, const std::string& buffer) = 0;
};
