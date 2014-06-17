#version 330

in vec2 passUV;

uniform sampler2D uniformPositionMap;
uniform sampler2D uniformNormalMap;
uniform sampler2D uniformColorMap;

uniform sampler2D uniformShadowMap;

uniform mat4 uniformProjectorView;
uniform mat4 uniformProjectorPerspective;

uniform mat4 uniformView;

uniform sampler1D uniformBitMask;

out vec4 fragmentLightFactor;

void main() {
	
	vec4 gbufferPosition = texture(uniformPositionMap, passUV);
	vec4 gbufferNormal = texture(uniformNormalMap, passUV);
	vec4 gbufferColor = texture(uniformColorMap, passUV);
	
	// retrieve Worldpositon point
	vec4 worldPos = inverse( uniformView ) * gbufferPosition;
	
	// project into shadow map
	vec4 LightPerspPos = uniformProjectorPerspective * uniformProjectorView * worldPos;
	
	LightPerspPos /= LightPerspPos.w;
	
	LightPerspPos.rgb *= 0.5;
	LightPerspPos.rgb += 0.5;
	
	vec2 shadowMapLookup   = LightPerspPos.xy ;
	
	// read pixel
	vec4 shadowMapVal = texture( uniformShadowMap, shadowMapLookup ) * 255.0;
	
	// actual depth of fragment in shadow map
	int depthBit = max( 0, min ( 32, int( LightPerspPos.z  * 32.0 + 2.0) ) );		
	
	// amount of slices which where set in direction to light source
	int fullSlices = 0;					
	
	// for every slice from fragment depth to light source
	for (int i = depthBit; i > 0; i-- )
	{
		// bitmask at this depth
		float currentDepth  = float(i) / 32.0;
		vec4 currentBitMask = texture(uniformBitMask, currentDepth) * 255.0;

		//determine r, g, b, or a channel to read	
		if ( i < 8)	// compare values of r channel
		{
			// if both bits are set
			if ( shadowMapVal.r / currentBitMask.r >= 1.0)
			{
				// write remainder
				shadowMapVal.r = shadowMapVal.r - currentBitMask.r;
				fullSlices ++;
			}
		}
		if ( i >= 8 && i < 16) // compare values of g channel
		{
			// if both bits are set
			if ( shadowMapVal.g / currentBitMask.g >= 1.0 )
			{
				shadowMapVal.g = shadowMapVal.g - currentBitMask.g;
				fullSlices ++;
			}
		}
		if ( i >= 16 && i < 24) // compare values of b channel
		{
			// if both bits are set
			if ( shadowMapVal.b / currentBitMask.b >= 1.0 )
			{
				shadowMapVal.b = shadowMapVal.b - currentBitMask.b;
				fullSlices ++;
			}
		}
		if ( i >= 24 && i < 32) // compare values of a channel
		{
			// if both bits are set
			if ( shadowMapVal.a / currentBitMask.a >= 1.0 )
			{
				shadowMapVal.a = shadowMapVal.a - currentBitMask.a;
				fullSlices++;
			}
		}
	}
	
	float opacityPerSlice = 1.0 / 16.0;
	
	fragmentLightFactor = vec4 ( vec3(1.0,1.0,1.0) * ( 1.0 - ( fullSlices * opacityPerSlice ) ), 1.0 );
}