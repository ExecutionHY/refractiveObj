//
//  glslprogram.hpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef glslprogram_hpp
#define glslprogram_hpp

#include <stdio.h>
#include <GL/glew.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>

class GLSLProgram {
private:
public:
    GLSLProgram();
    ~GLSLProgram();
    GLuint programID;
    bool initShader(const char * vertex_file_path,const char * fragment_file_path);
    
};

#endif /* glslprogram_hpp */
