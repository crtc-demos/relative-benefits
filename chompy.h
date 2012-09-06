#ifndef CHOMPY_H
#define CHOMPY_H 1

#include "timing.h"

typedef struct {
  GLint texture;
  GLuint wood_shader;
  GLint s_texture;
  GLint u_mvp;
} chompy_info;

extern chompy_info chompy_info_0;

extern effect_methods chompy_methods;

#endif
