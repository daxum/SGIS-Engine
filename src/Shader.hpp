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

#include "Camera.hpp"
#include "RenderComponent.hpp"
#include "MatrixStack.hpp"
#include "ShaderInterface.hpp"
#include "Screen.hpp"

//An interface to the shaders from game code.
//Used to set uniforms.
class Shader {
public:
	virtual ~Shader() {}

	/**
	 * Sets uniforms for all objects using this shader, like the projection and view matrices.
	 * @param camera The current camera.
	 * @param state User-set screen state.
	 */
	virtual void setGlobalUniforms(std::shared_ptr<Camera> camera, std::shared_ptr<ScreenState> state) = 0;

	/**
	 * Sets uniforms that change based on the object being rendered, such as translation
	 * and scale.
	 * @param object The render component of the object being rendered.
	 * @param matStack The current matrix stack. Will be pushed/popped before/after the function
	 *     call, and will usually contain just the view matrix at the start of the function.
	 * @param state User-set screen state.
	 */
	virtual void setPerObjectUniforms(std::shared_ptr<RenderComponent> object, MatrixStack& matStack, std::shared_ptr<ScreenState> state) = 0;

	/**
	 * Internal engine use only. Sets the interface to the graphics api to use when setting uniforms.
	 * @param program The interface to the rendering engine.
	 */
	void setRenderInterface(std::shared_ptr<ShaderInterface> interface) { shaderInterface = interface; }

	/**
	 * Internal use only. Gets the api interface for this shader.
	 * @return The shader interface object.
	 */
	std::shared_ptr<ShaderInterface> getRenderInterface() { return shaderInterface; }

protected:
	//The shader api interface. Used to set uniforms.
	std::shared_ptr<ShaderInterface> shaderInterface;
};
