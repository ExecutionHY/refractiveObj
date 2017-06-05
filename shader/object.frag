#version 330 core

// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 EyeDirection_worldspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler3D radianceDistribution;
uniform sampler3D refractiveIndex;

vec3 dirRemain = vec3(0, 0, 0);
// Some dimensions of direction may be too small to affect nextPos,
// so we should accumulate these values remained by nextPos().
// When they are large enough, they will reflect on next position.

// find next voxel according to direction and footstep
vec3 nextPos(vec3 pos, vec3 dir, int footstep) {
	vec3 npos = pos;
	dirRemain += dir;
	float dirx = abs(dirRemain.x);
	float diry = abs(dirRemain.y);
	float dirz = abs(dirRemain.z);
	// if that dimension is not choosed, add that dimension to dirRemain
	// if choosed, set that dimension to 0
	if (dirx >= diry && dirx >= dirz) {
		if (dirRemain.x > 0) npos += vec3(1, 0, 0)*footstep;
		else npos += vec3(-1, 0, 0)*footstep;
		dirRemain.x = 0;
	}
	if (diry >= dirx && diry >= dirx) {
		if (dirRemain.y > 0) npos += vec3(0, 1, 0)*footstep;
		else npos += vec3(0, -1, 0)*footstep;
		dirRemain.y = 0;
	}
	if (dirz >= dirx && dirz >= diry) {
		if (dirRemain.z > 0) npos += vec3(0, 0, 1)*footstep;
		else npos += vec3(0, 0, -1)*footstep;
		dirRemain.z = 0;
	}
	return npos;
}

void main(){
    
    vec3 radiance = vec3(0, 0, 0);
    vec3 pos = Position_worldspace;
	vec3 dir = EyeDirection_worldspace;
	
    for (int i = 0; i < 3; i++) {
        radiance += texture(radianceDistribution, (pos+vec3(1,1,1))*0.5).rgb;
        pos = nextPos(pos, dir, 1);
    }
	
    color = radiance;
    
}
