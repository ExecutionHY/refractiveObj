//
//  controller.hpp
//  refractiveObj
//
//  Created by Execution on 28/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef controller_hpp
#define controller_hpp

#include <stdio.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
using namespace glm;
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Controller {
private:
    GLFWwindow *window;
    
    bool mousePressed;
    double lastXpos;
    double lastYpos;
    float horizontalAngle;
    float verticalAngle;
    float mouseSpeed;
    vec3 direction;
    vec3 right;
    vec3 up;
    
    float dist;
public:
    GLuint MVPMatrixID;
    GLuint ModelMatrixID;
    GLuint ViewMatrixID;
    mat4 MVP;
    mat4 Projection;
    mat4 View;
    mat4 Model;
    
    Controller();
    ~Controller();
    void init(GLFWwindow *window);
    void update();
};
#endif /* controller_hpp */
