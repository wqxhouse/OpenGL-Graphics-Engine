<vertex>
  #version 120

  attribute vec3 att_0;
  attribute vec3 att_1;
  attribute vec3 att_2;
  attribute vec3 att_3;
  attribute vec3 att_4;

  uniform vec3 s_camera_inverse;
  uniform vec3 s_light_pos_inverse;
  uniform float s_light_iradius;

  //shadows
  uniform vec4 s_light_position;
  uniform mat4 s_transform;


  void main()
  {
  gl_Position = gl_ModelViewProjectionMatrix * vec4(att_0,1.0);

  gl_TexCoord[0] = vec4(att_4, 0);

  vec3 dir = (s_light_pos_inverse - att_0) * s_light_iradius;

  gl_TexCoord[1].x = dot(dir, att_2);
  gl_TexCoord[1].y = dot(dir, att_3);
  gl_TexCoord[1].z = dot(dir, att_1);
  gl_TexCoord[1].w = 1;

  dir = s_camera_inverse - att_0;
  gl_TexCoord[2].x = dot(dir, att_2);
  gl_TexCoord[2].y = dot(dir, att_3);
  gl_TexCoord[2].z = dot(dir, att_1);

  gl_TexCoord[3] = vec4(0.4, 0.4, 0.4, 1.0); // e_color

  dir = (s_transform * vec4(att_0, 1.0)).xyz - vec3(s_light_position);
  gl_TexCoord[6] = vec4(dir * s_light_iradius, 1.0);
  }

  <fragment>
#version 120

uniform sampler2D s_texture_0;
uniform sampler2D s_texture_1;
uniform samplerCube s_texture_5;

void main() 
{
	float attenuation = clamp(gl_TexCoord[1].w - dot(gl_TexCoord[1].xyz,gl_TexCoord[1].xyz),0.0,1.0);
	
	vec3 light_dir = normalize(gl_TexCoord[1].xyz);
	
	vec4 base = texture2D(s_texture_0,gl_TexCoord[0].xy);
	vec4 gloss = texture2D(s_texture_1,gl_TexCoord[0].xy);
	vec3 normal = normalize(gloss.xyz * 2.0 - 1.0);
	
	//shadow map
	float dist_0 = length(gl_TexCoord[6].xyz);
	
	vec4 depth = textureCube(s_texture_5,gl_TexCoord[6].xyz);
	float dist_1 = dot(depth,vec4(1.0,0.00390625,0.0000152587890625,0.000000059604644775390625));
	
	if(dist_0 > dist_1 + 0.005) attenuation = 0.0;

	//specular
	vec3 camera_dir = normalize(gl_TexCoord[2].xyz);
	
	//	gl_FragColor = (dot(light_dir,normal) * base + pow(clamp(dot(reflect(-light_dir,normal),camera_dir),0.0,1.0),16.0) * gloss.w) * gl_TexCoord[3] * attenuation;
 // gl_FragColor = vec4(dot(light_dir,normal), dot(light_dir,normal) , dot(light_dir,normal), 1);
  
  gl_FragColor = depth;

	//gl_FragColor = dot(light_dir, normal) * base * attenuation * gl_TexCoord[3];
  
 // gl_FragColor = vec4(attenuation, attenuation, attenuation, 1);

}
