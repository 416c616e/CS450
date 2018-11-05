#version 330 compatibility

uniform float	uKa, uKd, uKs;		// coefficients of each type of lighting
uniform vec3	uColor;			// object color
uniform vec3	uSpecularColor;		// light color
uniform float   uShininess;		// specular exponent
uniform float	uTime;
uniform bool	uAnimateFragment;

in  vec2  vST;			// texture coords
in  vec3  vN;			// normal vector
in  vec3  vL;			// vector from point to light
in  vec3  vE;			// vector from point to eye
in  vec4  vColor;		// vertex color
in  vec4  st;			// texture coordinates


void
main( )
{
	vec3 Normal = normalize(vN);
	vec3 Light     = normalize(vL);
	vec3 Eye        = normalize(vE);

	vec3 myColor = vColor.xyz * uColor;
	
	float xPos = (st.x - 0.5) * (st.x - 0.5);
	float yPos = (st.y - 0.5) * (st.y - 0.5);

	float timeOffset = sin(uTime*3.14159) * 0.25;

	if ( uAnimateFragment ) {
		if ( xPos + yPos <= timeOffset ) {
			myColor = vec3( 1.0 - vColor.r, 1.0 - vColor.g, 1.0 - vColor.b );

			if ( st.x >= 0.5 - timeOffset * 1.5 ) {
				if ( st.x <= 0.5 + timeOffset * 1.5 ) {
					if ( st.y >= 0.5 - timeOffset * 1.5 ) {
						if ( st.y <= 0.5 + timeOffset * 1.5 ) {
							discard;
						}
					}
				}
			}
		}
	}

	vec3 ambient = uKa * myColor;

	float d = max( dot(Normal,Light), 0. );       // only do diffuse if the light can see the point
	vec3 diffuse = uKd * d * myColor;

	float s = 0.;
	if( dot(Normal,Light) > 0. )	          // only do specular if the light can see the point
	{
		vec3 ref = normalize(  reflect( -Light, Normal )  );
		s = pow( max( dot(Eye,ref),0. ), uShininess );
	}
	vec3 specular = uKs * s * uSpecularColor;
	gl_FragColor = vec4( ambient + diffuse + specular,  1. );
}
