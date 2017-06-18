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
    window = glfwCreateWindow( FRAME_WIDTH, FRAME_HEIGHT, "refractiveObj", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
	
	glfwGetFramebufferSize(window, &FRAME_WIDTH, &FRAME_HEIGHT);
	
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
    
    glDeleteBuffers(1, &vertexbuffer_table);
    glDeleteBuffers(1, &uvbuffer_table);
    glDeleteBuffers(1, &normalbuffer_table);
	glDeleteBuffers(1, &elementbuffer_table);
	
	glDeleteBuffers(1, &vertexbuffer_skybox);
	
	// Cleanup FBO
	glDeleteFramebuffers(1, &frameBuffer_photon);
	
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
    m_object.init("cube.obj");
    
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
    m_table.init("table.obj");
    
    // generate VBO
    glGenBuffers(1, &vertexbuffer_table);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_table);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_vertices.size() * sizeof(vec3), &m_table.indexed_vertices[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &uvbuffer_table);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_table);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_uvs.size() * sizeof(vec2), &m_table.indexed_uvs[0], GL_STATIC_DRAW);
    
    glGenBuffers(1, &normalbuffer_table);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_table);
    glBufferData(GL_ARRAY_BUFFER, m_object.indexed_normals.size() * sizeof(vec3), &m_table.indexed_normals[0], GL_STATIC_DRAW);
    
    // Generate a buffer for the indices as well
    glGenBuffers(1, &elementbuffer_table);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_table);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_object.indices.size() * sizeof(unsigned short), &m_table.indices[0] , GL_STATIC_DRAW);
	
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
	// program for table
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
	program_std.uniformID_Radiance  = glGetUniformLocation(program_std.programID, "radiance");
	program_std.uniformID_Vcnt = glGetUniformLocation(program_std.programID, "voxel_cnt");
	
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
	program_obj.uniformID_Table = glGetUniformLocation(program_obj.programID, "table");
	program_obj.uniformID_Vcnt = glGetUniformLocation(program_obj.programID, "voxel_cnt");
	
	// program for skybox
	if (program_sky.initShader("skybox.vert", "skybox.frag", NULL) == false) {
		getchar();
		exit(-1);
	}
	program_sky.uniformID_Projection = glGetUniformLocation(program_sky.programID, "P");
	program_sky.uniformID_View = glGetUniformLocation(program_sky.programID, "V");
	program_sky.uniformID_CubeMap = glGetUniformLocation(program_sky.programID, "CubeMap");
	
	// program for photon map
	
	program_photon.initShader("photonmap.vert", "photonmap.frag", NULL);
	program_photon.uniformID_LightMVP = glGetUniformLocation(program_photon.programID, "lightMVP");
}

int Render::photonMapping() {
	
	
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	glGenFramebuffers(1, &frameBuffer_photon);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_photon);
	
	texture_photon.initDepth();
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_photon.textureID, 0);
	
	// No color output in the bound framebuffer, only depth.
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	return -1;
	
	
	
	// Render to our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer_photon);
	//GLenum bufs[] = {GL_COLOR_ATTACHMENT0,};
	//glDrawBuffers(1, bufs);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
	
	glViewport(0,0,MAP_WIDTH,MAP_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Use our shader
	glUseProgram(program_photon.programID);
	
	vec3 lightInvDir = vec3(0,2,0);
	
	
	// Compute the MVP matrix from the light's point of view
	mat4 depthProjectionMatrix = ortho<float>(-1.0, 1.0, -1.0, 1.0, 0, 2);
	mat4 depthViewMatrix = lookAt(lightInvDir, vec3(0,0,0), vec3(1,0,0));
	
	// or, for spot light :
	
	//vec3 lightPos(5, 20, 20);
	//mat4 depthProjectionMatrix = perspective<float>(45.0f, 1.0f, 2.0f, 50.0f);
	//mat4 depthViewMatrix = lookAt(lightPos, lightPos-lightInvDir, vec3(0,1,0));
	mat4 depthModelMatrix = mat4(1.0);
	
	mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
	
	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	glUniformMatrix4fv(program_photon.uniformID_LightMVP, 1, GL_FALSE, &depthMVP[0][0]);
	
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_object);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	// Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_object);
	
	// Draw the triangles !
	glDrawElements(GL_TRIANGLES, m_object.indices.size(), GL_UNSIGNED_SHORT, (void*)0);
	
	glDisableVertexAttribArray(0);
	return 0;
}

