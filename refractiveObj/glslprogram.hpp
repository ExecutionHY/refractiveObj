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
using namespace std;

extern string dir;

class GLSLProgram {
private:
public:
    GLuint programID;
    
    // program_std
    GLuint uniformID_MVP;
    GLuint uniformID_Model;
    GLuint uniformID_View;
    GLuint uniformID_Light;
    GLuint uniformID_Texture;
    
    GLSLProgram();
    ~GLSLProgram();
    bool initShader(const char * vertex_file_path,const char * fragment_file_path);
    
};

#endif /* glslprogram_hpp */
