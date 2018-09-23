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
#include "GlShader.hpp"
#include "Engine.hpp"

GlShaderLoader::GlShaderLoader(RendererMemoryManager* memoryManager, std::unordered_map<std::string, std::shared_ptr<GlShader>>& shaderMap) :
	ShaderLoader(Engine::instance->getConfig().loaderLog),
	shaderMap(shaderMap),
	memoryManager(memoryManager) {

}

void GlShaderLoader::loadShader(std::string name, const ShaderInfo& info) {
	if (shaderMap.count(name) > 0) {
		ENGINE_LOG_WARN(logger, "Tried to load duplicate shader \"" + name + "\".");
		return;
	}

	ENGINE_LOG_DEBUG(logger, "Constructing shader from \"" + info.vertex + "\" and \"" + info.fragment + "\"");

	//Determine screen and object descriptor sets (from VkShaderLoader, move to parent?)
	std::string screenSet = "";
	std::string objectSet = "";

	for (const std::string& set : info.uniformSets) {
		if (memoryManager->getUniformSet(set).setType == UniformSetType::PER_SCREEN) {
			screenSet = set;
		}
		else if (memoryManager->getUniformSet(set).setType == UniformSetType::PER_OBJECT) {
			objectSet = set;
		}
	}

	std::shared_ptr<GlShader> shader = std::make_shared<GlShader>(createProgram(info.vertex, info.fragment), info.pass, screenSet, objectSet, info.pushConstants.pushConstants);
	shaderMap.insert({name, shader});

	ENGINE_LOG_DEBUG(logger, "Shader \"" + name + "\" loaded");
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

	int linked = 0;

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

	if (linked == 0) {
		char infoLog[1024];
		glGetProgramInfoLog(shaderProgram, 1024, nullptr, infoLog);
		ENGINE_LOG_FATAL(logger, "Program linking failed!");
		ENGINE_LOG_FATAL(logger, "------------ Program Link Log ------------");
		ENGINE_LOG_FATAL(logger, infoLog);
		ENGINE_LOG_FATAL(logger, "---------------- End Log -----------------");

		throw std::runtime_error("Linking failed for program using \"" + vertexName + "\" and \"" + fragmentName + "\"");
	}

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

	std::string source = loadShaderSource(filename);
	const char* sourceString = source.c_str();

	glShaderSource(shader, 1, &sourceString, nullptr);
	glCompileShader(shader);

	//Check if compilation somehow failed.

	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status == 0) {
		char infoLog[1024];
		glGetShaderInfoLog(shader, 1024, nullptr, infoLog);

		ENGINE_LOG_FATAL(logger, "Failed to compile shader \"" + filename + "\"!");
		ENGINE_LOG_FATAL(logger, "--------- Shader Compilation Log ---------");
		ENGINE_LOG_FATAL(logger, infoLog);
		ENGINE_LOG_FATAL(logger, "---------------- End Log -----------------");

		throw std::runtime_error("Failed to compile shader \"" + filename + "\"");
	}

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
		throw std::runtime_error("Couldn't read shader source for " + filename);
	}

	return sourceStream.str();
}
