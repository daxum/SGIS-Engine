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

#include "vk_mem_alloc.h"

#include "VkObjectHandler.hpp"
#include "Renderer/Buffer.hpp"

class VkMemoryManager;

class VkBufferContainer : public Buffer {
public:
	/**
	 * Creates the buffer with the given parameters.
	 * @param memoryManager The parent memory manager.
	 * @param objects The vulkan object container.
	 * @param allocator The allocator to use when creating the buffer.
	 * @param usage How the buffer will be used. Forwarded directly to Vulkan.
	 * @param storage Where the buffer should (preferably) be stored.
	 * @param size The size of the buffer.
	 */
	VkBufferContainer(VkMemoryManager* memoryManager, VkObjectHandler& objects, VmaAllocator allocator, uint32_t usage, BufferStorage storage, size_t size);

	/**
	 * Frees the allocation, etc etc.
	 */
	~VkBufferContainer() { vmaDestroyBuffer(allocator, buffer, allocation); }

	/**
	 * Gets the VkBuffer associated with this buffer.
	 * @return The VkBuffer handle.
	 */
	VkBuffer getBuffer() const { return buffer; }

	/**
	 * Writes the data into the buffer at the given offset. If the memory
	 * is directly writable, nothing special happens, but if it isn't, a
	 * transfer is queued to execute later.
	 * @param offset The offset into the buffer to write.
	 * @param size The size of the data to write, in bytes.
	 * @param data The data to write.
	 */
	void write(size_t offset, size_t size, const unsigned char* data) override;

private:
	//Logger, for logging.
	Logger logger;
	//Pointer to the memory manager, for transfers.
	VkMemoryManager* memoryManager;
	//The allocator the buffer was allocated from.
	VmaAllocator allocator;
	//The buffer object.
	VkBuffer buffer;
	//The memory object that the buffer is stored in.
	VmaAllocation allocation;
	//The mapped buffer memory, if host visible. If not, this will be nullptr.
	unsigned char* mappedMem;
};
