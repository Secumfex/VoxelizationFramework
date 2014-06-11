#version 330 

in vec3 passWorldPosition;
in vec3 passPosition;
in vec3 passNormal;

out vec4 position;

void main() { 
	if ( dot( passPosition, passNormal ) > 0.0f )
	{
		discard;	// back face culling
	}
	
	position = vec4( passWorldPosition, 1.0 );
}