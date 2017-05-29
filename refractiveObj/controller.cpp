//
//  controller.cpp
//  refractiveObj
//
//  Created by Execution on 28/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "controller.hpp"

Controller::Controller() {
}

Controller::~Controller() {
}

void Controller::init(GLFWwindow *window) {
    this->window = window;
    
    glfwGetCursorPos(window, &lastXpos, &lastYpos);
    horizontalAngle = 3.14159f;
    verticalAngle = 0.0f;
    mousePressed = true;
    mouseSpeed = 0.005f;
    direction = vec3(0,0,-1);
    right = vec3(1,0,0);
    up = vec3(0,1,0);
    
    dist = 5.0f;
    Projection = perspective(45.0f, 4.0f/3.0f, 0.1f, 100.0f);
    View = lookAt(direction*(-1.0f)*dist, vec3(0,0,0), vec3(0,1,0));
    Model = mat4(1.0f);
    MVP = Projection * View * Model;
}


void Controller::update() {
    // mouse - left button to drag the view
    
    
    // Get mous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (mousePressed) {
            
            // Compute new direction
            horizontalAngle += mouseSpeed * float(lastXpos-xpos);
            verticalAngle += mouseSpeed * float(lastYpos-ypos);
            direction = vec3(cos(verticalAngle) * sin(horizontalAngle),
                             sin(verticalAngle),
                             cos(verticalAngle) * cos(horizontalAngle)
                             );
            right = vec3(sin(horizontalAngle - 3.14159f/2.0f),
                         0,
                         cos(horizontalAngle - 3.14159f/2.0f));
            up = cross(right, direction);
            View = lookAt(direction*(-1.0f)*dist, vec3(0,0,0), up);
            MVP = Projection * View * Model;
        }
    }
    
    lastXpos = xpos;
    lastYpos = ypos;
}
