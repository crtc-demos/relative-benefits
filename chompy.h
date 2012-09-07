#ifndef CHOMPY_H
#define CHOMPY_H 1

#include "timing.h"

typedef struct {
  GLint texture;
  struct {
    GLuint shader;
    GLint u_mvp;
    GLint u_lightpos;
    GLint u_toothcolour;
  } tooth;
  struct {
    float r, g, b;
  } teeth[30];
  struct {
    GLuint shader;
    GLint u_mvp;
    GLint u_lightpos;
  } gum;
  struct {
    GLuint shader;
    GLint s_texture;
    GLint u_mvp;
  } wood;
  struct {
    GLuint shader;
    GLint u_mvp;
  } glass;
  float drop;
} chompy_info;

extern chompy_info chompy_info_0;

extern effect_methods chompy_methods;

#endif
