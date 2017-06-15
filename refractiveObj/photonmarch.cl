#define EPSILON 0.00001
__constant sampler_t sampler =
CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;

inline void AtomicAdd(volatile __global float *source, const float operand) {
	union {
		unsigned int intVal;
		float floatVal;
	} newVal;
	union {
		unsigned int intVal;
		float floatVal;
	} prevVal;
	do {
		prevVal.floatVal = *source;
		newVal.floatVal = prevVal.floatVal + operand;
	} while (atomic_cmpxchg((volatile __global unsigned int *)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);
}

__kernel void photonmarch(__read_only image2d_t map,
						  int frame_width,
						  int frame_height,
						  __global float4* gradN,
						  int voxel_cnt,
						  float3 light_pos,
						  __global float* rx,
						  __global float* ry,
						  __global float* rz) {
	
	
	int i = get_global_id(0);
	
	float4 pos_inmap;
	pos_inmap = read_imagef(map, sampler, (int2)(i/frame_width, i%frame_width));
	
	if (pos_inmap.w > EPSILON) {
		
		// pos_worldspace = lightspace * reverse(light_mvp) * M
		float3 pos = pos_inmap.xyz * 10.0f - float3(2,2,2);
		
		float3 dir = normalize(pos-light_pos);
		float stepsize = 2.0f / voxel_cnt;
		
		pos += dir*stepsize*3;
		int x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
		float n = gradN[index].w + 1.0f;
		float3 v = n * dir, nv, npos;
		
		
		float3 sv[25], sg[25];
		int pass = 0, end = 0;

		rx[index] = 1.0f;
		for (int ii = 0; ii < voxel_cnt; ii++) {
			x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
			n = gradN[index].w + 1.0f;
			
			if (pos.x > 1-EPSILON || pos.y > 1-EPSILON || pos.z > 1-EPSILON ||
				pos.x < -1+EPSILON || pos.y < -1+EPSILON || pos.z < -1+EPSILON) {
				end = 1;
				break;
			}
			
			if (n < 1 + EPSILON) {
				if (pass == 1) {
					AtomicAdd(&rx[index], 0.0003);
					AtomicAdd(&ry[index], 0.0003);
					AtomicAdd(&rz[index], 0.0003);
				}
			}
			else if (n > 1.02){
				pass = 1;
			}
			
			
			npos = pos + stepsize / n * v;
			nv = v + gradN[index].xyz;
			pos = npos;
			v = nv;
			
			sv[ii] = v;
			sg[ii] = gradN[index].xyz;
		}
		if (end == 1) rz[index] = 1.0f;
		else ry[index] = 1.0f;
		
		/*
		if (i == 3120) {
			for (int j = 0; j < 25; j++) {
				printf("%d: + (%f, %f, %f) = (%f, %f, %f)\n", j, sg[j].x, sg[j].y, sg[j].z, sv[j].x, sv[j].y, sv[j].z);
			}
		}
		 */
	}
}

__kernel void radianceblur(__global float* rx,
				   __global float* ry,
				   __global float* rz,
				   __global float4* radiance,
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
		float4 sum = 0.0f;
		for(int a = -maskSize; a < maskSize+1; a++) {
			for(int b = -maskSize; b < maskSize+1; b++) {
				for(int c = -maskSize; c < maskSize+1; c++) {
					int index = i+a*voxel_cnt*voxel_cnt+b*voxel_cnt+c;
					sum += mask[(a+maskSize)*(maskSize*2+1)*(maskSize*2+1)+(b+maskSize)*(maskSize*2+1)+(c+maskSize)] * (float4)(rx[index], ry[index], rz[index], 0.0f);
				}
			}
		}
		radiance[i] = sum;
	}
	else
		radiance[i] = (float4)(rx[i], ry[i], rz[i], 0.0f);
		
}

