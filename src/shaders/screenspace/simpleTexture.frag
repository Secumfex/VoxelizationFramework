#version 330

in vec2 passUV;

uniform sampler2D uniformTexture;

out vec4 fragmentColor;

void main() {
    fragmentColor = texture(uniformTexture, passUV);
}