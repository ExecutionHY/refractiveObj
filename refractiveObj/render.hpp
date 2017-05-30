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
#include "texture.hpp"
#include "text2D.hpp"

class Render {
private:
    GLFWwindow *window;
    GLSLProgram program_std;
    Controller controller;
    
    GLuint VertexArrayID;
    GLuint vertexbuffer_object;
    GLuint uvbuffer_object;
    GLuint normalbuffer_object;
    GLuint elementbuffer_object;
    
    GLuint vertexbuffer_background;
    GLuint uvbuffer_background;
    GLuint normalbuffer_background;
    GLuint elementbuffer_background;

    Model m_object;
    Model m_background;
    void loadModel();

    Texture bgTexture;
    Text2D text2d;

public:
    Render();
    ~Render();
    int run();
};

#endif /* render_hpp */
