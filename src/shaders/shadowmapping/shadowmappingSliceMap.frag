#version 330

in vec2 passUV;

uniform sampler2D uniformPositionMap;
uniform sampler2D uniformNormalMap;
uniform sampler2D uniformColorMap;

uniform sampler2D uniformSliceMap;
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
	
	// offset to sample point
	vec4 samplePoint = gbufferPosition;
	samplePoint.xyz += 0.1 * gbufferNormal.xyz;
	
	// retrieve Worldpositon point
	vec4 worldPos = inverse( uniformView ) * ( samplePoint );
	
	// project into shadow map
	vec4 LightPerspPos = uniformProjectorPerspective * uniformProjectorView * worldPos;
	
	LightPerspPos /= LightPerspPos.w;
	
	LightPerspPos.rgb *= 0.5;
	LightPerspPos.rgb += 0.5;
	
	vec2 shadowMapLookup   = LightPerspPos.xy ;
	
	float lightIntensity = 1.0;
	
	// first look up trunk shadow map
	vec4 shadowMapVal = texture( uniformShadowMap, shadowMapLookup);
	LightPerspPos.z = max( 0.0, min( 1.0, LightPerspPos.z ) );
	if ( LightPerspPos.z > shadowMapVal.x + 0.03) // invisible
	{
			lightIntensity = 0.1;	
	}
	else // visible
	{
		// if visible, look up transmittance shadow map
		
		// read pixel
		shadowMapVal = texture( uniformSliceMap, shadowMapLookup ) * 255.0;
		
		// actual depth of fragment in shadow map
		int depthBit = max( 0, min ( 31, int( LightPerspPos.z  * 32.0) ) );		
		
		// amount of slices which where set in direction to light source
		int fullSlices = 0;					
		
		// for every slice from fragment depth to light source
		for (int i = depthBit; i >= 0; i-- )
		{
			// bitmask at this depth
			float currentDepth  = ( float(i)+ 0.5) / 32.0;
			vec4 currentBitMask = texture(uniformBitMask, currentDepth) * 255.0;
	
			//determine r, g, b, or a channel to read	
			if ( i < 8)	// compare values of r channel
			{
				// if both bits are set
				if ( shadowMapVal.r / currentBitMask.r >= 1.0)
				{
					// write remainder
					shadowMapVal.r = int(shadowMapVal.r) % int(currentBitMask.r);
					fullSlices ++;
				}
			}
			if ( i >= 8 && i < 16) // compare values of g channel
			{
				// if both bits are set
				if ( shadowMapVal.g / currentBitMask.g >= 1.0 )
				{
					shadowMapVal.g = int(shadowMapVal.g) % int(currentBitMask.g);
					fullSlices ++;
				}
			}
			if ( i >= 16 && i < 24) // compare values of b channel
			{
				// if both bits are set
				if ( shadowMapVal.b / currentBitMask.b >= 1.0 )
				{
					shadowMapVal.b = int(shadowMapVal.b) % int(currentBitMask.b);
					fullSlices ++;
				}
			}
			if ( i >= 24 && i < 32) // compare values of a channel
			{
				// if both bits are set
				if ( shadowMapVal.a / currentBitMask.a >= 1.0 )
				{
					shadowMapVal.a = int(shadowMapVal.a) % int(currentBitMask.a);
					fullSlices++;
				}
			}
		}
		
		float sigma = 0.3;
		
		lightIntensity = pow( 1.0 - sigma, float(fullSlices) ); 
	}
	
	fragmentLightFactor = vec4 ( vec3(1.0,1.0,1.0) * lightIntensity, 1.0 );
}