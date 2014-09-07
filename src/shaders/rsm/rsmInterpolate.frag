#version 430 core

in vec2 passUV;

// low res rsm indirect light map to sample from
uniform sampler2D uniformRSMLowResIndirectLightMap;
//uniform sampler2D uniformRSMLowResPositionMap;
//uniform sampler2D uniformRSMLowResNormalMap;

// low res light map resolution
uniform float uniformRSMLowResX;
uniform float uniformRSMLowResY;

// gbuffer maps to read from
uniform sampler2D uniformGBufferPositionMap;
uniform sampler2D uniformGBufferNormalMap;

uniform mat4 uniformGBufferView;

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
	vec4 pixelWorldPosition = inverse( uniformGBufferView ) * texture( uniformGBufferPositionMap, passUV );
	vec4 pixelWorldNormal   = inverse( uniformGBufferView ) * texture( uniformGBufferNormalMap,   passUV );

	// retrieve nearest samples
	vec2 resolution = vec2( uniformRSMLowResX, uniformRSMLowResY );
	vec2 texelSize = 1.0 / resolution; // pixel size in tex coords
	
	// sample texel center
	vec2 sampleTexelCoords   = passUV * resolution;
	vec2 sampleTexelCenter = ( floor( sampleTexelCoords ) + vec2( 0.5, 0.5 ) );
	
	// sampling direction relative to center of texel
	vec2 offsetDirection= sign( sampleTexelCoords - sampleTexelCenter ); // {-1,0,1}
	if ( offsetDirection.x == 0.0 )
	{
		offsetDirection.x = 1.0;
	}
	if ( offsetDirection.y == 0.0 )
	{
		offsetDirection.y = 1.0;
	}
	
	// init values for interpolation
	vec2 q11 = min( sampleTexelCenter, sampleTexelCenter + offsetDirection ); // bottom left
	vec2 q12 = q11 + vec2( 0.0, 1.0); // top left
	vec2 q21 = q11 + vec2( 1.0, 0.0); // bottom right
	vec2 q22 = q11 + vec2( 1.0, 1.0); // top right
	
	vec2 xy = sampleTexelCoords - q11;// position in interpolation coordinates
	
	// retrieve texture values
	vec4 f00 = texture( uniformRSMLowResIndirectLightMap, q11 * texelSize ); 
	vec4 f10 = texture( uniformRSMLowResIndirectLightMap, q21 * texelSize );
	vec4 f01 = texture( uniformRSMLowResIndirectLightMap, q12 * texelSize );
	vec4 f11 = texture( uniformRSMLowResIndirectLightMap, q22 * texelSize );
	
	vec4 b1 = f00; 					 // f(0,0)
	vec4 b2 = f10 - f00; 			 // f(1,0) - f(0,0)
	vec4 b3 = f01 - f00; 			 // f(0,1) - f(0,0)
	vec4 b4 = f00 - f10 - f01 + f11; // f(0,0) - f(1,0) - f(0,1) + f(1,1)
	
	// interpolate
	vec4 fxy = b1 + b2 * xy.x + b3 * xy.y + b4 * xy.x * xy.y;
	
	// check sample thresholds
//	if ( distance( sampleCENTERPosition, pixelPosition ) <= uniformDistanceThreshold 
//			&& 1.0 - dot( sampleCENTERNormal, pixelNormal <= uniformNormalThreshold ) )
//	{
//	}

	// save interpolated value
	indirectLight = fxy;
}