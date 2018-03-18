#version 410 core

in vec3 pos;
in vec3 norm;
in vec2 tex;

out vec4 outColor;

uniform sampler2D tex0;
uniform vec3 color;

//Not really directional, but good enough.
vec3 directionalLight(float intensity) {
	vec3 normal = normalize(norm);
	vec3 position = -normalize(pos);

	vec3 ambient = vec3(0.1, 0.1, 0.1);
	vec3 diffuse = color * texture(tex0, tex).xyz * max(0, dot(position, normal));
	vec3 specular = vec3(0.1, 0.1, 0.1) * pow(max(0, dot(normalize(position + position), normal)), 5.0);

	return intensity * (ambient + diffuse + specular);
}

vec3 pointLight(vec3 lightPos) {
	vec3 normal = normalize(norm);
	vec3 position = -normalize(pos);

	vec3 light = normalize(lightPos - pos);

	vec3 ambient = vec3(0.05, 0.05, 0.05);
	vec3 diffuse = color * texture(tex0, tex).xyz * max(0, dot(light, normal));
	vec3 specular = vec3(0.4, 0.4, 0.4) * pow(max(0, dot(normalize(light + position), normal)), 200.0);

	return ambient + diffuse + specular;
}

void main() {
	outColor = vec4(directionalLight(0.9), 1.0);

	//Comment/uncomment to toggle cel shading
	//outColor = ceil(outColor * 20.0) / 20.0;
}
