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

#include <memory>

enum class BufferUsage {
	//Reference counted buffer located in gpu memory, if possible. Meshes remain in
	//the buffer even if nothing references them, for possible reuse later.
	//They are only evicted if memory runs out.
	DEDICATED_LAZY,
	//Similar to above, except unused meshes are immediately invalidated and
	//have to be reuploaded if used again.
	DEDICATED_SINGLE,
	//This is just a stream buffer.
	STREAM
};

class VertexBuffer {
public:
	/**
	 * Constructor.
	 * @param size The size of the buffer.
	 * @param usage Where the buffer is stored, and how meshes stored in it should be deleted.
	 */
	VertexBuffer(size_t size, BufferUsage usage) :
		bufferSize(size),
		usage(usage) {}

	/**
	 * Gets the BufferUsage provided in the constructor.
	 * @return The usage for the buffer.
	 */
	BufferUsage getUsage() const { return usage; }

	/**
	 * Gets the size of the vertex buffer, in bytes.
	 * @return The size of the vertex buffer.
	 */
	size_t getBufferSize() { return bufferSize; }

private:
	//Size of the vertex buffer.
	size_t bufferSize;
	//How the buffer is used.
	BufferUsage usage;
};
