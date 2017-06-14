//
//  glslprogram.cpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "glslprogram.hpp"

GLSLProgram::GLSLProgram() {
    
}
GLSLProgram::~GLSLProgram() {
    // clear the program
    glDeleteProgram(programID);
}
//#include <unistd.h>
bool GLSLProgram::initShader(const char * vertex_file_path, const char * fragment_file_path, const char * geometry_file_path){
    
    //char *dir = getcwd(NULL, 0);
    //printf("cwd: %s\n", dir);
	
	GLint Result = GL_FALSE;
	int InfoLogLength;
	bool gs = false;
	if (geometry_file_path != NULL) gs = true;
	
	
    // Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	
	// Read the Vertex Shader code from the file
	string vpath = dir + "shader/" + string(vertex_file_path);
	string VertexShaderCode;
	ifstream VertexShaderStream(vpath, ios::in);
	if(VertexShaderStream.is_open()){
		string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else{
		printf("Impossible to open %s.\n", vertex_file_path);
		return false;
	}
	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);
	
	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}
	
	
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	
	// Read the Fragment Shader code from the file
	string fpath = dir + "shader/" + string(fragment_file_path);
    string FragmentShaderCode;
    ifstream FragmentShaderStream(fpath, ios::in);
    if(FragmentShaderStream.is_open()){
        string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    else{
        printf("Impossible to open %s.\n", fragment_file_path);
        return false;
    }
	
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
	
	
	GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	if (gs) {
		// Read the Shader code from the file
		string gpath = dir + "shader/" + string(geometry_file_path);
		string GeometryShaderCode;
		ifstream GeometryShaderStream(gpath, ios::in);
		if(GeometryShaderStream.is_open()){
			string Line = "";
			while(getline(GeometryShaderStream, Line))
				GeometryShaderCode += "\n" + Line;
			GeometryShaderStream.close();
		}
		else{
			printf("Impossible to open %s.\n", geometry_file_path);
			return false;
		}
		
		// Compile Shader
		printf("Compiling shader : %s\n", geometry_file_path);
		char const * GeometrySourcePointer = GeometryShaderCode.c_str();
		glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
		glCompileShader(GeometryShaderID);
		
		// Check Shader
		glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if ( InfoLogLength > 0 ){
			vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
			glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
			printf("%s\n", &GeometryShaderErrorMessage[0]);
		}
	}
	
	
	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();

	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	if (gs) glAttachShader(ProgramID, GeometryShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    
    
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	if (gs) glDetachShader(ProgramID, GeometryShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	glDeleteShader(GeometryShaderID);
	
    programID = ProgramID;
    return true;
}

bool GLSLProgram::initComputeShader(const char * compute_file_path) {
	// Create the shaders
	return true;
}
