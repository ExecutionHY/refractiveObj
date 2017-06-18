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
						  __global int* octree,
						  float3 pos1
						  ) {
	
	
	int i = get_global_id(0);
	
	float4 pos_inmap;
	pos_inmap = read_imagef(map, sampler, (int2)(i/frame_width, i%frame_width));
	
	if (pos_inmap.w > EPSILON) {
		
		// pos_worldspace = lightspace * reverse(light_mvp) * M
		float3 pos = pos_inmap.xyz * 10.0f - float3(2,2,2);
		//float3 pos1 = (float3)(-0.3f, 0.3, 0.0f);
		float3 stddir = (normalize)(pos1-light_pos);
		
		float3 dir = (normalize)(pos-light_pos);
		float stepsize = 2.0f / voxel_cnt;
		
		pos += dir*stepsize*3;
		int x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
		float n = gradN[index].w + 1.0f;
		float3 v = n * dir;
		
		
		//float3 sv[25], sg[25];
		__private int pass = 0, end = 0, out = 1;

		//rx[index] = 1.0f;
		
		float rad = 0.5;
		
		
		for (int ii = 0; ii < voxel_cnt*1.2; ii++) {

			if (pos.x > 1-EPSILON || pos.y > 1-EPSILON || pos.z > 1-EPSILON ||
				pos.x < -1+EPSILON || pos.y < -1+EPSILON || pos.z < -1+EPSILON) {
				end = 1;
				break;
			}
			
			if (n < 1 + EPSILON) {
				/*if (pass == 0) {
					rx[index] = 0.01f;
					ry[index] = 0.01f;
					rz[index] = 0.01f;
				}
				else*/
				if (pass == 1) {
					rx[index] = 0.05f;
					ry[index] = 0.05f;
					rz[index] = 0.05f;
					//AtomicAdd(&rx[index], rad*0.001);// TODO: int faster?
					//AtomicAdd(&ry[index], rad*0.001);
					//AtomicAdd(&rz[index], rad*0.001);
					//rad *= 0.99f;
					if (y < 2) {
						rx[index] = 1.0f;
						ry[index] = 1.0f;
						rz[index] = 1.0f;
					}
				}
				else if (pass == 2) {
					rx[index] = 0.06f;
					ry[index] = 0.02f;
					rz[index] = 0.02f;
					
					if (y < 2) {
						rx[index] = 1.0f;
						ry[index] = 0.4f;
						rz[index] = 0.4f;
					}
				}
				out = 1;
			}
			else if (n > 1.03) {
				if (pos.x < 0) pass = 1;
				else {
					pass = 2;

				}
				out = 0;
			}
			
			//if (pass > 0)
				pos += stepsize / n * v;
			//else pos += stepsize*(octree[index]/2+1) / n * v;
			//if (pass > 0)
				v += gradN[index].xyz;
			//else v += gradN[index].xyz*(octree[index]/2+1);
			
			x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
			n = gradN[index].w + 1.0f;
			
			
			

			if (pass == 1 && out == 1) {
				if (dot(v, stddir) < 0.9) {
					pass = -1;
					break;
				}
				float3 odir = v - stddir*dot(v, stddir);
				float len = length(odir);
				float3 npos1 = pos1 + stddir*(pos.y-pos1.y)/stddir.y;
				float dist = distance(pos, npos1);
				if (dist < 0.01) continue;
				float alpha = stepsize*len/dist*(1-5*dist);
				v = (1.0f-alpha)*v + (alpha)*stddir;
				
			}
			else if (pass == 2 && out == 1) {
				if (length(v.xz) > 0.5) {
					pass = -1;
					break;
				}
				float3 odir = pos-(float3)(0.5f, pos.y, 0.0f);
				float dist = length(odir);
				if (dist < 0.01) continue;
				float alpha = stepsize*length(v.xz)/dist*(1-2*dist);
				v = (1.0f-alpha)*v + (alpha)*float3(0.0f, -1.0f, 0.0f);
				
			}
			
			
			//if (i == 686346) rx[index] = 1.0f;
			
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
		else if (pass == 1){
			
			x = pos.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			y = pos.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			z = pos.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
			
			y = 0;
			//if (y > 0) return;
			  
			//if (x >= voxel_cnt) x = voxel_cnt-1;
			//if (x < 0) x = 0;
			//if (z >= voxel_cnt) z = voxel_cnt-1;
			//if (z < 0) z = 0;
			index = x*voxel_cnt*voxel_cnt + y*voxel_cnt + z;
			
			//AtomicAdd(&rx[index], rad);// TODO: int faster?
			//AtomicAdd(&ry[index], rad);
			//AtomicAdd(&rz[index], rad);
			
		}
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
	
	float4 sum = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	for(int a = -maskSize; a < maskSize+1; a++) {
		for(int b = -maskSize; b < maskSize+1; b++) {
			int idx = index+a*voxel_cnt*voxel_cnt+b;
			sum += mask[(a+maskSize)*(maskSize*2+1)+(b+maskSize)] * (float4)(rx[idx], ry[idx], rz[idx], 0.0f);
			//if (i == 704576) printf("(%f, %f, %f)\n", rx[idx], ry[idx], rz[idx]);
		}
	}
	
	table[i] = sum;
	if (length(sum) < 0.1f) {
		index += voxel_cnt;
		float4 sum = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
		for(int a = -maskSize; a < maskSize+1; a++) {
			for(int b = -maskSize; b < maskSize+1; b++) {
				int idx = index+a*voxel_cnt*voxel_cnt+b;
				sum += mask[(a+maskSize)*(maskSize*2+1)+(b+maskSize)] * (float4)(rx[idx], ry[idx], rz[idx], 0.0f);
				//if (i == 704576) printf("(%f, %f, %f)\n", rx[idx], ry[idx], rz[idx]);
			}
		}
		table[i] = sum;
	}
	
	
	if (table[i].x > 1.0f) table[i].x = 1.0f;
	if (table[i].y > 1.0f) table[i].y = 1.0f;
	if (table[i].z > 1.0f) table[i].z = 1.0f;
	
	//if (ry[i] > 0.5 && rx[i] < 0.3 && rz[i] < 0.3) printf("%d %d %d (%f, %f, %f) (%f, %f, %f)\n", i, px, pz, rx[i], ry[i], rz[i], table[i].x, table[i].y, table[i].z);
	//if (px == 86 && pz == 7) printf("(%f, %f, %f) ", table[i].x, table[i].y, table[i].z);
	
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
		
		if (y > 0) radiance[i] = (float4)(rx[i], ry[i], rz[i], 0.0f);
		/*
		else if (y == 1) {
			float4 sum;
			for(int a = -maskSize2; a < maskSize2+1; a++) {
				for(int b = -maskSize2; b < maskSize2+1; b++) {
					int index = i+a*voxel_cnt+b;
					sum += mask2[(a+maskSize2)*(maskSize2*2+1)+(b+maskSize2)] * (float4)(rx[index], ry[index], rz[index], 0.0f);
				}
			}
			radiance[i] = sum;
		}*/
		else radiance[i] = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	}
	
	if (radiance[i].x > 1.0f) radiance[i].x = 1.0f;
	if (radiance[i].y > 1.0f) radiance[i].y = 1.0f;
	if (radiance[i].z > 1.0f) radiance[i].z = 1.0f;
	 
}

