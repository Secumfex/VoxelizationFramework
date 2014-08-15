#version 430

in vec2 passUV;

// slice map
layout( r32ui, binding = 0 ) uniform readonly uimage2D uniformSliceMapTexture;

// bit mask
layout( r32ui, binding = 1 ) uniform readonly uimage1D bitmaskTexture;

uniform float uniformBackgroundTransparency;

uniform sampler2D uniformPositionMap;
uniform sampler2D uniformBaseTexture;

uniform mat4 uniformWorldToVoxel;

uniform mat4 uniformView;

out vec4 fragmentColor;

void main() {
	// gbuffer values
	vec4 gbufferPosition = texture(uniformPositionMap, passUV);
	
	// discard if background pixel
	if ( gbufferPosition.a == 0.0 )
	{
		discard;
	}
	
	// compute world positon
	vec4 worldPos = inverse( uniformView ) * ( gbufferPosition );
	
	// project into slice map
	vec4 projPos = uniformWorldToVoxel * worldPos;
	
	// compute projected texture position
	vec2 projXY = projPos.xy ;
	int projDepth = int( projPos.z );
	
	// declare RGB value to be added
	vec4 sliceAdd = vec4(0.0);
	
	// retrieve actual texture size
	ivec2 sliceMapSize = imageSize( uniformSliceMapTexture );
	
	// ignore if not in actual voxel grid
	if ( ( projDepth < 32 && projDepth >= 0 ) && (projXY.x < sliceMapSize.x && projXY.x >= 0 ) && ( projXY.y < sliceMapSize.y && projXY.y >= 0) )
	{
	
		// retrieve byte corresponding to projected depth
		uvec4 projTex = imageLoad( bitmaskTexture, projDepth );
		uint projByte = projTex.r;
		
		// retrieve actual slice map byte
		uvec4 sliceTex = imageLoad( uniformSliceMapTexture, ivec2 ( projXY ) );
		uint sliceByte = sliceTex.r;
		
		// AND byte values ( 1, if voxel set )
		uint writeByte = sliceByte & projByte;
		
		// compute rgb values
		float r = float ( ( writeByte & 0xFF000000 ) >> 24u ) / 255.0;
		float g = float ( ( writeByte & 0x00FF0000 ) >> 16u ) / 255.0;
		float b = float ( ( writeByte & 0x0000FF00 ) >> 8u  ) / 255.0;
		float a = float ( ( writeByte & 0x000000FF ) >> 0u  ) / 255.0;
		
		// alpha is distributed among r,g,b channels --> white
		sliceAdd = vec4( r + a , g + a, b + a, 1.0);
		sliceAdd.rgb *= 2.0;	// more intense color
		
		// add some color for x / y coordinates aswell
		float xColor = float ( ( projPos.x ) / 32.0 ) + ( ( int( projPos.x ) % 2) * -0.25 );
		float yColor = float ( ( projPos.y ) / 32.0 ) + ( ( int( projPos.y ) % 2) * -0.25 );
		
		sliceAdd.r += xColor / 6.0;
		sliceAdd.g += yColor / 6.0;
	
	}
	
	// base color
	vec4 baseTex = texture(uniformBaseTexture, passUV) * ( 1.0 - min( 1.0, max( 0.0, uniformBackgroundTransparency ) ) );
		
	// add colors
    fragmentColor = vec4 ( 
    		baseTex.rgb 
    		+ sliceAdd.rgb
    		,baseTex.a);
}