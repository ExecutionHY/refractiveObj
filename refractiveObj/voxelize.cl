#define EPSILON 0.001
#define PI 3.14159

inline bool intersectRayTriangle(float3 orig,
						  float3 dir,
						  float3 v0,
						  float3 v1,
						  float3 v2) {
	
	float3 normal = cross(v1-v0, v2-v0);
	float distance = dot(v0-orig, normal)/dot(normal, dir);
	if (distance < 0.005) return false;
	float3 p = orig + dir*distance;
	float3 pv0 = normalize(v0 - p);
	float3 pv1 = normalize(v1 - p);
	float3 pv2 = normalize(v2 - p);
	float angle1 = acos(dot(pv0, pv1));
	float angle2 = acos(dot(pv1, pv2));
	float angle3 = acos(dot(pv2, pv0));
	if (angle1 + angle2 + angle3 >= 2*PI - EPSILON) return true;
	return false;
}
/*
inline static bool intersectRayTriangle(float3 orig,
						  float3 dir,
						  float3 v0,
						  float3 v1,
						  float3 v2) {
	float3 e1 = v1 - v0;
	float3 e2 = v2 - v0;
	float3 p = cross(dir, e2);
	
	float a = dot(e1, p);
	if (-EPSILON < a && a < EPSILON) return false;
	float f = 1.0 / a;
	
	float3 s = orig - v0;
	float x = dot(s, p) * f;
	if (x < 0 || x > 1) return false;
	
	float3 q = cross(s, e1);
	float y = dot(dir, q) * f;
	if (y < 0 || (x + y > 1)) return false;
	
	float z = dot(e2, q) * f;
	
	return z >= 0;
	
}
*/
inline float getValue(int x, int y, int z, __global float* refIndex, int voxel_cnt) {
	if (x < 0 || x >= voxel_cnt) return -1.0f;
	if (y < 0 || y >= voxel_cnt) return -1.0f;
	if (z < 0 || z >= voxel_cnt) return -1.0f;
	int index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
	return refIndex[index];
}

// Check 3x3x3 neighborhood. Return TRUE for any difference.
inline bool isBorder(int x, int y, int z, __global float* refIndex, int voxel_cnt) {
	float r, thisr = refIndex[x*voxel_cnt*voxel_cnt + y*voxel_cnt + z];

	for (int i = x-1; i <= x+1; i++) {
		for (int j = y-1; j <= y+1; j++) {
			for (int k = z-1; k <= z+1; k++) {
				r = getValue(i, j, k, refIndex, voxel_cnt);
				if (r > 0 && fabs(r - thisr) > EPSILON) return true;
			}
		}
	}
	
	return false;
}

