#version 330

in vec2 passUV;

uniform vec4 uniformColor;

out vec4 fragmentColor;

void main() {
  //  fragmentColor = uniformColor;
    fragmentColor = vec4(0.5,0.5,0.5,0.1);
}