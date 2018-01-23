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
