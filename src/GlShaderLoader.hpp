#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "ShaderLoader.hpp"
#include "GlShader.hpp"

class GlShaderLoader : public ShaderLoader {
public:
	/**
	 * Constructs a GlShaderLoader that stores loaded shaders in the provided map.
	 */
	GlShaderLoader(std::unordered_map<std::string, std::shared_ptr<GlShader>>& shaderMap);

	/**
	 * Loads a program object using the given shader files.
	 * @param name The name to associate the finished shader with.
	 * @param vertexPath The path to the vertex shader source.
	 * @param fragmentPath The path to the fragment shader source.
	 * @param flags Unused for the OpenGL loader.
	 * @throw runtime_error if loading failed.
	 */
	void loadShader(std::string name, std::string vertexPath, std::string fragmentPath, const void* flags);

private:
	std::unordered_map<std::string, std::shared_ptr<GlShader>>& shaderMap;

	/**
	 * Creates a program object using the shaders with the specified filenames.
	 * @param vertexName The path to the vertex shader.
	 * @param fragmentName The path to the fragment shader.
	 * @return A GLuint that references the completed program object.
	 * @throw runtime_error if loading failed.
	 */
	GLuint createProgram(std::string vertexName, std::string fragmentName);

	/**
	 * Creates a shader object for use in creating a program
	 * @param filename The path to the shader to be loaded
	 * @param type The type of shader - can be GL_VERTEX_SHADER,
	 *     GL_FRAGMENT_SHADER, or similar.
	 * @return A GLuint that references the loaded shader object.
	 * @throw runtime_error if shader creation failed.
	 */
	GLuint createShader(std::string filename, GLenum type);

	/**
	 * Loads shader source code from disk.
	 * @param filename The path to the file to be loaded.
	 * @return A string containing the contents of the file.
	 * @throw runtime_error if an error occurred during loading.
	 */
	std::string loadShaderSource(std::string filename);
};
