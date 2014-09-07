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
	mat4 invGBufferView = inverse( uniformGBufferView );
	
	// retrieve position and normal of pixel from gbuffer
	vec4 pixelWorldPosition = invGBufferView * texture( uniformGBufferPositionMap, passUV );
	vec4 pixelWorldNormal   = invGBufferView * texture( uniformGBufferNormalMap,   passUV );

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
	vec2 q00 = min( sampleTexelCenter, sampleTexelCenter + offsetDirection ); // bottom left
	vec2 q01 = q00 + vec2( 0.0, 1.0); // top left
	vec2 q10 = q00 + vec2( 1.0, 0.0); // bottom right
	vec2 q11 = q00 + vec2( 1.0, 1.0); // top right
	
	float x = sampleTexelCoords.x - q00.x;// position in interpolation coordinates
	float y = sampleTexelCoords.y - q00.y;
			
	// retrieve texture values
	vec4 f00 = texture( uniformRSMLowResIndirectLightMap, q00 * texelSize ); 
	vec4 f10 = texture( uniformRSMLowResIndirectLightMap, q10 * texelSize );
	vec4 f01 = texture( uniformRSMLowResIndirectLightMap, q01 * texelSize );
	vec4 f11 = texture( uniformRSMLowResIndirectLightMap, q11 * texelSize );
	
//	vec4 b1 = f00; 					 // f(0,0)
//	vec4 b2 = f10 - f00; 			 // f(1,0) - f(0,0)
//	vec4 b3 = f01 - f00; 			 // f(0,1) - f(0,0)
//	vec4 b4 = f00 - f10 - f01 + f11; // f(0,0) - f(1,0) - f(0,1) + f(1,1)
//	
//	// interpolate
//	vec4 fxy = b1 + b2 * x + b3 * y + b4 * x * y;
	
	// influences
	float i00 = ( 1.0 - x ) * ( 1.0 - y );
	float i01 = ( 1.0 - x ) *       y    ;
	float i10 =       x     * ( 1.0 - y );
	float i11 =       x     *       y    ;
	
	// bias in case only 3 samples are used
	float bias = 1.0;
	int validSamples = 4;
	
	// world positions
	vec4 worldPos00 = invGBufferView * texture( uniformGBufferPositionMap, q00 * texelSize ); 
	vec4 worldPos10 = invGBufferView * texture( uniformGBufferPositionMap, q10 * texelSize );
	vec4 worldPos01 = invGBufferView * texture( uniformGBufferPositionMap, q01 * texelSize );
	vec4 worldPos11 = invGBufferView * texture( uniformGBufferPositionMap, q11 * texelSize );
				
	// world normal
	vec4 worldNormal00 = invGBufferView * texture( uniformGBufferNormalMap, q00 * texelSize ); 
	vec4 worldNormal10 = invGBufferView * texture( uniformGBufferNormalMap, q10 * texelSize );
	vec4 worldNormal01 = invGBufferView * texture( uniformGBufferNormalMap, q01 * texelSize );
	vec4 worldNormal11 = invGBufferView * texture( uniformGBufferNormalMap, q11 * texelSize );

	// check sample thresholds
	if ( distance( worldPos00, pixelWorldPosition )  >= uniformDistanceThreshold 
			|| dot( worldNormal00, pixelWorldNormal ) <= uniformNormalThreshold )
	{
		bias =  1.0 / ( 1.0 - i00 );
		i00 = 0.0;
		validSamples--;
	}
	
	if ( distance( worldPos01, pixelWorldPosition )  >= uniformDistanceThreshold 
			|| dot( worldNormal01, pixelWorldNormal ) <= uniformNormalThreshold )
	{
		bias =  1.0 / ( 1.0 - i01 );
		i01 = 0.0;
		validSamples--;
	}
	
	if ( distance( worldPos10, pixelWorldPosition )  >= uniformDistanceThreshold 
			|| dot( worldNormal10, pixelWorldNormal ) <= uniformNormalThreshold )
	{
		bias =  1.0 / ( 1.0 - i10 );
		i10 = 0.0;
		validSamples--;
	}
	
	if ( distance( worldPos11, pixelWorldPosition )  >= uniformDistanceThreshold 
			|| dot( worldNormal11, pixelWorldNormal ) <= uniformNormalThreshold )
	{
		bias =  1.0 / ( 1.0 - i11 );
		i11 = 0.0;
		validSamples--;
	}
	
	// too few samples
	if ( validSamples < 3 )
	{
		// dont even try
		discard;
	}

	vec4 fxy = ( f00 * i00 + f01 * i01 + f10 * i10 + f11 * i11 ) * bias;
	
	// save interpolated value
	indirectLight = fxy;
}