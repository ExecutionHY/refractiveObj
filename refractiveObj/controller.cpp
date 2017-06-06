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
    mouseSpeed = 0.005f;
    horizontalAngle = 3.14159f * 0.75;
    verticalAngle = 0.0f;
    direction = vec3(cos(verticalAngle) * sin(horizontalAngle),
                     sin(verticalAngle),
                     cos(verticalAngle) * cos(horizontalAngle)
                     );
    right = vec3(sin(horizontalAngle - 3.14159f/2.0f),
                 0,
                 cos(horizontalAngle - 3.14159f/2.0f));
    up = cross(right, direction);
    
    zPressed = false;
    xPressed = false;
    dist = 5.0f;
    Projection = perspective(45.0f, 4.0f/3.0f, 0.1f, 100.0f);
    View = lookAt(direction*(-1.0f)*dist, vec3(0,0,0), vec3(0,1,0));
    Model_object = mat4(0.5f);
    MVP_object = Projection * View * Model_object;
    Model_background = mat4(1.0f);
    MVP_background = Projection * View * Model_background;
    lightPos = vec3(4,4,4);
    camera = direction*(-1.0f)*dist;
    
    lastTime = glfwGetTime();
    frameCount = 0;
}


void Controller::update() {
    
    
    // Get mous position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    // left button to drag the view
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
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
    }
    
    // right button to rotate the object
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        // Compute new direction
        float horizontalAngle_rotate, verticalAngle_rotate;
        horizontalAngle_rotate += mouseSpeed * float(lastXpos-xpos);
        verticalAngle_rotate += mouseSpeed * float(lastYpos-ypos);
        Model_object = rotate(Model_object, -horizontalAngle_rotate, up);
        Model_object = rotate(Model_object, -verticalAngle_rotate, right);
    }
    
    // press Z to zoom out, X to zoom in
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        dist *= 1.05;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        dist *= 0.95;
    
    camera = direction*(-1.0f)*dist;

    View = lookAt(direction*(-1.0f)*dist, vec3(0,0,0), up);
    MVP_object = Projection * View * Model_object;
    MVP_background = Projection * View * Model_background;
    
    lastXpos = xpos;
    lastYpos = ypos;
    
    // Measure the fps
    double currentTime = glfwGetTime();
    frameCount++;
    if (currentTime - lastTime >= 1.0) {
        fps = frameCount;
        frameCount = 0;
        lastTime += 1.0;
    }
}
