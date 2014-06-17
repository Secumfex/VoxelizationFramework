#version 330

in vec2 passUV;

uniform sampler2D uniformPositionMap;
uniform sampler2D uniformNormalMap;
uniform sampler2D uniformColorMap;

uniform sampler2D uniformShadowMap;

uniform mat4 uniformProjectorView;
uniform mat4 uniformProjectorPerspective;

uniform mat4 uniformView;

out vec4 fragmentColor;

void main() {
	vec4 sliceTex = texture(uniformShadowMap, passUV);
	vec4 baseTex = texture(uniformColorMap, passUV);
	
	vec4 sliceAdd = vec4( sliceTex.rgb + sliceTex.a , 0.0 );
	
    fragmentColor = vec4 ( baseTex.rgb + sliceAdd.rgb, baseTex.a);
}