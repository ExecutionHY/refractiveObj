#version 330 core

// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 EyeDirection_worldspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler3D radianceDistribution;
uniform sampler3D refractiveIndex;
uniform int voxel_cnt;

vec3 dirRemain = vec3(0, 0, 0);
// Some dimensions of direction may be too small to affect nextPos,
// so we should accumulate these values remained by nextPos().
// When they are large enough, they will reflect on next position.

// find next voxel according to direction and footstep
vec3 nextPos(vec3 pos, vec3 dir, float footstep) {
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
	else if (diry >= dirx && diry >= dirx) {
		if (dirRemain.y > 0) npos += vec3(0, 1, 0)*footstep;
		else npos += vec3(0, -1, 0)*footstep;
		dirRemain.y = 0;
	}
	else if (dirz >= dirx && dirz >= diry) {
		if (dirRemain.z > 0) npos += vec3(0, 0, 1)*footstep;
		else npos += vec3(0, 0, -1)*footstep;
		dirRemain.z = 0;
	}
	return npos;
}

void main(){
    
    vec3 radiance = vec3(0, 0, 0);
    vec3 pos = Position_worldspace;
	vec3 dir = normalize(EyeDirection_worldspace);
	vec3 npos;
	float refIndexRatio;
	float cnt = 0.00; // for debug
	
    for (int i = 0; i < voxel_cnt; i++) {
		if (abs(pos.x) > 1 || abs(pos.y) > 1 || abs(pos.z) > 1) break;
		if (texture(refractiveIndex, (pos+vec3(1,1,1))*0.5).r > 1.001) {
			radiance += texture(radianceDistribution, (pos+vec3(1,1,1))*0.5).rgb;
			cnt += 1.0/voxel_cnt;
		}
        npos = nextPos(pos, dir, 1.0/voxel_cnt);
		refIndexRatio = texture(refractiveIndex, (pos+vec3(1,1,1))*0.5).r / texture(refractiveIndex, (npos+vec3(1,1,1))*0.5).r;
		dir = refract(dir, normalize(pos-npos), refIndexRatio);
		// For a given incident vector I, surface normal N and ratio of indices of refraction, eta, refract returns the refraction vector, R.
		pos = npos;
    }
	
    color = radiance;
    
}
