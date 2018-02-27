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

#include <glm/gtc/matrix_transform.hpp>

#include "MatrixStack.hpp"

MatrixStack::MatrixStack() {
	matrices.emplace(1.0);
}

glm::mat4 MatrixStack::top() {
	return matrices.top();
}

void MatrixStack::push() {
	matrices.push(matrices.top());
}

void MatrixStack::pop() {
	matrices.pop();
}

void MatrixStack::multiply(const glm::mat4& matrix) {
	matrices.top() *= matrix;
}

void MatrixStack::translate(const glm::vec3& dist) {
	matrices.top() = glm::translate(matrices.top(), dist);
}

void MatrixStack::scale(const glm::vec3& amount) {
	matrices.top() = glm::scale(matrices.top(), amount);
}

void MatrixStack::rotate(const glm::vec3& angles) {
	matrices.top() = glm::rotate(matrices.top(), angles.x, glm::vec3(1.0, 0.0, 0.0));
	matrices.top() = glm::rotate(matrices.top(), angles.y, glm::vec3(0.0, 1.0, 0.0));
	matrices.top() = glm::rotate(matrices.top(), angles.z, glm::vec3(0.0, 0.0, 1.0));
}
