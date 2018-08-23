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
#include "ExtraMath.hpp"

RendererMemoryManager::RendererMemoryManager(const LogConfig& logConfig) :
	logger(logConfig) {}

void RendererMemoryManager::UniformBufferInit() {
	size_t staticModelSize = 0;
	size_t dynamicModelSize = 0;
	size_t screenObjectSize = 0;

	for (const auto& setPair : uniformSets) {
		const UniformSet& set = setPair.second;

		//Align individual values to std140 rules
		size_t partiallyAlignedSize = Std140Aligner::getAlignedSize(set);
		//Align aligned size to min uniform buffer alignment (at most 256)
		size_t alignedSize = ExMath::roundToVal(partiallyAlignedSize, getMinUniformBufferAlignment());
		alignedSize *= set.maxUsers;

		switch (set.setType) {
			case UniformSetType::MODEL_STATIC: staticModelSize += alignedSize; break;
			case UniformSetType::MODEL_DYNAMIC: dynamicModelSize += alignedSize; break;
			//Multiply by three because can get uploaded once for each pass, fix later.
			case UniformSetType::PER_SCREEN: screenObjectSize += alignedSize * 3; break;
			case UniformSetType::PER_OBJECT: screenObjectSize += alignedSize; break;
			default: throw std::runtime_error("Missing uniform buffer type!");
		}
	}

	createUniformBuffers(staticModelSize, dynamicModelSize, screenObjectSize);

	staticModelUniformAlloc = std::make_shared<MemoryAllocator>(staticModelSize);
	dynamicModelUniformAlloc = std::make_shared<MemoryAllocator>(dynamicModelSize);
}

void RendererMemoryManager::addBuffer(const std::string& name, const VertexBufferInfo& info) {
	std::shared_ptr<RenderBufferData> renderData = createBuffer(info.format, info.usage, info.size);
	VertexBuffer buffer(info.format, info.size, info.usage, renderData);

	buffers.insert({name, {
		buffer,
		MemoryAllocator(info.size),
		MemoryAllocator(info.size)
	}});

	ENGINE_LOG_INFO(logger, "Created buffer \"" + name + "\"");
}

void RendererMemoryManager::addMesh(const std::string& name, const std::string& buffer, const unsigned char* vertexData, size_t dataSize, std::vector<uint32_t> indices) {
	BufferData& bufferData = buffers.at(buffer);

	if (bufferData.vertexAllocations.count(name)) {
		throw std::runtime_error("Tried to reupload mesh!");
	}

	std::shared_ptr<AllocInfo> vertexAlloc = bufferData.vertexAllocator.getMemory(dataSize);
	std::shared_ptr<AllocInfo> indexAlloc = bufferData.indexAllocator.getMemory(sizeof(uint32_t) * indices.size());

	bufferData.vertexAllocations.insert({name, vertexAlloc});
	bufferData.indexAllocations.insert({name, indexAlloc});

	//Set index offsets
	for (size_t i = 0; i < indices.size(); i++) {
		indices.at(i) += vertexAlloc->start / bufferData.buffer.getVertexSize();
	}

	uploadMeshData(bufferData.buffer, name, vertexAlloc->start, vertexAlloc->size, vertexData, indexAlloc->start, indexAlloc->size, indices.data());

	ENGINE_LOG_DEBUG(logger, "Uploaded mesh \"" + name + "\" to rendering engine");
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
		ENGINE_LOG_DEBUG(logger, "Mesh \"" + mesh + "\" from buffer \"" + buffer + "\" requested, but was evicted");

		invalidateMesh(mesh);

		bufferData.vertexAllocations.erase(mesh);
		bufferData.indexAllocations.erase(mesh);

		return false;
	}

	//Neither evicted, so reuse old data
	if (!vertexAlloc->inUse || !indexAlloc->inUse) {
		ENGINE_LOG_DEBUG(logger, "Reactivated mesh \"" + mesh + "\" in buffer \"" + buffer + "\"");

		vertexAlloc->inUse = true;
		indexAlloc->inUse = true;
	}

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

	ENGINE_LOG_DEBUG(logger, "Marked mesh \"" + mesh + "\" in buffer \"" + buffer + "\" as unused");

	//If the mesh data is never needed again, just get rid of it.
	//Stream isn't really a stream buffer in the technical sense right now. It's just a
	//DEDICATED_SINGLE buffer in system memory.
	if (bufferData.buffer.getUsage() == BufferUsage::DEDICATED_SINGLE || bufferData.buffer.getUsage() == BufferUsage::STREAM) {
		invalidateMesh(mesh);

		bufferData.vertexAllocations.erase(mesh);
		bufferData.indexAllocations.erase(mesh);

		ENGINE_LOG_DEBUG(logger, "Deleted transitory mesh \"" + mesh + "\"");
	}
}

