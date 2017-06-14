#define EPSILON 0.000001
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
		
		float4 pos_worldspace; // lightspace * reverse(light_mvp) * M
		pos_worldspace.xyz = pos_inmap.xyz * 10.0f - float3(2,2,2);
		
		float3 dir = pos_worldspace.xyz - light_pos;
		float stepsize = 2.0/voxel_cnt;
		
		int x = pos_worldspace.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int y = pos_worldspace.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int z = pos_worldspace.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
		int index = x*voxel_cnt*voxel_cnt+y*voxel_cnt+z;
		
		for (int ii = 0; ii < voxel_cnt; ii++) {
			if (gradN[index].w > 1+EPSILON) {
				AtomicAdd(&rx[index], 0.001);
				AtomicAdd(&ry[index], 0.001);
				AtomicAdd(&rz[index], 0.001);
				pos_worldspace.xyz += normalize(dir)*stepsize;
				
				x = pos_worldspace.x * (voxel_cnt/2.0) + (voxel_cnt/2.0);
				y = pos_worldspace.y * (voxel_cnt/2.0) + (voxel_cnt/2.0);
				z = pos_worldspace.z * (voxel_cnt/2.0) + (voxel_cnt/2.0);
				index = x*voxel_cnt*voxel_cnt+y*voxel_cnt+z;
			}
		}
	}
}

