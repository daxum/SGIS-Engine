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

#include "GlShader.hpp"

GlShader::GlShader(GLuint id, RenderPass pass, const std::unordered_map<std::string, GlTextureData>& textureMap) :
	id(id),
	renderPass(pass),
	textureMap(textureMap) {

}

GlShader::~GlShader() {
	glDeleteProgram(id);
}

void GlShader::setUniform(UniformType type, const std::string& name, const void* data) {
	GLuint uniformLoc = glGetUniformLocation(id, name.c_str());

	const GLfloat* floatData = (const GLfloat*) data;

	switch (type) {
		case UniformType::FLOAT: glUniform1f(uniformLoc, *floatData); break;
		case UniformType::VEC2: glUniform2fv(uniformLoc, 1, floatData); break;
		case UniformType::VEC3: glUniform3fv(uniformLoc, 1, floatData); break;
		case UniformType::VEC4: glUniform4fv(uniformLoc, 1, floatData); break;
		case UniformType::MAT3x3: glUniformMatrix3fv(uniformLoc, 1, GL_FALSE, floatData); break;
		case UniformType::MAT4x4: glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, floatData); break;
		default: throw std::runtime_error("Missing case in GLShader uniform switch!");
	}
}

void GlShader::setTexture(const std::string& name, unsigned int index) {
	glActiveTexture(GL_TEXTURE0 + index);

	const GlTextureData& data = textureMap.at(name);

	switch (data.type) {
		case TextureType::TEX_2D: glBindTexture(GL_TEXTURE_2D, data.id); break;
		case TextureType::CUBEMAP: glBindTexture(GL_TEXTURE_CUBE_MAP, data.id); break;
		default: throw std::runtime_error("Missing texture type in GlShader::setTexture()!");
	}
}
