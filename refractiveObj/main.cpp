//
//  main.cpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include <iostream>
#include "render.hpp"
#include "main.hpp"


int FRAME_WIDTH = 800;
int FRAME_HEIGHT = 600;
int MAP_WIDTH = 256;
int MAP_HEIGHT = 256;

vec4 grad_n[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
vec4 radiance[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];

string dir = "/Users/mac/Codes/refractiveObj/";

int main(int argc, const char * argv[]) {
    
    Render mainRender;
    mainRender.run();
    
    return 0;
}
