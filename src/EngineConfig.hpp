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
#include <memory>
#include <cstdint>
#include <vector>

#include "Logger.hpp"

//List of available rendering engines.
enum class Renderer {
	OPEN_GL,
	VULKAN,
	OPEN_GL_PHYSICS_DEBUG
};

struct RenderConfig {
	//Which rendering engine to use.
	Renderer renderType;
	//The initial window width, in pixels.
	int windowWidth;
	//The initial window height, in pixels.
	int windowHeight;
	//The title of the window.
	std::string windowTitle;
	//Validation layers to enable, if supported by the renderer (ie. vulkan).
	std::vector<std::string> validationLayers;
	//Used to force the gpu used for rendering, instead of determining normally.
	//This should only be used during development!
	bool deviceOverride;
	//The device index to force the engine to use. Device indices can be found
	//using vulkaninfo.
	size_t forceIndex;
};

struct EngineConfig {
	//The name of the game, used when initializing the rendering engine if available.
	std::string gameName;
	//The version number of the game, used in the same way as the above.
	uint32_t gameVersion;
	//Information about how the rendering engine should be set up.
	RenderConfig renderer;
	//The time taken for each tick of the engine, in milliseconds.
	double timestep;
	//Time for physics engine updates, in seconds.
	float physicsTimestep;
	//Controls how often average frame times are reported, in milliseconds.
	double frameReportFrequency;
	//Directory to append to all resource filenames when loading (this should include the final '/').
	std::string resourceBase;
	//General logging for engine.
	LogConfig generalLog;
	//General renderer logger.
	LogConfig rendererLog;
	//Loader logger.
	LogConfig loaderLog;
	//Model manager logger.
	LogConfig modelLog;
	//Logger for components.
	LogConfig componentLog;
};
