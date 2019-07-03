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

#include <list>
#include <array>
#include <string>
#include <memory>

struct AllocInfo {
	//Start of the allocated segment, in bytes from the front of the buffer.
	size_t start;
	//Size of the segment, in bytes.
	size_t size;
	//Whether the block is actively being used. Users of the allocator should
	//set this to false when it's time to free the block (it can also be set
	//to true again as long as evicted is still false).
	bool inUse;
	//Set to true when the block is removed from the allocator.
	//When this is set, all other data in this structure is invalid.
	bool evicted;
};

class MemoryAllocator {
public:
	/**
	 * Creates a memory allocator.
	 * @param bufferSize The size of the memory that this allocator manages.
	 */
	MemoryAllocator(size_t bufferSize);

	/**
	 * Copy Constructor.
	 */
	MemoryAllocator(const MemoryAllocator& memAlloc);

	/**
	 * Move constructor.
	 */
	MemoryAllocator(MemoryAllocator&& memAlloc);

	/**
	 * Allocates a new block of memory for the given name. When the memory is
	 * no longer needed, simply set the inUse member of the result to false
	 * and the block will be evicted as needed. In addition, the evicted value
	 * of the returned block should always be checked before setting inUse to
	 * true (for when a block is needed again).
	 * @param size The number of bytes to allocate.
	 * @param alignment The minimum alignment required for the returned block.
	 * @return Allocation information for the given name.
	 * @throw std::runtime_error if out of memory, std::out_of_range if size is zero.
	 */
	std::shared_ptr<AllocInfo> getMemory(size_t size, size_t alignment = 1);

	/**
	 * Defragments the memory pool by moving the allocations to be tightly packed
	 * in memory. Note that whatever is actually managing the memory needs to be
	 * updated to reflect the new values, or else very bad things are likely to
	 * happen. This will also remove all unused cached values, so if something is
	 * currently unused but needed after this operation, it is probably a good
	 * idea to set it to used first and reset it after.
	 */
	void defragment();

	/**
	 * For debugging, prints out the contents of the allocation list and currentPos.
	 */
	std::string printMemory();

	/**
	 * Ensures that the allocation list contains a contigous sequence of blocks, and
	 * throws an exception if it does not.
	 */
	void checkForLeak();

	/**
	 * Copy assignment.
	 */
	MemoryAllocator& operator=(const MemoryAllocator& memAlloc) {
		if (this != &memAlloc) {
			allocationList = memAlloc.allocationList;

			//Find correct current position by iterating through other's allocation list.
			currentPos = allocationList.begin();

			for (auto i = memAlloc.allocationList.begin(); i != memAlloc.allocationList.end(); i++) {
				if (i == memAlloc.currentPos) {
					break;
				}

				currentPos++;
			}
		}

		return *this;
	}

	/**
	 * Move assignment.
	 */
	MemoryAllocator& operator=(MemoryAllocator&& memAlloc) {
		allocationList = std::move(memAlloc.allocationList);
		currentPos = std::move(memAlloc.currentPos);

		return *this;
	}

private:
	typedef std::list<std::shared_ptr<AllocInfo>>::iterator AllocPos;

	//Contains all the allocation information.
	std::list<std::shared_ptr<AllocInfo>> allocationList;
	//Current position in the allocation list.
	AllocPos currentPos;

	/**
	 * Finds a range of free or evictable blocks that can be used for a new
	 * allocation of size bytes. Starts searching at currentPos.
	 * @param size The number of bytes to allocate.
	 * @param alignment The minimum alignment required by the returned range.
	 * @return The begin and one-past-the-end of the found range, in that order.
	 * @throw std::runtime_error if out of memory.
	 */
	std::array<AllocPos, 2> findFreeRange(size_t size, size_t alignment = 1);
};
