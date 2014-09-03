#version 430 core

in vec2 passUV;

// rsm
uniform sampler2D uniformRSMPositionMap;
uniform sampler2D uniformRSMNormalMap;
uniform sampler2D uniformRSMFluxMap;
uniform sampler2D uniformRSMDepthMap;

uniform mat4 uniformRSMView;
uniform mat4 uniformRSMProjection;

// gbuffer
uniform sampler2D uniformGBufferPositionMap;
uniform sampler2D uniformGBufferNormalMap;

uniform mat4 uniformGBufferView;

// sampling pattern
uniform sampler1D uniformSamplingPattern;

uniform int uniformNumSamples;

// voxel grid
//layout( r32ui, binding = 0 ) uniform usampler2D voxel_grid_texture;
uniform usampler2D voxel_grid_texture;

uniform usampler1D uniformBitMask;
uniform usampler1D uniformBitXORMask;

uniform mat4 uniformWorldToVoxel;
uniform mat4 uniformVoxelToVoxelParam;
uniform mat4 uniformWorldToVoxelParam;

// occlusion related uniforms
uniform bool uniformEnableOcclusionTesting;
uniform bool uniformUseHierarchicalIntersectionTesting;
uniform float uniformStartMipMapLevel;
uniform float uniformMaxMipMapLevel;
uniform float uniformNormalOffset;
uniform int uniformMaxTestIterations;

