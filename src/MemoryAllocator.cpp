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

#include <vector>

#include "MemoryAllocator.hpp"

MemoryAllocator::MemoryAllocator(size_t bufferSize) :
	allocationList(1, std::make_shared<AllocInfo>(AllocInfo{0, bufferSize, false, false})),
	currentPos(allocationList.begin()) {

}

MemoryAllocator::MemoryAllocator(const MemoryAllocator& memAlloc) :
	allocationList(memAlloc.allocationList),
	currentPos(allocationList.begin()) {

	for (auto i = memAlloc.allocationList.begin(); i != memAlloc.allocationList.end(); i++) {
		if (i == memAlloc.currentPos) {
			break;
		}

		currentPos++;
	}
}

MemoryAllocator::MemoryAllocator(MemoryAllocator&& memAlloc) :
	allocationList(std::move(memAlloc.allocationList)),
	currentPos(std::move(memAlloc.currentPos)) {

}

std::shared_ptr<AllocInfo> MemoryAllocator::getMemory(size_t size) {
	if (size == 0) {
		throw std::out_of_range("Attempt to allocate 0 bytes!");
	}

	//Find free block or list of free blocks that are large enough
	std::array<AllocPos, 2> foundRange = findFreeRange(size);

	//Invalidate and chuck old value, replace with new one
	std::shared_ptr<AllocInfo> newVal = std::make_shared<AllocInfo>(**foundRange.at(0));
	(*foundRange.at(0))->evicted = true;
	*foundRange.at(0) = newVal;

	//Merge range
	AllocPos pos = std::next(foundRange.at(0));

	while (pos != foundRange.at(1)) {
		(*foundRange.at(0))->size += (*pos)->size;
		(*pos)->evicted = true;
		pos++;
		allocationList.erase(std::prev(pos));
	}

	//Add extra
	size_t extra = (*foundRange.at(0))->size - size;

	if (extra > 0) {
		(*foundRange.at(0))->size = size;

		AllocInfo extraInfo = {
			(*foundRange.at(0))->start + (*foundRange.at(0))->size,
			extra,
			false,
			false
		};

		allocationList.insert(foundRange.at(1), std::make_shared<AllocInfo>(extraInfo));
	}

	//Update currentPos
	currentPos = std::next(foundRange.at(0));

	if (currentPos == allocationList.end()) {
		currentPos = allocationList.begin();
	}

	//Return allocated block
	(*foundRange.at(0))->inUse = true;

	return *foundRange.at(0);
}

void MemoryAllocator::defragment() {
	std::vector<std::shared_ptr<AllocInfo>> keptAllocations;
	size_t memorySize = 0;
	size_t keptSize = 0;

	//Move all allocations to vector, caculate max size
	for (std::shared_ptr<AllocInfo> info : allocationList) {
		memorySize += info->size;

		if (info->inUse) {
			keptAllocations.push_back(info);
			keptSize += info->size;
		}
		else {
			info->evicted = true;
		}
	}

	allocationList.clear();

	//Put all kept allocations back into list
	for (std::shared_ptr<AllocInfo> info : keptAllocations) {
		info->start = allocationList.empty() ? 0 : (allocationList.back()->start + allocationList.back()->size);
		allocationList.push_back(info);
	}

	//Put remaining empty block at end
	std::shared_ptr<AllocInfo> freeBlock = std::make_shared<AllocInfo>(AllocInfo{
		keptSize,
		memorySize - keptSize,
		false,
		false
	});

	allocationList.push_back(freeBlock);

	//Update currentPos
	currentPos = std::prev(allocationList.end());
}

std::string MemoryAllocator::printMemory() {
	std::string out = "Allocation list (<inUse>: <start>, <size>):\n";

	for (std::shared_ptr<AllocInfo> info : allocationList) {
		out += info->inUse ? "Allocated: " : "Free: ";
		out += std::to_string(info->start) + ", ";
		out += std::to_string(info->size) + "\n";
	}

	out += std::string("CurrentPos: ") + ((*currentPos)->inUse ? "Allocated, " : "Free, ") + "Start - " + std::to_string((*currentPos)->start) + ", Size - " + std::to_string((*currentPos)->size);

	return out;
}

std::array<MemoryAllocator::AllocPos, 2> MemoryAllocator::findFreeRange(size_t size) {
	std::array<AllocPos, 2> range = {currentPos, std::next(currentPos)};
	AllocPos startPos = currentPos;

	while (true) {
		//Find free block
		while ((*range.at(0))->inUse) {
			range.at(0)++;

			if (range.at(0) == allocationList.end()) {
				range.at(0) = allocationList.begin();
			}

			if (range.at(0) == startPos) {
				throw std::runtime_error("Allocation error: Out of memory");
			}
		}

		//Find size of free section
		range.at(1) = range.at(0);
		size_t totalSize = 0;

		while (totalSize < size && !(*range.at(1))->inUse) {
			totalSize += (*range.at(1))->size;
			range.at(1)++;

			//Stop searching if we hit the end
			if (range.at(1) == allocationList.end()) {
				break;
			}
		}

		//If section big enough, return it, else keep looking
		if (totalSize >= size) {
			return range;
		}

		range.at(0) = range.at(1);

		if (range.at(0) == allocationList.end()) {
			range.at(0) = allocationList.begin();
		}
	}
}
