#include <GLES2/gl2.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "transform.h"

void transform_identitymatrix(ttd_matrix* mat)
{
  int i;
  
  for (i=0; i<3; i++)
  {
    mat->col[i].x = (i==0);
    mat->col[i].y = (i==1);
    mat->col[i].z = (i==2);
  }
}

void transform_zerovector(ttd_point3d* vec)
{
  vec->x = vec->y = vec->z = 0;
}

void transform_point(ttd_point3d* dest, ttd_matrix* mat, ttd_point3d* src)
{
  /* making copies of these stops them from being reloaded */
  float x=src->x, y=src->y, z=src->z;
  
  dest->x = mat->col[0].x * x
          + mat->col[1].x * y
          + mat->col[2].x * z;

  dest->y = mat->col[0].y * x
          + mat->col[1].y * y
          + mat->col[2].y * z;

  dest->z = mat->col[0].z * x
          + mat->col[1].z * y
          + mat->col[2].z * z;
}

/* transform pt (dest,1) by 4x4 GL-style matrix */
void transform_transpose_point4 (ttd_point3d* dest, GLfloat mat[16],
				 ttd_point3d* src)
{
  /* making copies of these stops them from being reloaded */
  float x=src->x, y=src->y, z=src->z;
  
  dest->x = mat[0] * x
          + mat[1] * y
          + mat[2] * z
          + mat[3];

  dest->y = mat[4] * x
          + mat[5] * y
          + mat[6] * z
          + mat[7];

  dest->z = mat[8] * x
          + mat[9] * y
          + mat[10] * z
          + mat[11];
}

void transform_point4 (ttd_point3d* dest, GLfloat mat[16], ttd_point3d* src)
{
  /* making copies of these stops them from being reloaded */
  float x=src->x, y=src->y, z=src->z;
  
  dest->x = mat[0] * x
          + mat[4] * y
          + mat[8] * z
          + mat[12];

  dest->y = mat[1] * x
          + mat[5] * y
          + mat[9] * z
          + mat[13];

  dest->z = mat[2] * x
          + mat[6] * y
          + mat[10] * z
          + mat[14];
}

void transform_glpoint4 (GLfloat dest[4], GLfloat mat[16], GLfloat src[4])
{
  float x = src[0], y = src[1], z = src[2], w = src[3];
  
  dest[0] = mat[0] * x + mat[4] * y +  mat[8] * z + mat[12] * w;
  dest[1] = mat[1] * x + mat[5] * y +  mat[9] * z + mat[13] * w;
  dest[2] = mat[2] * x + mat[6] * y + mat[10] * z + mat[14] * w;
  dest[3] = mat[3] * x + mat[7] * y + mat[11] * z + mat[15] * w;
}

void transform_matrix(ttd_matrix* dest, ttd_matrix* a, ttd_matrix* b)
{
  float s1 = a->col[0].x, s2 = a->col[1].x, s3 = a->col[2].x;
  float s4 = a->col[0].y, s5 = a->col[1].y, s6 = a->col[2].y;
  float s7 = a->col[0].z, s8 = a->col[1].z, s9 = a->col[2].z;

  float t1 = b->col[0].x, t2 = b->col[1].x, t3 = b->col[2].x;
  float t4 = b->col[0].y, t5 = b->col[1].y, t6 = b->col[2].y;
  float t7 = b->col[0].z, t8 = b->col[1].z, t9 = b->col[2].z;

  dest->col[0].x = s1*t1 + s2*t4 + s3*t7;
  dest->col[0].y = s4*t1 + s5*t4 + s6*t7;
  dest->col[0].z = s7*t1 + s8*t4 + s9*t7;  

  dest->col[1].x = s1*t2 + s2*t5 + s3*t8;
  dest->col[1].y = s4*t2 + s5*t5 + s6*t8;
  dest->col[1].z = s7*t2 + s8*t5 + s9*t8;

  dest->col[2].x = s1*t3 + s2*t6 + s3*t9;
  dest->col[2].y = s4*t3 + s5*t6 + s6*t9;
  dest->col[2].z = s7*t3 + s8*t6 + s9*t9;  
}

