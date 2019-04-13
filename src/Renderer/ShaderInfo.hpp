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
#include <vector>
#include <stdexcept>

#include "UniformSet.hpp"

//Render passes for the engine.
//Each shader is part of one render pass.
enum class RenderPass {
	OPAQUE,
	TRANSPARENT,
	TRANSLUCENT
};

typedef UniformList PushConstantSet;

struct ShaderInfo {
	//Path to vertex shader.
	std::string vertex;
	//Path to fragment shader.
	std::string fragment;
	//The render pass the shader is part of.
	RenderPass pass;
	//The vertex buffer this shader takes its format from. This does not
	//restrict the shader to use only this buffer, other buffers with
	//the same format will also work.
	std::string buffer;
	//Names of all the uniform sets used in the shader. Each shader can only
	//have one of each type of uniform set.
	std::vector<std::string> uniformSets;
	//All push constant values used in the shader. The uniforms in this set
	//should take up at most 128 bytes.
	PushConstantSet pushConstants;
};