// output
layout(location = 0) out vec4 directLight;
layout(location = 1) out vec4 indirectLight;

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
			
		// dark if invisible
		if ( rsmPosition.z > rsmDepth + 0.03 )
		{
			directLightIntensity = vec4 ( 0.0f, 0.0f, 0.0f, 0.0f );	
		}		
	}
	
	return directLightIntensity;
}
/****************** **** ****************/
/****************** MISC ****************/
/****************** **** ****************/
// test whether provided voxel coordinates are within 0..1
bool validVoxelCoordinates( vec3 voxelNDC )
{
	if ( voxelNDC.x > 1.0 || voxelNDC.x < 0.0 || voxelNDC.y > 1.0 || voxelNDC.y < 0.0 || voxelNDC.z > 1.0 || voxelNDC.z < 0.0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

/****************** ***************** ****************/
/****************** MIP MAP OCCLUSION ****************/
/****************** ***************** ****************/
// returns the parameter t at which the ray intersects the provided clipping plane
float intersect( vec3 from, vec3 rayDir, vec3 clipPoint, vec3 clipNormal )
{
	vec3 w = clipPoint - from;
	
	return dot ( w, clipNormal ) / dot ( rayDir, clipNormal );
}

// test whether the a ray intersects with a set voxel at the provided mipmalLevel
bool testOcclusionMipMap( vec3 from, vec3 to, float startMipMapLevel, vec3 voxelSize )
{
	// ray direction
	vec3  rayDir = to - from;
	
	// signs decide which planes to test against during, at most 3 tests
	vec3  clipDirections = sign ( rayDir );	
	
	// skip test if outside of voxel grid 
	if ( !validVoxelCoordinates(from) || !validVoxelCoordinates(to) )
	{
		return false;
		// TODO move into voxel grid instead
	}
	
	// initialize values
	bool  foundIntersection = false;
	int   iteration         = 1;
	float t                 = 0.0;
	float currentMipMapLevel = startMipMapLevel;
		
	// declare some variables
	uint adaptiveBitMask;   // adaptive bitmask of ray intersecting bounding box
	uint currentTexelValue; // current bit value of texel
	uint intersectionBits;  // AND value of adaptive bitmask and current bit value of texel
	
	vec3  bboxTexel;        // bounding box of texel dimensions 
	vec3  bboxTexelMin;     // bounding box of texel lower left corner 
	vec3  bboxTexelMax;     // bounding box of texel upper right corner
	vec3  bboxOut;			// point at which ray exits bounding box of texel
	vec3  clipNormal;       // current clip plane normal
	vec3  clipPoint;        // current clip plane support point
	vec3  currentPosition;  // current position on ray
	
	vec3 currentTexelSize; // texel size of a texel at current mip map level
	float tClip;           // currently computed t-parameter to intersect plane
	float tOut;            // currently lowest t-parameter to leave bbox
	
	// break conditions
	while ( t < 1.0 && iteration < uniformMaxTestIterations && !foundIntersection )
	{
		// current position along ray
		currentPosition = from + t * rayDir;
					
		// current texel size ( in NDC ) at current mip map level
		currentTexelSize.xy = pow( 2.0, currentMipMapLevel ) * voxelSize.xy;
		currentTexelSize.z  = 1.0;
		
		// lower left corner
		bboxTexelMin   = vec3( 0.0, 0.0, 0.0 );	
		bboxTexelMin.x = floor( currentPosition.x / ( currentTexelSize.x ) ) * ( currentTexelSize.x );
		bboxTexelMin.y = floor( currentPosition.y / ( currentTexelSize.y ) ) * ( currentTexelSize.y );
		
		// bounding box dimensions
		bboxTexel   = vec3( 0.0, 0.0, 1.0 ); // depth  : always depth of a full collumn
		bboxTexel.x = currentTexelSize.x;    // width  : width of a voxel * texel width at this mip map level
		bboxTexel.y = currentTexelSize.y;    // height : height of a voxel * texel height at this mip map level
		
		// upper right corner
		bboxTexelMax = bboxTexelMin + bboxTexel;
		
		// init tOut
		tOut = 1.0;
		
		// compute OUTGOING intersection with BBOX	
		// find first outgoing intersection in ray direction
		if ( clipDirections.x != 0.0 ) // test only if plane not parallel to ray
		{
			clipNormal = vec3( clipDirections.x, 0.0, 0.0 );
			
			if ( clipNormal.x > 0.0 ) // left or right plane on x axis
			{   
				clipPoint = bboxTexelMax; // right plane
			}
			else
			{
				clipPoint = bboxTexelMin; // left plane
			}
			
			tClip = intersect( from, rayDir, clipPoint, clipNormal ); // parameter to intersect plane
			
			tOut = min( tOut, tClip );        // lowest parameter to leave bbox
		}
		
		if ( clipDirections.y != 0.0 ) // test only if plane not parallel to ray
		{
			clipNormal = vec3( 0.0, clipDirections.y,  0.0 );
				
			if ( clipNormal.y > 0.0 ) // upper or lower plane on y axis
			{ 
				clipPoint = bboxTexelMax; // upper plane
			}
			else
			{
				clipPoint = bboxTexelMin; // lower plane
			}
			
			tClip = intersect( from, rayDir, clipPoint, clipNormal ); // parameter to intersect plane
			tOut = min( tOut, tClip );        // lowest parameter to leave bbox
		}
		
		if ( clipDirections.z != 0.0 )
		{
			clipNormal = vec3( 0.0, 0.0, clipDirections.z );
					
			if ( clipNormal.z > 0.0 ) // front or back plane on z axis 
			{ 
				clipPoint = bboxTexelMax; // front plane
			}
			else
			{
				clipPoint = bboxTexelMin; // back plane
			}
			
			tClip = intersect( from, rayDir, clipPoint, clipNormal ); // parameter to intersect plane
			tOut = min( tOut, tClip );        // lowest parameter to leave bbox		
		}
		
		// test for valid tOut
		if ( tOut < t )
		{
			return false;
		}
		
		// intersection point at which ray leaves bbox ( TODO + offset )
		bboxOut = from + ( tOut + 0.01 ) * rayDir; 
		
		// generate adaptive bit mask by XOR-ing bit masks of current depth and box intersection depth
		float sampleBegin = floor( currentPosition.z / voxelSize.z ) * voxelSize.z + 0.5 * voxelSize.z;
		float sampleEnd   = floor( bboxOut.z / voxelSize.z )         * voxelSize.z + 0.5 * voxelSize.z;
		adaptiveBitMask   = texture( uniformBitXORMask, sampleBegin ).r ^ texture( uniformBitXORMask, sampleEnd ).r;
		
		// retrieve texel bit value at current mip map level
		vec2 sampleTexel  = floor( currentPosition.xy / currentTexelSize.xy ) * currentTexelSize.xy + 0.5 * currentTexelSize.xy;
		currentTexelValue = textureLod( voxel_grid_texture, sampleTexel, currentMipMapLevel ).r;
		
		// test for collision by AND-ing bit masks of adaptive bit mask and actual voxel collumn value
		intersectionBits = adaptiveBitMask & currentTexelValue;
		
		// found collision
		if ( intersectionBits != 0 )
		{
			// lowest mip map level
			if ( currentMipMapLevel == 0.0 )
			{
				foundIntersection = true;	
			}
			else
			{
				// test next mipmap level
				currentMipMapLevel = max( 0.0, currentMipMapLevel - 1.0 );
			}
		}
		// no collision at this level
		else
		{
			// move forward and go to coarser mipmap level
			t = tOut+ 0.01;
			currentMipMapLevel = min( uniformMaxMipMapLevel, currentMipMapLevel + 1.0 );
		}
		
		iteration++;
	}
	
	return foundIntersection;
}

/****************** *********************** ****************/
/****************** RAY MARCHING OCCLUSION  ****************/
/****************** *********************** ****************/
bool testOcclusionRayMarching( vec3 fromVoxel, vec3 toVoxel, vec3 voxelSize )
{
	// compute start and end voxel
	vec3 currentVoxel = fromVoxel; // 0..1
	vec3 currentVoxelBase = floor( fromVoxel / voxelSize ) * voxelSize; // 0..1, lower left corner
	
	vec3 rayDir = toVoxel - currentVoxel; // t --> 0..1 between both voxels
		
	// init step direction
	vec3 step = voxelSize * sign( rayDir );
	
//	vec3 step = voxelSize;
//	if ( rayDir.x < 0.0 )
//	{
//		step.x *= -1.0;
//	}
//	if ( rayDir.y < 0.0)
//	{
//		step.y *= -1.0;
//	}
//	if ( rayDir.z < 0.0)
//	{
//		step.z *= -1.0;
//	}

	// init max t traversal before next voxel
	vec3 tMax = vec3( 0.0, 0.0, 0.0 );
	tMax = ( currentVoxelBase + voxelSize - currentVoxel) / rayDir;
	
	// init max Delta per axis to traverse a voxel in that direction
	vec3 tDelta = voxelSize / rayDir;
	
	// early break condition
	int numSteps = 0;
	
	float t = 0.0;
	// traverse ray
	while ( numSteps < uniformMaxTestIterations && t <= 1.0 )
	{			
		// proceed to next voxel
		if ( tMax.x < tMax.y )
		{
			if ( tMax.x < tMax.z )
			{
				currentVoxel.x += step.x;
				tMax.x += tDelta.x;
				t += tDelta.x;
			}
			else
			{
				currentVoxel.z += step.z;
				tMax.z += tDelta.z;
				t += tDelta.z;
			}
		}
		else
		{
			if ( tMax.y < tMax.z )
			{
				currentVoxel.y += step.y;
				tMax.y += tDelta.y;
				t += tDelta.y;
			}
			else
			{
				currentVoxel.z += step.z;
				tMax.z +=tDelta.z;
				t += tDelta.z;
			}
		}
		
		// only test if current voxel inside voxel grid
		if ( validVoxelCoordinates ( currentVoxel ) )
		{
			// retrieve BYTE value from bitmask corresponding to depth
			float depth = currentVoxel.z;
			uvec4 bitMask = texture( uniformBitMask, floor ( depth / voxelSize.z ) * voxelSize.z + 0.5 * voxelSize.z );			
			uint byte = bitMask.r;	
			
			// retrieve current voxel collumn
			uvec4 voxelGridTexel = texture( voxel_grid_texture, floor ( currentVoxel.xy / voxelSize.xy ) * voxelSize.xy + 0.5 * voxelSize.xy );
			uint voxelGridCollumn = voxelGridTexel.x;
			
			// AND with byte currently written in voxel grid texture
			uint test = ( voxelGridCollumn & byte );
				
			if ( test != 0 ) // voxel is set
			{
				return true;
			}	
		}
		
		numSteps++;
	}
	
	return false;
}

/****************** ***************** ****************/
/******************     OCCLUSION     ****************/
/****************** ***************** ****************/
bool testOcclusion( vec3 from, vec3 fromNormal, vec3 to , vec3 toNormal)
{	
	// size of a voxel in NDC
	vec3 voxelSizeNDC = ( uniformVoxelToVoxelParam * vec4( 1.0, 1.0, 1.0, 0.0 ) ).xyz; // 0..res --> 0..1 
	
	// move a little bit out along normals 
	vec3 offsetFrom = uniformNormalOffset * fromNormal;
	vec3 offsetTo   = uniformNormalOffset * toNormal;
	
	// project into NDC voxel space
	vec3 fromNDC = ( uniformWorldToVoxelParam * vec4( from + offsetFrom, 1.0 ) ).xyz; // 0..1
	vec3 toNDC   = ( uniformWorldToVoxelParam * vec4( to   + offsetTo,   1.0 ) ).xyz; // 0..1

	if ( uniformUseHierarchicalIntersectionTesting )
	{
		// use mip map traversal approach
		return testOcclusionMipMap( fromNDC, toNDC, uniformStartMipMapLevel, voxelSizeNDC );
	}
	else
	{
		// use ray marching approach
		return testOcclusionRayMarching( fromNDC, toNDC, voxelSizeNDC );
	}
}

/****************** ***************** ****************/
/******************   INDIRECT LIGHT  ****************/
/****************** ***************** ****************/
vec3 computeIndirectLight( vec2 center, vec3 surfacePosition, vec3 surfaceNormal )
{ 
	vec3 irradiance = vec3( 0.0, 0.0, 0.0 );
	for ( int i = 0; i < uniformNumSamples; i++ )
	{
		// retrieve sampling properties for next sample
		vec4 samplingProperties= texture( uniformSamplingPattern, float( i ) / float( uniformNumSamples ) );
			
		vec2 sampleUV		   = center + samplingProperties.xy;
		float sampleWeight     = samplingProperties.z;

		// retrieve sample point light information
		vec3 rsmSamplePosition = texture( uniformRSMPositionMap, sampleUV ).xyz;	// world position of sample
		vec3 rsmSampleNormal   = texture( uniformRSMNormalMap  , sampleUV ).xyz;	// world normal   of sample
		vec4 rsmSampleFlux     = texture( uniformRSMFluxMap    , sampleUV );		// diffuse light intensity of sample
			
		// test for occlusion
		if ( uniformEnableOcclusionTesting )
		{
			if ( testOcclusion( surfacePosition, surfaceNormal, rsmSamplePosition, rsmSampleNormal ) )
			{
				// skip this light source
				continue;
			}
		}
			
		// move pixel light a little bit back
		rsmSamplePosition -= rsmSampleNormal * 0.1;

		vec3 sampleToSurface = surfacePosition - rsmSamplePosition;
		vec3 surfaceToSample = rsmSamplePosition - sampleToSurface;
			
		// radiance between the surfaces
		float radiantIntensitySampleToSurface = ( max ( 0.0, dot ( rsmSampleNormal, sampleToSurface ) ) );
		float radiantIntensitySurfaceToSample = ( max ( 0.0, dot ( surfaceNormal,   surfaceToSample ) ) );
						
		// compute irradiance at surface point due to sample point light
		vec3 sampleIrradiance = 
				rsmSampleFlux.xyz  // sample point light color
			  * rsmSampleFlux.w    // sample point light flux
			  *	radiantIntensitySampleToSurface // radiant intensity from sample point light to surface 
			  * radiantIntensitySurfaceToSample // radiant intensity from surface to sample point light
			  / pow( length( sampleToSurface ), 4 ); // distance to the power of 4
			
		// apply weight
		sampleIrradiance *= sampleWeight;
			
		// add to total irradiance
		irradiance += sampleIrradiance;
	}
		
	// normalize
	irradiance *= 1.0 / float( uniformNumSamples );
		
	return irradiance;
}

/****************** ***************** ****************/
/******************        MAIN       ****************/
/****************** ***************** ****************/
void main()
{
	// retrieve position from GBuffer
	vec4 worldPosition   = inverse( uniformGBufferView ) * texture( uniformGBufferPositionMap, passUV );
	vec4 worldNormal   =   inverse( uniformGBufferView ) * texture( uniformGBufferNormalMap,   passUV );
	
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