void transform_genmatrix(ttd_matrix* dest, ttd_point3d* axis, float angle)
{
  float s = sinf (angle), c = cosf (angle);
  float v = 1-c, sx = axis->x, sy = axis->y, sz = axis->z;

  dest->col[0].x = sx*sx*v + c;
  dest->col[1].x = sx*sy*v - sz*s;
  dest->col[2].x = sx*sz*v + sy*s;
  
  dest->col[0].y = sx*sy*v + sz*s;
  dest->col[1].y = sy*sy*v + c;
  dest->col[2].y = sy*sz*v - sx*s;
  
  dest->col[0].z = sx*sz*v - sy*s;
  dest->col[1].z = sy*sz*v + sx*s;
  dest->col[2].z = sz*sz*v + c;

/* can't find a derivation for this any more...
  float x, y, z, w;
  float sinang = sin(angle);
  
  x = sinang * axis->x;
  y = sinang * axis->y;
  z = sinang * axis->z;
  w = cos(angle);
  
  dest->col[0].x = 1.0 - (y*y + z*z)*2;
  dest->col[0].y =       (x*y + w*z)*2;
  dest->col[0].z =       (x*z + w*y)*2;
  
  dest->col[1].x =       (x*y - w*z)*2;
  dest->col[1].y = 1.0 - (x*x + z*z)*2;
  dest->col[1].z =       (y*z + w*x)*2;
  
  dest->col[2].x =       (x*z + w*y)*2;
  dest->col[2].y =       (y*z - w*x)*2;
  dest->col[2].z = 1.0 - (x*x + y*y)*2;
  */
}

void transform_rotate2d(ttd_point2d* dest, ttd_point2d* src, float angle)
{
  float cosa = cos(angle), sina = sin(angle);
  float sx = src->x, sy = src->y;
  dest->x = cosa * sx - sina * sy;
  dest->y = sina * sx + cosa * sy;
}

void transform_transposematrix(ttd_matrix* dest, ttd_matrix* src)
{
  dest->col[0].x = src->col[0].x;
  dest->col[0].y = src->col[1].x;
  dest->col[0].z = src->col[2].x;
  
  dest->col[1].x = src->col[0].y;
  dest->col[1].y = src->col[1].y;
  dest->col[1].z = src->col[2].y;
  
  dest->col[2].x = src->col[0].z;
  dest->col[2].y = src->col[1].z;
  dest->col[2].z = src->col[2].z;
}

void transform_invertmatrix(ttd_matrix* dest, ttd_matrix* src)
{
  float e11 = src->col[0].x, e12 = src->col[1].x, e13 = src->col[2].x;
  float e21 = src->col[0].y, e22 = src->col[1].y, e23 = src->col[2].y;
  float e31 = src->col[0].z, e32 = src->col[1].z, e33 = src->col[2].z;
  float det = e11*e22*e33 - e11*e32*e23 +
               e21*e32*e13 - e21*e12*e33 +
               e31*e12*e23 - e31*e22*e13;
  float idet = (det==0) ? 1.0 : 1.0/det;
  
  dest->col[0].x =  (e22*e33 - e23*e32) * idet;
  dest->col[1].x = -(e12*e33 - e13*e32) * idet;
  dest->col[2].x =  (e12*e23 - e13*e22) * idet;
  
  dest->col[0].y = -(e21*e33 - e23*e31) * idet;
  dest->col[1].y =  (e11*e33 - e13*e31) * idet;
  dest->col[2].y = -(e11*e23 - e13*e21) * idet;
  
  dest->col[0].z =  (e21*e32 - e22*e31) * idet;
  dest->col[1].z = -(e11*e32 - e12*e31) * idet;
  dest->col[2].z =  (e11*e22 - e12*e21) * idet;
}

/* stolen from Intel, tsk tsk
 * Matrix inversion using Cramer's rule
 */
