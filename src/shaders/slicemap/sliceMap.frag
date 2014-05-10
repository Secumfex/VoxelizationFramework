#version 330 

// a fragment shader which gets a MVP matrix multiplied vertex Position 
// and sets the corresponding pixel ( voxel ) in the slice map texture to 1

in float passDistanceToCam;

uniform float uniformNearFarDistance; // Distance to Far Plane

// out : layout positions for fbos
layout(location = 0) out vec4 slice0_31;

// layout(location = 1) out vec4 slice32_63;
// layout(location = 2) out vec4 slice64_93;



void main() { 

	//TODO determine depth ( distance to camera, not z-value )
	float z = passDistanceToCam / uniformNearFarDistance;
	
	//TODO determine texture and bit ( uniform grid to begin with )
	int bit = int( z * 31.0 ); 				// bit index
	
	//TODO compute new texture value by using OR with old texture value
		//TODO how do I read the current framebuffer value
		vec4 old_value = slice0_31;	// read old value
		vec4 new_value = old_value; // init new value with 0
		
		//TODO how do I use OR with the old value? try -->     output = old_output | new_output 
		// min( v1+v2, 1 ) is equal to OR if v1, v2 are within 0.0 , 1.0
		if ( bit / 8 < 1 )	// value < 256 
		{
//			new_value.r = old_value.r | bit;
			
			float bitValue = pow (2 , bit);			// bit value to be written
//			new_value.r = min( ( (old_value.r ) + ( bitValue / 255.0f ) ), 1.0f ) * 255.0f;
		}
		else
		if ( bit < 16 && bit >= 8 ) // 256 <= value < 65536
		{
//			new_value.g = old_value.g | ( bit - 8 );
			
			float bitValue = pow (2 , bit - 8); 		// bit value to be written
//			new_value.g = min( ( (old_value.g ) + ( bitValue / 255.0f ) ), 1.0f ) * 255.0f;
		}
		else
		if ( bit < 24 && bit >= 16 ) // 65536 <= value < 16777216
		{
//			new_value.b = old_value.b | ( bit - 16);
			
			float bitValue = pow (2 , bit - 16); 		// bit value to be written
//			new_value.b = min( ( (old_value.b ) + ( bitValue / 255.0f ) ), 1.0f ) * 255.0f;
		}
		else
		if ( bit < 32 && bit >= 24 ) // 16777216 <= value < 4294967296
		{
//			new_value.a = old_value.a | (bit - 24);
			
			float bitValue = pow (2 , bit - 24); 		// bit value to be written
//			new_value.a = min( ( (old_value.a ) + ( bitValue / 255.0f ) ), 1.0f ) * 255.0f;
		}
		
		//TODO how do I write the new output value ?
		//slice0_31 = new_value;
		slice0_31 = vec4(gl_FragDepth * 255.0f,gl_FragDepth * 255.0f, gl_FragDepth * 255.0f,gl_FragDepth * 255.0f);
}