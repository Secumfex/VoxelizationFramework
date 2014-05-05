#version 330 

in vec3 passPosition;
in vec2 passUV;
in vec3 passNormal;

out vec4 fragColor;

void main() { 
	vec4 lightPos = vec4(5,5,-2,1);
	
	vec4 diffuseColor = vec4( 1.0, 0.0 , 0.0, 1.0 );
	
    vec3 posToLight = normalize( vec3( lightPos.xyz - passPosition.xyz ) );
    float ambient = 0.1;
    float diffuse = max( dot( passNormal.xyz, posToLight ), 0 );
	
	fragColor = diffuse * diffuseColor + ambient;
}