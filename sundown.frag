varying vec2 v_texcoord;
uniform sampler2D s_texture;
uniform float u_time;

float dist (float x, float y, float i, float j)
{
  x = x - i;
  y = y - j;
  return sqrt (x * x + y * y);
}

vec4 sun1 (void)
{
  vec4 sky;
  float g = smoothstep (0.05, 0.06,
			dist (v_texcoord[0], v_texcoord[1], 0.5,
			      u_time / 20.0));
  return mix (vec4 (1.0, 1.0, 0.0, 1.0), vec4 (0.0, 0.8, 1.0, 1.0), g);
}

/*vec4 sun2 (void)
{
  float x = 0.5 - v_texcoord[0];
  float y = u_time / 10.0 - v_texcoord[1];
  float len = x * x + y * y;
  float pi = 3.141592654;
  float d = step (0.5, fract (u_time + atan (y, x) * 10.0 / (2.0 * pi)));
  float param = max (d, 1.0 - step (0.025, len));
  return vec4 (param, param, param, param);
}*/

void main ()
{
  vec4 colour = texture2D (s_texture, v_texcoord);
  vec4 sky;
  //if (fract (u_time * 0.5) < 0.5)
    sky = sun1 ();
  //else
  //  sky = sun2 ();
  gl_FragColor = mix (sky, colour.rgba, colour.a);
}
