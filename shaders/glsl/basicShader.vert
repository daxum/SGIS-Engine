#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inputTexCoords;

out vec2 texCoords;
out vec3 normal;
out vec3 pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	texCoords = inputTexCoords;
	normal = inNormal;
	pos = vec3(model * vec4(position, 1.0));
	gl_Position = projection * view * model * vec4(position, 1.0);
}
