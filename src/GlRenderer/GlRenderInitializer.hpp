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

#include "RenderInitializer.hpp"

class GlRenderInitializer : public RenderInitializer {
public:
	/**
	 * Constructor.
	 * @param memoryManager The memory manager to upload things to.
	 */
	GlRenderInitializer(GlMemoryManager& memoryManager) :
		RenderInitializer(&memoryManager),
		memoryManager(memoryManager) {

	}

	/**
	 * Adds a set of uniforms that can be used in shaders and models.
	 * @param name The name of the set.
	 * @param set The set to add.
	 */
	void addUniformSet(const std::string& name, const UniformSet& set) override { memoryManager.addUniformSet(name, set); }

private:
	//Memory manager to insert uniform sets into.
	GlMemoryManager& memoryManager;
};
