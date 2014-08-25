#version 430 core
 
layout(location = 0) in vec4 positionAttribute;

layout(location = 1) in vec2 uvCoordAttribute;

layout(location = 2) in vec4 normalAttribute;

uniform mat4 uniformModel;
uniform mat4 uniformView;
uniform mat4 uniformProjection;

out vec4 passViewPosition;
out vec4 passWorldPosition;

out vec2 passUVCoord;

out vec3 passWorldNormal;
out vec3 passViewNormal;

void main(){
    passUVCoord = uvCoordAttribute;

    passWorldPosition = uniformModel * positionAttribute;
    passViewPosition = uniformView * uniformModel * positionAttribute;
    gl_Position =  uniformProjection * uniformView * uniformModel * positionAttribute;

    passViewNormal = normalize ( ( transpose( inverse( uniformView * uniformModel ) ) * normalAttribute ).xyz );
    passWorldNormal = normalize ( ( transpose( inverse( uniformModel ) ) * normalAttribute ).xyz );
}
