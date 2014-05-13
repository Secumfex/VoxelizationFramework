#version 330 

// a fragment shader which sets the corresponding 
// pixel bit ( voxel ) in the slice map to 1

in float passDistanceToCam;

uniform sampler1D uniformBitMask;

// out : layout positions for multiple render targets
layout(location = 0) out vec4 slice0_31;

// TODO use multiple render targets
// layout(location = 1) out vec4 slice32_63;
// layout(location = 2) out vec4 slice64_93;

void main() { 
	// determine depth from distance to camera 
	float z = ( passDistanceToCam + 1.0 ) * 0.5;
	
	// bit mask lookup determines bit value
	vec4 bit_value = texture( uniformBitMask, z );
	slice0_31 = bit_value;
}