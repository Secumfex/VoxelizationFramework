#version 430 core 

// a fragment shader which sets the corresponding 
// pixel bit ( voxel ) in the slice map to 1

in float passDistanceToCam;

uniform usampler1D uniformBitMask;

// out : layout positions for multiple render targets
layout(location = 0) out uvec4 slice0_31;

void main() { 
	// determine depth from distance to camera 
	float z = 1.0 - passDistanceToCam;
	
	// bit mask lookup determines bit value
	uvec4 bit_value = texture( uniformBitMask, z );
	
	slice0_31 = bit_value;
}