void transform_invert4(GLfloat dst[16], GLfloat mat[16])
{
  GLfloat tmp[12];
  GLfloat src[16];
  GLfloat det;
  int i;
  
  /* transpose matrix */
  for (i = 0; i < 4; i++)
  {
    src[i] = mat[i*4];
    src[i + 4] = mat[i*4 + 1];
    src[i + 8] = mat[i*4 + 2];
    src[i + 12] = mat[i*4 + 3];
  }
  
  /* calculate pairs for first 8 elements (cofactors) */
  tmp[0] = src[10] * src[15];
  tmp[1] = src[11] * src[14];
  tmp[2] = src[9] * src[15];
  tmp[3] = src[11] * src[13];
  tmp[4] = src[9] * src[14];
  tmp[5] = src[10] * src[13];
  tmp[6] = src[8] * src[15];
  tmp[7] = src[11] * src[12];
  tmp[8] = src[8] * src[14];
  tmp[9] = src[10] * src[12];
  tmp[10] = src[8] * src[13];
  tmp[11] = src[9] * src[12];
  
  /* calculate first 8 elements (cofactors) */
  dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7]
         - tmp[1]*src[5] - tmp[2]*src[6] - tmp[5]*src[7];
  dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7]
         - tmp[0]*src[4] - tmp[7]*src[6] - tmp[8]*src[7];
  dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7]
         - tmp[3]*src[4] - tmp[6]*src[5] - tmp[11]*src[7];
  dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6]
         - tmp[4]*src[4] - tmp[9]*src[5] - tmp[10]*src[6];
  dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3]
         - tmp[0]*src[1] - tmp[3]*src[2] - tmp[4]*src[3];
  dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3]
         - tmp[1]*src[0] - tmp[6]*src[2] - tmp[9]*src[3];
  dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3]
         - tmp[2]*src[0] - tmp[7]*src[1] - tmp[10]*src[3];
  dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2]
         - tmp[5]*src[0] - tmp[8]*src[1] - tmp[11]*src[2];

  /* calculate pairs for second 8 elements (cofactors) */
  tmp[0] = src[2]*src[7];
  tmp[1] = src[3]*src[6];
  tmp[2] = src[1]*src[7];
  tmp[3] = src[3]*src[5];
  tmp[4] = src[1]*src[6];
  tmp[5] = src[2]*src[5];
  tmp[6] = src[0]*src[7];
  tmp[7] = src[3]*src[4];
  tmp[8] = src[0]*src[6];
  tmp[9] = src[2]*src[4];
  tmp[10] = src[0]*src[5];
  tmp[11] = src[1]*src[4];

  /* calculate second 8 elements (cofactors) */
  dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15]
         - tmp[1]*src[13] - tmp[2]*src[14] - tmp[5]*src[15];
  dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15]
         - tmp[0]*src[12] - tmp[7]*src[14] - tmp[8]*src[15];
  dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15]
          - tmp[3]*src[12] - tmp[6]*src[13] - tmp[11]*src[15];
  dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14]
          - tmp[4]*src[12] - tmp[9]*src[13] - tmp[10]*src[14];
  dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9]
          - tmp[4]*src[11] - tmp[0]*src[9] - tmp[3]*src[10];
  dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10]
          - tmp[6]*src[10] - tmp[9]*src[11] - tmp[1]*src[8];
  dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8]
          - tmp[10]*src[11] - tmp[2]*src[8] - tmp[7]*src[9];
  dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9]
          - tmp[8]*src[9] - tmp[11]*src[10] - tmp[5]*src[8];

  /* calculate determinant */
  det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];

  /* calculate matrix inverse */
  det = 1/det;
  for (i = 0; i < 16; i++)
    dst[i] *= det;
}

void
transform_crossprod (ttd_point3d* r, const ttd_point3d* u, const ttd_point3d* v)
{
  float ux = u->x, uy = u->y, uz = u->z, vx = v->x, vy = v->y, vz = v->z;
  r->x = uy*vz - uz*vy;
  r->y = uz*vx - ux*vz;
  r->z = ux*vy - uy*vx;
}

float
transform_dotprod (const ttd_point3d* a, const ttd_point3d* b)
{
  return a->x*b->x + a->y*b->y + a->z*b->z;
}