int Render::run() {
	
	// init controller
    controller.init(window);
	
	// init programs
	loadPrograms();
	
	// init models
	loadModels();
	
	// init textures
	text2d.init("Holstein.DDS");
	texture_skybox.loadCubeMap("river");
	texture_table.loadBMP("marble.bmp");
	
	vec3 pos1 = vec3(-0.2, 0.3, 0);
	
	// voxelization
	float t1 = glfwGetTime();
	//vec3 pos =
	voxelizer.work(m_object.indexed_vertices, m_object.indices, pos1);
	//voxelizer.print();
	printf("Voxelization time = %6f s\n", glfwGetTime()-t1);
	
	
	// octree construction
	t1 = glfwGetTime();
	octreeManager.construct();
	printf("Octree construction time = %6f s\n", glfwGetTime()-t1);
	
	
	// photon mapping
	t1 = glfwGetTime();
	photonMapping();
	printf("Photon mapping time = %6f s\n", glfwGetTime()-t1);
	
	
	// photon marching
	t1 = glfwGetTime();
	photonManager.march(texture_photon.textureID, pos1);
	printf("Photon marching time = %6f s\n", glfwGetTime()-t1);
	
	texture_gradN.load3D(grad_n);
	texture_radiance.load3D(radiance);
	texture_tableR.load2D(table);
	
	
	
	do {
		float frameStart = glfwGetTime();
		
		controller.update();
		
		
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles
		
		
		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, FRAME_WIDTH, FRAME_HEIGHT); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		
		
		//************* Draw skybox
		
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
		
		
		//************* Draw m_table
		
		// Use our shader
		glUseProgram(program_std.programID);
		
		glUniformMatrix4fv(program_std.uniformID_View, 1, GL_FALSE, &controller.View[0][0]);
		glUniformMatrix4fv(program_std.uniformID_Model, 1, GL_FALSE, &controller.Model_table[0][0]);
		glUniformMatrix4fv(program_std.uniformID_MVP, 1, GL_FALSE, &controller.MVP_table[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_table.textureID);
		glUniform1i(program_std.uniformID_Texture, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_tableR.textureID);
		glUniform1i(program_std.uniformID_Radiance, 1);
		
		glUniform1i(program_std.uniformID_Vcnt, VOXEL_CNT);
		glUniform3f(program_std.uniformID_Light, controller.lightPos.x, controller.lightPos.y, controller.lightPos.z);
		
		// attribute buffer
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_table);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer_table);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer_table);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer_table);
		
		glDrawElements(GL_TRIANGLES, m_table.indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		
        //************ Draw m_object
		
        // Use our shader
        glUseProgram(program_obj.programID);
        
        // send data to shader
        glUniformMatrix4fv(program_obj.uniformID_View, 1, GL_FALSE, &controller.View[0][0]);
        glUniformMatrix4fv(program_obj.uniformID_Model, 1, GL_FALSE, &controller.Model_object[0][0]);
        glUniformMatrix4fv(program_obj.uniformID_MVP, 1, GL_FALSE, &controller.MVP_object[0][0]);
		glUniform3f(program_obj.uniformID_Camera, controller.camera.x, controller.camera.y, controller.camera.z);
		glUniform1i(program_obj.uniformID_Vcnt, VOXEL_CNT);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_3D, texture_gradN.textureID);
		glUniform1i(program_obj.uniformID_GradN, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, texture_radiance.textureID);
		glUniform1i(program_obj.uniformID_Radiance, 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox.textureID);
		glUniform1i(program_obj.uniformID_CubeMap, 2);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texture_table.textureID);
		glUniform1i(program_obj.uniformID_Table, 3);
		
		
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
		
		
		glDisable(GL_BLEND);
		
		
		//************** draw text
		
        char text[256];
        sprintf(text,"fps: %.2f, frameTime: %.2fs, time: %.2f s", controller.fps, glfwGetTime()-frameStart, glfwGetTime());
        text2d.print(text, 10, 560, 16);

		
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );

	
	
    return 0;
}
