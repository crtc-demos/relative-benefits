#ifndef ATTR_MASKS_H
#define ATTR_MASKS_H 1

#define ATTR_MASK_POS		0x01
#define ATTR_MASK_NORM		0x02
#define ATTR_MASK_VTXCOLOUR	0x04
#define ATTR_MASK_TEXCOORD	0x08
#define ATTR_MASK_BINORM	0x10
#define ATTR_MASK_TANGENT	0x20
#define ATTR_LAST		0x20

typedef struct {
  unsigned int attrs;
  struct {
    int pos;
    int norm;
    int vtxcolour;
    int texcoord;
    int binorm;
    int tangent;
  } attr;
  size_t vertex_size;
  struct {
    void *pos;
    void *norm;
    void *vtxcolour;
    void *texcoord;
    void *binorm;
    void *tangent;
  } data;
  int striplength;
  GLuint shader_program;
} object_info;

#define ARRAY_SIZE(X) (sizeof (X) / sizeof ((X)[0]))

#define INSTANTIATE(OBJNAME,INCUP,INCLO)			\
  object_info OBJNAME = {					\
    .attrs = INCUP##_ATTRS,					\
    .attr = {							\
      .pos = INCUP##_ATTR_POS,					\
      .norm = INCUP##_ATTR_NORM,				\
      .vtxcolour = INCUP##_ATTR_COLOUR,				\
      .texcoord = INCUP##_ATTR_TEXCOORD,			\
      .binorm = INCUP##_ATTR_BINORM,				\
      .tangent = INCUP##_ATTR_TANGENT,				\
    },								\
    .vertex_size = sizeof (INCLO##_vertex),			\
    .data = {							\
      .pos = INCUP##_DATA_POS,					\
      .norm = INCUP##_DATA_NORM,				\
      .vtxcolour = INCUP##_DATA_COLOUR,				\
      .texcoord = INCUP##_DATA_TEXCOORD,			\
      .binorm = INCUP##_DATA_BINORM,				\
      .tangent = INCUP##_DATA_TANGENT,				\
    },								\
    .striplength = ARRAY_SIZE (INCLO##_vertices)		\
  }

#endif
