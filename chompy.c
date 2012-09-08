#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "objects.h"
#include "chompy.h"
#include "shader.h"
#include "transform.h"
#include "readpng.h"
#include "sundown.h"


chompy_info chompy_info_0;

void
preinit_chompy (void)
{
  int width, height, alpha;
  char *imgdata;

  imgdata = readpng_image ("wood.png", &width, &height, &alpha);
  chompy_info_0.texture = readpng_bindgl2d (imgdata, width, height, alpha);
  free (imgdata);

  imgdata = readpng_image ("greets.png", &width, &height, &alpha);
  chompy_info_0.greet.texture = readpng_bindgl2d (imgdata, width, height,
						  alpha);
  free (imgdata);
}

void
draw_object (object_info *obj)
{
 // glUseProgram (obj->shader_program);

  glDisableVertexAttribArray (0);
  glDisableVertexAttribArray (1);
  glDisableVertexAttribArray (2);

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

  if ((obj->attrs & ATTR_MASK_TEXCOORD) != 0)
    {
      glVertexAttribPointer (obj->attr.texcoord, 2, GL_FLOAT, GL_FALSE,
			     obj->vertex_size, obj->data.texcoord);
      glEnableVertexAttribArray (obj->attr.texcoord);
    }

  glDrawArrays (GL_TRIANGLE_STRIP, 0, obj->striplength);
}

#define GREET_ATTR_POS 0
#define GREET_ATTR_TEX 1

static GLfloat view[16];
static GLfloat model[16];
static GLfloat modelview[16];
static GLfloat inv_modelview[16];
static GLfloat ident[16];
static GLfloat chomp[16];
static GLfloat xlate[16];
static GLfloat xlate[16];
static GLfloat perspective[16];
static GLfloat mvp[16];

void
draw_dent (sync_info *sync, dent* d, int bottom, int i);

void
draw_dent (sync_info *sync, dent* d, int bottom, int i)
{
  ttd_point3d light0 = { 20, 20, -20 };
  ttd_point3d light0_t;
  int bar = sync->bar;
  uint32_t time_offset = sync->time_offset;
  float angle = (float) sync->time_offset / 1000.0;
  float angle2 = 0.5 + 0.5 * cosf (sync->bar_pos * 2 * M_PI);

  transform_identity4 (model);
  transform_identity4 (ident);

  transform_translate4_mat (xlate, 0, 0, 3.5);
  transform_rotate4_mat (chomp, 1, 0, 0, angle2 / 2);

  transform_mul4 (model, chomp, xlate);
  transform_translate4_mat (xlate, 0, chompy_info_0.drop, -3.5);
  transform_mul4 (model, xlate, model);

  transform_rotate4_mat (xlate, 0, 1, 0, angle);
  transform_mul4 (model, xlate, model);
  if (!bottom)
    {
      transform_translate4_mat (xlate, 0, 0, 3.5);
      transform_rotate4_mat (chomp, 1, 0, 0, -angle2 / 3);

      transform_mul4 (model, chomp, xlate);
      transform_translate4_mat (xlate, 0, chompy_info_0.drop, -3.5);
      transform_mul4 (model, xlate, model);

      transform_rotate4_mat (xlate, 0, 1, 0, angle);
      transform_mul4 (model, xlate, model);
    }

  /* Translate to tooth-specific location.  */
  transform_translate4_mat (xlate, d->x, d->y, d->z);
  transform_mul4 (model, xlate, model);

  transform_mul4 (modelview, view, model);
  transform_invert4 (inv_modelview, modelview);
  transform_point4 (&light0_t, inv_modelview, &light0);
  transform_mul4 (mvp, perspective, modelview);

  glUseProgram (chompy_info_0.tooth.shader);


  glUniform3f (chompy_info_0.tooth.u_lightpos, light0_t.x, light0_t.y,
               light0_t.z);
  glUniformMatrix4fv (chompy_info_0.tooth.u_mvp, 1, GL_FALSE, mvp);

  glUniform4f (chompy_info_0.tooth.u_toothcolour, d->r, d->g, d->b, 0);
  draw_object(d->obj);


  /* Update tooth location.  */
  d->x += d->vx;
  d->y += d->vy;
  d->z += d->vz;

  /* Update tooth velocity.  */
  static const float force = 0.1f;
  static const float friction = 0.02f;
  d->vx += (d->tx - d->x)*force - d->vx*friction;
  d->vy += (d->ty - d->y)*force - d->vy*friction;
  d->vz += (d->tz - d->z)*force - d->vz*friction;

  /* Update tooth target location.  */
  if (bar == 27 && !d->randomized)
    {
      d->randomized = 1;
      d->tx = (drand48()*5.0-2.5);
      d->ty = (drand48()*5.0-2.5);
      d->tz = (drand48()*5.0-2.5);
    }

  if (bar >= 30 && bar < 35)
    {
      float angle = (float)time_offset / 250.0 + i*0.2f;
      d->tx = 2.5f*sinf(angle)/5.0f;
      d->ty = 2.5f*cosf(angle)/5.0f;
      d->tz = 0;
    }

  if (bar == 35)
    {
      d->tx = d->ty = d->tz = 0;
    }
}

