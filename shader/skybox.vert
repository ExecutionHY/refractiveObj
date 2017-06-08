#version 410 core


in vec3 VertexPostion_modelspace;
out vec3 texcoords;

uniform mat4 P;
uniform mat4 V;
//uniform vec3 CameraPos_worldspace;

void main() {
	texcoords = VertexPostion_modelspace;
	vec4 rightHandPos = P * V * vec4(VertexPostion_modelspace, 1.0);
	gl_Position = vec4(rightHandPos.x, rightHandPos.y, rightHandPos.z, rightHandPos.w);
}
