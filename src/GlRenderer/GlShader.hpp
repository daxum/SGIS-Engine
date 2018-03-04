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
