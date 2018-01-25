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

//List of available rendering engines
enum class Renderer {
	OPEN_GL
};

struct EngineConfig {
	//Which rendering engine to use
	Renderer renderer;
	//The initial window width, in pixels
	int windowWidth;
	//The initial window height, in pixels
	int windowHeight;
	//The title of the window
	std::string windowTitle;
	//The time taken for each tick of the engine, in milliseconds.
	double physicsTimestep;
	//The path to the directory the engine will load its default shaders from.
	//The specific set of shaders depends on the rendering engine used.
	//Note that the shader names will be appended to this, so the directory
	//should specified as "path/to/shaders/" and not "path/to/shaders".
	std::string shaderPath;
};
