//
//  main.cpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include <iostream>
#include "render.hpp"

string dir = "/Users/mac/Codes/refractiveObj/";
vec4 refIndex[VOXEL_CNT][VOXEL_CNT][VOXEL_CNT];
vec4 radiance[VOXEL_CNT][VOXEL_CNT][VOXEL_CNT];


int main(int argc, const char * argv[]) {
    
    Render mainRender;
    mainRender.run();
    
    return 0;
}
