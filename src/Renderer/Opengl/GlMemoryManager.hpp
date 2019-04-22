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

#include "CombinedGl.h"
#include "Renderer/RendererMemoryManager.hpp"
#include "GlBuffer.hpp"

class GlMemoryManager : public RendererMemoryManager {
public:
	/**
	 * Initializes the memory manager.
	 * @param logConfig The configuration of the logger with which to log.
	 */
	GlMemoryManager(const LogConfig& logConfig);

	/**
	 * Deletes all created opengl objects in preparation for shutting down.
	 */
	void deleteObjects();

	/**
	 * Does nothing, as OpenGL doesn't have descriptor sets.
	 */
	void initializeDescriptors() override {}

protected:
	/**
	 * Creates a buffer with the underlying rendering api.
	 * @param usage Flags specifying what is allowed to be done with the buffer.
	 * @param storage Where the buffer will be stored.
	 * @param size The size of the buffer to create.
	 * @throw std::runtime_error if out of memory.
	 */
	std::shared_ptr<Buffer> createBuffer(uint32_t usage, BufferStorage storage, size_t size) { return std::make_shared<GlBuffer>(usage, storage, size); }

	/**
	 * Creates a type of uniform set for which descriptors can be allocated.
	 * @param name The name of the set.
	 * @param set The set itself. Mostly used for creating descriptor layouts.
	 */
	void createUniformSetType(const std::string& name, const UniformSet& set);

	/**
	 * Minimum uniform buffer alignment, not used until uniform buffers are.
	 * @return 1.
	 */
	virtual size_t getMinUniformBufferAlignment() override { return 1; }

	/**
	 * Allocates a descriptor set for the material. This can be called more than once
	 * for the same material - subsequent calls should be ignored.
	 * @param material The material to allocate a descriptor set for.
	 */
	void addMaterialDescriptors(const Material* material);

private:
	//Transfer buffer for uploading mesh data to static buffers on the gpu.
	GLuint transferBuffer;
	//Current size of the transfer buffer, will grow for larger meshes.
	size_t transferSize;
};