void
init_chompy (void *params, display_info *disp)
{
  int i;

  for (i = 0; i < 30; i++)
    {
      dent *d = &chompy_info_0.teeth[i];
      d->r = d->g = d->b = 0;
      d->x = d->y = d->z = 0;
      d->vx = d->vy = d->vz = 0;
      d->tx = d->ty = d->tz = 0;
      d->randomized = 0;
    }
  chompy_info_0.teeth[0].obj = &tl1_obj;
  chompy_info_0.teeth[1].obj = &tl2_obj;
  chompy_info_0.teeth[2].obj = &tl3_obj;
  chompy_info_0.teeth[3].obj = &tl4_obj;
  chompy_info_0.teeth[4].obj = &tl5_obj;
  chompy_info_0.teeth[5].obj = &tl6_obj;
  chompy_info_0.teeth[6].obj = &tl7_obj;
  chompy_info_0.teeth[7].obj = &tr1_obj;
  chompy_info_0.teeth[8].obj = &tr2_obj;
  chompy_info_0.teeth[9].obj = &tr3_obj;
  chompy_info_0.teeth[10].obj = &tr4_obj;
  chompy_info_0.teeth[11].obj = &tr5_obj;
  chompy_info_0.teeth[12].obj = &tr6_obj;
  chompy_info_0.teeth[13].obj = &tr7_obj;
  chompy_info_0.teeth[14].obj = &bl1_obj;
  chompy_info_0.teeth[15].obj = &bl2_obj;
  chompy_info_0.teeth[16].obj = &bl3_obj;
  chompy_info_0.teeth[17].obj = &bl4_obj;
  chompy_info_0.teeth[18].obj = &bl5_obj;
  chompy_info_0.teeth[19].obj = &bl6_obj;
  chompy_info_0.teeth[20].obj = &bl7_obj;
  chompy_info_0.teeth[21].obj = &bl8_obj;
  chompy_info_0.teeth[22].obj = &br1_obj;
  chompy_info_0.teeth[23].obj = &br2_obj;
  chompy_info_0.teeth[24].obj = &br3_obj;
  chompy_info_0.teeth[25].obj = &br4_obj;
  chompy_info_0.teeth[26].obj = &br5_obj;
  chompy_info_0.teeth[27].obj = &br6_obj;
  chompy_info_0.teeth[28].obj = &br7_obj;
  chompy_info_0.teeth[29].obj = &br8_obj;

  /* Teeth.  */
  chompy_info_0.tooth.shader = create_program_with_shaders ("teeth.vtx",
    "teeth.frag");

  glBindAttribLocation (chompy_info_0.tooth.shader, tl1_obj.attr.pos,
			"a_position");
  glBindAttribLocation (chompy_info_0.tooth.shader, tl1_obj.attr.norm,
			"a_normal");

  link_and_check (chompy_info_0.tooth.shader);

  chompy_info_0.tooth.u_mvp
    = get_uniform_location (chompy_info_0.tooth.shader, "u_mvp");
  //printf ("u_mvp uniform number: %d\n", chompy_info_0.tooth.u_mvp);

  chompy_info_0.tooth.u_lightpos
    = get_uniform_location (chompy_info_0.tooth.shader, "u_lightpos");
  //printf ("u_lightpos uniform number: %d\n", chompy_info_0.tooth.u_lightpos);
  chompy_info_0.tooth.u_toothcolour
    = get_uniform_location (chompy_info_0.tooth.shader, "u_toothcolour");

  /* Gum.  */
  chompy_info_0.gum.shader = create_program_with_shaders ("gum.vtx",
    "gum.frag");

  glBindAttribLocation (chompy_info_0.gum.shader, topgum_obj.attr.pos,
			"a_position");
  glBindAttribLocation (chompy_info_0.gum.shader, topgum_obj.attr.norm,
			"a_normal");

  link_and_check (chompy_info_0.gum.shader);

  chompy_info_0.gum.u_mvp
    = get_uniform_location (chompy_info_0.gum.shader, "u_mvp");
  //printf ("u_mvp uniform number: %d\n", chompy_info_0.gum.u_mvp);

  chompy_info_0.tooth.u_lightpos
    = get_uniform_location (chompy_info_0.tooth.shader, "u_lightpos");
  //printf ("u_lightpos uniform number: %d\n", chompy_info_0.tooth.u_lightpos);

  /* Set up wood shader.  */
  chompy_info_0.wood.shader = create_program_with_shaders ("wood.vtx",
    "wood.frag");

  glBindAttribLocation (chompy_info_0.wood.shader, table_obj.attr.pos,
			"a_position");
  glBindAttribLocation (chompy_info_0.wood.shader, table_obj.attr.texcoord,
			"a_texcoord");

  link_and_check (chompy_info_0.wood.shader);

  chompy_info_0.wood.u_mvp = get_uniform_location (chompy_info_0.wood.shader,
						   "u_mvp");
  chompy_info_0.wood.s_texture
    = get_uniform_location (chompy_info_0.wood.shader, "s_texture");

  glActiveTexture (GL_TEXTURE0);
  glBindTexture (GL_TEXTURE_2D, chompy_info_0.texture);
  glGenerateMipmap (GL_TEXTURE_2D);

  glActiveTexture (GL_TEXTURE1);
  glBindTexture (GL_TEXTURE_2D, chompy_info_0.greet.texture);

  /* Set up glass shader.  */
  chompy_info_0.glass.shader = create_program_with_shaders ("glass.vtx",
    "glass.frag");
  glBindAttribLocation (chompy_info_0.glass.shader, glass_obj.attr.pos,
			"a_position");
  glBindAttribLocation (chompy_info_0.glass.shader, glass_obj.attr.norm,
			"a_normal");
  link_and_check (chompy_info_0.glass.shader);

  chompy_info_0.glass.u_mvp = get_uniform_location (chompy_info_0.glass.shader,
 						    "u_mvp");
  chompy_info_0.drop = 20.0;

  /* Set up greetings shader.  */
  chompy_info_0.greet.shader = create_program_with_shaders ("sundown.vtx",
    "greet.frag");
  glBindAttribLocation (chompy_info_0.greet.shader, GREET_ATTR_POS,
			"a_position");
  glBindAttribLocation (chompy_info_0.greet.shader, GREET_ATTR_TEX,
			"a_texcoord");
  link_and_check (chompy_info_0.greet.shader);
  chompy_info_0.greet.u_mvp = get_uniform_location (chompy_info_0.greet.shader,
						    "u_mvp");
  chompy_info_0.greet.s_texture
    = get_uniform_location (chompy_info_0.greet.shader, "s_texture");
}

