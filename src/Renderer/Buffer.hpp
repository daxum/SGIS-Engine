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

#include <unordered_map>

#include "MemoryAllocator.hpp"

enum class BufferStorage {
	HOST,
	DEVICE,
	DEVICE_HOST_VISIBLE
};

enum class BufferType {
	VERTEX,
	INDEX
};

class Buffer {
public:
	//Should match VkBufferUsageFlagBits
	enum Usage {
		TRANSFER_SRC = 0x00000001,
		TRANSFER_DST = 0x00000002,
		UNIFORM_BUFFER = 0x00000010,
		INDEX_BUFFER = 0x00000040,
		VERTEX_BUFFER = 0x00000080
	};

	/**
	 * Creates a buffer of the given size.
	 * @param bufferSize The size of the buffer, in bytes.
	 */
	Buffer(size_t bufferSize) :
		bufferSize(bufferSize),
		bufferAlloc(bufferSize) {}

	/**
	 * Allows deletion by subclasses.
	 */
	virtual ~Buffer() {}

	/**
	 * Allocates memory from the buffer and stores the allocation under
	 * the given name. Also handles eviction / reactivation logic.
	 * @param name The name used to identify the allocation.
	 * @param size The size of the allocation, in bytes.
	 * @param alignment The minimum required alignment of the allocation, in bytes.
	 * @return Information about where the allocation is in the buffer.
	 */
	std::shared_ptr<AllocInfo> allocate(const void* name, size_t size, size_t alignment = 1);

	/**
	 * Returns whether the buffer contains an allocation with the given name.
	 * If the allocation exists, it will be marked as used, on the assumption
	 * that the caller wouldn't be checking if it didn't want the memory.
	 * @param name The name to check.
	 * @return Whether an allocation has been made using the provided name.
	 */
	bool hasAlloc(const void* name) {
		if (allocations.count(name) && !allocations.at(name)->evicted) {
			allocations.at(name)->inUse = true;
			return true;
		}

		return false;
	}

	/**
	 * Marks an allocation as not in use, so that its memory can be repurposed
	 * if space runs out.
	 * @param name The name referencing the allocation.
	 */
	void setUnused(const void* name) { allocations.at(name)->inUse = false; }

	/**
	 * Completely frees an allocation from the buffer.
	 * @param name The allocation to free.
	 */
	void free(const void* name) {
		allocations.at(name)->inUse = false;
		allocations.erase(name);
	}

	/**
	 * Writes the data into the buffer at the provided offset. This does
	 * not have to upload the data immediately, only before the next
	 * rendering cycle.
	 * @param offset The offset into the buffer to place the data.
	 * @param size The size of the data to write, in bytes.
	 * @param data The data to write.
	 */
	virtual void write(size_t offset, size_t size, const unsigned char* data) = 0;

	/**
	 * Gets the size of the buffer.
	 * @return The size of the buffer, in bytes.
	 */
	size_t getBufferSize() const { return bufferSize; }

private:
	//Size of the buffer.
	size_t bufferSize;
	//Allocator for this buffer.
	MemoryAllocator bufferAlloc;
	//All allocations made from this buffer.
	std::unordered_map<const void*, std::shared_ptr<AllocInfo>> allocations;
};
