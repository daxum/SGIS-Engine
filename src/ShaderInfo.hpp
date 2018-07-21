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

#include "Shader.hpp"

//Render passes for the engine.
//Each shader is part of one render pass.
enum class RenderPass {
	OPAQUE,
	TRANSPARENT,
	TRANSLUCENT
};

struct ShaderInfo {
	//Path to vertex shader.
	std::string vertex;
	//Path to fragment shader.
	std::string fragment;
	//Shader object for setting uniforms.
	std::shared_ptr<Shader> shaderObject;
	//The render pass the shader is part of.
	RenderPass pass;
};
