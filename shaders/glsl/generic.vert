#version 410 core

layout (location = 0) in vec3 posIn;
layout (location = 1) in vec3 normIn;
layout (location = 2) in vec2 texIn;

out vec3 pos;
out vec3 norm;
out vec2 tex;

uniform mat4 modelView;
uniform mat4 projection;

void main() {
	vec4 posCameraSpace = modelView * vec4(posIn, 1.0);

	pos = posCameraSpace.xyz;
	norm = (modelView * vec4(normIn, 0.0)).xyz;
	tex = texIn;

	gl_Position = projection * posCameraSpace;
}
