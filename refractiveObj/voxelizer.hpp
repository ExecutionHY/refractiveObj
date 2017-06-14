//
//  voxelizer.hpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef voxelizer_hpp
#define voxelizer_hpp

#include "main.hpp"

class Voxelizer {
private:
	int voxelize_CL(
		vector<vec3> & indexed_vertices,
		vector<unsigned short> & indices);
public:
	Voxelizer();
	~Voxelizer();
	bool work(vector<vec3> & indexed_vertices,
			  vector<unsigned short> & indices);
	void print();
};

#endif /* voxelizer_hpp */
