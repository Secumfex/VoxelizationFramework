#version 330

out vec4 fragmentColor;

void main() {
	float depth = gl_FragDepth;
    fragmentColor = vec4 ( depth, depth, depth, 1.0);
}