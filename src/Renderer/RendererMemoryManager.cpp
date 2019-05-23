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
#include "RenderingEngine.hpp"
#include "ExtraMath.hpp"

RendererMemoryManager::RendererMemoryManager(const LogConfig& logConfig) :
	logger(logConfig),
	currentUniformOffset(0),
	screenObjectBufferSize(0) {}

void RendererMemoryManager::uniformBufferInit() {
	size_t materialSize = 0;
	size_t screenObjectSize = 0;

	for (const auto& setPair : uniformSets) {
		const UniformSet& set = setPair.second;

		//Align individual values to std140 rules
		size_t partiallyAlignedSize = Std140Aligner::getAlignedSize(set);
		//Align aligned size to min uniform buffer alignment (at most 256)
		size_t alignedSize = ExMath::roundToVal(partiallyAlignedSize, getMinUniformBufferAlignment());
		alignedSize *= set.getMaxUsers();

		switch (set.getType()) {
			case UniformSetType::MATERIAL: materialSize += alignedSize; break;
			//Multiply by three because can get uploaded once for each pass, fix later.
			case UniformSetType::PER_SCREEN: screenObjectSize += alignedSize * 3; break;
			case UniformSetType::PER_OBJECT: screenObjectSize += alignedSize; break;
			default: throw std::runtime_error("Missing uniform buffer type!");
		}
	}

	screenObjectBufferSize = screenObjectSize;
	screenObjectSize *= RenderingEngine::MAX_ACTIVE_FRAMES;

	uniformBuffers.at(UniformBufferType::MATERIAL) = createBuffer(Buffer::Usage::UNIFORM_BUFFER | Buffer::Usage::TRANSFER_DST, BufferStorage::DEVICE, materialSize);
	uniformBuffers.at(UniformBufferType::SCREEN_OBJECT) = createBuffer(Buffer::Usage::UNIFORM_BUFFER, BufferStorage::DEVICE_HOST_VISIBLE, screenObjectSize);
}

void RendererMemoryManager::addBuffer(const std::string& name, size_t size, BufferType type, BufferStorage storage) {
	if (buffers.count(name)) {
		throw std::runtime_error("Attempted to add duplicate buffer \"" + name + "\"");
	}

	uint32_t transferUsage = 0;

	if (storage == BufferStorage::DEVICE) {
		transferUsage |= Buffer::Usage::TRANSFER_DST;
	}

	uint32_t typeFlag = 0;

	switch (type) {
		case BufferType::VERTEX: typeFlag = Buffer::Usage::VERTEX_BUFFER; break;
		case BufferType::INDEX: typeFlag = Buffer::Usage::INDEX_BUFFER; break;
		default: throw std::runtime_error("Missing buffer type in RenderMemoryManager::addBuffer");
	};

	std::shared_ptr<Buffer> buffer = createBuffer(typeFlag | transferUsage, storage, size);

	buffers.emplace(name, buffer);

	ENGINE_LOG_INFO(logger, "Created buffer \"" + name + "\"");
}

void RendererMemoryManager::addMesh(const std::string& name, Mesh* mesh) {
	const Mesh::BufferInfo& bufferInfo = mesh->getBufferInfo();
	Buffer* vertexBuffer = buffers.at(bufferInfo.vertexName).get();
	Buffer* indexBuffer = buffers.at(bufferInfo.indexName).get();

	bool vertexPresent = vertexBuffer->hasAlloc(name);
	bool indexPresent = indexBuffer->hasAlloc(name);

	if (vertexPresent && indexPresent) {
		return;
	}

	const unsigned char* vertexData = std::get<0>(mesh->getMeshData());
	size_t vertexSize = std::get<1>(mesh->getMeshData());
	std::vector<uint32_t> indices = std::get<2>(mesh->getMeshData());

	std::shared_ptr<AllocInfo> vertexAlloc = vertexBuffer->allocate(name, vertexSize);
	std::shared_ptr<AllocInfo> indexAlloc = indexBuffer->allocate(name, sizeof(uint32_t) * indices.size());

	if (!indexPresent) {
		//Set index offsets
		for (size_t i = 0; i < indices.size(); i++) {
			indices.at(i) += vertexAlloc->start / mesh->getFormat()->getVertexSize();
		}

		indexBuffer->write(indexAlloc->start, indexAlloc->size, (const unsigned char*) indices.data());
		mesh->setIndexOffset(indexAlloc->start / sizeof(uint32_t));
	}

	if (!vertexPresent) {
		vertexBuffer->write(vertexAlloc->start, vertexAlloc->size, vertexData);
	}

	ENGINE_LOG_DEBUG(logger, "Uploaded mesh \"" + name + "\" to rendering engine");
}

void RendererMemoryManager::freeMesh(const std::string& name, const Mesh* mesh, bool persist) {
	const Mesh::BufferInfo& bufferInfo = mesh->getBufferInfo();
	Buffer* vertexBuffer = buffers.at(bufferInfo.vertexName).get();
	Buffer* indexBuffer = buffers.at(bufferInfo.indexName).get();

	if (persist) {
		//Set as not in use
		vertexBuffer->setUnused(name);
		indexBuffer->setUnused(name);

		ENGINE_LOG_DEBUG(logger, "Marked mesh \"" + name + "\" in buffer \"" + bufferInfo.vertexName + "\" as unused");
	}
	else {
		//If the mesh data is never needed again, just get rid of it.
		vertexBuffer->free(name);
		indexBuffer->free(name);

		ENGINE_LOG_DEBUG(logger, "Deleted transitory mesh \"" + name + "\"");
	}
}

void RendererMemoryManager::addMaterial(Material* material) {
	std::shared_ptr<Buffer> matBuffer = uniformBuffers.at(UniformBufferType::MATERIAL);

	//Only upload uniforms if the material has them and they haven't been uploaded already
	if (material->hasBufferedUniforms && !matBuffer->hasAlloc(material->name)) {
		ENGINE_LOG_DEBUG(logger, "Uploading material uniform data for \"" + material->name + "\" to rendering engine");

		const unsigned char* materialData = material->uniforms.getData().first;
		size_t dataSize = material->uniforms.getData().second;

		//Align the model data to the minimum alignment before allocating. If every allocation
		//does this, all allocated memory will end up implicitly aligned.
		size_t allocSize = ExMath::roundToVal(dataSize, getMinUniformBufferAlignment());

		//Upload uniform data
		std::shared_ptr<AllocInfo> uniAlloc = matBuffer->allocate(material->name, allocSize);
		matBuffer->write(uniAlloc->start, uniAlloc->size, materialData);
		material->uniformOffset = uniAlloc->start;

		ENGINE_LOG_DEBUG(logger, "Uploaded material uniform data for \"" + material->name + "\" to rendering engine");
	}

	//Allocate descriptor set
	addMaterialDescriptors(material);
}

uint32_t RendererMemoryManager::writePerFrameUniforms(const Std140Aligner& uniformProvider, size_t currentFrame) {
	const unsigned char* writeData = uniformProvider.getData().first;
	const size_t writeSize = uniformProvider.getData().second;

	//Handle uniform alignment
	currentUniformOffset = ExMath::roundToVal<uint32_t>(currentUniformOffset, getMinUniformBufferAlignment());

	const size_t writeOffset = screenObjectBufferSize * currentFrame + currentUniformOffset;

	uniformBuffers.at(UniformBufferType::SCREEN_OBJECT)->write(writeOffset, writeSize, writeData);
	currentUniformOffset += writeSize;

	return writeOffset;
}
