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
#include <unordered_map>

#include "CombinedGl.h"
#include "ShaderInfo.hpp"

class GlShader {
public:
	//The program id for this shader.
	const GLuint id;
	//The render pass this shader is in.
	const RenderPass renderPass;
	//Name of the screen uniforms set.
	const std::string screenSet;
	//Name of the object uniform set.
	const std::string objectSet;
	//Push constants used in the shader.
	const std::vector<UniformDescription> pushConstants;

	/**
	 * Creates a GlShader with the given id.
	 * @param id The id of the program object for this shader.
	 * @param renderPass The render (transparency) pass the shader belongs in.
	 * @param screenSet The per-screen uniform set, empty string if not present.
	 * @param objectSet The per-object uniform set, empty string if not present.
	 * @param pushConstants Push constants for this shader, these will never be buffered
	 *     even if uniform buffers are implemented (will always use glUniform*).
	 */
	GlShader(GLuint id, RenderPass pass, const std::string& screenSet, const std::string& objectSet, const std::vector<UniformDescription>& pushConstants) :
		id(id),
		renderPass(pass),
		screenSet(screenSet),
		objectSet(objectSet),
		pushConstants(pushConstants) {

	}

	/**
	 * Destructor. Destroys the program object.
	 */
	~GlShader() {
		glDeleteProgram(id);
	}

	/**
	 * Adds a uniform location to the shader, for faster retrieval during rendering.
	 * @param uniform The name of the uniform to add.
	 */
	void addUniformLoc(const std::string& uniform) {
		uniformLocCache[uniform] = glGetUniformLocation(id, uniform.c_str());
	}

	/**
	 * Gets the uniform location for the given shader variable name.
	 * @param The name of the variable in the shader.
	 * @return The location the uniform is stored at.
	 */
	GLuint getUniformLocation(const std::string& name) const { return uniformLocCache.at(name); }

private:
	std::unordered_map<std::string, GLuint> uniformLocCache;
};
