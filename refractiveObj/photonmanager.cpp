//
//  photonmanager.cpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "photonmanager.hpp"
float rx[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
float ry[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
float rz[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];

PhotonManager::PhotonManager() {}
PhotonManager::~PhotonManager() {}

void PhotonManager::generate(GLuint textureID_photonmap) {
	this->textureID_photonmap = textureID_photonmap;
	march_init();
	march_share();
	march_execute();
}


#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "photonmarch.cl"
#define KERNEL_FUNC "photonmarch"

int PhotonManager::march_init() {
	
	
	/* Identify a platform */
	err = clGetPlatformIDs(1, &platform, NULL);
	if(err < 0) {
		perror("Couldn't find any platforms");
		exit(1);
	}
	
	/* Access a device */
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if(err < 0) {
		perror("Couldn't find any devices");
		exit(1);
	}
	
	CGLContextObj kCGLContext = CGLGetCurrentContext(); // GL Context
	CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext); // Share Group
	
	cl_context_properties props[] =
	{
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
		(cl_context_properties) kCGLShareGroup,
		CL_CONTEXT_PLATFORM,
		(cl_context_properties) platform,
		0
	};
	
	context = clCreateContext(props, 1, &device, NULL, NULL, &err);
	
	/* Create the context */
	
	if(err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}
	
	string program_path = dir + "refractiveObj/" + string(PROGRAM_FILE);
	/* Read program file and place content into buffer */
	program_handle = fopen(program_path.c_str(), "r");
	if(program_handle == NULL) {
		perror("Couldn't find the program file");
		exit(1);
	}
	fseek(program_handle, 0, SEEK_END);
	program_size = ftell(program_handle);
	rewind(program_handle);
	program_buffer = (char*)malloc(program_size + 1);
	program_buffer[program_size] = '\0';
	fread(program_buffer, sizeof(char), program_size, program_handle);
	fclose(program_handle);
	
	/* Create program from file */
	program = clCreateProgramWithSource(context, 1,
										(const char**)&program_buffer, &program_size, &err);
	if(err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);
	
	/* Build program */
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if(err < 0) {
		
		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  log_size + 1, program_log, NULL);
		printf("Build program %d: %s\n", err, program_log);
		free(program_log);
		exit(1);
	}
	
	/* Create a CL command queue for the device*/
	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) {
		perror("Couldn't create the command queue");
		exit(1);
	}
	
	/* Create kernel for the mat_vec_mult function */
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	return 0;
}

int PhotonManager::march_share() {
	
	
	/* Create CL buffers to hold input and output data */
	map_buff = clCreateFromGLTexture(context, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, textureID_photonmap, &err);
	if(err < 0) {
		perror("Couldn't create a buffer object");
		exit(1);
	}
	gradN_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(vec4)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, &grad_n[0], &err);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't create a buffer object 2");
		exit(1);
	}
	
	vec3 lightInvDir = glm::vec3(2,2,2);
	rx_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
							 sizeof(cl_float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	ry_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
							 sizeof(cl_float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	rz_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
							 sizeof(cl_float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &map_buff);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_int), &MAP_WIDTH);
	clSetKernelArg(kernel, 2, sizeof(cl_int), &MAP_HEIGHT);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &gradN_buff);
	int voxel_cnt = VOXEL_CNT;
	clSetKernelArg(kernel, 4, sizeof(cl_int), &voxel_cnt);
	clSetKernelArg(kernel, 5, sizeof(cl_float3), &lightInvDir);
	clSetKernelArg(kernel, 6, sizeof(cl_mem), &rx_buff);
	clSetKernelArg(kernel, 7, sizeof(cl_mem), &ry_buff);
	clSetKernelArg(kernel, 8, sizeof(cl_mem), &rz_buff);
	
	return 0;
}

int PhotonManager::march_execute() {
	
	
	glFinish();
	// All pending GL calls have finished -> safe to acquire the buffer in CL
	err = clEnqueueAcquireGLObjects(queue, 1, &map_buff, 0, 0, 0);
	if(err < 0) {
		perror("Couldn't acquire the GL objects");
		exit(1);
	}
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = MAP_WIDTH * MAP_HEIGHT;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	
	/* Read the result */
	err = clEnqueueReadBuffer(queue, rx_buff, CL_TRUE, 0, sizeof(float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, rx, 0, NULL, NULL);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	clEnqueueReadBuffer(queue, ry_buff, CL_TRUE, 0, sizeof(float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, ry, 0, NULL, NULL);
	clEnqueueReadBuffer(queue, rz_buff, CL_TRUE, 0, sizeof(float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, rz, 0, NULL, NULL);
	
	clEnqueueReleaseGLObjects(queue, 1, &map_buff, 0, 0, 0);
	clFinish(queue);
	
	
	for (int i = 0; i < VOXEL_CNT; i++)
	for (int j = 0; j < VOXEL_CNT; j++)
	for (int k = 0; k < VOXEL_CNT; k++) {
	radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k] = vec4(
		rx[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k],
		ry[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k],
		rz[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k],
		0
	);
		printf("i = %d, j = %d, k = %d : %6f (%6f, %6f, %6f)\n", i, j, k,
			   grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].w,
			   radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].x,
			   radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].y,
			   radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].z);
	}
	
	//printf("....%6f\n", rx[0][0][0]);
	
	/* Deallocate resources */
	clReleaseMemObject(map_buff);
	clReleaseMemObject(gradN_buff);
	clReleaseMemObject(rx_buff);
	clReleaseMemObject(ry_buff);
	clReleaseMemObject(rz_buff);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	
	
	return 0;
}
