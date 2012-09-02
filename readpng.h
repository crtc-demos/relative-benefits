#ifndef READPNG_H
#define READPNG_H 1

#include <png.h>

extern char* readpng_image(char* filename, int* width, int* height);
extern GLuint readpng_bindgl2d(char* pixels, int w, int h);

#endif
