#version 330 core

// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 EyeDirection_worldspace;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler3D radianceDistribution;
uniform sampler3D grad_n;
uniform samplerCube CubeMap;
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
	float stepSize = 2.0/voxel_cnt;	// stepSize = voxel_width
	
	// pos: x_i, npos: x_{i+1}
    vec3 pos = Position_worldspace, npos;
	// v: v_i, nv: v_{i+1}
	// v0 = n*dir
	float n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a;
	vec3 dir = normalize(EyeDirection_worldspace);
	vec3 v = n*dir, nv;
	
	float refIndexRatio;
	float cnt = 0.00; // for debug
	
    for (int i = 0; i < voxel_cnt; i++) {
		// only focus on a cube
		if (abs(pos.x) >= 1 || abs(pos.y) >= 1 || abs(pos.z) >= 1) break;
		n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a;
		npos = pos + stepSize / n * v;
		nv = v + stepSize * texture(grad_n, (pos+vec3(1,1,1))*0.5).rgb;
		// sum up radiance
		if (n > 1)
			radiance += texture(radianceDistribution, (pos+vec3(1,1,1))*0.5).rgb;
		
		pos = npos;
		v = nv;
		
		cnt += 0.02;
    }
	
    //color = radiance;
	
	// direction * 10
	color = texture(CubeMap, normalize(v)*10.0f).rgb;
	//color = vec3(texture(grad_n, (pos+vec3(1,1,1))*0.5).a/2, 0, 0);
}
