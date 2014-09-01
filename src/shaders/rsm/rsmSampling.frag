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
uniform mat4 uniformWorldToVoxel;
uniform mat4 uniformVoxelToVoxelParam;
uniform mat4 uniformWorldToVoxelParam;

// occlusion related uniforms
uniform bool uniformEnableOcclusionTesting;
uniform bool uniformUseHierarchicalIntersectionTesting;
uniform int uniformHighestMipMapLevel;
uniform int uniformMaxNumSteps;

// output
layout(location = 0) out vec4 directLight;
layout(location = 1) out vec4 indirectLight;

vec4 computeDirectLight( vec3 rsmPosition )
{
	vec4 directLightIntensity = vec4( 0.0f, 0.0f, 0.0f, 0.0f );
		
	// inside of light view
	if ( rsmPosition.x < 1.0 && rsmPosition.y < 1.0 && rsmPosition.x > 0.0 && rsmPosition.y > 0.0)
	{
		float rsmDepth = texture( uniformRSMDepthMap, rsmPosition.xy ).x;
		directLightIntensity = texture( uniformRSMFluxMap , rsmPosition.xy);
			
		// dark if invisible
		if ( rsmPosition.z > rsmDepth + 0.003 )
		{
			directLightIntensity = vec4 ( 0.0f, 0.0f, 0.0f, 0.0f );	
		}		
	}
	
	return directLightIntensity;
}

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

bool testOcclusionFromMipMapLevel( vec3 from, vec3 to, float mipMapLevel, vec3 voxelSize )
{
	vec3 rayDir = to-from;
	
	// retrieve start voxel column value
	uvec4 voxelStart = textureLod( voxel_grid_texture, from.xy, mipMapLevel );
//	uvec4 voxelEnd   = textureLod( voxel_grid_texture, to.xy,   mipMapLevel );

	// texture value at this mipmap level aka. bitmask in texel
	uint voxelStartValue = voxelStart.x;
//	uint voxelEndValue   = voxelEnd.x;
				
	// compute "bounding box" of texel
	float texelSize = pow( 2.0, mipMapLevel );
	
	vec3 bboxMin = vec3( 0.0, 0.0, 0.0);
	bboxMin.x = ( floor( from.x / voxelSize.x ) * voxelSize.x ) * texelSize;
	bboxMin.y = ( floor( from.y / voxelSize.y ) * voxelSize.y ) * texelSize;
			
	vec3 bbox = vec3( 0.0, 0.0, 1.0 );// depth  : constantly depth of a full collumn
	bbox.x = voxelSize.x * texelSize; // width  : width of a voxel * texel width at this mipmap level
	bbox.y = voxelSize.y * texelSize; // height : height of a voxel * texel height at this mipmap level

	
	// generate adaptive bit mask
	uint adaptiveBitMask = 0;
	
	// TODO compute intersection with BBOX	
	vec3 bboxIn;
	vec3 bboxOut;
	
	// compute adaptive bit mask from ray bbox	
	for ( float voxel = bboxIn.z + 0.5 * voxelSize.z; voxel <= bboxOut.z+ 0.5 * voxelSize.z; voxel += voxelSize.z )
	{
		adaptiveBitMask = adaptiveBitMask & texture( uniformBitMask, voxel );
	}
	
	// TODO AND bit masks of adaptive bit mask and actual voxel value
	uint intersectionBits = adaptiveBitMask & voxelStartValue;
	
	if ( intersectionBits != 0 )
	{
		return true;
		// TODO go to next mipMapLevel
	}
}

bool testOcclusionTraverseRay( vec3 fromVoxel, vec3 toVoxel, vec3 voxelSize )
{
	// compute start and end voxel
	vec3 currentVoxel = fromVoxel; // 0..1
	vec3 currentVoxelIndex = floor ( ( inverse( uniformVoxelToVoxelParam ) * vec4( fromVoxel, 1.0) ).xyz ); // 0..res
	vec3 currentVoxelBase = ( uniformVoxelToVoxelParam * vec4( currentVoxelIndex, 1.0 ) ).xyz; // 0..1, lower left corner
	
	vec3 rayDir = toVoxel - currentVoxel; // t --> 0..1 between both voxels
		
	// init step direction
	vec3 step = voxelSize;
	if ( rayDir.x < 0.0 )
	{
		step.x *= -1.0;
	}
	if ( rayDir.y < 0.0)
	{
		step.y *= -1.0;
	}
	if ( rayDir.z < 0.0)
	{
		step.z *= -1.0;
	}

	// init max t traversal before next voxel
	vec3 tMax = vec3(0.0, 0.0, 0.0);
	tMax = ( currentVoxelBase + voxelSize - currentVoxel) / rayDir;
	
	// init max Delta per axis to traverse a voxel in that direction
	vec3 tDelta = voxelSize / rayDir;
	
	// early break condition
	int numSteps = 0;
	
	float t = 0.0;
	// traverse ray
	while ( numSteps < uniformMaxNumSteps && t <= 1.0 )
	{	
		// only test if current voxel inside voxel grid
		if ( validVoxelCoordinates ( currentVoxel ) )
		{
			// retrieve BYTE value from bitmask corresponding to depth
			float depth = currentVoxel.z;
			uvec4 bitMask = texture( uniformBitMask, depth );			
			uint byte = bitMask.r;	
			
			// retrieve current voxel collumn
			uvec4 voxelGridTexel = texture( voxel_grid_texture, currentVoxel.xy );
			uint voxelGridCollumn = voxelGridTexel.x;
			
			// AND with byte currently written in voxel grid texture
			uint test = ( voxelGridCollumn & byte );
				
			if ( test != 0 ) // voxel is set
			{
				return true;
			}	
		}
		
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
		
		numSteps++;
	}
	
	return false;
}

bool testOcclusion( vec3 from, vec3 fromNormal, vec3 to , vec3 toNormal )
{	
	// size of a voxel in NDC
	vec3 voxelSize = ( vec4( 1.0, 1.0, 1.0, 0.0 ) * uniformVoxelToVoxelParam ).xyz; // 0..res --> 0..1 
	
	// move a little bit out along normals 
	vec3 offsetFrom = 0.2 * fromNormal;
	vec3 offsetTo   = 0.2 * toNormal;
	
	// project into voxel space
	vec3 fromVoxel = ( uniformWorldToVoxelParam * vec4( from + offsetFrom, 1.0 ) ).xyz ; // 0..1
	vec3 toVoxel   = ( uniformWorldToVoxelParam * vec4( to   + offsetTo, 1.0 ) ).xyz;    // 0..1

	if ( uniformUseHierarchicalIntersectionTesting )
	{
		// start at coarsest mip map level
		float mipMapLevel = float ( uniformHighestMipMapLevel );
		float rayLength   = distance ( fromVoxel, toVoxel );
					
		while ( rayLength > 0.0 && mipMapLevel >= 0.0)
		{
			bool occlusionFound = testOcclusionFromMipMapLevel( fromVoxel, toVoxel, mipMapLevel, voxelSize ) ;
					
			if ( occlusionFound ) // go deeper
			{
				mipMapLevel -= 1.0;	
			}
			else	// proceed in ray direction
			{
				// TODO clip ray to next voxel column
//						clipRay( from, to ); // TODO at bounding box 
			}
		}
	}
	else
	{
		// use ray traversal approach
		return testOcclusionTraverseRay( fromVoxel, toVoxel, voxelSize );
	}
	
	return false;
}

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
				bool isOccluded = testOcclusion( surfacePosition, surfaceNormal, rsmSamplePosition, rsmSampleNormal );
				if ( isOccluded )
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