void
transform_addvec (ttd_point3d* dest, const ttd_point3d* src1,
		  const ttd_point3d* src2)
{
  float x0 = src1->x, y0 = src1->y, z0 = src1->z;
  float x1 = src2->x, y1 = src2->y, z1 = src2->z;

  dest->x = x0 + x1;
  dest->y = y0 + y1;
  dest->z = z0 + z1;
}

void
transform_subvec (ttd_point3d* dest, const ttd_point3d* src1,
		  const ttd_point3d* src2)
{
  float x0 = src1->x, y0 = src1->y, z0 = src1->z;
  float x1 = src2->x, y1 = src2->y, z1 = src2->z;

  dest->x = x0 - x1;
  dest->y = y0 - y1;
  dest->z = z0 - z1;
}

void transform_normalize(ttd_point3d* dest, ttd_point3d* pt)
{
  float x = pt->x, y = pt->y, z = pt->z;
  float rlen = 1.0/sqrt(x*x + y*y + z*z);
  
  dest->x = x * rlen;
  dest->y = y * rlen;
  dest->z = z * rlen;
}

float transform_veclength(ttd_point3d* vec)
{
  float x = vec->x, y = vec->y, z = vec->z;
  return sqrt(x*x + y*y + z*z);
}

/* fix matrix drifting away from orthogonality */
void transform_cleanmatrix(ttd_matrix* m)
{
  transform_crossprod(&m->col[2], &m->col[0], &m->col[1]);
  transform_crossprod(&m->col[0], &m->col[1], &m->col[2]);
  transform_normalize(&m->col[0], &m->col[0]);
  transform_normalize(&m->col[1], &m->col[1]);
  transform_normalize(&m->col[2], &m->col[2]);
}

void
transform_glmatrix_from_matrix (GLfloat out[16], ttd_matrix *mat,
				ttd_point3d *translation)
{
  out[0] = mat->col[0].x;
  out[1] = mat->col[0].y;
  out[2] = mat->col[0].z;
  out[3] = 0.0;
  
  out[4] = mat->col[1].x;
  out[5] = mat->col[1].y;
  out[6] = mat->col[1].z;
  out[7] = 0.0;
  
  out[8] = mat->col[2].x;
  out[9] = mat->col[2].y;
  out[10] = mat->col[2].z;
  out[11] = 0.0;
  
  out[12] = translation->x;
  out[13] = translation->y;
  out[14] = translation->z;
  out[15] = 1.0;
}

void transform_identity4(GLfloat out[16])
{
  int i, j;
  for (j=0; j<4; j++)
  {
    for (i=0; i<4; i++)
    {
      out[j*4+i] = (i==j) ? 1.0 : 0.0;
    }
  }
}

/* GL-equivalent matrix multiply */
void transform_mul4(GLfloat out[16], GLfloat a[16], GLfloat b[16])
{
  int i, j, k;
  GLfloat tmp[16];
  
  for (i=0; i<4; i++)
  {
    for (j=0; j<4; j++)
    {
      GLfloat sum = 0.0;
      for (k=0; k<4; k++)
      {
        sum += a[j+k*4] * b[i*4+k];
      }
      tmp[j+i*4] = sum;
    }
  }
  
  memcpy (out, tmp, sizeof (GLfloat) * 16);
}

/* GL-equivalent frustum */
void transform_frustum4 (GLfloat mat[16], GLfloat left, GLfloat right,
  GLfloat bottom, GLfloat top, GLfloat near, GLfloat far)
{
  GLfloat a = (right+left)/(right-left);
  GLfloat b = (top+bottom)/(top-bottom);
  GLfloat c = (far+near)/(far-near);
  GLfloat d = (-2*far*near)/(far-near);

  mat[0] = 2*near / (right-left);
  mat[4] = 0;
  mat[8] = 0;
  mat[12] = 0;

  mat[1] = 0;
  mat[5] = 2*near / (top-bottom);
  mat[9] = 0;
  mat[13] = 0;

  mat[2] = a;
  mat[6] = b;
  mat[10] = c;
  mat[14] = -1;

  mat[3] = 0;
  mat[7] = 0;
  mat[11] = d;
  mat[15] = 0;
}

