#ifndef TRANSFORM_H
#define TRANSFORM_H 1

#include <stdlib.h>
#include <GLES2/gl2.h>

#include "vecmat.h"

extern void transform_identitymatrix(ttd_matrix* mat);

extern void transform_zerovector(ttd_point3d* vec);

extern void transform_point(ttd_point3d* dest, ttd_matrix* mat,
  ttd_point3d* src);

extern void transform_transpose_point4(ttd_point3d* dest, GLfloat mat[16],
  ttd_point3d* src);

extern void transform_point4(ttd_point3d* dest, GLfloat mat[16],
  ttd_point3d* src);

extern void transform_glpoint4 (GLfloat dst[4], GLfloat mat[16],
				GLfloat src[4]);

extern void transform_matrix(ttd_matrix* dest, ttd_matrix* by, ttd_matrix* src);

extern void transform_genmatrix(ttd_matrix* dest, ttd_point3d* axis,
  float angle);

extern void transform_rotate2d(ttd_point2d* dest, ttd_point2d* src,
  float angle);

extern void transform_transposematrix(ttd_matrix* dest, ttd_matrix* src);

extern void transform_invertmatrix(ttd_matrix* dest, ttd_matrix* src);

extern void transform_invert4 (GLfloat dst[16], GLfloat src[16]);

extern void transform_crossprod (ttd_point3d* r, const ttd_point3d* u,
				 const ttd_point3d* v);
  
extern float transform_dotprod (const ttd_point3d* a, const ttd_point3d* b);

extern void transform_addvec (ttd_point3d*, const ttd_point3d*,
			      const ttd_point3d*);

extern void transform_subvec (ttd_point3d*, const ttd_point3d*,
			      const ttd_point3d*);

extern void transform_normalize(ttd_point3d* dest, ttd_point3d* pt);

extern float transform_veclength(ttd_point3d* vec);

extern void transform_cleanmatrix(ttd_matrix* mat);

extern void transform_glmultmatrix(ttd_matrix* matrix);

extern void transform_glmatrix_from_matrix (GLfloat out[16], ttd_matrix *in,
					    ttd_point3d *translation);

extern void transform_identity4(GLfloat out[16]);

extern void transform_mul4(GLfloat out[16], GLfloat a[16], GLfloat b[16]);

extern void transform_lookat4(GLfloat mat[16], const ttd_point3d* eye,
  const ttd_point3d* centre, const ttd_point3d* up);
  
extern void transform_frustum4(GLfloat mat[16], GLfloat left, GLfloat right,
  GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

extern void transform_perspective4 (GLfloat mat[16], GLfloat fovy,
				    GLfloat aspect, GLfloat nearz,
				    GLfloat farz);

extern void transform_translate4 (GLfloat dst[16], GLfloat mat[16],
				  GLfloat dx, GLfloat dy, GLfloat dz);

extern void transform_rotate4 (GLfloat dst[16], GLfloat mat[16],
                               GLfloat ax, GLfloat ay, GLfloat az,
                               GLfloat angle);

extern void transform_translate4_mat (GLfloat dst[16],
				      GLfloat dx, GLfloat dy, GLfloat dz);

extern void transform_rotate4_mat (GLfloat dst[16],
				   GLfloat ax, GLfloat ay, GLfloat az,
				   GLfloat angle);

extern void transform_rot_only4 (GLfloat dst[16], GLfloat src[16]);

extern void transform_rotatetovector(ttd_point3d* from, ttd_point3d* to,
  ttd_point3d* axis, float* angle);

extern float transform_distance(ttd_point3d* a, ttd_point3d* b);

extern void transform_writevector(FILE* to, ttd_point3d* vec);

extern void transform_writematrix(FILE* to, ttd_matrix* mat);

#endif
