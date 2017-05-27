//
//  model.cpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#include "model.hpp"

Model::Model() {
    vertex_count = 0;
}
Model::~Model() {
    vertices.clear();
}
void Model::init() {
    vertices.push_back(vec3(-1.0f, -1.0f, 0.0f));
    vertices.push_back(vec3( 1.0f, -1.0f, 0.0f));
    vertices.push_back(vec3( 0.0f,  1.0f, 0.0f));
    vertex_count = 3;
}
