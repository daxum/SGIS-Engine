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
	 * Removes the mesh from the rendering engine by deleting its command buffer. No allocated memory is modified.
	 * @param mesh The mesh being removed.
	 */
	void invalidateMesh(const std::string& mesh) override { /** TODO **/ }

private:
	//Object handler for vulkan objects.
	VkObjectHandler& objects;
	//Allocator for the vertex buffers.
	VmaAllocator allocator;
	//Transfer buffer.
	VkBuffer transferBuffer;
	//Allocation for transfer buffer.
	VmaAllocation transferAllocation;
	//Fence to prevent overwriting in-transit memory.
	VkFence transferFence;
	//Current offset into transfer buffer, reset to 0 after each transfer operation.
	size_t transferOffset;
	//Size of the transfer buffer.
	size_t transferSize;
	//All queued transfer operations.
	std::queue<TransferOperation> pendingTransfers;

	/**
	 * Adds a transfer operation to the pending transfer queue.
	 * @param buffer The destination buffer for the transfer.
	 * @param offset The offset into the destination buffer to place the data.
	 * @param size The size of the data.
	 * @param data The data to transfer.
	 */
	void queueTransfer(VkBuffer buffer, size_t offset, size_t size, const unsigned char* data);
};