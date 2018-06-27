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
enum RenderPass {
	OPAQUE = 0,
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

	//Extra, renderer-specific data. Unused for GlRenderingEngine,
	//VkRenderingEngine needs this set to a VkShaderInfo struct.
	const void* extra = nullptr;
};

//Extra shader info for vulkan rendering engine.
struct VkShaderInfo {
	//Temporary hack to get the renderer set up. This specifies whether the shader uses
	//the text vertex struct or the standard one. This will be moved to somewhere more
	//rational later, to allow games to set their own vertex formats.
	bool textVertex = false;
};
