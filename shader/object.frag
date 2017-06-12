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

void main(){
    
    vec3 radiance = vec3(0, 0, 0);
	float stepSize = 2.0/voxel_cnt;	// stepSize = voxel_width
	// initial direction
	vec3 dir = normalize(EyeDirection_worldspace);
	// pos: x_i, npos: x_{i+1}
	vec3 pos = Position_worldspace - dir*stepSize*3, npos;
	// v: v_i, nv: v_{i+1}
	// v0 = n*dir
	float n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a;
	vec3 v = n*dir, nv;
	
	float cnt = 0.00; // for debug
	
    for (int i = 0; i < voxel_cnt; i++) {
		// only focus on a cube
		//if (abs(pos.x) >= 1 || abs(pos.y) >= 1 || abs(pos.z) >= 1) break;
		n = texture(grad_n, (pos+vec3(1,1,1))*0.5).a;
		npos = pos + stepSize / n * v;
		nv = v + stepSize * texture(grad_n, (pos+vec3(1,1,1))*0.5).rgb;
		// sum up radiance
		radiance += texture(radianceDistribution, (pos+vec3(1,1,1))*0.5).rgb;
		
		pos = npos;
		v = nv;
	}
	
	// direction * 10
	color = radiance + texture(CubeMap, pos+normalize(v)*10.0f).rgb;
	
	if (color.r > 0.99) color.r = 0.99;
	if (color.g > 0.99) color.g = 0.99;
	if (color.b > 0.99) color.b = 0.99;
}
