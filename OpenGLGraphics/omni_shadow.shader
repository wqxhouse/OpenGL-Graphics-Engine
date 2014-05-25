<vertex>
  #version 120

  attribute vec3 att_0; //xyz
  varying vec4 v_position;

  uniform mat4 cameraToShadowView;
  uniform mat4 cameraToShadowProjector;
  uniform mat4 s_modelview;

  void main()
  {
  gl_Position = cameraToShadowProjector * s_modelview * vec4(att_0, 1.0);
  v_position  = cameraToShadowView * s_modelview * vec4(att_0, 1.0);
  }

  <fragment>
#version 120
varying vec4 v_position;

void main()
{
	float depth = length( vec3(v_position) ) / 20;

	float moment1 = depth;
	float moment2 = depth * depth;

	float dx = dFdx(depth);
	float dy = dFdy(depth);
	moment2 += 0.25*(dx*dx+dy*dy);

	gl_FragColor = vec4( moment1, moment2, 0.0, 0.0);
  //gl_FragColor = vec4(1, 1, 0, 1);
}