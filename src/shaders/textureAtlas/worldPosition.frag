#version 330 

in vec3 passWorldPosition;

out vec4 position;

void main() { 
	position = vec4( passWorldPosition, 1.0 );
}