//
//  render.hpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef render_hpp
#define render_hpp

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
using namespace glm;

#include "glslprogram.hpp"
#include "model.hpp"

class Render {
private:
    GLFWwindow *window;
    GLSLProgram program;
    GLuint VertexArrayID;
    GLuint vertexbuffer;
    Model m_object;
    Model m_background;
    void loadModel();
public:
    Render();
    ~Render();
    int run();
};

#endif /* render_hpp */
