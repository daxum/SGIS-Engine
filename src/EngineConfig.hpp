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

//List of available rendering engines.
enum class Renderer {
	OPEN_GL
};

//Used to avoid passing references in the config.
enum class LogType {
	STDOUT,
	FILE
};

//Specifies logging information for the different subsystems.
struct LogConfig {
	//The type of output for the logger.
	LogType type;
	//Only used for LogType FILE
	std::string outputFile;
	//The level mask - can be {DEBUG|INFO|WARN|ERROR|FATAL}
	//If a bit is unset, that level will be disabled.
	uint32_t mask;
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
	//The near plane distance (Warning: very small values can cause precision errors).
	float nearPlane;
	//The far plane distance.
	float farPlane;
};

struct EngineConfig {
	//Information about how the rendering engine should be set up.
	RenderConfig renderer;
	//The time taken for each tick of the engine, in milliseconds.
	double timestep;
	//Time for physics engine updates, in seconds.
	float physicsTimestep;
	//General logging for engine.
	LogConfig generalLog;
	//General renderer logger.
	LogConfig rendererLog;
	//Loader logger.
	LogConfig loaderLog;
};