// part1 - get raw refractive index
__kernel void voxelize(__global ushort* indices,
					   __global float3* indexed_vertices,
					   __global float* refIndex,
					   int index_cnt,
					   int voxel_cnt,
					   float refConst,
					   float3 pos1
					   ) {
	
	
	int i = get_global_id(0);
	
	int x = i / (voxel_cnt * voxel_cnt);
	int y = (i % (voxel_cnt * voxel_cnt)) / voxel_cnt;
	int z = i % voxel_cnt;
	float halfv = float(voxel_cnt)/2.0f;
	float3 pos = (float3)(((float)(x)-halfv)/halfv, ((float)(y)-halfv)/halfv, ((float)(z)-halfv)/halfv);
	/*
	int intersectCnt = 0;
	for (int i = 0; i < index_cnt; i += 3) {
		if (intersectRayTriangle(pos, (float3)(1.0f,0.0f,0.0f), indexed_vertices[indices[i]], indexed_vertices[indices[i+1]], indexed_vertices[indices[i+2]]))
			intersectCnt++;
	}
	if (intersectCnt % 2 == 1) refIndex[i] = refConst;
	else refIndex[i] = 1.0f;
	*/
	//float3 pos1 = (float3)(-0.3, 0.3, 0);
	float3 pos2 = (float3)(0.5, -0.3, 0);
	
	if (distance(pos, pos1) < 0.4f || distance(pos, pos2) < 0.2f) refIndex[i] = refConst;
	else refIndex[i] = 1.0f;
	
	// part2 - super-sample those border voxels
	bool isborder = isBorder(x, y, z, refIndex, voxel_cnt);
	if (isborder) {
		// super sample
		refIndex[i] = 0.0f;
		float voxel_width = 2.0f / voxel_cnt;
		
		for (float a = -0.375; a <= 0.375; a += 0.25) {
			for (float b = -0.375; b <= 0.375; b += 0.25) {
				for (float c = -0.375; c <= 0.375; c += 0.25) {
					float3 newPos = pos + float3(a, b, c)*voxel_width;
					if (distance(newPos, pos1) < 0.4f || distance(newPos, pos2) < 0.2f) refIndex[i] += refConst/64.0f;
					else refIndex[i] += 1.0f/64.0f;
					/*
					int intersectCnt = 0;
					for (int i = 0; i < index_cnt; i += 3) {
						if (intersectRayTriangle(newPos, float3(1,0,0), indexed_vertices[indices[i]], indexed_vertices[indices[i+1]], indexed_vertices[indices[i+2]]))
							intersectCnt++;
					}
					if (intersectCnt % 2 == 1) refIndex[i] += refConst/64;
					else refIndex[i] += 1.0f/64;
					 */
				}
			}
		}
	}

}


// part3 - blur the values

// Gaussian filter
__kernel void blur(__global float* raw,
				   __global	float* blured,
				   __global float* mask,
				   int maskSize,
				   int voxel_cnt
				   ) {

	
	
	int i = get_global_id(0);
	
	int x = i / (voxel_cnt * voxel_cnt);
	int y = (i % (voxel_cnt * voxel_cnt)) / voxel_cnt;
	int z = i % voxel_cnt;
	
	
	if (maskSize < x && x < voxel_cnt - maskSize &&
		maskSize < y && y < voxel_cnt - maskSize &&
		maskSize < z && z < voxel_cnt - maskSize) {
		float sum = 0.0f;
		for(int a = -maskSize; a < maskSize+1; a++) {
			for(int b = -maskSize; b < maskSize+1; b++) {
				for(int c = -maskSize; c < maskSize+1; c++) {
					sum += mask[(a+maskSize)*(maskSize*2+1)*(maskSize*2+1)+(b+maskSize)*(maskSize*2+1)+(c+maskSize)] * raw[i+a*voxel_cnt*voxel_cnt+b*voxel_cnt+c];
				}
			}
		}
		blured[i] = sum - 1.0f;
	}
	else
		blured[i] = raw[i] - 1.0f;
	
	
	
	
}

// part4 - compute gradiants
__kernel void gradient(__global float* refIndex,
					   __global float4* gradn,
					   int voxel_cnt
					   ) {
	
	int i = get_global_id(0);
	
	int x = i / (voxel_cnt * voxel_cnt);
	int y = (i % (voxel_cnt * voxel_cnt)) / voxel_cnt;
	int z = i % voxel_cnt;
	
	if (x == voxel_cnt-1 || y == voxel_cnt-1 || z == voxel_cnt-1 ||
		x == 0 || y == 0 || z == 0) {
		gradn[i] = (float4)(0.0f, 0.0f, 0.0f, refIndex[i]);
	}
	else {
		gradn[i] = (float4)((refIndex[i+voxel_cnt*voxel_cnt] - refIndex[i-voxel_cnt*voxel_cnt])/2.0f,
							(refIndex[i+voxel_cnt] - refIndex[i-voxel_cnt])/2.0f,
							(refIndex[i+1] - refIndex[i-1])/2.0f,
							refIndex[i]);
	}
}

