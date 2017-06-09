//
//  photonmanager.hpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef photonmanager_hpp
#define photonmanager_hpp

#include "main.hpp"
class PhotonManager {
private:
	GLuint textureID_photonmap;
	
	/* Host/device data structures */
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_int err;
	
	/* Program/kernel data structures */
	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	cl_kernel kernel;
	
	/* Data and buffers */
	cl_int voxel_3 = VOXEL_CNT*VOXEL_CNT*VOXEL_CNT;
	cl_mem input_buff, output_buff;
	size_t work_units_per_kernel;
	
	
public:
	
	PhotonManager();
	~PhotonManager();
	void generate(GLuint textureID_photonmap);
	
	int march_init();
	int march_share();
	int march_execute();

};

#endif /* photonmanager_hpp */
