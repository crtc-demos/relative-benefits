typedef struct {
  GLfloat pos[3];
  GLfloat norm[3];
  GLfloat texcoord[2];
} table_vertex;
static table_vertex table_vertices[] = {
  {
    { 45.096500, -5.713585, -47.119450 },
    { 0.000000, 1.000000, 0.000000 },
    { 0.000000, 1.000000 },
  },
  {
    { -45.502620, -5.713585, -47.119450 },
    { 0.000000, 1.000000, 0.000000 },
    { 0.000000, 1.000000 },
  },
  {
    { 45.096500, -5.713585, 43.479650 },
    { 0.000000, 1.000000, 0.000000 },
    { 0.000000, 1.000000 },
  },
  {
    { -45.502600, -5.713585, 43.479670 },
    { 0.000000, 1.000000, 0.000000 },
    { 0.000000, 1.000000 },
  },
};
#define TABLE_ATTRS (ATTR_MASK_TEXCOORD | ATTR_MASK_POS | ATTR_MASK_NORM)
#define TABLE_ATTR_POS 0
#define TABLE_DATA_POS (void*) table_vertices[0].pos
#define TABLE_ATTR_NORM 1
#define TABLE_DATA_NORM (void*) table_vertices[0].norm
#define TABLE_ATTR_COLOUR -1
#define TABLE_DATA_COLOUR (void*) 0
#define TABLE_ATTR_TEXCOORD 2
#define TABLE_DATA_TEXCOORD (void*) table_vertices[0].texcoord
#define TABLE_ATTR_BINORM -1
#define TABLE_DATA_BINORM (void*) 0
#define TABLE_ATTR_TANGENT -1
#define TABLE_DATA_TANGENT (void*) 0
