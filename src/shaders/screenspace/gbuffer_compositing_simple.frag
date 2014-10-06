#version 430 core

in vec2 passUV;

uniform sampler2D uniformPositionMap;
uniform sampler2D uniformNormalMap;
uniform sampler2D uniformColorMap;

uniform mat4 uniformLightViewMatrix;
uniform mat4 uniformViewMatrix;

//writable textures for deferred screen space calculations
out vec4 fragment_color;

void main(){  
	// raw gbuffer information
	vec4 gbufferPosition = texture( uniformPositionMap, passUV );
	vec4 gbufferNormal   = texture( uniformNormalMap, passUV );
	vec4 gbufferColor    = texture( uniformColorMap, passUV );
	
	// eye vector : negative gbufferPosition
	vec3 toEye    = normalize( - gbufferPosition.xyz );
	vec3 lightPos = ( uniformViewMatrix * uniformLightViewMatrix * vec4( 0.0, 0.0, 0.0, 1.0 ) ).xyz;
	vec3 toLight  = lightPos.xyz - gbufferPosition.xyz;
	vec3 reflect  = normalize ( reflect ( - toLight, gbufferNormal.xyz ) );
	
	vec4 diffuseColor = gbufferColor;
	
	// diffuse : dot between toLight and surface normal
	float diffuseStrength     = max ( 0.0, min( 1.0, dot ( gbufferNormal.xyz, normalize( toLight ) ) ) );
	// specular : dot between reflected toLight and toEye
//	float specularStrength    = pow( max( 0.0, min( 1.0, dot ( toEye, reflect ) ) ), 10.0 );
	// ambient minimal amount of brightness
	vec4 ambientLight = 0.1 * diffuseColor;
	
	// final color
	fragment_color = 
			( diffuseStrength  ) * diffuseColor 
//			+ ambientStrength  * diffuseColor
			+ ambientLight
//			+ specularStrength * vec4( 1.0, 1.0, 1.0, 1.0 )
			;
	
	fragment_color.a = 1.0;
}
