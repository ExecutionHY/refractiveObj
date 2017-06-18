//
//  photon_manager.hpp
//  refractiveObj
//
//  Created by Execution on 09/06/2017.
//  Copyright © 2017 Execution. All rights reserved.
//

#ifndef photon_manager_hpp
#define photon_manager_hpp

#include "main.hpp"
class PhotonManager {
private:
	
public:
	
	PhotonManager();
	~PhotonManager();
	
	int march(GLuint textureID_photonmap, vec3 pos1);

};

#endif /* photon-manager_hpp */
