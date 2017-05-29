//
//  texture.hpp
//  refractiveObj
//
//  Created by Execution on 29/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Texture {
private:
public:
    Texture();
    ~Texture();
    void loadBMP(const char * imagepath);
    
    GLuint texture;
    GLuint textureID;
};

#endif /* texture_hpp */
