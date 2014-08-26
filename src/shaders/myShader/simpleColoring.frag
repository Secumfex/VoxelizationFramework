#version 430 core

uniform bool uniformHasTexture;
uniform sampler2D uniformTexture;
uniform float uniformTextureTransparency;

uniform bool uniformHasColor;
uniform float uniformRed;
uniform float uniformGreen;
uniform float uniformBlue;
uniform float uniformAlpha;

in vec2 passUV;

out vec4 fragmentColor;

void main() {
	vec4 texColor = vec4( 0, 0, 0, 0 );
	if ( uniformHasTexture )
	{
		texColor = texture(uniformTexture, passUV);
	}
	
	vec4 color = vec4(0,0,0,0);
	if ( uniformHasColor )
	{
		color = vec4( uniformRed, uniformGreen, uniformBlue, uniformAlpha );
	}
	
    fragmentColor = vec4 ( 
    		max ( 0.0, min( 1.0 ,         ( texColor.a - uniformTextureTransparency ) ) )   * texColor.rgb 
    	  + max ( 0.0, min( 1.0 , ( 1.0 - ( texColor.a - uniformTextureTransparency ) ) ) ) * color.rgb   , uniformAlpha + texColor.a );
}