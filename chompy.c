#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <math.h>
#include <stdio.h>

#include "objects.h"
#include "chompy.h"
#include "shader.h"
#include "transform.h"

void draw_object (object_info *obj)
{
 // glUseProgram (obj->shader_program);

  if ((obj->attrs & ATTR_MASK_POS) != 0)
    {
      glVertexAttribPointer (obj->attr.pos, 3, GL_FLOAT, GL_FALSE,
			     obj->vertex_size, obj->data.pos);
      glEnableVertexAttribArray (obj->attr.pos);
    }

  if ((obj->attrs & ATTR_MASK_NORM) != 0)
    {
      glVertexAttribPointer (obj->attr.norm, 3, GL_FLOAT, GL_FALSE,
			     obj->vertex_size, obj->data.norm);
      glEnableVertexAttribArray (obj->attr.norm);
    }

  glDrawArrays (GL_TRIANGLE_STRIP, 0, obj->striplength);
}

void
render_object (int top)
{
  /* Top gum.  */

  if (top)
    {
      draw_object (&topgum_obj);
      draw_object (&tl1_obj);
      draw_object (&tl1_obj);
      draw_object (&tl2_obj);
      draw_object (&tl3_obj);
      draw_object (&tl4_obj);
      draw_object (&tl5_obj);
      draw_object (&tl6_obj);
      draw_object (&tl7_obj);
      draw_object (&tr1_obj);
      draw_object (&tr2_obj);
      draw_object (&tr3_obj);
      draw_object (&tr4_obj);
      draw_object (&tr5_obj);
      draw_object (&tr6_obj);
      draw_object (&tr7_obj);
    }
  else
    {
      /* Bottom gum.  */
      draw_object (&bottomgum_obj);
      draw_object (&bl1_obj);
      draw_object (&bl1_obj);
      draw_object (&bl2_obj);
      draw_object (&bl3_obj);
      draw_object (&bl4_obj);
      draw_object (&bl5_obj);
      draw_object (&bl6_obj);
      draw_object (&bl7_obj);
      draw_object (&bl8_obj);
      draw_object (&br1_obj);
      draw_object (&br2_obj);
      draw_object (&br3_obj);
      draw_object (&br4_obj);
      draw_object (&br5_obj);
      draw_object (&br6_obj);
      draw_object (&br7_obj);
      draw_object (&br8_obj);
    }
}

void
glass (void)
{
  draw_object (&glass_obj);
}

float angle = 0;
float angle2 = 0;

static GLfloat perspective[16];
static GLfloat mvp[16];
static GLint u_mvp;
static GLint u_lightpos;

static GLuint shaderProgram;

void
init_chompy (void *params, display_info *disp)
{
  shaderProgram = create_program_with_shaders ("teeth.vtx", "teeth.frag");

  glBindAttribLocation (shaderProgram, topgum_obj.attr.pos, "a_position");
  glBindAttribLocation (shaderProgram, topgum_obj.attr.norm, "a_normal");

  link_and_check (shaderProgram);

  u_mvp = get_uniform_location (shaderProgram, "u_mvp");
  printf ("u_mvp uniform number: %d\n", u_mvp);

  u_lightpos = get_uniform_location (shaderProgram, "u_lightpos");
  printf ("u_lightpos uniform number: %d\n", u_lightpos);

}

void
display_chompy (sync_info *sync, void *params, int iparam, display_info *disp)
{
  GLfloat view[16];
  GLfloat model[16];
  GLfloat modelview[16];
  GLfloat inv_modelview[16];
  GLfloat ident[16];
  ttd_point3d light0 = { 20, 20, -20 };
  ttd_point3d light0_t;
  GLfloat chomp[16];
  GLfloat xlate[16];

  transform_lookat4 (view, &(ttd_point3d) { 0, 0, 15 },
		     &(ttd_point3d) { 0, 0, 0 },
		     &(ttd_point3d) { 0, 1, 0 });

  transform_perspective4 (perspective, 60,
			  (float) disp->width / (float) disp->height,
			  1.0, 200.0);

  glViewport (0, 0, disp->width, disp->height);
  glUseProgram (shaderProgram);

  glUniform3f (u_lightpos, light0_t.x, light0_t.y, light0_t.z);

  transform_identity4 (model);
  transform_identity4 (ident);

  angle2 = 0.5 + 0.5 * sinf (angle * 10);

  transform_translate4_mat (xlate, 0, 0, 3.5);
  transform_rotate4_mat (chomp, 1, 0, 0, angle2 / 2);

  transform_mul4 (model, chomp, xlate);
  transform_translate4_mat (xlate, 0, 0, -3.5);
  transform_mul4 (model, xlate, model);

  transform_rotate4_mat (xlate, 0, 1, 0, angle);
  transform_mul4 (model, xlate, model);

  transform_mul4 (modelview, view, model);
  transform_invert4 (inv_modelview, modelview);
  transform_point4 (&light0_t, inv_modelview, &light0);
  transform_mul4 (mvp, perspective, modelview);
  glUniformMatrix4fv (u_mvp, 1, GL_FALSE, mvp);

  render_object (0);

  transform_translate4_mat (xlate, 0, 0, 3.5);
  transform_rotate4_mat (chomp, 1, 0, 0, -angle2 / 3);

  transform_mul4 (model, chomp, xlate);
  transform_translate4_mat (xlate, 0, 0, -3.5);
  transform_mul4 (model, xlate, model);

  transform_rotate4_mat (xlate, 0, 1, 0, angle);
  transform_mul4 (model, xlate, model);

  transform_mul4 (modelview, view, model);
  transform_invert4 (inv_modelview, modelview);
  transform_point4 (&light0_t, inv_modelview, &light0);
  transform_mul4 (mvp, perspective, modelview);
  glUniformMatrix4fv (u_mvp, 1, GL_FALSE, mvp);

  render_object (1);

  transform_identity4 (model);

  transform_mul4 (modelview, view, model);
  transform_invert4 (inv_modelview, modelview);
  transform_point4 (&light0_t, inv_modelview, &light0);
  transform_mul4 (mvp, perspective, modelview);
  glUniformMatrix4fv (u_mvp, 1, GL_FALSE, mvp);
  angle += 0.02;
}

effect_methods chompy_methods =
{
  .preinit_assets = NULL,
  .init_effect = &init_chompy,
  .prepare_frame = NULL,
  .display_effect = &display_chompy,
  .uninit_effect = NULL,
  .finalize = NULL
};

