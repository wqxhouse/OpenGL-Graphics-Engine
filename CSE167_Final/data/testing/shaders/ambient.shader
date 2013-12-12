<vertex>

#version 120
attribute vec3 att_0;
attribute vec3 att_4;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vec4(att_0, 1);
	gl_TexCoord[0] = vec4(att_4, 1);
	gl_FrontColor = vec4(0.2, 0.2, 0.2, 1);
}

<fragment>
#version 120

uniform sampler2D s_texture_0;

void main()
{
	gl_FragColor = texture2D(s_texture_0, gl_TexCoord[0].xy) * gl_Color;
}