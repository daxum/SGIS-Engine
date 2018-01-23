#pragma once

#include <string>

//A generic shader loader. This only really exists for
//completeness sake, as shaders aren't currently loaded from
//outside the engine. As such, there probably won't be any
//out-of-rendering-engine shader load calls that need the abstraction.
class ShaderLoader {
public:
	/**
	 * Default constructor. Does nothing.
	 */
	ShaderLoader() {}

	/**
	 * Destructor. Here for subclasses.
	 */
	 virtual ~ShaderLoader() {}

	/**
	 * Loads the shaders from disk and constructs a program object from them.
	 * More arguments might need to be added for things like geometry and tesselation shaders.
	 * @param name A name given to the loaded shader. Present for when custom
	 *     shaders are allowed to be loaded by the game.
	 * @param vertex The path of the vertex shader to be loaded.
	 * @param fragment The path of the fragment shader to be loaded.
	 * @param flags A renderer specific pointer to any extra data it might need to
	 *     create the shader (ie. vulkan would probably need state information here, as this
	 *     might create an entire pipeline).
	 */
	 virtual void loadShader(std::string name, std::string vertex, std::string fragment, const void* flags) = 0;
};
