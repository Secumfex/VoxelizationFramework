#version 330

uniform mat4 uniformModel;
uniform mat4 uniformView;
uniform mat4 uniformProjection;

layout (location = 0) in vec4 positionAttribute;
layout (location = 1) in vec2 uvCoordAttribute;
layout (location = 2) in vec4 normalAttribute;

out vec3 passPosition;
out vec3 passWorldPosition;
out vec2 passUV;
out vec3 passNormal;
out vec3 lightPos;

void main() {
	vec4 worldPos = (uniformModel * positionAttribute );
	passWorldPosition = worldPos.xyz;
	
	lightPos = (uniformView * vec4(2.5,2.5,2.5,1.0)).xyz;
	
	passPosition = ( uniformView * worldPos ).xyz;
	
	passUV = uvCoordAttribute;
	passNormal = normalize ( ( transpose( inverse( uniformView * uniformModel ) ) * normalAttribute ).xyz );
    
    gl_Position = uniformProjection * uniformView * uniformModel * positionAttribute;
}