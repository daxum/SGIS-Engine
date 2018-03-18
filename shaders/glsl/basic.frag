#version 410 core

in vec3 pos;
in vec3 norm;
in vec2 tex;

out vec4 outColor;

uniform sampler2D tex0;
uniform vec3 color;

void main() {
	outColor = texture(tex0, tex) * vec4(color, 1.0);
}
