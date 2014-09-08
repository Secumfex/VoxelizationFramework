#version 430 core

in vec2 passUV;

// rsm
uniform sampler2D uniformRSMFluxMap;
uniform sampler2D uniformRSMDepthMap;

uniform mat4 uniformRSMView;
uniform mat4 uniformRSMProjection;

// gbuffer
uniform sampler2D uniformGBufferPositionMap;

uniform mat4 uniformGBufferView;

// settings
uniform bool uniformOrthoLightSource;

// output
layout(location = 0) out vec4 directLight;

/****************** ************ ****************/
/****************** DIRECT LIGHT ****************/
/****************** ************ ****************/

// compute light intensity to position based on it's visibility to light source
vec4 computeDirectLight( vec3 rsmPosition )
{
	// default : dark
	vec4 directLightIntensity = vec4( 0.0f, 0.0f, 0.0f, 0.0f );
		
	// inside of light view
	if ( rsmPosition.x < 1.0 && rsmPosition.y < 1.0 && rsmPosition.x > 0.0 && rsmPosition.y > 0.0)
	{
		float rsmDepth = texture( uniformRSMDepthMap, rsmPosition.xy ).x;
		directLightIntensity = texture( uniformRSMFluxMap , rsmPosition.xy);
		
		float epsilon = 0.0005;
		if ( uniformOrthoLightSource )
		{
			epsilon = 0.01;
		}
		
		// dark if invisible
		if ( rsmPosition.z > rsmDepth + epsilon )
		{
			directLightIntensity = vec4 ( 0.0f, 0.0f, 0.0f, 0.0f );	
		}		
	}
	
	return directLightIntensity;
}

/****************** ***************** ****************/
/******************        MAIN       ****************/
/****************** ***************** ****************/
void main()
{
	// retrieve position from GBuffer
	vec4 worldPosition   = inverse( uniformGBufferView ) * texture( uniformGBufferPositionMap, passUV );
	
	// project into reflective shadow map
	vec4 rsmPosition = uniformRSMProjection * uniformRSMView * worldPosition; // -w..w
	rsmPosition.xyz /= rsmPosition.w; //-1..1
	rsmPosition.w = 1.0;
	rsmPosition.xyz += 1.0;	// 0..2
	rsmPosition.xyz *= 0.5; // 0..1
	
	// compute direct light intensity
	directLight = computeDirectLight( rsmPosition.xyz );
}