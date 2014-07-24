#version 330

in vec2 passUV;

uniform sampler2D uniformBaseTexture;
uniform usampler2D uniformSliceMapTexture;

out vec4 fragmentColor;

void main() {
	vec4 sliceTex = texture(uniformSliceMapTexture, passUV);
	vec4 baseTex = texture(uniformBaseTexture, passUV);
	
	vec4 sliceAdd = vec4( sliceTex.rgb + sliceTex.a , 0.0 );
	
    fragmentColor = vec4 ( baseTex.rgb + sliceAdd.rgb, baseTex.a);
}