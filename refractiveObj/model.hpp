//
//  model.hpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef model_hpp
#define model_hpp

#include "main.hpp"

struct PackedVertex{
    vec3 position;
    vec2 uv;
    vec3 normal;
    bool operator<(const PackedVertex that) const{
        return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
    };
};

class Model {
private:
    bool loadOBJ(const char * path,
                 vector<vec3> & out_vertices,
                 vector<vec2> & out_uvs,
                 vector<vec3> & out_normals
                 );
    bool getSimilarVertexIndex_fast(
		PackedVertex & packed,
		map<PackedVertex,unsigned short> & VertexToOutIndex,
		unsigned short & result
	);
    void indexVBO(
		vector<vec3> & in_vertices,
		vector<vec2> & in_uvs,
		vector<vec3> & in_normals,

		vector<unsigned short> & out_indices,
		vector<vec3> & out_vertices,
		vector<vec2> & out_uvs,
		vector<vec3> & out_normals
	);

public:
    Model();
    ~Model();
    vector<unsigned short> indices;
    vector<vec3> indexed_vertices;
    vector<vec2> indexed_uvs;
    vector<vec3> indexed_normals;
    void init(const char *path);
};

#endif /* model_hpp */
