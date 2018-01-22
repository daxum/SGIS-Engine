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
};
