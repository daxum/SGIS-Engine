#include <glm/gtc/type_ptr.hpp>
#include "GlShader.hpp"

GlShader::GlShader(GLuint id) : id(id) {}

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