void transform_perspective4 (GLfloat mat[16], GLfloat fovy, GLfloat aspect,
			    GLfloat nearz, GLfloat farz)
{
  float sine, cot, fov_rad = fovy / 360.0 * M_PI, deltaz;
  
  deltaz = farz - nearz;
  sine = sin (fov_rad);
  cot = cos (fov_rad) / sine;
    
  memset (mat, 0, sizeof (GLfloat) * 16);
  mat[0] = cot / aspect;
  mat[5] = cot;
  mat[10] = -(farz + nearz) / deltaz;
  mat[11] = -1;
  mat[14] = -2 * nearz * farz / deltaz;
  mat[15] = 0;
}

void transform_translate4 (GLfloat dst[16], GLfloat mat[16],
			   GLfloat dx, GLfloat dy, GLfloat dz)
{
#if 1
  memcpy (dst, mat, sizeof (GLfloat) * 12);
  dst[12] = mat[0] * dx + mat[4] * dy + mat[8] * dz + mat[12];
  dst[13] = mat[1] * dx + mat[5] * dy + mat[9] * dz + mat[13];
  dst[14] = mat[2] * dx + mat[6] * dy + mat[10] * dz + mat[14];
  dst[15] = mat[3] * dx + mat[7] * dy + mat[11] * dz + mat[15];
#else
  GLfloat xlat[16];
  
  xlat[0] = 1; xlat[4] = 0; xlat[8] = 0; xlat[12] = dx;
  xlat[1] = 0; xlat[5] = 1; xlat[9] = 0; xlat[13] = dy;
  xlat[2] = 0; xlat[6] = 0; xlat[10] = 1; xlat[14] = dz;
  xlat[3] = 0; xlat[7] = 0; xlat[11] = 0; xlat[15] = 1;
  
  transform_mul4 (dst, mat, xlat);
#endif
}

void transform_translate4_mat (GLfloat dst[16], GLfloat dx, GLfloat dy,
			       GLfloat dz)
{
  dst[0] = 1; dst[4] = 0; dst[8] = 0; dst[12] = dx;
  dst[1] = 0; dst[5] = 1; dst[9] = 0; dst[13] = dy;
  dst[2] = 0; dst[6] = 0; dst[10] = 1; dst[14] = dz;
  dst[3] = 0; dst[7] = 0; dst[11] = 0; dst[15] = 1;
}

/* GL-equivalent lookat */
void transform_lookat4 (GLfloat dst[16], const ttd_point3d* eye,
			const ttd_point3d* centre, const ttd_point3d* up)
{
  ttd_point3d l, s, uq;
  GLfloat mat[16] /*, tmp[16]*/;

  transform_subvec(&l, centre, eye);
  transform_normalize(&l, &l);
  transform_crossprod(&s, &l, up);
  transform_normalize(&s, &s);
  transform_crossprod(&uq, &s, &l);

  /* Note: this is transposed, for the inverse transform.  */
  mat[0] = s.x; mat[1] = uq.x; mat[2] = -l.x;  mat[3] = 0;
  mat[4] = s.y; mat[5] = uq.y; mat[6] = -l.y;  mat[7] = 0;
  mat[8] = s.z; mat[9] = uq.z; mat[10] = -l.z; mat[11] = 0;
  mat[12] = 0;  mat[13] = 0;   mat[14] = 0;    mat[15] = 1;

  transform_translate4 (dst, mat, -eye->x, -eye->y, -eye->z);

  /*for (i = 0; i < 16; i++)
    printf ("%f%s", dst[(i >> 2) + (i & 3) * 4], (i & 3) == 3 ? "\n" : " ");
  printf ("\n");*/

  //memcpy (dst, mat, sizeof (GLfloat) * 16);
}

