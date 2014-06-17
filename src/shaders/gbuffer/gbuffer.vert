#version 330 core
 
layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in vec2 uvCoordAttribute;
layout(location = 2) in vec4 normalAttribute;

uniform mat4 uniformModel;
uniform mat4 uniformView;
uniform mat4 uniformProjection;

out vec4 passPosition;
out vec2 passUVCoord;
out vec3 passNormal;

void main(){
    passUVCoord = uvCoordAttribute;

    passPosition = uniformView * uniformModel * positionAttribute;
    gl_Position =  uniformProjection * uniformView * uniformModel * positionAttribute;

	passNormal = normalize ( ( transpose( inverse( uniformView * uniformModel ) ) * normalAttribute ).xyz );
}
