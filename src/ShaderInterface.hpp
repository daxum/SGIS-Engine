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

enum class UniformType {
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	MAT3x3,
	MAT4x4
};

//Engine implementation of shaders to interface with the rendering engine.
//GlShader and similar all extend this.
class ShaderInterface {
public:
	virtual ~ShaderInterface() {}

	/**
	 * Binds the shader for use in rendering.
	 */
	virtual void bind() = 0;

	/**
	 * Sets a uniform variable.
	 * @param type The type of the uniform.
	 * @param name The name of the uniform in the shader.
	 * @param data The value to set the uniform data to.
	 */
	virtual void setUniform(UniformType type, const std::string& name, const void* data) = 0;

	/**
	 * Sets the texture at the given index to the provided name.
	 * @param name The name of the texture to bind.
	 * @param index The texture unit to bind the texture to.
	 */
	virtual void setTexture(const std::string& name, unsigned int index) = 0;
};
