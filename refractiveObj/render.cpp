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
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
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
    
    
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(program_std.programID);
    glDeleteTextures(1, &bgTexture.texture);

    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    
}
void Render::loadModel() {
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

}

int Render::run() {
    
    if (program_std.initShader( "stdVertex.glsl", "stdFragment.glsl" ) == false) {
        getchar();
        glfwTerminate();
        exit(-1);
    }
    
    controller.init(window);
    // Get a handle for our "MVP" uniform
    controller.MVPMatrixID = glGetUniformLocation(program_std.programID, "MVP");
    controller.ViewMatrixID = glGetUniformLocation(program_std.programID, "V");
    controller.ModelMatrixID = glGetUniformLocation(program_std.programID, "M");
    // Get a handle for our "LightPosition" uniform
    controller.LightID = glGetUniformLocation(program_std.programID, "LightPosition_worldspace");
    
    // Load the texture
    bgTexture.loadBMP("/Users/mac/Codes/refractiveObj/background.bmp");
    // Get a handle for our "myTextureSampler" uniform
    bgTexture.textureID  = glGetUniformLocation(program_std.programID, "myTextureSampler");
    
    

    loadModel();
    
    do{
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
        glUseProgram(program_std.programID);
        
        
        controller.update();
        glUniformMatrix4fv(controller.ViewMatrixID, 1, GL_FALSE, &controller.View[0][0]);
        
        
        glUniform3f(controller.LightID, controller.lightPos.x, controller.lightPos.y, controller.lightPos.z);

        // Draw m_object
        
        // send data to shader
        glUniformMatrix4fv(controller.MVPMatrixID, 1, GL_FALSE, &controller.MVP_object[0][0]);
        glUniformMatrix4fv(controller.ModelMatrixID, 1, GL_FALSE, &controller.Model_object[0][0]);
        
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
        
        // Draw m_background
        glUniformMatrix4fv(controller.MVPMatrixID, 1, GL_FALSE, &controller.MVP_background[0][0]);
        glUniformMatrix4fv(controller.ModelMatrixID, 1, GL_FALSE, &controller.Model_background[0][0]);
        
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
        
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
          glfwWindowShouldClose(window) == 0 );
    
    return 0;
}
