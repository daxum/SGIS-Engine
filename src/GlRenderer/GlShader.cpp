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

#include <glm/gtc/type_ptr.hpp>
#include "GlShader.hpp"

GlShader::GlShader(GLuint id, const ShaderInfo info) : id(id), info(info) {}

GlShader::~GlShader() {
	glDeleteProgram(id);
}

void GlShader::use() {
	glUseProgram(id);
}

void GlShader::setUniformMat4(std::string name, glm::mat4 matrix) {
	GLuint uniformLoc = glGetUniformLocation(id, name.c_str());
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void GlShader::setUniformVec2(std::string name, glm::vec2 vec) {
	GLuint uniformLoc = glGetUniformLocation(id, name.c_str());
	glUniform2fv(uniformLoc, 1, glm::value_ptr(vec));
}

void GlShader::setUniformVec3(std::string name, glm::vec3 vec) {
	GLuint uniformLoc = glGetUniformLocation(id, name.c_str());
	glUniform3fv(uniformLoc, 1, glm::value_ptr(vec));
}

void GlShader::setUniformFloat(std::string name, float val) {
	GLuint uniformLoc = glGetUniformLocation(id, name.c_str());
	glUniform1f(uniformLoc, val);
}
