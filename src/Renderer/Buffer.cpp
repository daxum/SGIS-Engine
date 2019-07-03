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

#include "Buffer.hpp"

std::shared_ptr<AllocInfo> Buffer::allocate(const void* name, size_t size, size_t alignment) {
	if (allocations.count(name)) {
		//Have some sort of allocation already
		std::shared_ptr<AllocInfo> alloc = allocations.at(name);

		if (alloc->evicted) {
			//Evicted, get new memory block
			allocations.erase(name);
		}
		else {
			//For now, assume size is correct. This is true for everything
			//that currently uses the buffers (meshes, materials)
			alloc->inUse = true;
			return alloc;
		}
	}

	//No usable previous allocation, so make a new one

	std::shared_ptr<AllocInfo> alloc = bufferAlloc.getMemory(size, alignment);
	allocations.emplace(name, alloc);
	return alloc;
}
