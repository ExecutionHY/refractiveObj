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
#include <glm/gtc/matrix_transform.hpp>

#include "glslprogram.hpp"
#include "model.hpp"
#include "controller.hpp"

class Render {
private:
    GLFWwindow *window;
    GLSLProgram program;
    Controller controller;
    
    GLuint VertexArrayID;
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;
    GLuint Texture;

    Model m_object;
    Model m_background;
    void loadModel();
public:
    Render();
    ~Render();
    int run();
};

#endif /* render_hpp */
