#version 330

uniform sampler2D uniformTexture;

uniform mat4 uniformView;
uniform mat4 uniformProjection;

layout (location = 0) in vec4 positionAttribute;
layout (location = 1) in vec2 uvCoordAttribute;

out vec2 passUV;

void main() {
	passUV = uvCoordAttribute;

	vec4 position = texture( uniformTexture, positionAttribute.xy );
	
    gl_Position = uniformProjection * uniformView * position;
}