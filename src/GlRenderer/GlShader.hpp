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
#include "ShaderInterface.hpp"
#include "GlTextureLoader.hpp"
#include "ShaderInfo.hpp"

//Encapsulates an OpenGL program object, to make things like setting
//uniforms easier.
class GlShader : public ShaderInterface {
public:
	//The program id for this shader.
	const GLuint id;
	//The render pass this shader is in.
	const RenderPass renderPass;

	/**
	 * Creates a GlShader with the given id.
	 * @param id The id of the program object for this shader.
	 * @param textureMap A reference to the rendering engine's texture map, used
	 *     for setting textures.
	 */
	GlShader(GLuint id, RenderPass pass, const std::unordered_map<std::string, GlTextureData>& textureMap);

	/**
	 * Destructor. Destroys the program object.
	 */
	~GlShader();

	/**
	 * Sets this shader program as active with OpenGL.
	 */
	void bind();

	/**
	 * Sets a uniform variable.
	 * @param type The type of the uniform.
	 * @param name The name of the uniform in the shader.
	 * @param data The value to set the uniform data to.
	 */
	void setUniform(UniformType type, const std::string& name, const void* data);

	/**
	 * Sets the texture at the given index to the provided name.
	 * @param name The name of the texture to bind.
	 * @param index The texture unit to bind the texture to.
	 */
	void setTexture(const std::string& name, unsigned int index);

private:
	//A reference to the texture map for setting textures.
	const std::unordered_map<std::string, GlTextureData>& textureMap;
};
