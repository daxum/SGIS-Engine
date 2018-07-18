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
#include <string>
#include <unordered_map>

#include "VertexBuffer.hpp"
#include "MemoryAllocator.hpp"

//All the information needed to create a vertex buffer.
struct VertexBufferInfo {
	//Format of the buffer (vertex layout).
	std::vector<VertexElement> format;
	//Buffer usage, determines where the buffer is stored and the eviction policy.
	BufferUsage usage;
	//Size of the buffer.
	size_t size;
};

//Stores a buffer with its allocators and allocations.
struct BufferData {
	//The buffer metadata.
	VertexBuffer buffer;

	//Allocators for the vertex and index buffers.
	MemoryAllocator vertexAllocator;
	MemoryAllocator indexAllocator;

	//All allocations made from this buffer.
	std::unordered_map<std::string, std::shared_ptr<AllocInfo>> vertexAllocations;
	std::unordered_map<std::string, std::shared_ptr<AllocInfo>> indexAllocations;
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
	 * @throw std::runtime_error if not enough memory.
	 */
	void addBuffer(const std::string& name, const VertexBufferInfo& info);

	/**
	 * Returns the vertex buffer with the given name, primarily used during model loading.
	 * @param name The name to retrieve.
	 * @return The vertex buffer with the given name.
	 * @throw std::out_of_range if the buffer doesn't exist.
	 */
	VertexBuffer& getBuffer(const std::string& name) { return buffers.at(name).buffer; }

	/**
	 * Adds a mesh to the provided buffer, and creates any resources needed to render it.
	 * @param name The name to store the mesh under.
	 * @param buffer The buffer to add the mesh to.
	 * @param vertexData The data to add to the buffer.
	 * @param dataSize The size of the data to add, in bytes.
	 * @param indices Indices for the mesh to load into an index buffer.
	 * @throw std::runtime_error If the name already exists or if out of memory.
	 */
	void addMesh(const std::string& name, const std::string& buffer, const unsigned char* vertexData, size_t dataSize, const std::vector<uint32_t>& indices);

	/**
	 * Checks whether the mesh's data is stored in the given buffer. If it is, it is marked
	 * as in use if it wasn't already. If it isn't present, nothing happens, and the data
	 * needs to be reuploaded to be usable.
	 * @param mesh The name of the mesh to check.
	 * @param buffer The buffer to check for the name.
	 * @return true if the mesh was present, false if it wasn't and the data needs to be reuploaded.
	 */
	bool markUsed(const std::string& mesh, const std::string& buffer);

	/**
	 * Marks the mesh as unused. If the mesh is transitory (BufferUsage::DEDICATED_SINGLE),
	 * all references to it will be completely deleted from the buffer, otherwise it will
	 * only be marked as unused. If the mesh doesn't exist, nothing happens.
	 * @param mesh The mesh to free.
	 * @param buffer The buffer that contains the mesh.
	 */
	void freeMesh(const std::string& mesh, const std::string& buffer);

protected:
	/**
	 * Creates a buffer with the underlying rendering api and returns a pointer to the data
	 * to be stored with the buffer object.
	 * @param usage The way the buffer is intended to be used - determines which memory type
	 *     it is stored in.
	 * @param size The size of the buffer to create.
	 * @return A pointer to renderer-specific data to be stored with the buffer object.
	 * @throw std::runtime_error if out of memory.
	 */
	virtual std::shared_ptr<RenderBufferData> createBuffer(BufferUsage usage, size_t size) = 0;

	/**
	 * Uploads the vertex and index data into the given buffer.
	 * @param buffer The render data for the buffer to upload to.
	 * @param offset The offset into the vertex buffer to place the vertex data.
	 * @param size The size of the vertex data.
	 * @param vertexData The vertex data to upload.
	 * @param indexOffset The offset into the index buffer to place the index data.
	 * @param indexSize The size of the index data.
	 * @param indexData The index data to upload.
	 */
	virtual void uploadMeshData(std::shared_ptr<RenderBufferData> buffer, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) = 0;

	/**
	 * Marks the provided range as invalid, so other objects can be uploaded there. The implementor of this
	 * function should ensure that the old data is not overwritten while still in use if a new mesh is uploaded
	 * to the memory (due to the nature of the allocator, this should not happen often, and only in memory constrained
	 * scenarios).
	 * @param buffer The buffer the range is for (vertex and index at the moment, will be split later).
	 * @param offset The offset into the vertex buffer.
	 * @param size The size of the range in the vertex buffer.
	 * @param indexOffset The offset into the index buffer.
	 * @param indexSize The size of the range into the index buffer.
	 */
	virtual void invalidateRange(std::shared_ptr<RenderBufferData> buffer, size_t offset, size_t size, size_t indexOffset, size_t indexSize) = 0;

private:
	//Stores all created vertex buffers.
	std::unordered_map<std::string, BufferData> buffers;
};
