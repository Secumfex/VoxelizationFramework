#version 330

uniform mat4 uniformModel;
uniform mat4 uniformView;
uniform mat4 uniformProjection;

layout (location = 0) in vec4 positionAttribute;
layout (location = 1) in vec2 uvCoordAttribute;
layout (location = 2) in vec4 normalAttribute;

out vec3 passPosition;
out vec2 passUV;
out vec3 passNormal;

void main() {
	passPosition = ( uniformView * uniformModel * positionAttribute ).xyz;
	passUV = uvCoordAttribute;
	passNormal = vec3(transpose(inverse(uniformView * uniformModel)) * normalAttribute);
    
    gl_Position = uniformProjection * uniformView * uniformModel * positionAttribute;
}