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
    gl_TexCoord[3] = vec4(0.2, 0.2, 0.2, 1.0);
    // e_color

  dir = (s_transform * vec4(att_0, 1.0)).xyz - vec3(s_light_position);
    gl_TexCoord[6] = vec4(dir * s_light_iradius, 1.0);
}


  <fragment>
    #version 120

    uniform sampler2D s_texture_0;
uniform sampler2D s_texture_1;
uniform samplerCube s_texture_7;
void main()
    {
    float attenuation = clamp(gl_TexCoord[1].w - dot(gl_TexCoord[1].xyz,gl_TexCoord[1].xyz),0.0,1.0);
    vec3 light_dir = normalize(gl_TexCoord[1].xyz);
    vec4 base = texture2D(s_texture_0,gl_TexCoord[0].xy);
    vec4 gloss = texture2D(s_texture_1,gl_TexCoord[0].xy);
    vec3 normal = normalize(gloss.xyz * 2.0 - 1.0);
    //shadow map
    float dist_0 = length(gl_TexCoord[6].xyz);
    vec4 depth = textureCube(s_texture_7, gl_TexCoord[6].xyz);
    float dist_1 = dot(depth,vec4(1.0,0.00390625,0.0000152587890625,0.000000059604644775390625));
    if(dist_0 > dist_1 + 0.005) attenuation = 0.0;
    //specular
    vec3 camera_dir = normalize(gl_TexCoord[2].xyz);
    gl_FragColor = (dot(light_dir,normal) * base + pow(clamp(dot(reflect(-light_dir,normal),camera_dir),0.0,1.0),16.0) * gloss.w) * gl_TexCoord[3] * attenuation;
    //gl_FragColor = dot(light_dir, normal) * base * attenuation * gl_TexCoord[3];


    }



    float chebyshevUpperBound(float distance, vec3 dir)
    {
    distance = distance/20 ;
    vec2 moments = texture(shadowCube, dir).rg;
    // Surface is fully lit. as the current fragment is before the light occluder
    if (distance <= moments.x)
    return 1.0;
    // The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
    // How likely this pixel is to be lit (p_max)
    float variance = moments.y - (moments.x*moments.x);
    //variance = max(variance, 0.000002);
    variance = max(variance, 0.00002);
    float d = distance - moments.x;
    float p_max = variance / (variance + d*d);
    return p_max;
}



    void main()
    {
    vec3 fragment = vec3(vpeye);
    vec3 normal   = vec3(normalize(vneye));
    vec3 fragmentToCamera_eye  = normalize(-fragment);
    /* Diffuse lighting */
    // Convert to eye-space (TODO could precompute)
    vec3 light = vec3(view * vec4(light0.position, 1.0));
    // Vectors
    vec3 fragmentToLight     = light - fragment;
    vec3 fragmentToLightDir  = normalize(fragmentToLight);
    /* Shadows */
    vec4 fragmentToLight_world = inverse(view) * vec4(fragmentToLightDir, 0.0);
    float shadowFactor = chebyshevUpperBound(length(fragmentToLight), -fragmentToLight_world);
    vec4 diffColor = vec4(1,1,1,1);
    //if(doTexture != 0)
    //diffColor = texture(shadowCube, -fragmentToLight_world.xyz);
    //diffColor = texture(shadowCube, lightSpacePos);
    //diffColor = texture(shadowCube, vec3(Texcoord.x, 1-Texcoord.y, -1));
    //diffColor = texture(shadowCube, vec3(Texcoord.x*2-1, +1, 2-Texcoord.y*2-1.0)); //+y
    //diffColor = texture(shadowCube, vec3(Texcoord.x*2-1, -1, 2-Texcoord.y*2-1.0)); //-y
    //diffColor = texture(shadowCube, vec3(Texcoord.x*2-1, 2-Texcoord.y*2-1.0, -1)); //-z
    //diffColor = texture(shadowCube, vec3(Texcoord.x*2-1, 2-Texcoord.y*2-1.0, +1)); //+z
    //diffColor = texture(shadowCube, vec3(+1, Texcoord.x*2-1, 2-Texcoord.y*2-1.0)); //+x
    //diffColor = texture(shadowCube, vec3(-1, Texcoord.x*2-1, 2-Texcoord.y*2-1.0)); //-x

    // Angle between fragment-normal and incoming light
    float cosAngIncidence = dot(fragmentToLightDir, normal);
    cosAngIncidence = clamp(cosAngIncidence, 0, 1);
    float attenuation = 1.0f;
    attenuation = 1.0 / (light0.constantAttenuation + light0.linearAttenuation * length(fragmentToLight) + light0.quadraticAttenuation * pow(length(fragmentToLight),2));
    vec4 diffuse  = diffColor * light0.diffuse  * cosAngIncidence * attenuation;
    vec4 total_lighting;
    total_lighting += vec4(0.1, 0.1, 0.1, 1.0) * diffColor;
    // Ambient
    total_lighting += diffuse * shadowFactor;
    // Diffuse

    outColor = vec4(vec3(total_lighting), 1.0);
}
