#version 430

in vec2 passUV;

uniform sampler2D uniformBaseTexture;

uniform float uniformBackgroundTransparency;

uniform bool enabled;

layout( r32ui, binding = 0 ) uniform readonly uimage2D uniformSliceMapTexture;

out vec4 fragmentColor;

void main() {
	
	vec4 sliceAdd = vec4(0.0, 0.0, 0.0, 0.0);
	
	if (enabled)
	{
		// size
		vec2 sliceTexSize = vec2 ( imageSize( uniformSliceMapTexture ) );
		
		// load texel
		uvec4 sliceTex = imageLoad( uniformSliceMapTexture, ivec2 ( passUV * sliceTexSize ) );
		
		// load byte value
		uint byte = sliceTex.r;
		
		// compute rgb values
		float r = float ( ( byte & 0xFF000000 ) >> 24u ) / 255.0;
		float g = float ( ( byte & 0x00FF0000 ) >> 16u ) / 255.0;
		float b = float ( ( byte & 0x0000FF00 ) >> 8u  ) / 255.0;
		float a = float ( ( byte & 0x000000FF ) >> 0u  ) / 255.0;
		
		// alpha is distributed among r,g,b channels --> white
		sliceAdd = vec4( r + a , g + a, b + a, 1.0);	
	}
	
	// base color
	vec4 baseTex = texture(uniformBaseTexture, passUV) * ( 1.0 - min( 1.0, max( 0.0, uniformBackgroundTransparency ) ) );
	
	// add
	fragmentColor = vec4 ( baseTex.rgb + sliceAdd.rgb, 1.0);
}