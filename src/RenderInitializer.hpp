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

#include <string>

#include "Renderer/RendererMemoryManager.hpp"
#include "Renderer/ShaderInfo.hpp"
#include "Engine.hpp"

//Low-level renderer initialization, such as buffers and descriptor sets.
class RenderInitializer {
public:
	/**
	 * Constructor.
	 */
	RenderInitializer(RendererMemoryManager* memoryManager) :
		logger(Engine::instance->getConfig().loaderLog),
		memoryManager(memoryManager) {}

	/**
	 * Creates a buffer for meshes to load to and shaders to read from.
	 * @param name The name of the buffer.
	 * @param size The size of the buffer, in bytes.
	 * @param type What the buffer is going to be used for.
	 * @param storage In what type of memory the buffer should be stored.
	 */
	void createBuffer(const std::string& name, size_t size, BufferType type, BufferStorage storage) { memoryManager->addBuffer(name, size, type, storage); }

	/**
	 * Adds a vertex format to be used by meshes and shaders.
	 * @param name The name of the format to add.
	 * @param format The format to add.
	 */
	void addVertexFormat(const std::string& name, const VertexFormat& format) { Engine::instance->getModelManager().addFormat(name, format); }

	/**
	 * Adds a set of uniforms that can be used in shaders and materials.
	 * @param name The name of the set.
	 * @param type The type of the set. Determines which uniform providers are allowed.
	 * @param maxUsers The maximum allowed users for this set. Determines how many
	 *     different materials can be created, or screens allowed on the screen stack.
	 * @param set The list of uniforms to add. When creating the shader bindings, the
	 *     uniform buffer, if present, will always recieve binding 0, followed by non-
	 *     buffered unforms, such as samplers, in the order they are listed.
	 */
	void addUniformSet(const std::string& name, UniformSetType type, size_t maxUsers, const UniformList& uniforms);

private:
	//The logger.
	Logger logger;
	//Memory manager.
	RendererMemoryManager* memoryManager;
};
