#version 330

in vec2 passUV;

uniform float uniformRed;
uniform float uniformGreen;
uniform float uniformBlue;
uniform float uniformAlpha;

out vec4 fragmentColor;

void main() {
  //  fragmentColor = uniformColor;
    fragmentColor = vec4( uniformRed, uniformGreen, uniformBlue, uniformAlpha );
}