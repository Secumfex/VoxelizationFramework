#version 430 core

in vec2 passUV;

// low res rsm indirect light map to sample from
uniform sampler2D uniformRSMLowResIndirectLightMap;
uniform sampler2D uniformRSMLowResPositionMap;
uniform sampler2D uniformRSMLowResNormalMap;

// low res light map resolution
uniform float uniformRSMLowResX;
uniform float uniformRSMLowResY;

// gbuffer maps to read from
uniform sampler2D uniformGBufferPositionMap;
uniform sampler2D uniformGBufferNormalMap;

// thresholds to test against
uniform float uniformNormalThreshold;
uniform float uniformDistanceThreshold;

// output
layout(location = 0) out vec4 indirectLight;

/****************** ***************** ****************/
/******************        MAIN       ****************/
/****************** ***************** ****************/
void main()
{
	// retrieve position and normal of pixel from gbuffer
	vec4 pixelPosition = inverse( uniformGBufferView ) * texture( uniformGBufferPositionMap, passUV );
	vec4 pixelNormal   = inverse( uniformGBufferView ) * texture( uniformGBufferNormalMap,   passUV );

	// retrieve nearest samples
	vec2 resolution = vec2( uniformRSMLowResX, uniformRSMLowResY );
	vec2 texelSize = 1.0 / resolution; // pixel size in tex coords
	
	vec2 centerUV = ( floor( passUV * resolution ) + vec2( 0.5, 0.5 ) ) / resolution );
	vec2 offsetCENTER   = centerUV - ( 0.5 * texelSize ); // in UV coord system
	vec2 offsetDIAGONAL = sign( offsetCenter ) * texelSize;  // sample that is diagonal to center
	if ( offsetDIAGONAL.x == 0.0 )
	{
		offsetDIAGONAL.x  = texelSize.x;	
	}
	if ( offsetDIAGONAL.x == 0.0 )
	{
		offsetDIAGONAL.y  = texelSize.y;	
	}
	vec2 offsetHORIZONTAL= vec2( offsetDIAGONAL.x, 0.0 );  // sample that is left or right
	vec2 offsetVERTICAL  = vec2( 0.0, offsetDIAGONAL.y );  // sample that is up or down from center
	
	// retrieve position and normal of sample from low res maps
	vec4 sampleCENTERPosition     = texture( uniformRSMLowResPositionMap, centerUV );
	vec4 sampleCENTERNormal       = texture( uniformRSMLowResNormalMap,   centerUV );	
	vec4 sampleDIAGONALPosition   = texture( uniformRSMLowResPositionMap, centerUV + offsetDIAGONAL);
	vec4 sampleDIAGONALNormal     = texture( uniformRSMLowResNormalMap,   centerUV + offsetDIAGONAL );	
	vec4 sampleHORIZONTALPosition = texture( uniformRSMLowResPositionMap, centerUV + offsetHORIZONTAL );
	vec4 sampleHORIZONTALNormal   = texture( uniformRSMLowResNormalMap,   centerUV + offsetHORIZONTAL );
	vec4 sampleVERTICALPosition   = texture( uniformRSMLowResPositionMap, centerUV + offsetVERTICAL );
	vec4 sampleVERTICALNormal     = texture( uniformRSMLowResNormalMap,   centerUV + offsetVERTICAL );
	
	// Init values for interpolation
	int numValidSamples = 0;
	
	indirectLight = vec4( 0.0, 0.0, 0.0, 0.0);
	vec4 sampleCENTERIndirectLight     = vec4 ( 0.0, 0.0, 0.0, 0.0 );
	vec4 sampleDIAGONALIndirectLight   = vec4 ( 0.0, 0.0, 0.0, 0.0 );
	vec4 sampleHORIZONTALIndirectLight = vec4 ( 0.0, 0.0, 0.0, 0.0 );
	vec4 sampleVERTICALIndirectLight   = vec4 ( 0.0, 0.0, 0.0, 0.0 );
	
	float influenceSampleCENTER     = 0.0;
	float influenceSampleDIAGONAL   = 0.0;
	float influenceSampleHORIZONTAL = 0.0;
	float influenceSampleVERTICAL   = 0.0;
	
	// check sample thresholds
	if ( distance( sampleCENTERPosition, pixelPosition ) <= uniformDistanceThreshold 
			&& 1.0 - dot( sampleCENTERNormal, pixelNormal <= uniformNormalThreshold ) )
	{
		// read sample indirect light value
		vec4 sampleCENTERIndirectLight = texture( uniformRSMLowResIndirectLightMap, centerUV);
		numValidSamples++;
	}
	
	// project into reflective shadow map
	vec4 rsmPosition = uniformRSMProjection * uniformRSMView * worldPosition; // -w..w
	rsmPosition.xyz /= rsmPosition.w; //-1..1
	rsmPosition.w = 1.0;
	rsmPosition.xyz += 1.0;	// 0..2
	rsmPosition.xyz *= 0.5; // 0..1
	
	// INDIRECT LIGHT
	vec2 center = rsmPosition.xy;

	vec3 surfacePosition = worldPosition.xyz;
	vec3 surfaceNormal   = worldNormal.xyz;
	
	// perform light gathering
	vec3 irradiance = computeIndirectLight( center, surfacePosition, surfaceNormal );
	
	// save indirect light intensity
	indirectLight = vec4( irradiance * 200.0, 1.0 );
	
	// DIRECT LIGHT
	vec4 directLightIntensity = computeDirectLight( rsmPosition.xyz );	// surface point position in rsm
	
	// save direct light intensity
	directLight = directLightIntensity;
}