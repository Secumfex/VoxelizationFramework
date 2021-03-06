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

uniform float uniformAngle; // light spot opening angle
uniform float uniformMinCosAngle; // maximum angle of light

uniform float uniformFlux;	// should be set if light source is orthographic

uniform bool uniformHasReflectionCoefficient; // should be set per material, false by default
uniform float uniformReflectionCoefficient; // should be set per material

//writable textures for deferred screen space calculations
layout(location = 0) out vec4 positionOutput;
layout(location = 1) out vec4 normalOutput;
layout(location = 2) out vec4 fluxOutput;
 
void main(){  
	
	// cull if necessary
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
    normalOutput = vec4( passWorldNormal , 0);
   
    // save flux 
    float flux;
    
    if ( uniformOrtho )
    {
    	// constant flux if light is uniform parallel
    	flux = uniformFlux;
    }
    else
    {
    	// compute angle to light direction
    	vec3 pixelDirection = normalize ( passViewPosition.xyz );
    	vec3 lightDirection = vec3( 0.0, 0.0, -1.0 );
    	
    	// compute flux factor
    	flux = dot ( pixelDirection, lightDirection ); // [ -1.0..1.0 ]
    	
    	// bring into correct interval
//    	flux -= uniformMinCosAngle; // [ -minCos -1.0 .. 1.0 - minCos ]
    	flux = min( 1.0, max( 0.0, (flux - uniformMinCosAngle) / ( 1.0 - uniformMinCosAngle ) ) ); // [ 0.0 .. 1.0 ]	
    }
 	
    // if material has reflection coefficient
    if ( uniformHasReflectionCoefficient )
   	{	
   		flux *= uniformReflectionCoefficient;
   	}
       
    vec4 diffuseColor = texture( diffuseTexture, passUVCoord );
        
    // save flux as homogeneous coordinate
//    fluxOutput = vec4 ( diffuseColor.rgb * flux, flux );
    fluxOutput = vec4 ( diffuseColor.rgb, flux );
}
