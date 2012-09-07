varying vec3 v_normal;

void main ()
{
  vec3 eye = vec3 (0.0, 0.0, -1.0);
  vec3 refl = reflect (eye, normalize (v_normal));
  float amt = max (dot (refl, vec3 (1.0, 0.0, 0.0)), 0.0);
  float amt2 = max (dot (refl, vec3 (-1.0, 0.0, 0.0)), 0.0);
  float col = pow (amt, 30.0) + pow (amt2, 30.0);
  gl_FragColor = vec4 (col, col, col, 0.5);
}
