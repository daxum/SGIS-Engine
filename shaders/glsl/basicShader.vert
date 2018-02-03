#version 410 core

layout (location = 0) in vec3 posIn;
layout (location = 1) in vec3 normIn;
layout (location = 2) in vec2 texIn;

out vec3 pos;
out vec3 norm;
out vec2 tex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	pos = posIn;
	norm = normIn;
	tex = texIn;
	gl_Position = projection * view * model * vec4(pos, 1.0);
}
