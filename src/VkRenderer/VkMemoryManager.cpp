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
	allocator(VK_NULL_HANDLE) {

	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.physicalDevice = objects.getPhysicalDevice();
	allocatorCreateInfo.device = objects.getDevice();

	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
}

VkMemoryManager::~VkMemoryManager() {
	vmaDestroyAllocator(allocator);
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

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = memoryUsage;

	VkBuffer buffer;
	VmaAllocation allocation;

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	return std::make_shared<VkBufferData>(allocator, buffer, allocation);
}
