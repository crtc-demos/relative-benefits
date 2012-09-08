#ifndef SUNDOWN_H
#define SUNDOWN_H 1

#include "timing.h"

typedef struct {
  char *image_data;
  int image_width;
  int image_height;
  GLuint texhandle;
  GLuint shader;
  GLint u_mvp;
  GLint s_texture;
  GLint u_time;
} sundown_info;

extern effect_methods sundown_methods;
extern sundown_info sundown_info_0;

extern void render_screen_quad (display_info *disp, GLint mvp_uniform,
				int borders);

#endif
