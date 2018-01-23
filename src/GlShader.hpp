#pragma once

#include <string>
#include <glm/glm.hpp>
#include "CombinedGl.h"

//Encapsulates an OpenGL program object, to make things like setting
//uniforms easier.
class GlShader {
public:
	//The program id for this shader
	const GLuint id;

	/**
	 * Creates a GlShader with the given id.
	 * @param id The id of the program object for this shader.
	 */
	GlShader(GLuint id);

	/**
	 * Destructor. Destroys the program object.
	 */
	~GlShader();

	/**
	 * Sets this shader program as active with OpenGL.
	 */
	void use();

	/**
	 * The following functions all set uniforms for this shader, and have
	 * similar parameters.
	 * @param name The name of the uniform to set.
	 * @param {matrix, vec} The value to set the uniform to.
	 */
	void setUniformMat4(std::string name, glm::mat4 matrix);
	void setUniformVec2(std::string name, glm::vec2 vec);
	void setUniformVec3(std::string name, glm::vec3 vec);
};
