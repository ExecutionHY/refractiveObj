//
//  texture.cpp
//  refractiveObj
//
//  Created by Execution on 29/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "texture.hpp"
Texture::Texture() {}
Texture::~Texture() {
    // clear the texture
    glDeleteTextures(1, &textureID);
}

void Texture::loadBMP(const char * imagepath){
    
    printf("Reading image %s\n", imagepath);
    
    string path = dir + "pictures/" + string(imagepath);
    
    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;
    
    // Open the file
    FILE * file = fopen(path.c_str(),"rb");
    if (!file) {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        getchar();
        exit(-1);
    }
    
    // Read the header, i.e. the 54 first bytes
    
    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 ) {
        printf("Not a correct BMP file\n");
        exit(-1);
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' ) {
        printf("Not a correct BMP file\n");
        exit(-1);
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  ) {
        printf("Not a correct BMP file\n");
        exit(-1);
    }
    if ( *(int*)&(header[0x1C])!=24 ) {
        printf("Not a correct BMP file\n");
        exit(-1);
    }
    
    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    
    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way
    
    // Create a buffer
    data = new unsigned char [imageSize];
    
    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);
    
    // Everything is in memory now, the file wan be closed
    fclose (file);
    
    // Create one OpenGL texture
    glGenTextures(1, &textureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    
    // OpenGL has now copied the data. Free our own version
    delete [] data;
    
    // Poor filtering, or ...
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    // ... nice trilinear filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
    glGenerateMipmap(GL_TEXTURE_2D);
    
}


#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

void Texture::loadDDS(const char * imagepath) {
    
    printf("Reading image %s\n", imagepath);
    unsigned char header[124];
    
    FILE *fp;
    
    string path = dir + "pictures/" + string(imagepath);
    /* try to open the file */
    fp = fopen(path.c_str(), "rb");
    if (fp == NULL){
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        getchar();
        exit(-1);
    }
    
    /* verify the type of file */
    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        exit(-1);
    }
    
    /* get the surface desc */
    fread(&header, 124, 1, fp);
    
    unsigned int height      = *(unsigned int*)&(header[8 ]);
    unsigned int width	     = *(unsigned int*)&(header[12]);
    unsigned int linearSize	 = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC      = *(unsigned int*)&(header[80]);
    
    
    unsigned char * buffer;
    unsigned int bufsize;
    /* how big is it going to be including all mipmaps? */
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    /* close the file pointer */
    fclose(fp);
    
    unsigned int format;
    switch(fourCC)
    {
        case FOURCC_DXT1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
        case FOURCC_DXT3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
        case FOURCC_DXT5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
        default:
        free(buffer);
        exit(-1);
    }
    
    // Create one OpenGL texture
    glGenTextures(1, &textureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;
    
    /* load the mipmaps */
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
    {
        unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
                               0, size, buffer + offset);
        
        offset += size; 
        width  /= 2; 
        height /= 2; 
        
        // Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
        if(width < 1) width = 1;
        if(height < 1) height = 1;
        
    } 
    
    free(buffer); 
    
}

void Texture::loadCubeMap(const char * mapname){
	
	printf("Reading cube map %s\n", mapname);
	
	string filename[6] = {"right.bmp", "left.bmp", "top.bmp", "bottom.bmp", "back.bmp", "front.bmp"};

	
	
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data[6];
	
	for (int i = 0; i < 6; i++) {
		
		string path = dir + "pictures/" + string(mapname) + "-" + filename[i];
		
		// Open the file
		FILE * file = fopen(path.c_str(), "rb");
		if (!file) {
			printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", mapname);
			getchar();
			exit(-1);
		}
		
		// Read the header, i.e. the 54 first bytes
		
		// If less than 54 bytes are read, problem
		if ( fread(header, 1, 54, file)!=54 ) {
			printf("Not a correct BMP file\n");
			exit(-1);
		}
		// A BMP files always begins with "BM"
		if ( header[0]!='B' || header[1]!='M' ) {
			printf("Not a correct BMP file\n");
			exit(-1);
		}
		// Make sure this is a 24bpp file
		if ( *(int*)&(header[0x1E])!=0  ) {
			printf("Not a correct BMP file\n");
			exit(-1);
		}
		if ( *(int*)&(header[0x1C])!=24 ) {
			printf("Not a correct BMP file\n");
			exit(-1);
		}
		
		// Read the information about the image
		dataPos    = *(int*)&(header[0x0A]);
		imageSize  = *(int*)&(header[0x22]);
		width      = *(int*)&(header[0x12]);
		height     = *(int*)&(header[0x16]);
		
		// Some BMP files are misformatted, guess missing information
		if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
		if (dataPos==0)      dataPos=54; // The BMP header is done that way
		
		// Create a buffer
		data[i] = new unsigned char [imageSize];
		
		// Read the actual data from the file into the buffer
		fread(data[i], 1, imageSize, file);
		
		// Everything is in memory now, the file wan be closed
		fclose (file);
	}
	
	
	
	// Create one OpenGL texture
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	// Give the image to OpenGL
	for(int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data[i]);
	}

	
	// OpenGL has now copied the data. Free our own version
	for (int i = 0; i < 6; i++)
		delete [] data[i];
	
	
	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);
}

void Texture::load3D(vec4 arr[]) {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_3D, textureID);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, VOXEL_CNT, VOXEL_CNT, VOXEL_CNT, 0, GL_RGBA, GL_FLOAT, arr);
}

void Texture::initDepth() {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, MAP_WIDTH, MAP_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	 */
}
