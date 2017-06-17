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
int MAP_WIDTH = 1024;
int MAP_HEIGHT = 1024;
float refConst = 1.25f;

vec4 grad_n[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
vec4 radiance[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
int octree[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
vec4 table[1024*1024];

string dir = "/Users/mac/Codes/refractiveObj/";

int main(int argc, const char * argv[]) {
    
    Render mainRender;
    mainRender.run();
    
    return 0;
}


float * createBlurMask3D(float sigma, int * maskSizePointer) {
	int maskSize = (int)ceil(3.0f*sigma);
	float * mask = new float[(maskSize*2+1)*(maskSize*2+1)*(maskSize*2+1)];
	float sum = 0.0f;
	for(int a = -maskSize; a <= maskSize; a++) {
		for(int b = -maskSize; b <= maskSize; b++) {
			for (int c = -maskSize; c <= maskSize; c++) {
				float temp = exp(-((float)(a*a+b*b+c*c) / (2*sigma*sigma)));
				sum += temp;
				mask[(a+maskSize)*(maskSize*2+1)*(maskSize*2+1)+(b+maskSize)*(maskSize*2+1)+(c+maskSize)] = temp;
			}
		}
	}
	// Normalize the mask
	for(int i = 0; i < (maskSize*2+1)*(maskSize*2+1)*(maskSize*2+1); i++)
		mask[i] = mask[i] / sum;
 
	*maskSizePointer = maskSize;
 
	return mask;
}

float * createBlurMask2D(float sigma, int * maskSizePointer) {
	int maskSize = (int)ceil(3.0f*sigma);
	float * mask = new float[(maskSize*2+1)*(maskSize*2+1)];
	float sum = 0.0f;
	for(int a = -maskSize; a <= maskSize; a++) {
		for(int b = -maskSize; b <= maskSize; b++) {
			float temp = exp(-((float)(a*a+b*b) / (2*sigma*sigma)));
			sum += temp;
			mask[(a+maskSize)*(maskSize*2+1)+(b+maskSize)] = temp;
		}
	}
	// Normalize the mask
	for(int i = 0; i < (maskSize*2+1)*(maskSize*2+1); i++)
		mask[i] = mask[i] / sum;
 
	*maskSizePointer = maskSize;
 
	return mask;
}
