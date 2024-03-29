/******************************************************************************
 * SGIS-Engine - the engine for SGIS
 * Copyright (C) 2018, 2021
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

#include "VkBufferContainer.hpp"
#include "VkMemoryManager.hpp"
#include "Engine.hpp"

VkBufferContainer::VkBufferContainer(VkMemoryManager* memoryManager, VkObjectHandler& objects, VmaAllocator allocator, uint32_t usage, BufferStorage storage, size_t size) :
	Buffer(size),
	logger(Engine::instance->getConfig().rendererLog),
	memoryManager(memoryManager),
	allocator(allocator),
	buffer(VK_NULL_HANDLE),
	allocation(VK_NULL_HANDLE),
	mappedMem(nullptr) {

	//Happens with uniform buffers with no descriptor sets
	if (size == 0) {
		return;
	}

	VmaMemoryUsage memoryUsage;

	switch (storage) {
		case BufferStorage::DEVICE:
			memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case BufferStorage::DEVICE_HOST_VISIBLE:
			memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			break;
		case BufferStorage::HOST:
			memoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
			break;
		default: throw std::runtime_error("Missing buffer storage type in memory manager");
	}

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t bufferUsers[] = { objects.getGraphicsQueueIndex(), objects.getTransferQueueIndex() };

	if (objects.hasUniqueTransfer()) {
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferCreateInfo.queueFamilyIndexCount = 2;
		bufferCreateInfo.pQueueFamilyIndices = bufferUsers;
	}

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = memoryUsage;
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo allocInfo;

	if (vmaCreateBuffer(allocator, &bufferCreateInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	mappedMem = (unsigned char*) allocInfo.pMappedData;

	ENGINE_LOG_DEBUG(logger, "Created " + std::to_string(size) + " byte Vkbuffer stored in " + (mappedMem ? "host visible" : "device") + " memory");
}

void VkBufferContainer::write(size_t offset, size_t size, const unsigned char* data) {
	if (offset + size > getBufferSize()) {
		ENGINE_LOG_ERROR(logger, "Bad buffer write: offset=" + std::to_string(offset) + ", size=" + std::to_string(size) + ", bufferSize=" + std::to_string(getBufferSize()));
		throw std::runtime_error("Attempt to write past end of buffer!");
	}

	if (mappedMem) {
		memcpy(mappedMem + offset, data, size);
	}
	else {
		memoryManager->queueTransfer(buffer, offset, size, data);
	}
}
