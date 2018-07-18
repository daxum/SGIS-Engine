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

#include "RendererMemoryManager.hpp"

void RendererMemoryManager::addBuffer(const std::string& name, const VertexBufferInfo& info) {
	std::shared_ptr<RenderBufferData> renderData = createBuffer(info.usage, info.size);
	VertexBuffer buffer(info.format, info.size, info.usage, renderData);

	buffers.insert({name, {
		buffer,
		MemoryAllocator(info.size),
		MemoryAllocator(info.size)
	}});
}

void RendererMemoryManager::addMesh(const std::string& name, const std::string& buffer, const unsigned char* vertexData, size_t dataSize, const std::vector<uint32_t>& indices) {
	BufferData& bufferData = buffers.at(buffer);

	if (bufferData.vertexAllocations.count(name)) {
		throw std::runtime_error("Tried to reupload mesh!");
	}

	std::shared_ptr<AllocInfo> vertexAlloc = bufferData.vertexAllocator.getMemory(dataSize);
	std::shared_ptr<AllocInfo> indexAlloc = bufferData.indexAllocator.getMemory(sizeof(uint32_t) * indices.size());

	bufferData.vertexAllocations.insert({name, vertexAlloc});
	bufferData.indexAllocations.insert({name, indexAlloc});

	uploadMeshData(bufferData.buffer.getRenderData(), vertexAlloc->start, vertexAlloc->size, vertexData, indexAlloc->start, indexAlloc->size, indices.data());
}

bool RendererMemoryManager::markUsed(const std::string& mesh, const std::string& buffer) {
	BufferData& bufferData = buffers.at(buffer);

	//Mesh was never uploaded in the first place
	if (!bufferData.vertexAllocations.count(mesh)) {
		return false;
	}

	std::shared_ptr<AllocInfo> vertexAlloc = bufferData.vertexAllocations.at(mesh);
	std::shared_ptr<AllocInfo> indexAlloc = bufferData.indexAllocations.at(mesh);

	//If any of the mesh's data was evicted, it needs to be reuploaded. If only the indices/vertices were evicted,
	//then the other needs to be changed to match, because addMesh always loads both.
	//TODO: Change later?
	if (vertexAlloc->evicted || indexAlloc->evicted) {
		invalidateRange(bufferData.buffer.getRenderData(), vertexAlloc->start, vertexAlloc->size, indexAlloc->start, indexAlloc->size);

		bufferData.vertexAllocations.erase(mesh);
		bufferData.indexAllocations.erase(mesh);

		return false;
	}

	//Neither evicted, so reuse old data
	vertexAlloc->inUse = true;
	indexAlloc->inUse = true;

	return true;
}

void RendererMemoryManager::freeMesh(const std::string& mesh, const std::string& buffer) {
	BufferData& bufferData = buffers.at(buffer);

	if (!bufferData.vertexAllocations.count(mesh)) {
		return;
	}

	//Set as not in use
	std::shared_ptr<AllocInfo> vertexAlloc = bufferData.vertexAllocations.at(mesh);
	std::shared_ptr<AllocInfo> indexAlloc = bufferData.indexAllocations.at(mesh);

	vertexAlloc->inUse = false;
	indexAlloc->inUse = false;

	//If the mesh data is never needed again, just get rid of it.
	//Stream isn't really a stream buffer in the technical sense right now. It's just a
	//DEDICATED_SINGLE buffer in system memory.
	if (bufferData.buffer.getUsage() == BufferUsage::DEDICATED_SINGLE || bufferData.buffer.getUsage() == BufferUsage::STREAM) {
		invalidateRange(bufferData.buffer.getRenderData(), vertexAlloc->start, vertexAlloc->size, indexAlloc->start, indexAlloc->size);

		bufferData.vertexAllocations.erase(mesh);
		bufferData.indexAllocations.erase(mesh);
	}
}
