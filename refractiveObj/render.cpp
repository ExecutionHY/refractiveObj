//
//  render.cpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "render.hpp"

Render::Render() {
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        exit(-1);
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 800, 600, "refractiveObj", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        exit(-1);
    }
    
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
}
Render::~Render() {
    
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer_object);
    glDeleteBuffers(1, &uvbuffer_object);
    glDeleteBuffers(1, &normalbuffer_object);
    glDeleteBuffers(1, &elementbuffer_object);
    
    glDeleteBuffers(1, &vertexbuffer_background);
    glDeleteBuffers(1, &uvbuffer_background);
    glDeleteBuffers(1, &normalbuffer_background);
	glDeleteBuffers(1, &elementbuffer_background);
	
	glDeleteBuffers(1, &vertexbuffer_skybox);
	
    // Clean up VAO
    glDeleteVertexArrays(1, &VertexArrayID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
}
void Render::loadModels() {
    // generate VAO
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    // load object
    m_object.init("suzanne.obj");
    
    // generate VBO
    glGenBuffers(1, &vertexbuffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_object);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_vertices.size() * sizeof(vec3), &m_object.indexed_vertices[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &uvbuffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_object);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_uvs.size() * sizeof(vec2), &m_object.indexed_uvs[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &normalbuffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_object);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_normals.size() * sizeof(vec3), &m_object.indexed_normals[0], GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_object.indices.size() * sizeof(unsigned short), &m_object.indices[0] , GL_STATIC_DRAW);
    
    // load background
    m_background.init("background.obj");
    
    // generate VBO
    glGenBuffers(1, &vertexbuffer_background);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_background);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_vertices.size() * sizeof(vec3), &m_background.indexed_vertices[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &uvbuffer_background);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_background);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_uvs.size() * sizeof(vec2), &m_background.indexed_uvs[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &normalbuffer_background);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_background);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_normals.size() * sizeof(vec3), &m_background.indexed_normals[0], GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer_background);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_background);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_object.indices.size() * sizeof(unsigned short), &m_background.indices[0] , GL_STATIC_DRAW);
	
	// skybox
	
	float points[] = {
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		
		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,
		
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		
		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,
		
		-10.0f,  10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,
		
		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f,  10.0f
	};
	glGenBuffers(1, &vertexbuffer_skybox);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_skybox);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &points, GL_STATIC_DRAW);
	
}

void Render::loadPrograms() {
	// program for background
	if (program_std.initShader( "standard.vert", "standard.frag", NULL ) == false) {
		getchar();
		exit(-1);
	}
	
	// Get handles for our uniforms
	program_std.uniformID_MVP = glGetUniformLocation(program_std.programID, "MVP");
	program_std.uniformID_View = glGetUniformLocation(program_std.programID, "V");
	program_std.uniformID_Model = glGetUniformLocation(program_std.programID, "M");
	program_std.uniformID_Light = glGetUniformLocation(program_std.programID, "LightPosition_worldspace");
	program_std.uniformID_Texture  = glGetUniformLocation(program_std.programID, "myTextureSampler");
	
	// program for object
	if (program_obj.initShader("object.vert", "object.frag", NULL) == false) {
		getchar();
		exit(-1);
	}
	program_obj.uniformID_MVP = glGetUniformLocation(program_obj.programID, "MVP");
	program_obj.uniformID_View = glGetUniformLocation(program_obj.programID, "V");
	program_obj.uniformID_Model = glGetUniformLocation(program_obj.programID, "M");
	program_obj.uniformID_Camera = glGetUniformLocation(program_obj.programID, "CameraPos_worldspace");
	program_obj.uniformID_Radiance = glGetUniformLocation(program_obj.programID, "radianceDistribution");
	program_obj.uniformID_GradN = glGetUniformLocation(program_obj.programID, "grad_n");
	program_obj.uniformID_CubeMap = glGetUniformLocation(program_obj.programID, "CubeMap");
	program_obj.uniformID_Vcnt = glGetUniformLocation(program_obj.programID, "voxel_cnt");
	
	// program for skybox
	if (program_sky.initShader("skybox.vert", "skybox.frag", NULL) == false) {
		getchar();
		exit(-1);
	}
	program_sky.uniformID_Projection = glGetUniformLocation(program_sky.programID, "P");
	program_sky.uniformID_View = glGetUniformLocation(program_sky.programID, "V");
	program_sky.uniformID_CubeMap = glGetUniformLocation(program_sky.programID, "CubeMap");
}

