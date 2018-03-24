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

struct ShaderInfo {
	//Path to vertex shader.
	std::string vertex;
	//Path to fragment shader.
	std::string fragment;

	//Uniforms to be passed in. Must have these names in the shader (Really don't want to mess with OpenGL extensions for location on uniforms...).

	//mat4, the modelView matrix.
	bool modelView = false;
	//mat4, the projection matrix.
	bool projection = false;
	//vec3, the color from the RenderComponent.
	bool color = false;
	//sampler2D, The first texture used.
	bool tex0 = false;
	//vec3, normalized light direction for directional lighting (camera space).
	bool lightDir = false;
	//Model lighting information:
	//	vec3 ka - ambient light.
	//	vec3 ks - specular light.
	//	float s - shininess.
	bool lighting = false;

	//Extra, renderer-specific data. Always blank for now.
	const void* extra = nullptr;
};
