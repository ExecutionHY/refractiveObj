//
//  photonmanager.cpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "photonmanager.hpp"
vec4 out[1600][1200];
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
		printf("%s\n", program_log);
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
	input_buff = clCreateFromGLTexture(context, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, textureID_photonmap, &err);
	if(err < 0) {
		perror("Couldn't create a buffer object");
		exit(1);
	}
	output_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
								sizeof(vec4)*FRAME_WIDTH*FRAME_HEIGHT, NULL, NULL);
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input_buff);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buff);
	
	return 0;
}

int PhotonManager::march_execute() {
	
	
	glFinish();
	// All pending GL calls have finished -> safe to acquire the buffer in CL
	err = clEnqueueAcquireGLObjects(queue, 1, &input_buff, 0,0,0);
	if(err < 0) {
		perror("Couldn't acquire the GL objects");
		exit(1);
	}
	
	err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = 1024*1024;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	
	
	/* Read the result */
	err = clEnqueueReadBuffer(queue, output_buff, CL_TRUE, 0, sizeof(vec4)*FRAME_WIDTH*FRAME_HEIGHT, out, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	clEnqueueReleaseGLObjects(queue, 1, &input_buff, 0,0,0);
	clFinish(queue);
	/*
	int cnt = 0;
	for (int i = 0; i < FRAME_WIDTH; i++) {
		for (int j = 0; j < FRAME_HEIGHT; j++)
			if (out[i][j].x > 0.5) cnt++;
	}
	printf("%d\n", cnt);
	*/
	
	/* Deallocate resources */
	clReleaseMemObject(input_buff);
	clReleaseMemObject(output_buff);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	
	
	return 0;
}
