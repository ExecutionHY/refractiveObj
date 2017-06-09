__constant sampler_t sampler =
CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;

__kernel void photonmarch(__read_only image2d_t input,
						  __global float4* output) {
	
	
	
	int i = get_global_id(0);
	
	output[i] = read_imagef(input, sampler, (int2)(i/1024, i%1024));
}