void RendererMemoryManager::addModel(const std::string& name, const Model& model) {
	//If data is present, don't reupload
	if (modelDataMap.count(name)) {
		ENGINE_LOG_SPAM(logger, "Model \"" + name + "\" possibly present on rendering engine");

		ModelUniformData& uniformData = modelDataMap.at(name);

		//If data was evicted, remove old allocation so the insert works below
		if (uniformData.allocation->evicted) {
			ENGINE_LOG_DEBUG(logger, "Model \"" + name + "\" evicted, reuploading");
			modelDataMap.erase(name);
		}
		//Data not evicted, so just reactivate it. Descriptor sets are always persistent, so those
		//don't need checking.
		else {
			uniformData.allocation->inUse = true;
			return;
		}
	}

	ENGINE_LOG_DEBUG(logger, "Uploading model uniform data for \"" + name + "\" to rendering engine");

	//Determine set type, model type, and retrieve data

	const UniformSet& set = getUniformSet(model.uniformSet);
	bool staticModel = false;
	size_t dataSize = model.uniforms.getData().second;
	const unsigned char* modelData = model.uniforms.getData().first;

	switch (set.setType) {
		case UniformSetType::MODEL_STATIC : staticModel = true; break;
		case UniformSetType::MODEL_DYNAMIC: staticModel = false; break;
		default: throw std::runtime_error("Model descriptor set isn't a model type!");
	}

	//Align the model data to the minimum alignment before allocating. If every allocation
	//does this, all allocated memory will end up implicitly aligned.
	size_t allocSize = ExMath::roundToVal(dataSize, getMinUniformBufferAlignment());

	//Make allocation and upload uniform data

	std::shared_ptr<AllocInfo> allocation = staticModel ? staticModelUniformAlloc->getMemory(allocSize) : dynamicModelUniformAlloc->getMemory(allocSize);

	uploadModelData(staticModel ? UniformBufferType::STATIC_MODEL : UniformBufferType::DYNAMIC_MODEL, allocation->start, dataSize, modelData);

	//Allocate descriptor set for static models. If the model already has a descriptor set, this call will be ignored.

	if (staticModel) {
		addModelDescriptors(model);
	}

	//Lastly, add to data map

	//Use dataSize instead of allocation->size to avoid the padding, as the alignment only applies to offset
	modelDataMap.insert({name, ModelUniformData{allocation, allocation->start, dataSize}});

	ENGINE_LOG_DEBUG(logger, "Uploaded model uniform data for \"" + name + "\" to rendering engine");
}

void RendererMemoryManager::freeModel(const std::string& name, const Model& model) {
	const UniformSet& set = getUniformSet(model.uniformSet);
	bool staticModel = false;

	switch (set.setType) {
		case UniformSetType::MODEL_STATIC : staticModel = true; break;
		case UniformSetType::MODEL_DYNAMIC: staticModel = false; break;
		default: throw std::runtime_error("Model descriptor set isn't a model type!");
	}

	//Mark static models as not in use, and just remove dynamic models completely
	if (staticModel) {
		modelDataMap.at(name).allocation->inUse = false;
	}
	else {
		modelDataMap.erase(name);
	}
}
