#version 330 core

out vec3 pos;
// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;


// Values that stay constant for the whole mesh.
uniform mat4 lightMVP;

void main(){
	gl_Position =  lightMVP * vec4(vertexPosition_modelspace,1);
	pos = vertexPosition_modelspace;
}

