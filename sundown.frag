varying vec2 v_texcoord;
uniform sampler2D s_texture;
uniform float u_time;

float dist (float x, float y, float i, float j)
{
  x = x - i;
  y = y - j;
  return sqrt (x * x + y * y);
}

void main ()
{
  vec4 colour = texture2D (s_texture, v_texcoord);
  if (colour == vec4 (1.0, 0.0, 1.0, 1.0))
    {
      float g = dist (v_texcoord[0], v_texcoord[1], 0.5, u_time / 10.0);
      gl_FragColor = vec4 (1.0, 1.0 - g, 0.0, 1.0);
    }
  else
    gl_FragColor = colour;
}
