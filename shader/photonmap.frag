#version 330 core

in vec3 pos;
// Ouput data
layout(location = 0) out vec4 res;

void main(){
	res.a = gl_FragCoord.z;
	res.rgb = (pos + vec3(2, 2, 2)) / 10.0f;
}
