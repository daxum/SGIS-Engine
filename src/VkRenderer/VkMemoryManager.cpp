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

#define VMA_IMPLEMENTATION

//Unused variable due to VMA_ASSERT
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "vk_mem_alloc.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "VkMemoryManager.hpp"

VkMemoryManager::VkMemoryManager(const LogConfig& logConfig, VkObjectHandler& objects) :
	RendererMemoryManager(logConfig),
	objects(objects),
	allocator(VK_NULL_HANDLE),
	transferBuffer(VK_NULL_HANDLE),
	transferAllocation(VK_NULL_HANDLE),
	transferFence(VK_NULL_HANDLE),
	transferOffset(0),
	transferSize(0),
	pendingTransfers() {

}

void VkMemoryManager::init() {
	//Create allocator
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = objects.getPhysicalDevice();
	allocatorCreateInfo.device = objects.getDevice();

	vmaCreateAllocator(&allocatorCreateInfo, &allocator);

	//Create fence
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateFence(objects.getDevice(), &fenceInfo, nullptr, &transferFence) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create transfer fence!");
	}
}

void VkMemoryManager::deinit() {
	deleteBuffers();

	if (transferSize != 0) {
		vmaDestroyBuffer(allocator, transferBuffer, transferAllocation);
	}

	vkDestroyFence(objects.getDevice(), transferFence, nullptr);
	vmaDestroyAllocator(allocator);

	while (!pendingTransfers.empty()) {
		TransferOperation& transferOp = pendingTransfers.front();
		delete transferOp.data;
		pendingTransfers.pop();
	}
}

std::shared_ptr<RenderBufferData> VkMemoryManager::createBuffer(const std::vector<VertexElement>& vertexFormat, BufferUsage usage, size_t size) {
	VkBufferUsageFlags transferFlags = 0;
	VmaMemoryUsage memoryUsage;

	switch (usage) {
		case BufferUsage::DEDICATED_LAZY:
			transferFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case BufferUsage::DEDICATED_SINGLE:
			transferFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case BufferUsage::STREAM:
			memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			break;
		default: throw std::runtime_error("Missing buffer usage in memory manager");
	}

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | transferFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t bufferUsers[] = { objects.getGraphicsQueueIndex(), objects.getTransferQueueIndex() };

	if (objects.hasUniqueTransfer()) {
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferCreateInfo.queueFamilyIndexCount = 2;
		bufferCreateInfo.pQueueFamilyIndices = bufferUsers;
	}

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = memoryUsage;

	VkBuffer vertexBuffer;
	VmaAllocation vertexAllocation;

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &vertexBuffer, &vertexAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | transferFlags;

	VkBuffer indexBuffer;
	VmaAllocation indexAllocation;

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &indexBuffer, &indexAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create index buffer!");
	}

	return std::make_shared<VkBufferData>(allocator, vertexBuffer, indexBuffer, vertexAllocation, indexAllocation);
}

void VkMemoryManager::uploadMeshData(const VertexBuffer& buffer, const std::string& mesh, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) {
	std::shared_ptr<const VkBufferData> bufferData = std::static_pointer_cast<const VkBufferData>(buffer.getRenderData());

	queueTransfer(bufferData->vertexBuffer, offset, size, vertexData);
	queueTransfer(bufferData->indexBuffer, indexOffset, indexSize, (const unsigned char*) indexData);
}

void VkMemoryManager::queueTransfer(VkBuffer buffer, size_t offset, size_t size, const unsigned char* data) {
	if (transferOffset + size < transferSize) {
		transferOffset = 0;
	}

	TransferOperation transferOp = {
		buffer,
		new unsigned char[size],
		size,
		offset,
		transferOffset
	};

	memcpy(transferOp.data, data, size);
	pendingTransfers.push(transferOp);

	transferOffset += size;
}
