//
//  texture.hpp
//  refractiveObj
//
//  Created by Execution on 29/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef texture_hpp
#define texture_hpp

#include "main.hpp"

extern string dir;

class Texture {
private:
public:
    Texture();
    ~Texture();
    void loadBMP(const char * imagepath);
    void loadDDS(const char * imagepath);
	
	void loadCubeMap(const char * mapname);
	void load3DArray(vec4 arr[VOXEL_CNT][VOXEL_CNT][VOXEL_CNT]);
	void initDepth();
	
    GLuint textureID;
};

#endif /* texture_hpp */
