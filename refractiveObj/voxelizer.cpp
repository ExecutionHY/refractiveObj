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
				printf("v-i: %2d, j: %2d, k: %2d, ri = %6f,  (%3f, %3f, %3f)\n", i, j, k, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].w, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].x, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].y, grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].z);
}


#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "voxelize.cl"
#define KERNEL_FUNC "voxelize"
#define KERNEL_FUNC2 "blur"
#define KERNEL_FUNC3 "gradient"

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
	cl_mem indices_buff, vertices_buff, refIndex_buff,
		gradn_buff, blured_buff, mask_buff;
	size_t work_units_per_kernel;
	float refConst = 1.25f;
	
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
	if(1) {
		
		/* Find size of log and print to std output */
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
	
	
	/* Create CL buffers to hold input and output data */
	indices_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned short)*indices.size(), &indices[0], &err);
	if(err < 0) {
		perror("Couldn't create a buffer object");
		exit(1);
	}
	vertices_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								   sizeof(vec3)*indexed_vertices.size(), &indexed_vertices[0], &err);
	refIndex_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
								   sizeof(float)*voxel_3, NULL, NULL);
	blured_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
								   sizeof(float)*voxel_3, NULL, NULL);
	gradn_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
								 sizeof(cl_float4)*voxel_3, NULL, NULL);
	
	int maskSize;
	float* mask = createBlurMask3D(1.0f, &maskSize);
	mask_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*(2*maskSize+1)*(2*maskSize+1)*(2*maskSize+1), mask, &err);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't create a buffer object");
		exit(1);
	}
	int index_cnt = (int)indices.size(), voxel_cnt = VOXEL_CNT;
	
	// *************** voxelize
	
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &indices_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &vertices_buff);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &refIndex_buff);
	clSetKernelArg(kernel, 3, sizeof(cl_int), &index_cnt);
	clSetKernelArg(kernel, 4, sizeof(cl_int), &voxel_cnt);
	clSetKernelArg(kernel, 5, sizeof(cl_float), &refConst);
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = voxel_3;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	
	
	clReleaseKernel(kernel);
	//************** blur
	
	kernel = clCreateKernel(program, KERNEL_FUNC2, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	// Create kernel arguments from the CL buffers
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &refIndex_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &blured_buff);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &mask_buff);
	clSetKernelArg(kernel, 3, sizeof(cl_int), &maskSize);
	clSetKernelArg(kernel, 4, sizeof(cl_int), &voxel_cnt);
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = voxel_3;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	clReleaseKernel(kernel);
	//************* gradient
	
	kernel = clCreateKernel(program, KERNEL_FUNC3, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &blured_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &gradn_buff);
	clSetKernelArg(kernel, 2, sizeof(cl_int), &voxel_cnt);
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = voxel_3;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}

	
	// Read the result
	err = clEnqueueReadBuffer(queue, gradn_buff, CL_TRUE, 0, sizeof(vec4)*voxel_3, grad_n, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	
	
	/*
	for(int a = -maskSize; a <= maskSize; a++) {
		for(int b = -maskSize; b <= maskSize; b++) {
			for (int c = -maskSize; c <= maskSize; c++) {
				printf("%6f ", mask[(a+maskSize)*(maskSize*2+1)*(maskSize*2+1)+(b+maskSize)*(maskSize*2+1)+(c+maskSize)]);
				
			}
			printf("\n");
		}
	}
	*/
	
	/*
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++) {
				int index = i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k;
				grad_n[index] = vec4(gradnx[index], gradny[index], gradnz[index], blured[index]);
			}
	*/
	
	/* Deallocate resources */
	clReleaseMemObject(indices_buff);
	clReleaseMemObject(vertices_buff);
	clReleaseMemObject(refIndex_buff);
	clReleaseMemObject(blured_buff);
	clReleaseMemObject(mask_buff);
	clReleaseMemObject(gradn_buff);
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	
	return 0;
}
