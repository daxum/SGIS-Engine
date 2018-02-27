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

#include <stack>

#include <glm/glm.hpp>

class MatrixStack {
public:
	MatrixStack();

	/**
	 * Fetches the top of the stack.
	 * @return the matrix on top of the stack.
	 */
	glm::mat4 top();

	/**
	 * Pushes the matrix stack. The new top is a direct copy of the old one.
	 */
	void push();

	/**
	 * Pops the top off the stack.
	 */
	void pop();

	/**
	 * Right multiplies the top of the stack by the given matrix.
	 * @param matrix The matrix to multiply by.
	 */
	void multiply(const glm::mat4& matrix);

	/**
	 * Translates the top matrix by the given amount.
	 * @param dist The amount to translate by.
	 */
	void translate(const glm::vec3& dist);

	/**
	 * Scales the top matrix by the given amount.
	 * @param amount The amount to scale by.
	 */
	void scale(const glm::vec3& amount);

	/**
	 * Rotates the top matrix by the given amount.
	 * @param angles A vector of angles representing the rotation in the x, y, and z directions. Probably in degrees?
	 */
	void rotate(const glm::vec3& angles);

private:
	//The matrix stack.
	std::stack<glm::mat4> matrices;
};
