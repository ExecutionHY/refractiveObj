#version 330 core
#define EPSILON 0.001

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

vec3 bgpos(vec3 pos, vec3 dir) {
	float step1, minstep;
	if (dir.x > EPSILON) minstep = (10-pos.x) / dir.x;
	else if (dir.x < -EPSILON) minstep = (-10-pos.x) / dir.x;
	if (dir.y > EPSILON) step1 = (10-pos.y) / dir.y;
	else if (dir.y < -EPSILON) step1 = (-10-pos.y) / dir.y;
	if (step1 < minstep) minstep = step1;
	if (dir.z > EPSILON) step1 = (10-pos.z) / dir.z;
	else if (dir.z < -EPSILON) step1 = (-10-pos.z) / dir.z;
	if (step1 < minstep) minstep = step1;
	return pos+dir*minstep;
}

float getn(vec3 pos) {
	float n;
	if (length(pos) < 0.8) n = 1.5f;
	else if (0.8-EPSILON <= length(pos) && length(pos) <= 0.8) n = 1.34f;
	else if (0.8 <= length(pos) && length(pos) <= 0.8+EPSILON) n = 1.17f;
	else n = 1.0f;
	return n;
}

void main(){
    
    vec3 radiance = vec3(0, 0, 0);
	float stepSize = 2.0/voxel_cnt;	// stepSize = voxel_width
	// initial direction
	vec3 dir = normalize(EyeDirection_worldspace);
	// pos: x_i, npos: x_{i+1}
	vec3 pos = Position_worldspace - dir*stepSize*3, npos;
	// v: v_i, nv: v_{i+1}
	// v0 = n*dir
	float n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a + 1.0f;
	vec3 v = n*dir, nv;
	
	float cnt = 0.00; // for debug
	vec3 gradn;
	float gradnx, gradny, gradnz;
	
    for (int i = 0; i < voxel_cnt+3; i++) {
		// only focus on a cube
		//if (abs(pos.x) >= 1 || abs(pos.y) >= 1 || abs(pos.z) >= 1) break;
		//if (n < 1 + EPSILON && i > voxel_cnt/3) break;
		n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a + 1.0f;
		npos = pos + stepSize / n * v;
		gradn = texture(grad_n, (pos+vec3(1,1,1))*0.5).rgb;
		nv = v + gradn;
		// sum up radiance
		//radiance += texture(radianceDistribution, (pos+vec3(1,1,1))*0.5).rgb;
		
		pos = npos;
		v = nv;
		dir = normalize(v);
	}
	
	// direction * 10
	color = texture(CubeMap, bgpos(pos, normalize(v))).rgb;
	
}
