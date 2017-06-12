//
//  voxelizer.cpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "voxelizer.hpp"
Voxelizer::Voxelizer() {}
Voxelizer::~Voxelizer() {}

bool Voxelizer::work(vector<vec3> & indexed_vertices,
					 vector<unsigned short> & indices) {
	printf("Voxelizing mesh...\n");
	
	voxelize_CL(indexed_vertices, indices);
	
	/*
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++)
				radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k] = vec4(0.03, 0.06, 0.06, 1);
	*/
	
	return true;
}

void Voxelizer::print() {
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++)
				printf("i: %2d, j: %2d, k: %2d, ri = %6f,  (%3f, %3f, %3f)\n", i, j, k, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].w, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].x, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].y, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].z);
}


#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "voxelize.cl"
#define KERNEL_FUNC "voxelize"

int Voxelizer::voxelize_CL(vector<vec3> & indexed_vertices,
						   vector<unsigned short> & indices) {
	
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
	cl_mem indices_buff, vertices_buff, refIndex_buff, gradN_buff;
	size_t work_units_per_kernel;
	float refConst = 1.5f;
	
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
	
	/* Create the context */
	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
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
	
	/* Create kernel for the mat_vec_mult function */
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	/* Create CL buffers to hold input and output data */
	indices_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								  sizeof(unsigned short)*indices.size(), &indices[0], &err);
	if(err < 0) {
		perror("Couldn't create a buffer object");
		exit(1);
	}
	vertices_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								   sizeof(vec3)*indexed_vertices.size(), &indexed_vertices[0], &err);
	refIndex_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
								   sizeof(float)*voxel_3, NULL, NULL);
	gradN_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
								sizeof(vec4)*voxel_3, NULL, NULL);
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &indices_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &vertices_buff);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &refIndex_buff);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &gradN_buff);
	int index_cnt = (int)indices.size(), voxel_cnt = VOXEL_CNT;
	clSetKernelArg(kernel, 4, sizeof(cl_int), &index_cnt);
	clSetKernelArg(kernel, 5, sizeof(cl_int), &voxel_cnt);
	clSetKernelArg(kernel, 6, sizeof(cl_float), &refConst);
	
	/* Create a CL command queue for the device*/
	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) {
		perror("Couldn't create the command queue");
		exit(1);
	}
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = voxel_3;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	/* Read the result */
	err = clEnqueueReadBuffer(queue, gradN_buff, CL_TRUE, 0, sizeof(vec4)*voxel_3, grad_n, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	
	/* Deallocate resources */
	clReleaseMemObject(indices_buff);
	clReleaseMemObject(vertices_buff);
	clReleaseMemObject(refIndex_buff);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	
	return 0;
}
