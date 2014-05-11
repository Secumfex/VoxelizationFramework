#version 330 

// a fragment shader which sets the corresponding 
// pixel bit ( voxel ) in the slice map texture to 1

in float passDistanceToCam;

uniform float uniformZNear; // near plane
uniform float uniformZFar;  // far plane

// out : layout positions for multiple render targets
layout(location = 0) out vec4 slice0_31;

// layout(location = 1) out vec4 slice32_63;
// layout(location = 2) out vec4 slice64_93;



void main() { 

	// determine depth from distance to camera 
	// ( is distance to cam already in 0..1 ??? )
	// float z = ( ( passDistanceToCam + uniformZNear ) / ( uniformZNear + uniformZFar ) ) * 10.0f;
	float z = passDistanceToCam;
	
	// determine texture and bit ( uniform grid to begin with )
	int bit = int( z * 31.0f ); 				// bit index
	
	// compute bit to be set
	vec4 new_value = vec4(0.0f); // init to be insertedvalue with 0
		
	if ( bit / 8 < 1 )	// value < 256 
	{
		float bitValue = pow (2 , bit);			// bit value to be written
		new_value.r = bitValue;
	}
	else
	if ( bit < 16 && bit >= 8 ) // 256 <= value < 65536
	{
		float bitValue = pow (2 , bit - 8); 		// bit value to be written
		new_value.g = bitValue;
	}
	else
	if ( bit < 24 && bit >= 16 ) // 65536 <= value < 16777216
	{
		float bitValue = pow (2 , bit - 16); 		// bit value to be written
		new_value.b = bitValue;
	}
	else
	if ( bit < 32 && bit >= 24 ) // 16777216 <= value < 4294967296
	{
		float bitValue = pow (2 , bit - 24); 		// bit value to be written
		new_value.a = bitValue;
	}
		
	slice0_31 = new_value;

	slice0_31 = vec4(z,z,z,1.0f);
}