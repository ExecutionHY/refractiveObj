#version 410 core

in vec3 texcoords;
out vec3 color;

uniform samplerCube CubeMap;

void main() {
	color = texture(CubeMap, texcoords).rgb;
}
