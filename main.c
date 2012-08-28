#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <alloca.h>
#include <stddef.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <bcm_host.h>

#include "vecmat.h"
#include "transform.h"
#include "shader.h"

#include "attr_masks.h"

#include "topgum.inc"
#include "bottomgum.inc"

#include "tl1.inc"
#include "tl2.inc"
#include "tl3.inc"
#include "tl4.inc"
#include "tl5.inc"
#include "tl6.inc"
#include "tl7.inc"

#include "tr1.inc"
#include "tr2.inc"
#include "tr3.inc"
#include "tr4.inc"
#include "tr5.inc"
#include "tr6.inc"
#include "tr7.inc"

#include "bl1.inc"
#include "bl2.inc"
#include "bl3.inc"
#include "bl4.inc"
#include "bl5.inc"
#include "bl6.inc"
#include "bl7.inc"
#include "bl8.inc"

#include "br1.inc"
#include "br2.inc"
#include "br3.inc"
#include "br4.inc"
#include "br5.inc"
#include "br6.inc"
#include "br7.inc"
#include "br8.inc"

#include "cylinder.inc"

#define ARRAYLENGTH(X) (sizeof (X) / sizeof ((X)[0]))

static EGLDisplay edpy;
static EGLContext ectxt;
static EGLSurface esfc;

static GLfloat perspective[16];
static GLfloat mvp[16];
static GLint u_mvp;
static GLint u_lightpos;

