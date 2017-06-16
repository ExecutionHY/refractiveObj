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
						  __global float* rz,
						  __global int* octree) {
	
	
	int i = get_global_id(0);
	
	float4 pos_inmap;
	pos_inmap = read_imagef(map, sampler, (int2)(i/frame_width, i%frame_width));
	
	if (pos_inmap.w > EPSILON) {
		
		// pos_worldspace = lightspace * reverse(light_mvp) * M
		float3 pos = pos_inmap.xyz * 10.0f - float3(2,2,2), npos;
		
		float3 dir = normalize(pos-light_pos);
		float stepsize = 2.0f / voxel_cnt;
		
		pos += dir*stepsize*3;
		int x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z, lastindex = index;
		float n = gradN[index].w + 1.0f;
		float3 v = n * dir;
		
		
		//float3 sv[25], sg[25];
		int pass = 0, end = 0;

		//rx[index] = 1.0f;
		
		float rad = 0.02;
		
		
		for (int ii = 0; ii < voxel_cnt*1.2; ii++) {

			if (pos.x > 1-EPSILON || pos.y > 1-EPSILON || pos.z > 1-EPSILON ||
				pos.x < -1+EPSILON || pos.y < -1+EPSILON || pos.z < -1+EPSILON) {
				end = 1;
				break;
			}
			
			if (n < 1 + EPSILON) {
				if (pass == 1) {
					AtomicAdd(&rx[index], rad*0.002);// TODO: int faster?
					AtomicAdd(&ry[index], rad*0.002);
					AtomicAdd(&rz[index], rad*0.002);
					rad *= 0.998;
				}
			}
			else if (n > 1.05){
				pass = 1;
			}
			
			if (pass == 1) pos += stepsize / n * v;
			else
				pos += stepsize*(octree[index]/2+1) / n * v;
			
			x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
			
			if (pass == 1) v += gradN[index].xyz;
			else
				if (index != lastindex) v += gradN[index].xyz*(octree[index]/2+1);
			// else no change to v
			
			n = gradN[index].w + 1.0f;
			
			//sv[ii] = v;
			//sg[ii] = gradN[index].xyz;
		}
		if (pass == 0) {
			pos = pos_inmap.xyz * 10.0f - float3(2,2,2);
			if (dir.y < -EPSILON) {
				float steps = (-1-pos.y) / dir.y;
				pos += dir * steps;
				x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
				y = 0;
				z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
				if (x >= voxel_cnt) x = voxel_cnt-1;
				if (x < 0) x = 0;
				if (z >= voxel_cnt) z = voxel_cnt-1;
				if (z < 0) z = 0;
				index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
				
				AtomicAdd(&rx[index], rad);// TODO: int faster?
				AtomicAdd(&ry[index], rad);
				AtomicAdd(&rz[index], rad);
			}
		}
		else {
			/*
			x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			y = 0;
			z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			if (x >= voxel_cnt) x = voxel_cnt-1;
			if (x < 0) x = 0;
			if (z >= voxel_cnt) z = voxel_cnt-1;
			if (z < 0) z = 0;
			index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
			
			AtomicAdd(&rx[index], rad);// TODO: int faster?
			AtomicAdd(&ry[index], rad);
			AtomicAdd(&rz[index], rad);
			 */
		}
		//else ry[index] = 1.0f;
		
		/*
		if (i == 3120) {
			for (int j = 0; j < 25; j++) {
				printf("%d: + (%f, %f, %f) = (%f, %f, %f)\n", j, sg[j].x, sg[j].y, sg[j].z, sv[j].x, sv[j].y, sv[j].z);
			}
		}
		 */
	}
}

__kernel void tableradiance(__global float* rx,
							__global float* ry,
							__global float* rz,
							__global float4* table,
							int voxel_cnt,
							int table_width,
							__global float* mask,
							int maskSize
							) {
	int i = get_global_id(0);
	int x = i / table_width;
	int z = i % table_width;
	int px = (float)(x) / (float)(table_width) * voxel_cnt;
	int pz = (float)(z) / (float)(table_width) * voxel_cnt;
	int py = 0;
	int index = px*voxel_cnt*voxel_cnt + py*voxel_cnt + pz;
	
	float4 sum;
	for(int a = -maskSize; a < maskSize+1; a++) {
		for(int b = -maskSize; b < maskSize+1; b++) {
			int idx = index+a*voxel_cnt*voxel_cnt+b;
			sum += mask[(a+maskSize)*(maskSize*2+1)+(b+maskSize)] * (float4)(rx[idx], ry[idx], rz[idx], 0.0f);
		}
	}
	
	table[i] = sum;
	
}

__kernel void radianceblur(__global float* rx,
						   __global float* ry,
						   __global float* rz,
						   __global float4* radiance,
						   __global float* mask,
						   int maskSize,
						   __global float* mask2,
						   int maskSize2,
						   int voxel_cnt
						   ) {
	
	int i = get_global_id(0);
	
	int x = i / (voxel_cnt * voxel_cnt);
	int y = (i % (voxel_cnt * voxel_cnt)) / voxel_cnt;
	int z = i % voxel_cnt;
	
	
	if (maskSize < x && x < voxel_cnt - maskSize &&
		maskSize < y && y < voxel_cnt - maskSize &&
		maskSize < z && z < voxel_cnt - maskSize) {
		float4 sum;
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
	else {
		/*
		if (y == 0) {
			float4 sum;
			for(int a = -maskSize2; a < maskSize2+1; a++) {
				for(int b = -maskSize2; b < maskSize2+1; b++) {
					int index = i+a*voxel_cnt+b;
					sum += mask2[(a+maskSize2)*(maskSize2*2+1)+(b+maskSize2)] * (float4)(rx[index], ry[index], rz[index], 0.0f);
				}
			}
			radiance[i] = sum*10;
		}
		 */
		if (y > 0) radiance[i] = (float4)(rx[i], ry[i], rz[i], 0.0f);
	}
		
}

