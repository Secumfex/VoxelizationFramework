#version 330 

// a fragment shader which sets the corresponding 
// pixel bit voxel in the slice map to 1

in float passDistanceToCam;

uniform sampler1D uniformBitMask;

uniform int uniformNumSliceMaps;	// should still be integer values

// out : layout positions for multiple render targets
layout(location = 0) out vec4 slice0_31;

// use multiple render targets
layout(location = 1) out vec4 slice32_63;
layout(location = 2) out vec4 slice64_93;
 
void main() { 
	// determine depth from distance to camera 
	float z = ( passDistanceToCam + 1.0 ) * 0.5;
	
	int sliceMapTarget = uniformNumSliceMaps * int ( z );	// slice map to write into
	z = ( z * float( uniformNumSliceMaps ) ) - float( sliceMapTarget); // map from z to [0..1] in slice map target
	
	// bit mask lookup determines bit value
	vec4 bit_value = texture( uniformBitMask, z );
	
	if (sliceMapTarget == 0)
	{
		slice0_31 = bit_value;	
	}
	if (sliceMapTarget == 1)
	{
		slice32_63 = bit_value;
	}
	if (sliceMapTarget == 2)
	{
		slice64_93 = bit_value;
	}
}