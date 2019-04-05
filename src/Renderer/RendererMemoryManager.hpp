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
#include "EngineConfig.hpp"
#include "Logger.hpp"
#include "ShaderInfo.hpp"
#include "Model.hpp"

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

//The three different uniform buffers. Model buffers
//act similarly to vertex buffers, and PER_SCREEN_OBJECT
//behaves like a rotating stream buffer in device memory.
enum class UniformBufferType {
	STATIC_MODEL,
	PER_SCREEN_OBJECT
};

//An interface to the rendering engine's memory manager.
class RendererMemoryManager {
public:
	/**
	 * Creates a memory manager and initializes a logger.
	 * @param logConfig The logger configuration.
	 */
	RendererMemoryManager(const LogConfig& logConfig);

	/**
	 * Destructor.
	 */
	virtual ~RendererMemoryManager() {
		if (materialData) {
			delete[] materialData;
		}
	}

	/**
	 * Called by the engine after descriptor sets are loaded to initialize the uniform
	 * buffers and their corresponding memory managers.
	 */
	void UniformBufferInit();

	/**
	 * Initializes descriptor sets or similar, if the rendering engine supports that kind of thing.
	 * This is mainly used to allocate descriptor pools in the vulkan renderer.
	 * Textures should be loaded before this is called!
	 */
	virtual void initializeDescriptors() = 0;

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
	//Let's see what this breaks
	//VertexBuffer& getBuffer(const std::string& name) { return buffers.at(name).buffer; }

	/**
	 * Returns the uniform set with the given name, used during model loading.
	 * @param set The name of the set to get.
	 * @return The set with the given name.
	 */
	const UniformSet& getUniformSet(const std::string& set) { return uniformSets.at(set); }

	/**
	 * Adds a mesh to the provided buffer, and creates any resources needed to render it.
	 * @param name The name to store the mesh under.
	 * @param buffer The buffer to add the mesh to.
	 * @param vertexData The data to add to the buffer.
	 * @param dataSize The size of the data to add, in bytes.
	 * @param indices Indices for the mesh to load into an index buffer.
	 * @throw std::runtime_error If the name already exists or if out of memory.
	 */
	void addMesh(const std::string& name, const std::string& buffer, const unsigned char* vertexData, size_t dataSize, std::vector<uint32_t> indices);

	/**
	 * Checks whether the mesh's data is stored in the given buffer. If it is, it is marked
	 * as in use if it wasn't already. If it isn't present, nothing happens, and the data
	 * needs to be reuploaded to be usable.
	 * @param mesh The name of the mesh to check.
	 * @param buffer The buffer to check for the name.
	 * @return true if the mesh was present, false if it wasn't and the data needs to be reuploaded.
	 */
	bool attemptSetUsed(const std::string& mesh, const std::string& buffer);

	/**
	 * Marks the mesh as unused. If the mesh is transitory (BufferUsage::DEDICATED_SINGLE),
	 * all references to it will be completely deleted from the buffer, otherwise it will
	 * only be marked as unused. If the mesh doesn't exist, nothing happens.
	 * @param mesh The mesh to free.
	 * @param buffer The buffer that contains the mesh.
	 */
	void freeMesh(const std::string& mesh, const std::string& buffer);

	/**
	 * Adds a material's uniform data to the uniform buffers, and allocates a descriptor
	 * set (or similar) for it.
	 * @param name The name of the material to add.
	 * @param model The material to add.
	 */
	void addMaterial(const std::string& name, const Material& material);

protected:
	//Logger, logs things.
	Logger logger;
	//Stores all created uniform sets. This is filled out by subclasses.
	std::unordered_map<std::string, UniformSet> uniformSets;

	/**
	 * Function available for subclasses if they need buffers deleted before their destructor completes.
	 * Destroys all created buffers.
	 */
	void deleteBuffers() { buffers.clear(); }

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
	 * Creates the two uniform buffers with the given sizes, which have been calculated by
	 * the maxUsers variable of all added uniform sets.
	 * @param modelStaticSize The size of the uniform buffer which stores material data.
	 * @param screenObjectSize The size of the PER_SCREEN_OBJECT uniform buffer.
	 */
	virtual void createUniformBuffers(size_t materialSize, size_t screenObjectSize) = 0;

	/**
	 * Gets the minimum alignment for offsets into a uniform buffer.
	 * Some values for reference:
	 * The lowest value is Intel integrated on linux, which seems to mostly use 1,
	 * while windows almost always uses 32.
	 * RADV appears to always use 4, which is different from the AMD windows
	 * drivers, which use 16.
	 * Nvidia uses the maximum value of 256 for some reason.
	 * Mobile (android) seems to almost always use 64, but a few use 32.
	 * Nobody really uses 128.
	 * @return The minimum alignment.
	 */
	virtual size_t getMinUniformBufferAlignment() = 0;

	/**
	 * Uploads the vertex and index data into the given buffer. The implementor of this
	 * function should ensure that any old data that might have occupied that memory is not
	 * overwritten while still in use (due to the nature of the allocator, this should not
	 * happen often, and only in memory constrained scenarios).
	 * @param buffer The vertex buffer to upload to.
	 * @param mesh The name of the mesh being uploaded, used to store rendering data.
	 * @param offset The offset into the vertex buffer to place the vertex data.
	 * @param size The size of the vertex data.
	 * @param vertexData The vertex data to upload.
	 * @param indexOffset The offset into the index buffer to place the index data.
	 * @param indexSize The size of the index data.
	 * @param indexData The index data to upload.
	 */
	virtual void uploadMeshData(const VertexBuffer& buffer, const std::string& mesh, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) = 0;

	/**
	 * Completely removes the mesh from the rendering engine, so that it must be reuploaded to be used again.
	 * @param mesh The mesh being removed, used to delete renderer specific data (index offset, size, etc).
	 */
	virtual void invalidateMesh(const std::string& mesh) = 0;

	/**
	 * Allocates a descriptor set for the material. This can be called more than once
	 * for the same material - subsequent calls should be ignored.
	 * @param model The material to allocate a descriptor set for.
	 */
	virtual void addMaterialDescriptors(const Material& material) = 0;

	/**
	 * Uploads material uniform data to a uniform buffer.
	 * @param offset The offset of the data into the buffer.
	 * @param size The size of the data.
	 * @param data The data to upload.
	 */
	virtual void uploadMaterialData(const size_t offset, const size_t size, const unsigned char* data) = 0;

private:
	//Stores all created vertex buffers.
	std::unordered_map<std::string, BufferData> buffers;
	//List of materials which have been uploaded to the rendering engine.
	std::unordered_set<std::string> uploadedMaterials;
};
