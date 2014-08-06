#version 430

in vec2 passUV;

// slice map
layout( r32ui, binding = 1 ) uniform uimage2D uniformSliceMapTexture;

// bit mask
layout( r32ui, binding = 1 ) uniform readonly uimage1D bitmaskTexture;

uniform sampler2D uniformPositionMap;
uniform sampler2D uniformBaseTexture;

uniform mat4 uniformProjectorView;
uniform mat4 uniformProjectorPerspective;

uniform mat4 uniformView;

out vec4 fragmentColor;

void main() {
	// gbuffer values
	vec4 gbufferPosition = texture(uniformPositionMap, passUV);
	vec4 gbufferNormal = texture(uniformNormalMap, passUV);
	vec4 gbufferColor = texture(uniformColorMap, passUV);

	// compute world positon
	vec4 worldPos = inverse( uniformView ) * ( gbufferPosition );
	
	// project into slice map
	vec4 projPos = uniformProjectorPerspective * uniformProjectorView * worldPos;
	projPos /= projPos.w;
	projPos.rgb *= 0.5;
	projPos.rgb += 0.5;
	
	// compute projected texture position
	vec2 projUV = projPos.xy ;
	int projDepth = max( 0, min ( 31, int( projPos.z * 31 ) ) );
	
	// retrieve byte corresponding to projected depth
	uvec4 projTex = imageLoad( bitmaskTexture, projDepth );
	uint projByte = projTex.r;
	
	// retrieve actual slice map byte
	uvec4 sliceTex = imageLoad( uniformSliceMapTexture, ivec2 ( projUV * vec2 ( imageSize( uniformSliceMapTexture ) ) ) );
	uint sliceByte = sliceTex.r;
	
	// AND byte values ( 1, if voxel set )
	uint writeByte = sliceByte & projByte;
	
	// compute rgb values
	float r = float ( ( writeByte & 0xFF000000 ) >> 24u ) / 255.0;
	float g = float ( ( writeByte & 0x00FF0000 ) >> 16u ) / 255.0;
	float b = float ( ( writeByte & 0x0000FF00 ) >> 8u  ) / 255.0;
	float a = float ( ( writeByte & 0x000000FF ) >> 0u  ) / 255.0;
		
	// base color
	vec4 baseTex = texture(uniformBaseTexture, passUV);
		
	// alpha is distributed among r,g,b channels --> white
	vec4 sliceAdd = vec4( r + a , g + a, b + a, 1.0);
		
	// add colors
    fragmentColor = vec4 ( baseTex.rgb + sliceAdd.rgb, baseTex.a);
}