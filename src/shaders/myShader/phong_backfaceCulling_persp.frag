#version 330 

in vec3 passPosition;
in vec2 passUV;
in vec3 passNormal;

in vec3 lightPos;

uniform sampler2D diffuseTexture;

out vec4 fragColor;

void main() { 
	if ( dot( passPosition, passNormal ) >= 0.0f )
	{
		discard;	// back face culling
	}

	// vec4 lightPos = vec4(5, 5, -2, 1);
	
	vec4 diffuseColor = texture( diffuseTexture, passUV );
	
    vec3 posToLight = normalize( vec3( lightPos.xyz - passPosition ) );
    
	vec3 reflection = normalize( reflect( - posToLight, passNormal ) );
	
    float specular = pow( max( dot( reflection, - normalize( passPosition ) ),0), 50.0);
	
    float attenuation = 1.0 - min( max ( pow( posToLight.length() / 10.0f, 2.0) , 0.0 ), 1.0); 
	
	float ambient = 0.1;
	
    float diffuse = max( dot( passNormal, posToLight ), 0 );
	
	fragColor = ( diffuse * attenuation ) * diffuseColor + ambient * diffuseColor + specular * vec4( 1.0, 1.0, 1.0, 1.0 );

}