#version 330

in vec2 passUV;

uniform sampler2D uniformTexture;
uniform sampler2D uniformMultiply;

out vec4 fragmentColor;

void main() {
	vec4 multiply = texture(uniformMultiply, passUV);
	vec4 texture = texture(uniformTexture, passUV);
	
	texture.r *= multiply.r;
	texture.g *= multiply.g;
	texture.b *= multiply.b;
	
    fragmentColor = texture;
}