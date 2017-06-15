//
//  main.hpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef main_hpp
#define main_hpp

#include <cstdio>
#include <vector>
#include <map>
#include <cstdlib>
#include <string>
#include <fstream>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <OpenCL/cl.h>
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>
#include <OpenGL/CGLDevice.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenCL/gcl.h>

extern string dir;
// viewer
extern int FRAME_WIDTH;
extern int FRAME_HEIGHT;
extern int MAP_WIDTH;
extern int MAP_HEIGHT;
// voxelizer
#define VOXEL_CNT 128
// vec4(gradx, grady, gradz, n)
extern vec4 grad_n[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
extern vec4 radiance[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
extern int octree[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];


#endif /* main_hpp */
