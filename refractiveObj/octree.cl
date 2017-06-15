#define TOLERANCE 0.001

__kernel void init(__global float4* gradn,
				   __global float2* temp) {
	
	int i = get_global_id(0);
	// min, max
	temp[i] = (float2)(gradn[i].w, gradn[i].w);
}

__kernel void construct(__global float2* temp,
						__global int* octree,
						int level,
						int explevel,
						int voxel_cnt) {
	int i = get_global_id(0);
	
	int x = i / (voxel_cnt * voxel_cnt);
	int y = (i % (voxel_cnt * voxel_cnt)) / voxel_cnt;
	int z = i % voxel_cnt;
	
	if (x % explevel == 0 && y % explevel == 0 && z % explevel == 0) {
		if (temp[i].y - temp[i].x > TOLERANCE) return;
		
		float minv = 999, maxv = 0;
		for (int a = 0; a < 2; a++)
			for (int b = 0; b < 2; b++)
				for (int c = 0; c < 2; c++) {
					int index = i + (a*explevel/2)*voxel_cnt*voxel_cnt + (b*explevel/2)*voxel_cnt + (c*explevel/2);
					if (temp[index].x < minv) minv = temp[index].x;
					if (temp[index].y > maxv) maxv = temp[index].y;
				}
		temp[i] = (float2)(minv, maxv);
		if (maxv - minv <= TOLERANCE) {
			for (int a = 0; a < explevel; a++)
				for (int b = 0; b < explevel; b++)
					for (int c = 0; c < explevel; c++) {
						int index = i + a*voxel_cnt*voxel_cnt+b*voxel_cnt+c;
						octree[index] = level;
					}
		}
	}
	
	
}
