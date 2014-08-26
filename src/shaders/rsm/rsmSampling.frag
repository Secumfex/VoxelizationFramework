#version 430 core

in vec2 passUV;

uniform sampler2D uniformRSMPositionMap;
uniform sampler2D uniformRSMNormalMap;
uniform sampler2D uniformRSMFluxMap;
uniform sampler2D uniformRSMDepthMap;

uniform mat4 uniformRSMView;
uniform mat4 uniformRSMProjection;

uniform sampler2D uniformGBufferPositionMap;
uniform sampler2D uniformGBufferNormalMap;

uniform mat4 uniformGBufferView;

// TODO use texture 1D ?
uniform sampler1D uniformSamplingPattern;
uniform int uniformNumSamples;

layout(location = 0) out vec4 directLight;
layout(location = 1) out vec4 indirectLight;

void main()
{
	// retrieve position from GBuffer
	vec4 worldPosition   = inverse( uniformGBufferView ) * texture( uniformGBufferPositionMap, passUV );
	vec4 worldNormal   = transpose( uniformGBufferView ) * texture( uniformGBufferNormalMap,   passUV );
	
	// project into reflective shadow map
	vec4 rsmPosition = uniformRSMProjection * uniformRSMView * worldPosition; // -w..w
	rsmPosition.xyz /= rsmPosition.w; //-1..1
	rsmPosition.xyz += 1.0;	// 0..2
	rsmPosition.xyz *= 0.5; // 0..1
	
	// perform light gathering
	vec2 center = vec2( rsmPosition.x, rsmPosition.y );

	vec3 surfacePosition = worldPosition.xyz;
	vec3 surfaceNormal   = worldNormal.xyz;
	
	vec3 irradiance = vec3( 0.0, 0.0, 0.0 );
	for ( int i = 0; i < uniformNumSamples; i++ )
	{
		// retrieve sampling properties for next sample
		vec4 samplingProperties= texture( uniformSamplingPattern, i );
		
		vec2 sampleUV		   = samplingProperties.xy;
		float sampleWeight     = samplingProperties.z;

		// retrieve sample point light information
		vec3 rsmSamplePosition = texture( uniformRSMPositionMap, sampleUV ).xyz;
		vec3 rsmSampleNormal   = texture( uniformRSMNormalMap  , sampleUV ).xyz;
		vec4 rsmSampleFlux     = texture( uniformRSMFluxMap    , sampleUV );
		
		// help vectors
		vec3 sampleToSurface = surfacePosition - rsmSamplePosition;
		vec3 surfaceToSample = (-1.0) * sampleToSurface;
		
		// compute irradiance at surface point due to sample point light
		vec3 sampleIrradiance = 
				rsmSampleFlux.xyz  // sample point light color
			  * rsmSampleFlux.w    // sample point light flux
			  *	( max ( 0, dot ( rsmSampleNormal, sampleToSurface ) ) ) // radiant intensity from sample point light to surface 
			  * ( max ( 0, dot ( surfaceNormal,   surfaceToSample ) ) ) // radiant intensity from surface to sample point light
			  / pow( length( sampleToSurface ), 4);	// distance to the power of 4
		
		// apply weight
		sampleIrradiance *= sampleWeight;
		
		// add to total irradiance
		irradiance += sampleIrradiance;
	}
	// normalize
	irradiance /= float( uniformNumSamples );
	
	// save indirect light intensity
	indirectLight = vec4( irradiance, 0 );
	
	// test for occlusion
	float directLightIntensity = 1 - rsmPosition.z;
	float rsmDepth = texture( uniformRSMDepthMap, rsmPosition.xy ).x;
	
	if ( rsmPosition.z > rsmDepth + 0.02 )
	{
		// some silly values
		directLightIntensity = 0.1 + ( directLightIntensity / 10.0 );	
	}
	
	// save direct light intensity
	directLight = vec4 ( directLightIntensity, directLightIntensity, directLightIntensity, 1 );
}