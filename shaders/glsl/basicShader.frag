#version 410 core

in vec2 texCoords;
in vec3 normal;
in vec3 pos;

out vec4 outColor;

uniform sampler2D texture0;
uniform vec3 color;
uniform vec3 cameraPos;

void main() {
	vec4 unlitColor = texture(texture0, texCoords) * vec4(color, 1.0);

	float ambient = 0.1;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(vec3(0.525322, 1.0, -0.1));
	float diffuse = max(dot(norm, lightDir), 0.0);

	float specularStrength = 0.5;
	vec3 cameraDir = normalize(cameraPos - pos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float specular = specularStrength * pow(max(dot(cameraDir, reflectDir), 0.0), 32);

	outColor = vec4(((ambient + diffuse + specular) * unlitColor).rgb, 1.0);
}
