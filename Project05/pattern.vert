#version 330 compatibility

uniform float uTime;
uniform bool  uAnimateVertices;

out  vec3  vN;		// normal vector
out  vec3  vL;		// vector from point to light
out  vec3  vE;		// vector from point to eye
out  vec4  vColor;  // vertex color
out  vec4  st;		// texture coordinates

vec3 LightPosition = vec3(  0., 5., 5. );

void
main( )
{ 
	vec3 vert = gl_Vertex.xyz;

	if ( uAnimateVertices ) {
		vert.x = vert.x + sin(uTime*vert.x*3.14159);
		vert.y = sin(2.0*uTime*vert.y*3.14159);
	}

	vec4 ECposition = gl_ModelViewMatrix * vec4( vert, 1. );
	vN = normalize( gl_NormalMatrix * gl_Normal );	// normal vector
	vL = LightPosition - ECposition.xyz;		// vector from the point
							// to the light position
	vE = vec3( 0., 0., 0. ) - ECposition.xyz;	// vector from the point
							// to the eye position 
	gl_Position = gl_ModelViewProjectionMatrix * vec4( vert, 1. );
	vColor = gl_Color;
	st = gl_MultiTexCoord0;
}
