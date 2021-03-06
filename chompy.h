#ifndef CHOMPY_H
#define CHOMPY_H 1

#include "timing.h"

typedef struct
{
  float r, g, b;
  float x, y, z;
  float vx, vy, vz;
  float tx, ty, tz;
  object_info *obj;
  int randomized;
} dent;

typedef struct {
  GLint texture;
  struct {
    GLuint shader;
    GLint u_mvp;
    GLint u_lightpos;
    GLint u_toothcolour;
  } tooth;
  dent teeth[30];
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
  struct {
    GLuint shader;
    GLint texture;
    GLint u_mvp;
    GLint s_texture;
  } greet;
} chompy_info;

extern chompy_info chompy_info_0;

extern effect_methods chompy_methods;

#endif
