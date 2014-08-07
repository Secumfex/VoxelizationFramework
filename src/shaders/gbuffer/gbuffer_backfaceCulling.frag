#version 330 core

//incoming data for the single textures
in vec4 passPosition;
in vec2 passUVCoord;
in vec3 passNormal;

uniform sampler2D diffuseTexture;

uniform bool uniformEnableBackfaceCulling; // enable culling?
uniform bool uniformOrtho;	// orthographic or perspective projection?

//writable textures for deferred screen space calculations
layout(location = 0) out vec4 positionOutput;
layout(location = 1) out vec4 normalOutput;
layout(location = 2) out vec4 colorOutput;
 
void main(){  
	if ( uniformEnableBackfaceCulling )
	{
		if ( uniformOrtho )
		{
			if ( dot( vec3(0.0,0.0, -1.0), passNormal ) > 0.0f )
			{
				discard;
			}	
		}
		else
		{
			if( dot( passPosition.xyz, passNormal ) >= 0.0f )
			{
				discard;
			}
		}
	}

    positionOutput = passPosition;
    normalOutput = vec4(normalize(passNormal), 0);
    colorOutput = texture(diffuseTexture, passUVCoord);
}
