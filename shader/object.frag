#version 330 core
#define EPSILON 0.00001

// Interpolated values from the vertex shaders
in vec3 Position_worldspace;
in vec3 EyeDirection_worldspace;

// Ouput data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler3D radianceDistribution;
uniform sampler3D grad_n;
uniform samplerCube CubeMap;
uniform sampler2D table;
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


void main(){
    
    vec3 radiance = vec3(0, 0, 0);
	float stepSize = 2.0/voxel_cnt;	// stepSize = voxel_width
	// initial direction
	vec3 dir = normalize(EyeDirection_worldspace);
	// pos: x_i, npos: x_{i+1}
	vec3 pos = Position_worldspace + dir*stepSize*10;
	// v: v_i, nv: v_{i+1}
	// v0 = n*dir
	float n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a + 1.0f;
	vec3 v = n*dir;
	
	float cnt = 0.00; // for debug
	vec3 gradn;
	float gradnx, gradny, gradnz;
	
	bool isObj = false, isReflection = false;
	
    for (int i = 0; i < voxel_cnt*2; i++) {
		// only focus on a cube
		if (abs(pos.x) >= 1-EPSILON || abs(pos.y) >= 1-EPSILON || abs(pos.z) >= 1-EPSILON) // TODO: too much operation
			break;
		
		vec3 uv = (pos+vec3(1,1,1))*0.5;
		
		//if (n < 1 + EPSILON && i > voxel_cnt/3) break;
		n = texture(grad_n, uv).a + 1.0f;
		if (n > 1+EPSILON) isObj = true;
		pos += stepSize / n * v;
		v += texture(grad_n, uv).rgb;

		// sum up radiance
		if (isReflection) radiance += texture(radianceDistribution, uv).rgb*0.2;
		else radiance += texture(radianceDistribution, uv).rgb;
		
		if (pos.y <= -1+stepSize) {
			isReflection = true;
			v.y = -v.y;
			pos += stepSize / n * v;
		}
	}
	
	if (isObj) {
		if (isReflection) color = vec4(radiance + 0.2*texture(CubeMap, bgpos(pos, normalize(v))).rgb, 0.0f);
		else  color = vec4(radiance + texture(CubeMap, bgpos(pos, normalize(v))).rgb, 1.0f);
		
	}
	else {
		if (length(radiance) > 0.01) {
			color = vec4(radiance, 0);
		}
		else color = vec4(0,0,0,0);
	}
	
	/*
	bool isTable = false;
	vec3 p;
	if (dir.y < -EPSILON) {
		float steps = (-1-Position_worldspace.y) / dir.y;
		p = Position_worldspace + dir * steps;
		if (abs(p.x) < 1+EPSILON && abs(p.z) < 1+EPSILON)
			isTable = true;
	}

	if (isTable) {
		vec3 tablecolor = texture(table, (p.xz+vec2(1,1))*0.5).rgb;
		vec3 light = texture(radianceDistribution, (p+vec3(1,1,1))*0.5).rgb;
		color = radiance + vec3(tablecolor.r*light.r*100, tablecolor.g*light.g*100, tablecolor.b*light.b*100);
		//if (light.b > 0.5) color.b = 1.0f;
	}
	else
		color = radiance + texture(CubeMap, bgpos(pos, normalize(v))).rgb;
	
	*/
}
