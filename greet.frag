varying vec2 v_texcoord;
uniform sampler2D s_texture;

void main ()
{
  vec4 col = texture2D (s_texture, v_texcoord);
  gl_FragColor = vec4 (col[0], col[1], col[2], 1.0);
}
