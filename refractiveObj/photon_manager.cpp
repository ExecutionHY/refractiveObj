//
//  photon_manager.cpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "photon_manager.hpp"
float rx[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
float ry[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];
float rz[VOXEL_CNT*VOXEL_CNT*VOXEL_CNT];

PhotonManager::PhotonManager() {}
PhotonManager::~PhotonManager() {}

float * createBlurMask(float sigma, int * maskSizePointer) {
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


#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "photonmarch.cl"
#define KERNEL_FUNC "photonmarch"
#define KERNEL_FUNC2 "radianceblur"


int PhotonManager::march(GLuint textureID_photonmap) {
	
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
	cl_mem map_buff, gradn_buff, matrix_buff,rx_buff, ry_buff, rz_buff;
	size_t work_units_per_kernel;
	
	cl_mem input_buff, output_buff, radiance_buff, mask_buff;
	int maskSize;
	vec3 lightPos;
	
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
	if(1) {
		
		/* Find size of log and print to std output */
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  0, NULL, &log_size);
		program_log = (char*) malloc(log_size + 1);
		program_log[log_size] = '\0';
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
							  log_size + 1, program_log, NULL);
		printf("Build program %d: %s\n", err, program_log);
		free(program_log);
		//exit(1);
	}
	
	/* Create a CL command queue for the device*/
	queue = clCreateCommandQueue(context, device, 0, &err);
	if(err < 0) {
		perror("Couldn't create the command queue");
		exit(1);
	}
	
	
	/* Create CL buffers to hold input and output data */
	map_buff = clCreateFromGLTexture(context, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, textureID_photonmap, &err);
	if(err < 0) {
		perror("Couldn't create a buffer object");
		exit(1);
	}
	gradn_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(vec4)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, &grad_n[0], &err);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't create a buffer object 2");
		exit(1);
	}
	
	lightPos = vec3(0,2,0);
	rx_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
							 sizeof(cl_float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	ry_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
							 sizeof(cl_float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	rz_buff = clCreateBuffer(context, CL_MEM_READ_WRITE,
							 sizeof(cl_float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	radiance_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
							 sizeof(vec4)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, NULL, NULL);
	
	float* mask = createBlurMask(2.0f, &maskSize);
	mask_buff = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*(2*maskSize+1)*(2*maskSize+1)*(2*maskSize+1), mask, NULL);
	
	
	//************ marching
	
	kernel = clCreateKernel(program, KERNEL_FUNC, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &map_buff);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_int), &MAP_WIDTH);
	clSetKernelArg(kernel, 2, sizeof(cl_int), &MAP_HEIGHT);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), &gradn_buff);
	int voxel_cnt = VOXEL_CNT;
	clSetKernelArg(kernel, 4, sizeof(cl_int), &voxel_cnt);
	clSetKernelArg(kernel, 5, sizeof(cl_float3), &lightPos);
	clSetKernelArg(kernel, 6, sizeof(cl_mem), &rx_buff);
	clSetKernelArg(kernel, 7, sizeof(cl_mem), &ry_buff);
	clSetKernelArg(kernel, 8, sizeof(cl_mem), &rz_buff);
	
	
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
	/*
	err = clEnqueueReadBuffer(queue, rx_buff, CL_TRUE, 0, sizeof(float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, rx, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(queue, ry_buff, CL_TRUE, 0, sizeof(float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, ry, 0, NULL, NULL);
	err |= clEnqueueReadBuffer(queue, rz_buff, CL_TRUE, 0, sizeof(float)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, rz, 0, NULL, NULL);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	*/
	clEnqueueReleaseGLObjects(queue, 1, &map_buff, 0, 0, 0);
	//clFinish(queue);
	clReleaseKernel(kernel);
	
	/*
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
	*/

	
	//************* blur
	
	kernel = clCreateKernel(program, KERNEL_FUNC2, &err);
	if(err < 0) {
		perror("Couldn't create the kernel");
		exit(1);
	}
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &rx_buff);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &ry_buff);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &rz_buff);
	err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &radiance_buff);
	err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &mask_buff);
	err |= clSetKernelArg(kernel, 5, sizeof(cl_int), &maskSize);
	err |= clSetKernelArg(kernel, 6, sizeof(cl_int), &voxel_cnt);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	
	
	/* Enqueue the command queue to the device */
	work_units_per_kernel = VOXEL_CNT * VOXEL_CNT * VOXEL_CNT;
	// global_work_size cannot exceed the range given by the sizeof(size_t)
	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_units_per_kernel, NULL, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the kernel execution command");
		exit(1);
	}
	
	
	/* Read the result */
	err = clEnqueueReadBuffer(queue, radiance_buff, CL_TRUE, 0, sizeof(vec4)*VOXEL_CNT*VOXEL_CNT*VOXEL_CNT, radiance, 0, NULL, NULL);
	if(err < 0) {
		printf("%d ", err);
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	clReleaseKernel(kernel);
	
	/*
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++) {
				printf("pm-i = %d, j = %d, k = %d : %6f (%6f, %6f, %6f)\n", i, j, k,
					   grad_n[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].w,
					   radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].x,
					   radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].y,
					   radiance[i*VOXEL_CNT*VOXEL_CNT+j*VOXEL_CNT+k].z);
			}
	 */
	
	/* Deallocate resources */
	clReleaseMemObject(map_buff);
	clReleaseMemObject(gradn_buff);
	clReleaseMemObject(rx_buff);
	clReleaseMemObject(ry_buff);
	clReleaseMemObject(rz_buff);
	clReleaseMemObject(radiance_buff);
	clReleaseMemObject(mask_buff);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);
	return 0;
}
