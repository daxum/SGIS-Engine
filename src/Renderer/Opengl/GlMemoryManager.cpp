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

#include <stdexcept>
#include <cstring>

#include "GlMemoryManager.hpp"

GlMemoryManager::GlMemoryManager(const LogConfig& logConfig) :
	RendererMemoryManager(logConfig),
	transferBuffer(0),
	transferSize(0) {

}

void GlMemoryManager::deleteObjects() {
	glDeleteBuffers(1, &transferBuffer);
	deleteBuffers();
}

/*std::shared_ptr<RenderBufferData> GlMemoryManager::createBuffer(const std::vector<VertexElement>& vertexFormat, BufferUsage usage, size_t size) {
	std::shared_ptr<GlBufferData> data = std::make_shared<GlBufferData>();

	//Create array and buffers
	glGenVertexArrays(1, &data->vertexArray);
	glGenBuffers(1, &data->vertexBufferId);
	glGenBuffers(1, &data->indexBufferId);

	//Determine usage
	GLenum glUsage = 0;

	switch (usage) {
		case BufferUsage::DEDICATED_LAZY: glUsage = GL_STATIC_COPY; break;
		case BufferUsage::DEDICATED_SINGLE: glUsage = GL_STATIC_COPY; break;
		case BufferUsage::STREAM: glUsage = GL_STREAM_DRAW; break;
		default: throw std::runtime_error("Missing BufferUsage case in switch statement!");
	}

	data->useTransfer = usage == BufferUsage::DEDICATED_LAZY || usage == BufferUsage::DEDICATED_SINGLE;

	//Buffer memory allocation
	glBindVertexArray(data->vertexArray);

	ENGINE_LOG_DEBUG(logger, "Creating " + std::to_string(size) + " byte vertex buffer.");

	glBindBuffer(GL_ARRAY_BUFFER, data->vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, size, nullptr, glUsage);

	ENGINE_LOG_DEBUG(logger, "Creating " + std::to_string(size) + " byte index buffer.");

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, glUsage);

	//Enable array things from vertex format
	size_t vertexSize = 0;

	for (const VertexElement& elem : vertexFormat) {
		vertexSize += sizeFromVertexType(elem.type);
	}

	size_t offset = 0;

	ENGINE_LOG_DEBUG(logger, "Buffer format:");

	for (size_t i = 0; i < vertexFormat.size(); i++) {
		size_t elementSize = sizeFromVertexType(vertexFormat.at(i).type);
		uint32_t elementCount = (uint32_t) (elementSize / sizeof(float));

		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, elementCount, GL_FLOAT, GL_FALSE, vertexSize, (void*) offset);
		ENGINE_LOG_DEBUG(logger, "    Binding " + std::to_string(i) + ": Components=" + std::to_string(elementCount) + ", Stride=" + std::to_string(vertexSize) + ", offset=" + std::to_string(offset));

		offset += elementSize;
	}

	glBindVertexArray(0);

	return data;
}*/

/*void GlMemoryManager::uploadMeshData(const VertexBuffer& buffer, const std::string& mesh, size_t offset, size_t size, const unsigned char* vertexData, size_t indexOffset, size_t indexSize, const uint32_t* indexData) {
	std::shared_ptr<const GlBufferData> glBuffer = std::static_pointer_cast<const GlBufferData>(buffer.getRenderData());

	if (glBuffer->useTransfer) {
		ENGINE_LOG_DEBUG(logger, "Using transfer for mesh \"" + mesh + "\"");

		if (transferSize == 0) {
			glGenBuffers(1, &transferBuffer);
		}

		glBindBuffer(GL_COPY_READ_BUFFER, transferBuffer);

		if (transferSize < size || transferSize < indexSize) {
			transferSize = std::max(size, indexSize);
			glBufferData(GL_COPY_READ_BUFFER, transferSize, nullptr, GL_STREAM_DRAW);
		}

		//Transfer vertex data
		void* bufferData = glMapBufferRange(GL_COPY_READ_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(bufferData, vertexData, size);
		glUnmapBuffer(GL_COPY_READ_BUFFER);

		glBindBuffer(GL_COPY_WRITE_BUFFER, glBuffer->vertexBufferId);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, offset, size);

		ENGINE_LOG_DEBUG(logger, "Copied vertex data into buffer at offset " + std::to_string(offset) + " and size " + std::to_string(size));

		//Transfer index data
		bufferData = glMapBufferRange(GL_COPY_READ_BUFFER, 0, indexSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(bufferData, indexData, indexSize);
		glUnmapBuffer(GL_COPY_READ_BUFFER);

		glBindBuffer(GL_COPY_WRITE_BUFFER, glBuffer->indexBufferId);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, indexOffset, indexSize);

		ENGINE_LOG_DEBUG(logger, "Copied index data into buffer at offset " + std::to_string(indexOffset) + " and size " + std::to_string(indexSize));
	}
	//No transfer, eg. stream buffers (or integrated gpus, if there's a way to detect them...)
	else {
		ENGINE_LOG_DEBUG(logger, "Directly copying mesh data for mesh \"" + mesh + "\"");

		//Copy vertex data
		glBindBuffer(GL_COPY_WRITE_BUFFER, glBuffer->vertexBufferId);

		void* bufferData = glMapBufferRange(GL_COPY_WRITE_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(bufferData, vertexData, size);
		glUnmapBuffer(GL_COPY_WRITE_BUFFER);

		//Copy index data
		glBindBuffer(GL_COPY_WRITE_BUFFER, glBuffer->indexBufferId);

		bufferData = glMapBufferRange(GL_COPY_WRITE_BUFFER, indexOffset, indexSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(bufferData, indexData, indexSize);
		glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	}

	//Add mesh to data map
	meshData.insert({mesh, GlMeshRenderData{indexOffset, (uint32_t) (indexSize / sizeof(uint32_t))}});
}*/
