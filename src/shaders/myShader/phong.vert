#version 330

layout (location = 0) in vec4 positionAttribute;

void main() {
    gl_Position = positionAttribute;
}