void
display_chompy (sync_info *sync, void *params, int iparam, display_info *disp)
{
  ttd_point3d light0 = { 20, 20, -20 };
  ttd_point3d light0_t;
  int i;
  static int last_beat = -1;
  int beat = sync->bar * 4 + (int) (sync->bar_pos * 4);
  float angle = (float) sync->time_offset / 1000.0;
  float angle2 = 0.5 + 0.5 * cosf (sync->bar_pos * 2 * M_PI);

  if (sync->bar >= 24)
    {
      for (i = 0; i < 30; i++)
	{
          dent* d = &chompy_info_0.teeth[i];
          d->r *= 0.90;
          d->g *= 0.90;
          d->b *= 0.90;
	}

      if (last_beat != beat && sync->bar <= 30)
	{
	  for (i = 0; i < 30; i++)
            {
              dent* d = &chompy_info_0.teeth[i];
              d->r = drand48 ();
              d->g = drand48 ();
              d->b = drand48 ();
	    }
	  last_beat = beat;
	}
    }

  transform_lookat4 (view, &(ttd_point3d) { 0, 0, 15 },
		     &(ttd_point3d) { 0, 0, 0 },
		     &(ttd_point3d) { 0, 1, 0 });

  transform_perspective4 (perspective, 60,
			  (float) disp->width / (float) disp->height,
			  1.0, 200.0);

  glViewport (0, 0, disp->width, disp->height);
  glUseProgram (chompy_info_0.gum.shader);

  if (sync->bar >= 15)
    {
      glUniform3f (chompy_info_0.gum.u_lightpos, light0_t.x, light0_t.y,
		   light0_t.z);

      transform_identity4 (model);
      transform_identity4 (ident);

      transform_translate4_mat (xlate, 0, 0, 3.5);
      transform_rotate4_mat (chomp, 1, 0, 0, angle2 / 2);

      transform_mul4 (model, chomp, xlate);
      transform_translate4_mat (xlate, 0, chompy_info_0.drop, -3.5);
      transform_mul4 (model, xlate, model);

      transform_rotate4_mat (xlate, 0, 1, 0, angle);
      transform_mul4 (model, xlate, model);

      transform_mul4 (modelview, view, model);
      transform_invert4 (inv_modelview, modelview);
      transform_point4 (&light0_t, inv_modelview, &light0);
      transform_mul4 (mvp, perspective, modelview);
      glUniformMatrix4fv (chompy_info_0.gum.u_mvp, 1, GL_FALSE, mvp);

      draw_object (&bottomgum_obj);

      for (i = 14; i < 30; ++i) {
        draw_dent(sync, &chompy_info_0.teeth[i], 1, i);
      }

      glUseProgram (chompy_info_0.gum.shader);

      glUniform3f (chompy_info_0.gum.u_lightpos, light0_t.x, light0_t.y,
		   light0_t.z);

      transform_translate4_mat (xlate, 0, 0, 3.5);
      transform_rotate4_mat (chomp, 1, 0, 0, -angle2 / 3);

      transform_mul4 (model, chomp, xlate);
      transform_translate4_mat (xlate, 0, chompy_info_0.drop, -3.5);
      transform_mul4 (model, xlate, model);

   //   printf ("%f\n", chompy_info_0.drop);

      transform_rotate4_mat (xlate, 0, 1, 0, angle);
      transform_mul4 (model, xlate, model);

      transform_mul4 (modelview, view, model);
      transform_invert4 (inv_modelview, modelview);
      transform_point4 (&light0_t, inv_modelview, &light0);
      transform_mul4 (mvp, perspective, modelview);
      glUniformMatrix4fv (chompy_info_0.gum.u_mvp, 1, GL_FALSE, mvp);

      draw_object (&topgum_obj);

      for (i = 0; i < 14; ++i) {
        draw_dent(sync, &chompy_info_0.teeth[i], 0, i);
      }


      if (chompy_info_0.drop <= 0.5)
        chompy_info_0.drop = 0.0;
      else if (chompy_info_0.drop > 0)
        chompy_info_0.drop -= 0.5;

   //   printf ("%f\n", chompy_info_0.drop);
    }

  /*transform_identity4 (model);

  transform_mul4 (modelview, view, model);
  transform_invert4 (inv_modelview, modelview);
  transform_point4 (&light0_t, inv_modelview, &light0);
  transform_mul4 (mvp, perspective, modelview);
  glUniformMatrix4fv (u_mvp, 1, GL_FALSE, mvp);*/

  glUseProgram (chompy_info_0.wood.shader);

  transform_rotate4_mat (model, 0, 1, 0, angle);
  transform_mul4 (modelview, view, model);
  transform_mul4 (mvp, perspective, modelview);
  glUniformMatrix4fv (chompy_info_0.wood.u_mvp, 1, GL_FALSE, mvp);

  glUniform1i (chompy_info_0.wood.s_texture, 0);

  draw_object (&table_obj);

  glUseProgram (chompy_info_0.glass.shader);

  glUniformMatrix4fv (chompy_info_0.glass.u_mvp, 1, GL_FALSE, mvp);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc (GL_ONE, GL_ONE);
  //glDepthMask (GL_FALSE);

  glCullFace (GL_FRONT);
  draw_object (&glass_obj);
  glCullFace (GL_BACK);
  draw_object (&glass_obj);

  if (sync->bar >= 40)
    {
      glUseProgram (chompy_info_0.greet.shader);
      glUniform1i (chompy_info_0.greet.s_texture, 1);
      glBlendFunc (GL_ONE, GL_ONE);
      render_screen_quad (disp, chompy_info_0.greet.u_mvp, disp->borders);
    }

  glDisable (GL_BLEND);
  glEnable (GL_CULL_FACE);
  glDepthMask (GL_TRUE);
}

effect_methods chompy_methods =
{
  .preinit_assets = &preinit_chompy,
  .init_effect = &init_chompy,
  .prepare_frame = NULL,
  .display_effect = &display_chompy,
  .uninit_effect = NULL,
  .finalize = NULL
};

