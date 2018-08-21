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

#include "RendererMemoryManager.hpp"
#include "ShaderInfo.hpp"
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
	 * Destructor.
	 */
	virtual ~RenderInitializer() {}

	/**
	 * Creates a buffer for meshes to load to and shaders to read from.
	 * @param name The name of the buffer.
	 * @param info The information for the buffer (format, size, etc).
	 */
	void createBuffer(const std::string& name, const VertexBufferInfo& info) { memoryManager->addBuffer(name, info); }

	/**
	 * Adds a set of uniforms that can be used in shaders and models.
	 * @param name The name of the set.
	 * @param set The set to add.
	 */
	virtual void addUniformSet(const std::string& name, const UniformSet& set) = 0;

protected:
	//The logger.
	Logger logger;

private:
	//Memory manager.
	RendererMemoryManager* memoryManager;
};
