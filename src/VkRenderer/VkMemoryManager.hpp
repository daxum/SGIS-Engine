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

#include <queue>
#include <vector>
#include <unordered_map>
#include <array>

#include <vulkan/vulkan.h>

#include "RendererMemoryManager.hpp"
#include "VkObjectHandler.hpp"
#include "vk_mem_alloc.h"

struct VkBufferData : public RenderBufferData {
	VmaAllocator allocator;

	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VmaAllocation vertexAllocation;
	VmaAllocation indexAllocation;

	VkBufferData(VmaAllocator allocator, VkBuffer vertexBuffer, VkBuffer indexBuffer, VmaAllocation vertexAllocation, VmaAllocation indexAllocation) :
		allocator(allocator),
		vertexBuffer(vertexBuffer),
		indexBuffer(indexBuffer),
		vertexAllocation(vertexAllocation),
		indexAllocation(indexAllocation) {}

	~VkBufferData() {
		vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
		vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
	}
};

struct VkMeshRenderData {
	uintptr_t indexStart;
	uint32_t indexCount;
};

struct TransferOperation {
	//The buffer to transfer to.
	VkBuffer buffer;
	//The data to transfer.
	unsigned char* data;
	//The size of the data.
	size_t size;
	//Offset into the destination buffer to place the data.
	size_t dstOffset;
	//Offset into the transfer buffer to place the data.
	size_t srcOffset;
};

class VkMemoryManager : public RendererMemoryManager {
public:
	/**
	 * Initializes a VkMemoryManager.
	 * @param logConfig The configuration for the logger.
	 * @param objects Vulkan objects.
	 */
	VkMemoryManager(const LogConfig& logConfig, VkObjectHandler& objects);

	/**
	 * Creates internal allocator.
	 */
	void init();

	/**
	 * Destroys allocator.
	 */
	void deinit();

	/**
	 * Sends all pending transfer operations to their destinations.
	 */
	void executeTransfers();

	/**
	 * Gets the rendering data for the mesh with the given name.
	 * @param mesh The name of the mesh to get data for.
	 * @return The rendering data for the mesh.
	 */
	const VkMeshRenderData& getMeshRenderData(const std::string& mesh) const { return meshMap.at(mesh); }

	/**
	 * Gets the layout of the provided descriptor set, used in shader loading.
	 * @param name The name of the descriptor set layout to fetch.
	 * @return The layout with the given name.
	 */
	VkDescriptorSetLayout getSetLayout(const std::string& name) const { return descriptorLayouts.at(name); }

	/**
	 * Adds a descriptor set to the rendering engine, as well as a layout that can be used in shaders.
	 * @param name The name of the set to add.
	 * @param uniformSet The uniform set.
	 * @param setLayout The layout of the uniform set.
	 */
	void addDescriptorSet(const std::string& name, const UniformSet& uniformSet, VkDescriptorSetLayout setLayout) {
		uniformSets.insert({name, uniformSet});
		descriptorLayouts.insert({name, setLayout});
	}

protected:
	/**
	 * Creates a buffer and allocation with the given parameters.
	 * @param vertexFormat The format of the vertices in the buffer.
	 * @param usage The way the buffer is intended to be used - determines which memory type
	 *     it is stored in.
	 * @param size The size of the buffer to create.
	 * @return A pointer to a VkBufferData object.
	 * @throw std::runtime_error if out of memory.
	 */
	std::shared_ptr<RenderBufferData> createBuffer(const std::vector<VertexElement>& vertexFormat, BufferUsage usage, size_t size) override;

	/**
	 * Creates a uniform buffer with the given size.
	 * @param buffer The type of uniform buffer being created. This is used for
	 *     UniformBufferType::PER_SCREEN_OBJECT, which has one buffer for every active
	 *     frame.
	 * @param size The size of the buffer.
	 */
	void createUniformBuffers(size_t modelStaticSize, size_t modelDynamicSize, size_t screenObjectSize) override;

	/**
	 * Gets the minimum alignment for offsets into a uniform buffer.
	 * @return The minimum alignment.
	 */
	size_t getMinUniformBufferAlignment() override { return objects.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment; }

	/**
	 * Queues the index and vertex data to be transferred before the next frame is drawn.
	 * @param buffer The vertex buffer to upload to.
	 * @param mesh The name of the mesh being uploaded, used to store rendering data.
	 * @param offset The offset into the vertex buffer to place the vertex data.
	 * @param size The size of the vertex data.
	 * @param vertexData The vertex data to upload.
	 * @param indexOffset The offset into the index buffer to place the index data.
	 * @param indexSize The size of the index data.
	 * @param indexData The index data to upload.
	 */
	void uploadMeshData(const VertexBuffer& buffer, const std::string& mesh, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) override;

	/**
	 * Removes the mesh from the rendering engine. No allocated memory is modified.
	 * @param mesh The mesh being removed.
	 */
	void invalidateMesh(const std::string& mesh) override { meshMap.erase(mesh); }

	/**
	 * Allocates a descriptor set for the model. The model is guaranteed to be static,
	 * as dynamic models use descriptors that are determined during initialization and
	 * that use dynamic offsets. This can be called more than once for the same model -
	 * subsequent calls are ignored.
	 * @param name The name of the model.
	 * @param model The model to allocate a descriptor set for.
	 */
	void addModelDescriptors(const std::string& name, const Model& model) override { /** TODO **/ }

	/**
	 * Uploads model uniform data to a uniform buffer.
	 * @param buffer The uniform buffer to upload to.
	 * @param offset The offset of the data into the buffer.
	 * @param size The size of the data.
	 * @param data The data to upload.
	 */
	void uploadModelData(const UniformBufferType buffer, const size_t offset, const size_t size, const unsigned char* data) override { /** TODO **/ }

private:
	//Object handler for vulkan objects.
	VkObjectHandler& objects;
	//Allocator for the vertex buffers.
	VmaAllocator allocator;
	//Transfer buffer.
	VkBuffer transferBuffer;
	//Allocation for transfer buffer.
	VmaAllocation transferAllocation;
	//Uniform buffers and allocations.
	//0 - static model, 1 - dynamic model, 2 - screen / object.
	std::array<VkBuffer, 3> uniformBuffers;
	std::array<VmaAllocation, 3> uniformBufferAllocations;
	//Fence to prevent overwriting in-transit memory.
	VkFence transferFence;
	//Current offset into transfer buffer, reset to 0 after each transfer operation.
	size_t transferOffset;
	//Size of the transfer buffer.
	size_t transferSize;
	//Indicates that the transfer buffer needs to be reallocated.
	bool growTransfer;
	//All queued transfer operations.
	std::queue<TransferOperation> pendingTransfers;
	//Map of uploaded mesh data.
	std::unordered_map<std::string, VkMeshRenderData> meshMap;
	//Last used command buffer for vertex transfers.
	VkCommandBuffer lastVertexTransferBuffer;
	//All possible descriptor set layouts.
	std::unordered_map<std::string, VkDescriptorSetLayout> descriptorLayouts;

	/**
	 * Adds a transfer operation to the pending transfer queue.
	 * @param buffer The destination buffer for the transfer.
	 * @param offset The offset into the destination buffer to place the data.
	 * @param size The size of the data.
	 * @param data The data to transfer.
	 */
	void queueTransfer(VkBuffer buffer, size_t offset, size_t size, const unsigned char* data);
};
