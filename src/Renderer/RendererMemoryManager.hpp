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

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "Buffer.hpp"
#include "MemoryAllocator.hpp"
#include "EngineConfig.hpp"
#include "Logger.hpp"
#include "ShaderInfo.hpp"
#include "Models/Material.hpp"
#include "Models/Mesh.hpp"

//An interface to the rendering engine's memory manager.
class RendererMemoryManager {
public:
	/**
	 * Creates a memory manager and initializes a logger.
	 * @param logConfig The logger configuration.
	 */
	RendererMemoryManager(const LogConfig& logConfig);

	/**
	 * Destructor.
	 */
	virtual ~RendererMemoryManager() {}

	/**
	 * Called by the engine after descriptor sets are loaded to initialize the uniform
	 * buffers and their corresponding memory managers.
	 */
	void uniformBufferInit();

	/**
	 * Initializes descriptor sets or similar, if the rendering engine supports that kind of thing.
	 * This is mainly used to allocate descriptor pools in the vulkan renderer.
	 * Textures should be loaded before this is called!
	 */
	virtual void initializeDescriptors() = 0;

	/**
	 * Adds a buffer to the memory manager.
	 * @param name The name of the buffer.
	 * @param size The size of the buffer, in bytes.
	 * @param type The type of the buffer to create.
	 * @param storage Where the buffer will be stored.
	 * @throw std::runtime_error if not enough memory.
	 */
	void addBuffer(const std::string& name, size_t size, BufferType type, BufferStorage storage);

	/**
	 * Adds a uniform set to the memory manager, and allocates descriptor sets or
	 * similar for it.
	 * @param name The name of the uniform set.
	 * @param set The uniform set to add.
	 */
	void addUniformSet(const std::string& name, const UniformSet& set) {
		uniformSets.emplace(name, set);
		createUniformSetType(name, set);
	}

	/**
	 * Gets the buffer with the provided name.
	 * @param name The name of the buffer.
	 * @return the buffer stored under name.
	 * @throw std::out_of_range If there isn't a buffer with the given name.
	 */
	const Buffer* getBuffer(const std::string& name) { return buffers.at(name).get(); }

	/**
	 * Returns the uniform set with the given name, used during model loading.
	 * @param set The name of the set to get.
	 * @return The set with the given name.
	 */
	const UniformSet& getUniformSet(const std::string& set) { return uniformSets.at(set); }

	/**
	 * Adds a mesh to the provided buffer, and creates any resources needed to render it.
	 * If the mesh has already been added, nothing happens.
	 * @param name The name to store the mesh under.
	 * @param mesh The mesh to upload.
	 * @throw std::runtime_error If the name already exists or if out of memory.
	 */
	void addMesh(const std::string& name, Mesh* mesh);

	/**
	 * Marks the mesh as unused. If the mesh is transitory (BufferUsage::DEDICATED_SINGLE),
	 * all references to it will be completely deleted from the buffer, otherwise it will
	 * only be marked as unused. If the mesh doesn't exist, nothing happens.
	 * @param mesh The mesh to free.
	 * @param buffer The buffer that contains the mesh.
	 * @param persist Whether the mesh should remain in the buffer even though it isn't used.
	 */
	void freeMesh(const std::string& name, const Mesh* mesh, bool persist);

	/**
	 * Adds a material's uniform data to the uniform buffers, and allocates a descriptor
	 * set (or similar) for it.
	 * @param model The material to add.
	 */
	void addMaterial(Material* material);

protected:
	//Logger, logs things.
	Logger logger;

	/**
	 * Function available for subclasses if they need buffers deleted before their destructor completes.
	 * Destroys all created buffers.
	 */
	void deleteBuffers() { buffers.clear(); }

	/**
	 * Creates a buffer with the underlying rendering api.
	 * @param usage Flags specifying what is allowed to be done with the buffer.
	 * @param storage Where the buffer will be stored.
	 * @param size The size of the buffer to create.
	 * @throw std::runtime_error if out of memory.
	 */
	virtual std::shared_ptr<Buffer> createBuffer(uint32_t usage, BufferStorage storage, size_t size) = 0;

	/**
	 * Creates a type of uniform set for which descriptors can be allocated.
	 * @param name The name of the set.
	 * @param set The set itself. Mostly used for creating descriptor layouts.
	 */
	virtual void createUniformSetType(const std::string& name, const UniformSet& set) = 0;

	/**
	 * Gets the minimum alignment for offsets into a uniform buffer.
	 * Some values for reference:
	 * The lowest value is Intel integrated on linux, which seems to mostly use 1,
	 * while windows almost always uses 32.
	 * RADV appears to always use 4, which is different from the AMD windows
	 * drivers, which use 16.
	 * Nvidia uses the maximum value of 256 for some reason.
	 * Mobile (android) seems to almost always use 64, but a few use 32.
	 * Nobody really uses 128.
	 * @return The minimum alignment.
	 */
	virtual size_t getMinUniformBufferAlignment() = 0;

	/**
	 * Allocates a descriptor set for the material. This can be called more than once
	 * for the same material - subsequent calls should be ignored.
	 * @param material The material to allocate a descriptor set for.
	 */
	virtual void addMaterialDescriptors(const Material* material) = 0;

private:
	enum UniformBufferType {
		MATERIAL,
		SCREEN_OBJECT,
		NUM_TYPES
	};

	//Stores all created buffers.
	std::unordered_map<std::string, std::shared_ptr<Buffer>> buffers;
	//Stores all uniform buffers.
	std::array<std::shared_ptr<Buffer>, UniformBufferType::NUM_TYPES> uniformBuffers;
	//Stores all created uniform sets.
	std::unordered_map<std::string, UniformSet> uniformSets;
};
