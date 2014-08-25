#version 430 core

//incoming data for the single textures
in vec4 passWorldPosition;
in vec4 passViewPosition;
in vec2 passUVCoord;
in vec3 passWorldNormal;
in vec3 passViewNormal;

uniform sampler2D diffuseTexture;

uniform bool uniformEnableBackfaceCulling; // enable culling?
uniform bool uniformOrtho;	// orthographic or perspective projection?

uniform float uniformFlux;	// should be set if light source is orthographic

//writable textures for deferred screen space calculations
layout(location = 0) out vec4 positionOutput;
layout(location = 1) out vec4 normalOutput;
layout(location = 2) out vec4 fluxOutput;
 
void main(){  
	if ( uniformEnableBackfaceCulling )
	{
		if ( uniformOrtho )
		{
			if ( dot( vec3(0.0,0.0, -1.0), passViewNormal ) > 0.0f )
			{
				discard;
			}	
		}
		else
		{
			if( dot( passViewPosition.xyz, passViewNormal ) >= 0.0f )
			{
				discard;
			}
		}
	}

	// save world position
    positionOutput = passWorldPosition;
    
    // save normal
    normalOutput = vec4( normalize( passWorldNormal ), 0);
    vec4 diffuseColor = texture( diffuseTexture, passUVCoord );
    
    // save flux 
    float flux = 1.0;
    if ( uniformOrtho )
    {
    	// constant flux if light is uniform parallel
    	flux = uniformFlux;
    }
    else
    {
    	// TODO compute flux
    }
    
    fluxOutput = flux * diffuseColor;
}
