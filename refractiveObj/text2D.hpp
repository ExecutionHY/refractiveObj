//
//  text2D.hpp
//  refractiveObj
//
//  Created by Execution on 30/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef text2D_hpp
#define text2D_hpp

#include "main.hpp"
#include "texture.hpp"
#include "glslprogram.hpp"

class Text2D {
private:
    Texture texture;
    
public:
    GLuint vertexBufferID;
    GLuint uvBufferID;
    GLSLProgram program_text2d;
    GLuint uniformID;
    
    Text2D();
    ~Text2D();
    
    void init(const char* path);
    void print(const char* text, int x, int y, int size);
};

#endif /* text2D_hpp */
