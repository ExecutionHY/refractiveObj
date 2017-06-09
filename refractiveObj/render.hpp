//
//  render.hpp
//  refractiveObj
//
//  Created by Execution on 27/05/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef render_hpp
#define render_hpp

#include "main.hpp"
#include "glslprogram.hpp"
#include "model.hpp"
#include "controller.hpp"
#include "texture.hpp"
#include "text2D.hpp"
#include "voxelizer.hpp"
#include "photonmanager.hpp"

class Render {
private:
    GLFWwindow *window;
	Controller controller;
	
	GLSLProgram program_std;
	GLSLProgram program_obj;
	GLSLProgram program_sky;
	GLSLProgram program_photon;	// photon map
	void loadPrograms();
	
	Model m_object;
	Model m_background;
	void loadModels();
	
	Texture bgTexture;
	Texture texture_skybox;
	Texture texture_gradN;
	Texture texture_radiance;
	
	Text2D text2d;
	
	Voxelizer voxelizer;
	PhotonManager photonManager;
	
    GLuint VertexArrayID;
    GLuint vertexbuffer_object;
    GLuint uvbuffer_object;
    GLuint normalbuffer_object;
    GLuint elementbuffer_object;
    
    GLuint vertexbuffer_background;
    GLuint uvbuffer_background;
    GLuint normalbuffer_background;
    GLuint elementbuffer_background;

	GLuint vertexbuffer_skybox;
	

public:
    Render();
    ~Render();
    int run();
};

#endif /* render_hpp */