int Render::run() {
	
	
	// init controller
    controller.init(window);
	
	// init programs
	loadPrograms();
	
	// init models
	loadModels();
	
	// init textures
    bgTexture.loadBMP("background.bmp");
	text2d.init("Holstein.DDS");
	texture_skybox.loadCubeMap("river");
	
	
	
	float t1 = glfwGetTime();
	m_object.init();
	//m_object.printData();
	
	printf("Voxelization time = %6f s\n", glfwGetTime()-t1);
	
	GLuint texture_gradN;
	glGenTextures(1, &texture_gradN);
	glBindTexture(GL_TEXTURE_3D, texture_gradN);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, VOXEL_CNT, VOXEL_CNT, VOXEL_CNT, 0, GL_RGBA, GL_FLOAT, grad_n);
	
	GLuint texture_radiance;
	glGenTextures(1, &texture_radiance);
	glBindTexture(GL_TEXTURE_3D, texture_radiance);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, VOXEL_CNT, VOXEL_CNT, VOXEL_CNT, 0, GL_RGBA, GL_FLOAT, radiance);
	
	

    do{
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        controller.update();
    
        glUniform3f(program_std.uniformID_Light, controller.lightPos.x, controller.lightPos.y, controller.lightPos.z);

		
		// Draw skybox
		glUseProgram(program_sky.programID);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox.textureID);
		glUniform1i(program_sky.uniformID_CubeMap, 0);
		glUniformMatrix4fv(program_sky.uniformID_View, 1, GL_FALSE, &controller.View[0][0]);
		glUniformMatrix4fv(program_sky.uniformID_Projection, 1, GL_FALSE, &controller.Projection[0][0]);
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_skybox);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		
		
        // Draw m_object
        
        // Use our shader
        glUseProgram(program_obj.programID);
        
        // send data to shader
        glUniformMatrix4fv(program_obj.uniformID_View, 1, GL_FALSE, &controller.View[0][0]);
        glUniformMatrix4fv(program_obj.uniformID_Model, 1, GL_FALSE, &controller.Model_object[0][0]);
        glUniformMatrix4fv(program_obj.uniformID_MVP, 1, GL_FALSE, &controller.MVP_object[0][0]);
		glUniform3f(program_obj.uniformID_Camera, controller.camera.x, controller.camera.y, controller.camera.z);
		glUniform1i(program_obj.uniformID_Vcnt, VOXEL_CNT);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_gradN);
		glUniform1i(program_obj.uniformID_GradN, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, texture_radiance);
		glUniform1i(program_obj.uniformID_Radiance, 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox.textureID);
		glUniform1i(program_obj.uniformID_CubeMap, 2);
		
		
        // attribute buffer
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_object);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_object);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_object);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_object);
        
        glDrawElements(GL_TRIANGLES, m_object.indices.size(), GL_UNSIGNED_SHORT, (void*)0);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
		
		/*
		
        // Draw m_background
        
        // Use our shader
        glUseProgram(program_std.programID);
        
        glUniformMatrix4fv(program_std.uniformID_View, 1, GL_FALSE, &controller.View[0][0]);
        glUniformMatrix4fv(program_std.uniformID_Model, 1, GL_FALSE, &controller.Model_background[0][0]);
        glUniformMatrix4fv(program_std.uniformID_MVP, 1, GL_FALSE, &controller.MVP_background[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bgTexture.textureID);
        glUniform1i(program_std.uniformID_Texture, 0);
        
        // attribute buffer
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_background);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_background);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_background);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_background);
        
        glDrawElements(GL_TRIANGLES, m_background.indices.size(), GL_UNSIGNED_SHORT, (void*)0);
        
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
		
		*/
		
        char text[256];
        sprintf(text,"fps: %.2f, time: %.2f s", controller.fps, glfwGetTime());
        text2d.print(text, 10, 560, 20);
        
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );

	
    return 0;
}
