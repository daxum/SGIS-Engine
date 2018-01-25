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

#include <sstream>
#include <fstream>
#include "GlShaderLoader.hpp"

GlShaderLoader::GlShaderLoader(std::unordered_map<std::string, std::shared_ptr<GlShader>>& shaderMap) :
	shaderMap(shaderMap) {

}

void GlShaderLoader::loadShader(std::string name, std::string vertexPath, std::string fragmentPath, const void* flags) {
	if (shaderMap.count(name) > 0) {
		//TODO: duplicate warning
		return;
	}

	std::shared_ptr<GlShader> shader = std::make_shared<GlShader>(createProgram(vertexPath, fragmentPath));
	shaderMap.insert(std::make_pair(name, shader));
}

GLuint GlShaderLoader::createProgram(std::string vertexName, std::string fragmentName) {
	//Create shaders and program

	GLuint vertexShader = createShader(vertexName, GL_VERTEX_SHADER);
	GLuint fragmentShader = createShader(fragmentName, GL_FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();

	if (shaderProgram == 0) {
		throw std::runtime_error("Program loading failed");
	}

	//Link program

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	//TODO: Log link errors.

	//Delete leftover shader objects

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint GlShaderLoader::createShader(std::string filename, GLenum type) {
	//Create shader object

	GLuint shader = glCreateShader(type);

	if (shader == 0) {
		throw std::runtime_error("Could not allocate shader");
	}

	//Load source and compile shader

	//This might look weird, but errors happen if the c string is passed directly to glShaderSource().
	//Apparently that function takes a const GLchar* const* (const pointer to const char?), which is
	//an abomination of a type that can't be converted to directly from the c_str() function call,
	//due to how '&' works.
	const char* source = loadShaderSource(filename).c_str();

	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	//No need to get compilation logs, all shaders will be validated beforehand with the spir-v compiler.

	return shader;
}

std::string GlShaderLoader::loadShaderSource(std::string filename) {
	//Create input stream for file and stringStream for writing.

	std::ifstream inFile;
	std::ostringstream sourceStream;

	inFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	//Read source from disk.

	try {
		inFile.open(filename);
		sourceStream << inFile.rdbuf();
		inFile.close();
	}
	catch(const std::ifstream::failure& e) {
		throw std::runtime_error("Couldn't read shader source");
	}

	return sourceStream.str();
}
