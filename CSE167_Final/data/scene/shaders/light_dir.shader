<vertex>

attribute vec3 e_attribute_0;
attribute vec3 e_attribute_1;
attribute vec3 e_attribute_2;
attribute vec3 e_attribute_3;
attribute vec4 e_attribute_4;

uniform vec3 e_icamera;
uniform vec3 e_color;

uniform vec3 e_ilight;

uniform float e_light_iradius;
uniform float e_light_attenuation;
uniform mat4 e_light_projection;
uniform mat4 e_transform;


void main() {
	
	gl_Position = gl_ModelViewProjectionMatrix * vec4(e_attribute_0,1.0);
	
	gl_TexCoord[0] = e_attribute_4;
	
	vec3 dir = (e_ilight - e_attribute_0) * e_light_iradius * sqrt(e_light_attenuation);
	gl_TexCoord[1].x = dot(dir,e_attribute_2);
	gl_TexCoord[1].y = dot(dir,e_attribute_3);
	gl_TexCoord[1].z = dot(dir,e_attribute_1);
	gl_TexCoord[1].w = e_light_attenuation;
	
	dir = e_icamera - e_attribute_0;
	gl_TexCoord[2].x = dot(dir,e_attribute_2);
	gl_TexCoord[2].y = dot(dir,e_attribute_3);
	gl_TexCoord[2].z = dot(dir,e_attribute_1);
	
	gl_TexCoord[3] = vec4(e_color,1.0);
	
	gl_TexCoord[4] = e_light_projection * vec4((e_transform * vec4(e_attribute_0,1.0)).xyz,1.0);
	
<fragment>

uniform sampler2D e_texture_0;
uniform sampler2D e_texture_1;
uniform sampler2D e_texture_4;

void main() {
	
	float attenuation = clamp(gl_TexCoord[1].w - dot(gl_TexCoord[1].xyz,gl_TexCoord[1].xyz),0.0,1.0);
	
	vec3 light_dir = normalize(gl_TexCoord[1].xyz);
	
	vec4 base = texture2D(e_texture_0,gl_TexCoord[0].xy);
	vec4 gloss = texture2D(e_texture_1,gl_TexCoord[0].xy);
	vec3 normal = normalize(gloss.xyz * 2.0 - 1.0);
	
	vec4 modulate = texture2DProj(e_texture_4,max(gl_TexCoord[4],vec4(-1,-1,0,0)).xyzw);

	

	vec3 camera_dir = normalize(gl_TexCoord[2].xyz);
	
	gl_FragColor = (dot(light_dir,normal) * base + pow(clamp(dot(reflect(-light_dir,normal),camera_dir),0.0,1.0),16.0) * gloss.w) *
		modulate * attenuation * gl_TexCoord[3];

}