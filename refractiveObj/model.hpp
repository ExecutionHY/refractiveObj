//
//  model.hpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef model_hpp
#define model_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <cstdlib>
#include <string>
using namespace std;

extern string dir;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
using namespace glm;

#define VOXEL_CNT 20
extern vec4 refIndex[VOXEL_CNT][VOXEL_CNT][VOXEL_CNT];
extern vec4 radiance[VOXEL_CNT][VOXEL_CNT][VOXEL_CNT];

struct PackedVertex{
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    bool operator<(const PackedVertex that) const{
        return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
    };
};

class Model {
private:
    bool loadOBJ(const char * path,
                 std::vector<glm::vec3> & out_vertices,
                 std::vector<glm::vec2> & out_uvs,
                 std::vector<glm::vec3> & out_normals
                 );
    bool getSimilarVertexIndex_fast(
                                           PackedVertex & packed,
                                           std::map<PackedVertex,unsigned short> & VertexToOutIndex,
                                           unsigned short & result
                                    );
    void indexVBO(
                         std::vector<glm::vec3> & in_vertices,
                         std::vector<glm::vec2> & in_uvs,
                         std::vector<glm::vec3> & in_normals,
                         
                         std::vector<unsigned short> & out_indices,
                         std::vector<glm::vec3> & out_vertices,
                         std::vector<glm::vec2> & out_uvs,
                         std::vector<glm::vec3> & out_normals
                  );
	bool inside(float x, float y, float z);
public:
    Model();
    ~Model();
    vector<unsigned short> indices;
    vector<vec3> indexed_vertices;
    vector<vec2> indexed_uvs;
    vector<vec3> indexed_normals;
    void init(const char *path);
	bool voxelize();
	void printData();
};

#endif /* model_hpp */
