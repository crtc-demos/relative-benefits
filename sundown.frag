varying vec2 v_texcoord;
uniform sampler2D s_texture;
uniform float u_time;

void main ()
{
  vec4 colour = texture2D (s_texture, v_texcoord);
  if (colour == vec4 (1.0, 0.0, 1.0, 1.0))
    gl_FragColor = vec4 (1.0, sin(u_time), 0.0, 1.0);
  else
    gl_FragColor = colour;
}
