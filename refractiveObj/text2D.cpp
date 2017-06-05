//
//  text2D.cpp
//  refractiveObj
//
//  Created by Execution on 30/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "text2D.hpp"

Text2D::Text2D() {}
Text2D::~Text2D() {
    // Delete buffers
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &uvBufferID);
}

void Text2D::init(const char *path) {
    // Initialize texture
    texture.loadDDS(path);
    
    // Initialize VBO
    glGenBuffers(1, &vertexBufferID);
    glGenBuffers(1, &uvBufferID);
    
    // Initialize Shader
    program_text2d.initShader( "text.vert", "text.frag", NULL );
    
    // Initialize uniforms' IDs
    program_text2d.uniformID_Texture = glGetUniformLocation( program_text2d.programID, "myTextureSampler" );
}

void Text2D::print(const char *text, int x, int y, int size) {
    unsigned int length = strlen(text);
    
    // Fill buffers
    vector<vec2> vertices;
    vector<vec2> UVs;
    for ( unsigned int i=0 ; i<length ; i++ ){
        
        vec2 vertex_up_left    = vec2( x+i*size     , y+size );
        vec2 vertex_up_right   = vec2( x+i*size+size, y+size );
        vec2 vertex_down_right = vec2( x+i*size+size, y      );
        vec2 vertex_down_left  = vec2( x+i*size     , y      );
        
        vertices.push_back(vertex_up_left   );
        vertices.push_back(vertex_down_left );
        vertices.push_back(vertex_up_right  );
        
        vertices.push_back(vertex_down_right);
        vertices.push_back(vertex_up_right);
        vertices.push_back(vertex_down_left);
        
        char character = text[i];
        float uv_x = (character%16)/16.0f;
        float uv_y = (character/16)/16.0f;
        
        vec2 uv_up_left    = vec2( uv_x           , uv_y );
        vec2 uv_up_right   = vec2( uv_x+1.0f/16.0f, uv_y );
        vec2 uv_down_right = vec2( uv_x+1.0f/16.0f, (uv_y + 1.0f/16.0f) );
        vec2 uv_down_left  = vec2( uv_x           , (uv_y + 1.0f/16.0f) );
        UVs.push_back(uv_up_left   );
        UVs.push_back(uv_down_left );
        UVs.push_back(uv_up_right  );
        
        UVs.push_back(uv_down_right);
        UVs.push_back(uv_up_right);
        UVs.push_back(uv_down_left);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(vec2), &UVs[0], GL_STATIC_DRAW);
    
    // Bind shader
    glUseProgram(program_text2d.programID);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.textureID);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(program_text2d.uniformID_Texture, 0);
    
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw call
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
    
    glDisable(GL_BLEND);
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}