void transform_rotate4 (GLfloat dst[16], GLfloat mat[16], GLfloat ax,
			GLfloat ay, GLfloat az, GLfloat angle)
{
  ttd_point3d axis = { ax, ay, az };
  ttd_matrix tmp;
  GLfloat rot[16];

  transform_genmatrix (&tmp, &axis, angle);

  rot[0] = tmp.col[0].x;
  rot[4] = tmp.col[1].x;
  rot[8] = tmp.col[2].x;
  rot[12] = 0;

  rot[1] = tmp.col[0].y;
  rot[5] = tmp.col[1].y;
  rot[9] = tmp.col[2].y;
  rot[13] = 0;
  
  rot[2] = tmp.col[0].z;
  rot[6] = tmp.col[1].z;
  rot[10] = tmp.col[2].z;
  rot[14] = 0;
  
  rot[3] = 0;
  rot[7] = 0;
  rot[11] = 0;
  rot[15] = 1;
  
  transform_mul4 (dst, rot, mat);
}

void transform_rotate4_mat (GLfloat dst[16], GLfloat ax, GLfloat ay, GLfloat az,
			GLfloat angle)
{
  ttd_point3d axis = { ax, ay, az };
  ttd_matrix tmp;
    
  transform_genmatrix (&tmp, &axis, angle);

  dst[0] = tmp.col[0].x;
  dst[4] = tmp.col[1].x;
  dst[8] = tmp.col[2].x;
  dst[12] = 0;

  dst[1] = tmp.col[0].y;
  dst[5] = tmp.col[1].y;
  dst[9] = tmp.col[2].y;
  dst[13] = 0;
  
  dst[2] = tmp.col[0].z;
  dst[6] = tmp.col[1].z;
  dst[10] = tmp.col[2].z;
  dst[14] = 0;
  
  dst[3] = 0;
  dst[7] = 0;
  dst[11] = 0;
  dst[15] = 1;
}

void transform_rot_only4 (GLfloat dst[16], GLfloat src[16])
{
  dst[0] = src[0];
  dst[1] = src[1];
  dst[2] = src[2];
  dst[3] = 0.0;
  
  dst[4] = src[4];
  dst[5] = src[5];
  dst[6] = src[6];
  dst[7] = 0.0;
  
  dst[8] = src[8];
  dst[9] = src[9];
  dst[10] = src[10];
  dst[11] = 0.0;
  
  dst[12] = dst[13] = dst[14] = 0.0;
  dst[15] = 1.0;
}

/* given two unit-length vectors from & to, find the axis-angle rotation
   to map 'from' onto 'to' (output angle in radians)
*/
void transform_rotatetovector(ttd_point3d* from, ttd_point3d* to,
  ttd_point3d* axis, float* angle)
{
  ttd_point3d perp;
  int side;
  float temp, rotate;
  
  transform_crossprod(&perp, to, from);
  side = (transform_dotprod(to, from) < 0) ? 1 : 0;
  
  temp = transform_veclength(&perp);

  if (temp<-1.0 || temp>1.0)
  {
    fprintf(stderr, "Bad vectors! Normalise first.\n");
    abort();
  }

  rotate = side ? asin(temp) : M_PI-asin(temp);

  if (isnan(rotate))
  {
    fprintf(stderr, "rotate is nan\n");
    fprintf(stderr, "temp=%f\n", temp);
  }

  transform_normalize(axis, &perp);
  *angle = rotate;
}

/* distance between two points */
float transform_distance(ttd_point3d* a, ttd_point3d* b)
{
  float ax = a->x, ay = a->y, az = a->z;
  float bx = b->x, by = b->y, bz = b->z;
  ax -= bx;
  ay -= by;
  az -= bz;
  return sqrt(ax*ax + ay*ay + az*az);
}

void transform_writevector(FILE* to, ttd_point3d* vec)
{
  fprintf(to, "(%.4f %.4f %.4f)\n", vec->x, vec->y, vec->z);
}

void transform_writematrix(FILE* to, ttd_matrix* mat)
{
  fprintf(to, "(%.4f %.4f %.4f)\n", mat->col[0].x,
                                      mat->col[1].x,
                                      mat->col[2].x);
  fprintf(to, "(%.4f %.4f %.4f)\n", mat->col[0].y,
                                      mat->col[1].y,
                                      mat->col[2].y);
  fprintf(to, "(%.4f %.4f %.4f)\n", mat->col[0].z,
                                      mat->col[1].z,
                                      mat->col[2].z);
}
