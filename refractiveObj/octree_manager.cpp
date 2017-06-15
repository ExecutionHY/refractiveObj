//
//  octree_manager.cpp
//  refractiveObj
//
//  Created by Execution on 15/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "octree_manager.hpp"

OctreeManager::OctreeManager() {}
OctreeManager::~OctreeManager() {}


#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "octree.cl"
#define KERNEL_FUNC "init"
#define KERNEL_FUNC2 "construct"

int OctreeManager::construct() {
	
	// Host/device data structures
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue queue;
	cl_int err;
	
	// Program/kernel data structures
	cl_program program;
	FILE *program_handle;
	char *program_buffer, *program_log;
	size_t program_size, log_size;
	cl_kernel kernel;
	
	// Data and buffers
	cl_int voxel_cnt = VOXEL_CNT, level;
	cl_int voxel_3 = VOXEL_CNT*VOXEL_CNT*VOXEL_CNT;
	cl_mem gradn_buff, temp_buff, octree_buff;
	size_t work_units_per_kernel;
	
	// Identify a platform
	err = clGetPlatformIDs(1, &platform, NULL);
	if(err < 0) {
		perror("Couldn't find any platforms");
		exit(1);
	}
	
	// Access a device
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if(err < 0) {
		perror("Couldn't find any devices");
		exit(1);
	}
	
	// Create the context
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	if(err < 0) {
		perror("Couldn't create a context");
		exit(1);
	}
	
	string program_path = dir + "refractiveObj/" + string(PROGRAM_FILE);
	// Read program file and place content into buffer
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
	
	// Create program from file
	program = clCreateProgramWithSource(context, 1,
										(const char**)&program_buffer, &program_size, &err);
	if(err < 0) {
		perror("Couldn't create the program");
		exit(1);
	}
	free(program_buffer);
	
	// Build program
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if(1) {
		
		// Find size of log and print to std output
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  log_size + 1, program_log, NULL);
		printf("%s\n", program_log);
		free(program_log);
		//exit(1);
	}
	
	// Create a CL command queue for the device
	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) {
		perror("Couldn't create the command queue");
		exit(1);
	}
	
	
	// Create CL buffers to hold input and output data
	gradn_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float4)*voxel_3, grad_n, &err);
	if(err < 0) {
		perror("Couldn't create a buffer object");
		exit(1);
	}
	temp_buff = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float2)*voxel_3, NULL, NULL);
	octree_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int)*voxel_3, NULL, NULL);
	
	
	// *************** level 0 - init temp
	
	level = 0;
	
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	// Create kernel arguments from the CL buffers
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &gradn_buff);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &temp_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	
	// Enqueue the command queue to the device
	work_units_per_kernel = voxel_3;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	clReleaseKernel(kernel);
	
	
	// ************* level 1 ~ k-1 - construct
	
	int k = (int)ceil(log(VOXEL_CNT)/log(2));
	int explevel = 1;
	for (int level = 1; level < k; level++) {
		
		// kernel for level i
		kernel = clCreateKernel(program, KERNEL_FUNC2, &err);
		if(err < 0) {
			perror("Couldn't create the kernel");
			exit(1);
		}
		
		// Create kernel arguments from the CL buffers
		err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &temp_buff);
		err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &octree_buff);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_int), &level);
		explevel = explevel * 2;
		err |= clSetKernelArg(kernel, 3, sizeof(cl_int), &explevel);
		err |= clSetKernelArg(kernel, 4, sizeof(cl_int), &voxel_cnt);
		if(err < 0) {
			perror("Couldn't set the kernel argument");
			exit(1);
		}
		
		// Enqueue the command queue to the device
		work_units_per_kernel = voxel_3;
		// global_work_size cannot exceed the range given by the sizeof(size_t)
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
		if(err < 0) {
			perror("Couldn't enqueue the kernel execution command");
			exit(1);
		}
		
		clReleaseKernel(kernel);
	}
	
	// Read the result
	err = clEnqueueReadBuffer(queue, octree_buff, CL_TRUE, 0, sizeof(int)*voxel_3, octree, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	
	
	
	 for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++) {
				int index = i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k;
				printf("i:%3d, j:%3d, k:%3d L %d\n",i,j,k, octree[index]);
	 }
	
	
	/* Deallocate resources */
	clReleaseMemObject(gradn_buff);
	clReleaseMemObject(temp_buff);
	clReleaseMemObject(octree_buff);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	
	return 0;
}
