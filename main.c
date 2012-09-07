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

#include "IL/OMX_Broadcom.h"
#include "interface/vcos/vcos.h"

#include "vecmat.h"
#include "transform.h"
#include "shader.h"
#include "omxaudio.h"

#include "attr_masks.h"
#include "objects.h"
#include "timing.h"
#include "chompy.h"
#include "sundown.h"

#undef SKIP_TO_TIME
//#define SKIP_TO_TIME 20000

#ifdef SKIP_TO_TIME
uint64_t offset_time = 0;
#else
#define offset_time 0
#endif

uint64_t start_time;

effect_methods null_effect =
{
  .preinit_assets = NULL,
  .init_effect = NULL,
  .prepare_frame = NULL,
  .display_effect = NULL,
  .uninit_effect = NULL,
  .finalize = NULL
};

static do_thing_at sequence[] = {
  {      0,  11000, &sundown_methods, NULL, -1, 0 },
  {  11000,  60000, &chompy_methods, NULL, -1, 0 },
  {  60000,  62000, &null_effect, NULL, -1, 0 }
};

#define MAX_ACTIVE 20

static EGLDisplay edpy;
static EGLContext ectxt;
static EGLSurface esfc;

uint64_t
gettime (void)
{
  struct timeval now;
  struct timezone tz;

  gettimeofday (&now, &tz);
  
  return (uint64_t) now.tv_sec * 1000 + (uint64_t) now.tv_usec / 1000;
}

uint32_t
diff_msec (uint64_t starttime, uint64_t nowtime)
{
  return (uint32_t) (nowtime - starttime);
}

static int last_bar = -99;

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
  
  unsigned int next_effect;
  unsigned int num_active_effects;
  do_thing_at *active_effects[MAX_ACTIVE];
  const int num_effects = ARRAY_SIZE (sequence);
  int i, j;
  /*backbuffer_info backbuffers[MAX_BACKBUFFERS];*/
  display_info disp;

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

  for (i = 0; i < ARRAY_SIZE (sequence); i++)
    {
      if (sequence[i].methods->preinit_assets)
        sequence[i].methods->preinit_assets ();
    }

  num_active_effects = 0;
  next_effect = 0;

  glEnable (GL_CULL_FACE);
  glCullFace (GL_BACK);  
  
  glEnable (GL_DEPTH_TEST);

  disp.width = width;
  disp.height = height;

  omxaudio_init ();
  {
    OMX_ERRORTYPE err = omxaudio_load ("/home/pi/pidemo/crtc3.wav");
    if (err != OMX_ErrorNone)
      {
        fprintf (stderr, "OMX playback failed to initialise (%x)\n", err);
	exit (1);
      }
  }

  start_time = gettime ();

#ifdef SKIP_TO_TIME
  offset_time = SKIP_TO_TIME;
#endif
  
  omxaudio_play (offset_time);

  while (!quit)
    {
      uint32_t current_time;
      sync_info sync;

      current_time = diff_msec (start_time, gettime ()) + offset_time;

      {
	sync.bar_pos = ((float) current_time / 1000.0)
		       * (171 / 60.0) / 4.0;
	sync.bar = sync.bar_pos;
	sync.bar_pos = sync.bar_pos - floorf (sync.bar_pos);
	if (last_bar != sync.bar)
	  printf ("bar %d\n", sync.bar);
	last_bar = sync.bar;
      }

      /* Terminate old effects.  */
      for (i = 0; i < num_active_effects; i++)
        {
	  if (current_time >= active_effects[i]->end_time)
	    {
	      /*srv_printf ("uninit effect %d (iparam=%d)\n", i,
		      active_effects[i]->iparam);*/

	      if (active_effects[i]->methods->uninit_effect)
	        {
	          active_effects[i]->methods->uninit_effect (
		    active_effects[i]->params, &disp);
		}
	      /* If we're not going to use this one any more, release any
	         global resources (etc.).  */
	      if (active_effects[i]->finalize
		  && active_effects[i]->methods->finalize)
		{
		  active_effects[i]->methods->finalize (
		    active_effects[i]->params);
		}
	      active_effects[i] = NULL;
	    }
	}

      /* And remove from active list.  */
      for (i = 0, j = 0; j < num_active_effects;)
        {
	  active_effects[i] = active_effects[j];

	  if (active_effects[i] == NULL)
	    j++;
	  else
	    {
	      i++;
	      j++;
	    }
	}

      num_active_effects = i;

#ifdef DEBUG
      srv_printf ("start new effects\n");
#endif

      while (next_effect < num_effects
	     && current_time >= sequence[next_effect].start_time)
	{
	  /* Only start the effect if it's before its ending time.  */
	  if (current_time < sequence[next_effect].end_time)
	    {
	      active_effects[num_active_effects] = &sequence[next_effect];

	      /*srv_printf ("init effect %d (%p, iparam=%d)\n", next_effect,
		      sequence[next_effect].methods->init_effect,
		      sequence[next_effect].iparam);*/

#ifdef DEBUG
	      srv_printf ("next effect is number %d (active=%d)\n",
			  next_effect, num_active_effects);
#endif

	      if (sequence[next_effect].methods->init_effect)
		{
		  sequence[next_effect].methods->init_effect (
		    sequence[next_effect].params, &disp);
		}

	      num_active_effects++;
	    }
	  else if (sequence[next_effect].finalize)
	    {
	      /* We must have skipped over an effect: finalize it now.  */
	      if (sequence[next_effect].methods->finalize)
	        {
	          sequence[next_effect].methods->finalize (
		    sequence[next_effect].params);
		}
	    }

	  next_effect++;
	}

      if (next_effect == num_effects && num_active_effects == 0)
        quit = 1;

#ifdef DEBUG
      srv_printf ("prepare frame (active effects=%d)\n", num_active_effects);
#endif
      
      for (i = 0; i < num_active_effects; i++)
        {
	  if (active_effects[i]->methods->prepare_frame)
	    {
              sync.time_offset
	        = current_time - active_effects[i]->start_time;

#ifdef DEBUG
	      srv_printf ("prepare frame: %p\n",
		      active_effects[i]->methods->prepare_frame);
#endif
	      active_effects[i]->methods->prepare_frame (
		    &sync, active_effects[i]->params,
		    active_effects[i]->iparam);
	    }
	}

      glClearColor (0, 0, 0, 1);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef DEBUG
      srv_printf ("begin frame\n");
#endif

      for (i = 0; i < num_active_effects; i++)
	if (active_effects[i]->methods->display_effect)
	  {
	    sync.time_offset = current_time - active_effects[i]->start_time;
	    active_effects[i]->methods->display_effect (
	      &sync, active_effects[i]->params, active_effects[i]->iparam,
	      &disp);
	  }

      eglSwapBuffers (edpy, esfc);
    }

  eglDestroyContext(edpy, ectxt);
  eglDestroySurface(edpy, esfc);
  eglTerminate(edpy);
  
  return 0;
}
