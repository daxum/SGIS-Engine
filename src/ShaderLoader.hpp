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

#include "Logger.hpp"
#include "ShaderInfo.hpp"
#include "RendererMemoryManager.hpp"

class ShaderLoader {
public:
	/**
	 * Constructor.
	 * @param logger The logger to use.
	 * @param memoryManager The memory manager to load buffers to.
	 */
	ShaderLoader(Logger& logger, RendererMemoryManager* memoryManager) :
		logger(logger),
		memoryManager(memoryManager) {}

	/**
	 * Destructor. Here for subclasses.
	 */
	virtual ~ShaderLoader() {}

	/**
	 * Loads the shaders from disk and constructs a program object from them.
	 * More arguments might need to be added for things like geometry and tesselation shaders.
	 * @param name A name given to the loaded shader.
	 * @param info Information about the shader to be loaded.
	 */
	virtual void loadShader(std::string name, const ShaderInfo& info) = 0;

	/**
	 * Here for lack of a better place. Creates a buffer for meshes to load to and
	 * shaders to read from.
	 * @param name The name of the buffer.
	 * @param info The information for the buffer (format, size, etc).
	 */
	void createBuffer(const std::string& name, const VertexBufferInfo& info) { memoryManager->addBuffer(name, info); }

protected:
	Logger& logger;
	RendererMemoryManager* memoryManager;
};
