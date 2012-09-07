#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <stdlib.h>

#include "sundown.h"
#include "readpng.h"
#include "shader.h"
#include "transform.h"

sundown_info sundown_info_0;

#define ATTR_POS 0
#define ATTR_TEX 1

void
preinit_sundown (void)
{
  int alpha;
  printf ("Loading budleigh.png...\n");
  sundown_info_0.image_data = readpng_image ("budleigh.png",
    &sundown_info_0.image_width, &sundown_info_0.image_height, &alpha);
  sundown_info_0.texhandle = readpng_bindgl2d (sundown_info_0.image_data,
    sundown_info_0.image_width, sundown_info_0.image_height, alpha);
  free (sundown_info_0.image_data);
  printf ("done.\n");
}

void
init_sundown (void *params, display_info *disp)
{
  sundown_info_0.shader = create_program_with_shaders ("sundown.vtx",
    "sundown.frag");
  
  glBindAttribLocation (sundown_info_0.shader, ATTR_POS, "a_position");
  glBindAttribLocation (sundown_info_0.shader, ATTR_TEX, "a_texcoord");
  
  link_and_check (sundown_info_0.shader);

  sundown_info_0.u_mvp = get_uniform_location (sundown_info_0.shader, "u_mvp");
  sundown_info_0.s_texture = get_uniform_location (sundown_info_0.shader,
						   "s_texture");
  sundown_info_0.u_time = get_uniform_location (sundown_info_0.shader,
						"u_time");
}

static GLfloat triangles[] =
  { -1.0, -1.0, 0.0, 0.0, 1.0,
     1.0, -1.0, 0.0, 1.0, 1.0,
    -1.0,  1.0, 0.0, 0.0, 0.0,
     1.0,  1.0, 0.0, 1.0, 0.0 };

void
render_screen_quad (display_info *disp, GLint mvp_uniform)
{
  GLfloat ortho[16];
  GLfloat view[16];
  GLfloat mvp[16];

  transform_ortho4 (ortho, -1, 1, -1, 1, 1, 200);
  //transform_perspective4 (ortho, 60, 1.33, 1.0, 200.0);
  
  transform_lookat4 (view, &(ttd_point3d) { 0, 0, 15 },
    &(ttd_point3d) { 0, 0, 0 }, &(ttd_point3d) { 0, 1, 0 });
  glViewport (0, 0, disp->width, disp->height);

  transform_mul4 (mvp, ortho, view);
  
  glUniformMatrix4fv (mvp_uniform, 1, GL_FALSE, mvp);
  
  glVertexAttribPointer (ATTR_POS, 3, GL_FLOAT, GL_FALSE, 5 * sizeof (GLfloat),
			 &triangles[0]);
  glEnableVertexAttribArray (ATTR_POS);
  glVertexAttribPointer (ATTR_TEX, 2, GL_FLOAT, GL_FALSE, 5 * sizeof (GLfloat),
			 &triangles[3]);
  glEnableVertexAttribArray (ATTR_TEX);
  
  glDisable (GL_CULL_FACE);

  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

void
display_sundown (sync_info *sync, void *params, int iparam, display_info *disp)
{ 
  glUseProgram (sundown_info_0.shader);
  
  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, sundown_info_0.texhandle);
  glUniform1i (sundown_info_0.s_texture, 0);
  glUniform1f (sundown_info_0.u_time, (float) sync->time_offset / 1000.0);

  render_screen_quad (disp, sundown_info_0.u_mvp);
}

effect_methods sundown_methods =
{
  .preinit_assets = &preinit_sundown,
  .init_effect = &init_sundown,
  .prepare_frame = NULL,
  .display_effect = &display_sundown,
  .uninit_effect = NULL,
  .finalize = NULL
};

