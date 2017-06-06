//
//  model.cpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "model.hpp"

Model::Model() {
}
Model::~Model() {
    indices.clear();
    indexed_vertices.clear();
    indexed_uvs.clear();
    indexed_normals.clear();
}
bool Model::loadOBJ(const char * path,
                    std::vector<glm::vec3> & out_vertices,
                    std::vector<glm::vec2> & out_uvs,
                    std::vector<glm::vec3> & out_normals
                    ) {
    
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;
    
    
    FILE * file = fopen(path, "r");
    if( file == NULL ){
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        getchar();
        return false;
    }
    
    while( 1 ){
        
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        
        // else : parse lineHeader
        
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back(vertex);
        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            temp_uvs.push_back(uv);
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
                printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }else{
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
        
    }
    
    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){
        
        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];
        
        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        glm::vec2 uv = temp_uvs[ uvIndex-1 ];
        glm::vec3 normal = temp_normals[ normalIndex-1 ];
        
        // Put the attributes in buffers
        out_vertices.push_back(vertex);
        out_uvs     .push_back(uv);
        out_normals .push_back(normal);
        
    }
    
    return true;
}


bool Model::getSimilarVertexIndex_fast(
                                PackedVertex & packed,
                                std::map<PackedVertex,unsigned short> & VertexToOutIndex,
                                unsigned short & result
                                ){
    std::map<PackedVertex,unsigned short>::iterator it = VertexToOutIndex.find(packed);
    if ( it == VertexToOutIndex.end() ){
        return false;
    }else{
        result = it->second;
        return true;
    }
}

void Model::indexVBO(
              std::vector<glm::vec3> & in_vertices,
              std::vector<glm::vec2> & in_uvs,
              std::vector<glm::vec3> & in_normals,
              
              std::vector<unsigned short> & out_indices,
              std::vector<glm::vec3> & out_vertices,
              std::vector<glm::vec2> & out_uvs,
              std::vector<glm::vec3> & out_normals
              ){
    std::map<PackedVertex,unsigned short> VertexToOutIndex;
    
    // For each input vertex
    for ( unsigned int i=0; i<in_vertices.size(); i++ ){
        
        PackedVertex packed = {in_vertices[i], in_uvs[i], in_normals[i]};
        
        
        // Try to find a similar vertex in out_XXXX
        unsigned short index;
        bool found = getSimilarVertexIndex_fast( packed, VertexToOutIndex, index);
        
        if ( found ){ // A similar vertex is already in the VBO, use it instead !
            out_indices.push_back( index );
        }else{ // If not, it needs to be added in the output data.
            out_vertices.push_back( in_vertices[i]);
            out_uvs     .push_back( in_uvs[i]);
            out_normals .push_back( in_normals[i]);
            unsigned short newindex = (unsigned short)out_vertices.size() - 1;
            out_indices .push_back( newindex );
            VertexToOutIndex[ packed ] = newindex;
        }
    }
}


void Model::init(const char *path) {
    string file = dir + "models/" + string(path);
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
	
	printf("Loading OBJ file %s...", path);
	
    if (loadOBJ(file.c_str(), vertices, uvs, normals) == false) exit(-1);
    
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	printf(" %lu triangles.\n", indices.size()/3);
}

bool Model::inside(float x, float y, float z) {
	int intersectCnt = 0;
	vec3 intersects;
	for (int i = 0; i < indices.size(); i += 3) {
		if (intersectRayTriangle(vec3(x, y, z), vec3(1,0,0), indexed_vertices[indices[i]], indexed_vertices[indices[i+1]], indexed_vertices[indices[i+2]], intersects))
			intersectCnt++;
		/*
		else {
			printf("intersects = (%3f, %3f, %3f)\n", intersects.x, intersects.y, intersects.z);
		}
		
		if (-0.0001 < x && x < 0.0001 && -0.0001 < y && y < 0.0001 && -0.0001 < z && z < 0.0001)
			printf("(%3f, %3f, %3f) -> (%3f, %3f, %3f), (%3f, %3f, %3f), (%3f, %3f, %3f), cnt = %d\n", x, y, z, indexed_vertices[indices[i]].x, indexed_vertices[indices[i]].y, indexed_vertices[indices[i]].z, indexed_vertices[indices[i+1]].x, indexed_vertices[indices[i+1]].y, indexed_vertices[indices[i+1]].z, indexed_vertices[indices[i+2]].x, indexed_vertices[indices[i+2]].y, indexed_vertices[indices[i+2]].z, intersectCnt);
		 */
	}
	if (intersectCnt % 2 == 1) return true;
	else return false;
}

bool Model::init() {
	printf("Voxelizing mesh\n");
	/*
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++)
				if (inside((i-VOXEL_CNT/2)*0.1/(VOXEL_CNT/2), (j-VOXEL_CNT/2)*0.1/(VOXEL_CNT/2), (k-VOXEL_CNT/2)*0.1/(VOXEL_CNT/2))) {
					refIndex[i][j][k] = vec4(1.5, 0.0, 0.0, 0.0);
				}
				else refIndex[i][j][k] = vec4(1.0, 0.0, 0.0, 0.0);
	*/
	voxelize_CL();
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++)
				radiance[i][j][k] = vec4(0.01, 0.02, 0.02, 1);
	
	
	return true;
}

void Model::printData() {
	for (int i = 0; i < VOXEL_CNT; i++)
		for (int j = 0; j < VOXEL_CNT; j++)
			for (int k = 0; k < VOXEL_CNT; k++)
				printf("i: %2d, j: %2d, k: %2d, ri = %6f,  (%3f, %3f, %3f)\n", i, j, k, refIndex[i][j][k].x, radiance[i][j][k].x, radiance[i][j][k].y, radiance[i][j][k].z);
}
#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "voxelizer.cl"
#define KERNEL_FUNC "voxelize"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <OpenCL/cl.h>

int Model::voxelize_CL() {
	
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
	float* refIdx = (float*)malloc(sizeof(float)*voxel_3);
	cl_mem indices_buff, vertices_buff, refIndex_buff;
	size_t work_units_per_kernel;
	
	/* Initialize data to be processed by the kernel */
	
	
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
	refIndex_buff = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
							  sizeof(float)*voxel_3, NULL, NULL);
	
	/* Create kernel arguments from the CL buffers */
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &indices_buff);
	if(err < 0) {
		perror("Couldn't set the kernel argument");
		exit(1);
	}
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &vertices_buff);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &refIndex_buff);
	int index_cnt = (int)indices.size(), voxel_cnt = VOXEL_CNT;
	clSetKernelArg(kernel, 3, sizeof(cl_int), &index_cnt);
	clSetKernelArg(kernel, 4, sizeof(cl_int), &voxel_cnt);
	
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
	err = clEnqueueReadBuffer(queue, refIndex_buff, CL_TRUE, 0, sizeof(float)*voxel_3, refIdx, 0, NULL, NULL);
	if(err < 0) {
		perror("Couldn't enqueue the read buffer command");
		exit(1);
	}
	
	/* Test the result */
	for (int i = 0; i < voxel_3; i++) {
		int x = i / (voxel_cnt * voxel_cnt);
		int y = (i  % (voxel_cnt * voxel_cnt)) / voxel_cnt;
		int z = i % voxel_cnt;
		refIndex[x][y][z].x = refIdx[i];
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
