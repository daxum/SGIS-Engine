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

#include "Renderer/Buffer.hpp"
#include "Engine.hpp"
#include "Logger.hpp"

class GlBuffer : public Buffer {
public:
	/**
	 * Creates an OpenGL buffer.
	 * @param usage How the buffer will be used.
	 * @param storage Where to attempt to store the buffer.
	 * @param size The size of the buffer, in bytes.
	 */
	GlBuffer(uint32_t usage, BufferStorage storage, size_t size) :
		Buffer(size),
		bufferId(0),
		logger(Engine::instance->getConfig().rendererLog) {

		glGenBuffers(1, &bufferId);

		//Determine usage
		GLenum glUsage = 0;

		switch (storage) {
			case BufferStorage::DEVICE: glUsage = GL_STATIC_DRAW; break;
			case BufferStorage::DEVICE_HOST_VISIBLE: glUsage = GL_DYNAMIC_DRAW; break;
			case BufferStorage::HOST: glUsage = GL_STREAM_DRAW; break;
			default: throw std::runtime_error("Missing BufferUsage case in switch statement!");
		}

		GLenum bindPoint = 0;

		if (usage & Buffer::Usage::VERTEX_BUFFER) bindPoint = GL_ARRAY_BUFFER;
		else if (usage & Buffer::Usage::INDEX_BUFFER) bindPoint = GL_ELEMENT_ARRAY_BUFFER;
		else if (usage & Buffer::Usage::UNIFORM_BUFFER) bindPoint = GL_UNIFORM_BUFFER;

		//Allocate buffer memory
		glBindBuffer(bindPoint, bufferId);
		glBufferData(bindPoint, size, nullptr, glUsage);
	}

	/**
	 * Gets the buffer id, for rendering.
	 * @return The buffer id.
	 */
	GLuint getBufferId() const { return bufferId; }

	/**
	 * Writes the data into the buffer.
	 * @param offset The offset into the buffer to write.
	 * @param size The size of the data to write.
	 * @param data The data to write.
	 */
	void write(size_t offset, size_t size, const unsigned char* data) override {
		if (offset + size > getBufferSize()) {
			ENGINE_LOG_ERROR(logger, "Bad buffer write: offset=" + std::to_string(offset) + ", size=" + std::to_string(size) + ", bufferSize=" + std::to_string(getBufferSize()));
			throw std::runtime_error("Attempt to write past end of buffer!");
		}

		//Just let the driver figure it out for now, this is legacy anyway.
		glBindBuffer(GL_COPY_WRITE_BUFFER, bufferId);

		void* bufferData = glMapBufferRange(GL_COPY_WRITE_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
		memcpy(bufferData, data, size);
		glUnmapBuffer(GL_COPY_WRITE_BUFFER);
	}

private:
	//Buffer object.
	GLuint bufferId;
	//Logger for the buffer.
	Logger logger;
};
