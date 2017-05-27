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
#include <cstdlib>
using namespace std;

#include <GL/glew.h>
#include <glm/glm.hpp>
using namespace glm;

class Model {
private:
    int vertex_count;
public:
    Model();
    ~Model();
    vector<vec3> vertices;
    int vertexCount() {return vertex_count;};
    void init();
};

#endif /* model_hpp */