void
render_object (int top)
{
#define DRAWTOOTH(UNAME, LNAME)						\
  glVertexAttribPointer (UNAME##_ATTR_POS, 3, GL_FLOAT, GL_FALSE,	\
			 sizeof (LNAME##_vertex),			\
			 (void*) LNAME##_vertices[0].pos);		\
  glEnableVertexAttribArray (UNAME##_ATTR_POS);				\
									\
  glVertexAttribPointer (UNAME##_ATTR_NORM, 3, GL_FLOAT, GL_FALSE,	\
			 sizeof (LNAME##_vertex),			\
			 (void*) LNAME##_vertices[0].norm);		\
  glEnableVertexAttribArray (UNAME##_ATTR_NORM);			\
  									\
  glDrawArrays (GL_TRIANGLE_STRIP, 0, ARRAYLENGTH (LNAME##_vertices));
  /* Top gum.  */

  if (top)
    {
      glVertexAttribPointer (TOPGUM_ATTR_POS, 3, GL_FLOAT, GL_FALSE,
			     sizeof (topgum_vertex),
			     (void*) topgum_vertices[0].pos);
      glEnableVertexAttribArray (TOPGUM_ATTR_POS);

      glVertexAttribPointer (TOPGUM_ATTR_NORM, 3, GL_FLOAT, GL_FALSE,
			     sizeof (topgum_vertex),
			     (void*) topgum_vertices[0].norm);
      glEnableVertexAttribArray (TOPGUM_ATTR_NORM);

      /*printf ("draw array, length %d\n", ARRAYLENGTH (topgum_vertices));*/
      glDrawArrays (GL_TRIANGLE_STRIP, 0, ARRAYLENGTH (topgum_vertices));

      DRAWTOOTH(TL1, tl1)
      DRAWTOOTH(TL2, tl2)
      DRAWTOOTH(TL3, tl3)
      DRAWTOOTH(TL4, tl4)
      DRAWTOOTH(TL5, tl5)
      DRAWTOOTH(TL6, tl6)
      DRAWTOOTH(TL7, tl7)

      DRAWTOOTH(TR1, tr1)
      DRAWTOOTH(TR2, tr2)
      DRAWTOOTH(TR3, tr3)
      DRAWTOOTH(TR4, tr4)
      DRAWTOOTH(TR5, tr5)
      DRAWTOOTH(TR6, tr6)
      DRAWTOOTH(TR7, tr7)
    }
  else
    {
      /* Bottom gum.  */
      /*glBindAttribLocation (shader_program, BOTTOMGUM_ATTR_POS, "a_position");
      glBindAttribLocation (shader_program, BOTTOMGUM_ATTR_NORM, "a_normal");*/

      glVertexAttribPointer (BOTTOMGUM_ATTR_POS, 3, GL_FLOAT, GL_FALSE,
			     sizeof (bottomgum_vertex),
			     (void*) bottomgum_vertices[0].pos);
      glEnableVertexAttribArray (BOTTOMGUM_ATTR_POS);

      glVertexAttribPointer (BOTTOMGUM_ATTR_NORM, 3, GL_FLOAT, GL_FALSE,
			     sizeof (bottomgum_vertex),
			     (void*) bottomgum_vertices[0].norm);
      glEnableVertexAttribArray (BOTTOMGUM_ATTR_NORM);

      glDrawArrays (GL_TRIANGLE_STRIP, 0, ARRAYLENGTH (bottomgum_vertices));

      DRAWTOOTH(BL1, bl1)
      DRAWTOOTH(BL2, bl2)
      DRAWTOOTH(BL3, bl3)
      DRAWTOOTH(BL4, bl4)
      DRAWTOOTH(BL5, bl5)
      DRAWTOOTH(BL6, bl6)
      DRAWTOOTH(BL7, bl7)
      DRAWTOOTH(BL8, bl8)

      DRAWTOOTH(BR1, br1)
      DRAWTOOTH(BR2, br2)
      DRAWTOOTH(BR3, br3)
      DRAWTOOTH(BR4, br4)
      DRAWTOOTH(BR5, br5)
      DRAWTOOTH(BR6, br6)
      DRAWTOOTH(BR7, br7)
      DRAWTOOTH(BR8, br8)
    }
#undef DRAWTOOTH
}

void
glass (void)
{
  glVertexAttribPointer (GLASS_ATTR_POS, 3, GL_FLOAT, GL_FALSE,
			 sizeof (glass_vertex),
			 (void*) glass_vertices[0].pos);
  glEnableVertexAttribArray (GLASS_ATTR_POS);

  glVertexAttribPointer (GLASS_ATTR_NORM, 3, GL_FLOAT, GL_FALSE,
			 sizeof (glass_vertex),
			 (void*) glass_vertices[0].norm);
  glEnableVertexAttribArray (GLASS_ATTR_NORM);

  /*printf ("draw array, length %d\n", ARRAYLENGTH (topgum_vertices));*/
  glDrawArrays (GL_TRIANGLE_STRIP, 0, ARRAYLENGTH (glass_vertices));
}

int
main (int argc, char *argv[])
{
  uint32_t width, height;
  EGLConfig ecfg;
  EGLint num_config;

  EGLint attr[] =
    {
      EGL_BUFFER_SIZE, 16,
      EGL_DEPTH_SIZE, 16,
      EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL_NONE
    };

  EGLint ctxattr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

  static EGL_DISPMANX_WINDOW_T nativewindow;
  int success;
  int quit = 0;

  DISPMANX_ELEMENT_HANDLE_T dispman_element;
  DISPMANX_DISPLAY_HANDLE_T dispman_display;
  DISPMANX_UPDATE_HANDLE_T dispman_update;
  VC_RECT_T dst_rect;
  VC_RECT_T src_rect;
  
  GLuint shaderProgram;
  GLfloat view[16];
  GLfloat model[16];
  GLfloat modelview[16];
  GLfloat inv_modelview[16];
  GLfloat ident[16];
  ttd_point3d light0 = { 20, 20, -20 };
  ttd_point3d light0_t;
  float angle = 0;

  edpy = eglGetDisplay ((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);

  if (edpy == EGL_NO_DISPLAY)
    {
      fprintf (stderr, "Got no EGL display\n");
      return 1;
    }
	
  if (!eglInitialize (edpy, NULL, NULL))
    {
      fprintf (stderr, "Unable to initialize EGL (%d)\n", eglGetError ());
      return 1;
    }

  if (!eglChooseConfig (edpy, attr, &ecfg, 1, &num_config))
    {
      fprintf (stderr, "Failed to choose config (%x)\n", eglGetError());
      return 1;
    }

  if (num_config != 1)
    {
      fprintf (stderr, "Didn't get exactly one config, but %d\n", num_config);
      //return 1;
    }

  bcm_host_init ();

  success = graphics_get_display_size (0 /* LCD */, &width, &height);
  assert (success >= 0);

  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = width;
  dst_rect.height = height;

  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = width << 16;
  src_rect.height = height << 16;

  dispman_display = vc_dispmanx_display_open (0 /* LCD */);
  dispman_update = vc_dispmanx_update_start (0);

  dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
    0/*layer*/, &dst_rect, 0/*src*/,
    &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

  nativewindow.element = dispman_element;
  nativewindow.width = width;
  nativewindow.height = height;
  vc_dispmanx_update_submit_sync( dispman_update );

  esfc = eglCreateWindowSurface (edpy, ecfg, &nativewindow, NULL);

  if (esfc == EGL_NO_SURFACE)
    {
      fprintf (stderr, "Unable to create EGL surface (%x)\n", eglGetError());
      return 1;
    }
	
  ectxt = eglCreateContext (edpy, ecfg, EGL_NO_CONTEXT, ctxattr);

  if (ectxt == EGL_NO_CONTEXT)
    {
      fprintf (stderr, "Unable to create EGL context (%x)\n", eglGetError());
      return 1;
    }

  eglMakeCurrent(edpy, esfc, esfc, ectxt);

  {
    const GLubyte *extensions = glGetString (GL_EXTENSIONS);
    
    printf ("GL extensions provided: %s\n",
	    extensions ? (char *) extensions : "none.");
  }

  shaderProgram = create_program_with_shaders ("teeth.vtx", "teeth.frag");

  glBindAttribLocation (shaderProgram, TOPGUM_ATTR_POS, "a_position");
  glBindAttribLocation (shaderProgram, TOPGUM_ATTR_NORM, "a_normal");

  link_and_check (shaderProgram);

  u_mvp = get_uniform_location (shaderProgram, "u_mvp");
  printf ("u_mvp uniform number: %d\n", u_mvp);

  u_lightpos = get_uniform_location (shaderProgram, "u_lightpos");
  printf ("u_lightpos uniform number: %d\n", u_lightpos);

  glEnable (GL_CULL_FACE);
  glCullFace (GL_BACK);  
  
  glEnable (GL_DEPTH_TEST);

  transform_lookat4 (view, &(ttd_point3d) { 0, 0, 15 },
		     &(ttd_point3d) { 0, 0, 0 },
		     &(ttd_point3d) { 0, 1, 0 });

  transform_perspective4 (perspective, 60, (float) width / (float) height,
			  1.0, 200.0);

  transform_identity4 (model);
  transform_identity4 (ident);

  glViewport (0, 0, width, height);
  glUseProgram (shaderProgram);

  glUniform3f (u_lightpos, light0_t.x, light0_t.y, light0_t.z);

  while (!quit)
    {
      float angle2;
      GLfloat chomp[16];
      GLfloat xlate[16];
      static int alt = 0;

      glClearColor (0, 0, 0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

      /*if (alt == 0)
        {
	  transform_identity4 (model);

	  transform_mul4 (modelview, view, model);
	  transform_invert4 (inv_modelview, modelview);
	  transform_point4 (&light0_t, inv_modelview, &light0);
	  transform_mul4 (mvp, perspective, modelview);
	  glUniformMatrix4fv (u_mvp, 1, GL_FALSE, mvp);

	  glass ();
	}
      alt = !alt;*/

      eglSwapBuffers (edpy, esfc);
      angle += 0.02;
    }

  eglDestroyContext(edpy, ectxt);
  eglDestroySurface(edpy, esfc);
  eglTerminate(edpy);
  
  return 0;
}
