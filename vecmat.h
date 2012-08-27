#ifndef TTD_OBJECT_H
#define TTD_OBJECT_H 1

#include <stdio.h>

#include <GLES2/gl2.h>

typedef struct {
  float x, y;
} ttd_point2d;

typedef struct {
  int x, y;
} ttd_ipt2d;

typedef struct {
  float x, y, z, w;
} ttd_point3d;

typedef struct {
  ttd_point3d col[3];
} ttd_matrix;

typedef struct {
  ttd_point3d* pos;
  ttd_point2d* texcoord;
  ttd_point3d* normal;
} ttd_vertex;

#endif
