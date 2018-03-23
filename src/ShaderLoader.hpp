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

class ShaderLoader {
public:
	/**
	 * Default constructor. Sets up logging.
	 */
	ShaderLoader(Logger& logger) : logger(logger) {}

	/**
	 * Destructor. Here for subclasses.
	 */
	virtual ~ShaderLoader() {}

	/**
	 * Loads the shaders from disk and constructs a program object from them.
	 * More arguments might need to be added for things like geometry and tesselation shaders.
	 * @param name A name given to the loaded shader. Present for when custom
	 *     shaders are allowed to be loaded by the game.
	 * @param info Information about the shader to be loaded.
	 */
	virtual void loadShader(std::string name, const ShaderInfo& info) = 0;

protected:
	Logger& logger;
};
