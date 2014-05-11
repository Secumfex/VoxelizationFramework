#version 330

// a simple vertex shader which simply applies the MVP matrix to the vertex and passes it's camera distance to the fragment shader

uniform mat4 uniformModel;
uniform mat4 uniformView;
uniform mat4 uniformProjection;

layout (location = 0) in vec4 positionAttribute;

out float passDistanceToCam;

void main() {
	// vertex position
	vec4 position = uniformProjection * uniformView * uniformModel * positionAttribute;
    
	// distance to cam : z value of position after projection
	passDistanceToCam = ( position ).z ;	
	
    gl_Position = position;
}