/* Code stolen from "A description on how to use and modify libpng"
 * Glenn Randers-Pehrson 1999
 */

#include <stdlib.h>

#include <png.h>
#include <GLES2/gl2.h>

#include "readpng.h"

static void read_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
  // fprintf(stderr, "*");
}

char* readpng_image(char* filename, int* width, int* height)
{
  FILE* fp = fopen(filename, "rb");
  const int number = 8;
  png_structp png_ptr;
  png_infop info_ptr, end_info;
  png_bytepp row_pointers;
  unsigned char header[8];
  int is_png;
  int colour_type, bit_depth, image_width, image_height;
  char* pixels;
  int x, y;
  int nchannels;

  if (!fp)
  {
    fprintf(stderr, "Can't load file %s\n", filename);
    return 0;
  }
  
  fread(header, 1, number, fp);
  is_png = !png_sig_cmp(header, 0, number);
  if (!is_png)
  {
    fprintf(stderr, "File %s doesn't look like a PNG\n", filename);
    fclose(fp);
    return 0;
  }
  
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
    (png_voidp)NULL, NULL, NULL);

  if (!png_ptr)
  {
    fprintf(stderr, "Problem creating PNG read struct for %s\n", filename);
    fclose(fp);
    return 0;
  }
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fprintf(stderr, "Problem creating PNG info struct for %s\n", filename);
    fclose(fp);
    return 0;
  }
  
  end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fprintf(stderr, "Problem creating PNG end struct for %s\n", filename);
    fclose(fp);
    return 0;
  }
  
  if (setjmp(png_ptr->jmpbuf))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fprintf(stderr, "Problem loading PNG %s\n", filename);
    fclose(fp);
    return 0;
  }
  
  png_init_io(png_ptr, fp);
  
  png_set_sig_bytes(png_ptr, number);
  
  png_set_read_status_fn(png_ptr, read_row_callback);
  
  png_read_info(png_ptr, info_ptr);
  
  colour_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  image_width = png_get_image_width(png_ptr, info_ptr);
  image_height = png_get_image_height(png_ptr, info_ptr);
  
  if (colour_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8)
    png_set_expand(png_ptr);
  
  /* update stuff */
  png_read_update_info(png_ptr, info_ptr);
  
  /* now image data (width, height) etc is ready to be used */
  fprintf(stderr, "Reading image: %dx%dx%d\n",
    image_width, image_height, bit_depth);
  
  switch (colour_type)
    {
    case PNG_COLOR_TYPE_RGB:
      nchannels = 3;
      break;
    case PNG_COLOR_TYPE_RGBA:
      nchannels = 4;
      break;
    default:
      abort ();
    }
  
  row_pointers = calloc(image_height, sizeof(png_bytepp));
  for (y=0; y<image_height; y++)
  {
    /* FIXME number of channels */
    row_pointers[y] = malloc (nchannels * image_width);
  }
  
  png_read_image(png_ptr, row_pointers);
  
  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
  
  fprintf(stderr, "\n");

  pixels = malloc(3*image_width*image_height);
  
  for (y=0; y<image_height; y++)
  {
    char* destrow = &pixels[y*image_width*3];
    unsigned char* srcrow = row_pointers[y];

    switch (colour_type)
      {
      case PNG_COLOR_TYPE_RGB:
	for (x = 0; x < image_width * 3; x++)
	  destrow[x] = srcrow[x];
	break;
      case PNG_COLOR_TYPE_RGBA:
	for (x = 0; x < image_width * 3; x++)
	  destrow[x] = srcrow[(x * 4) / 3];
	break;
      default:
        abort ();
      }
  }
  
  for (y=0; y<image_height; y++)
  {
    free(row_pointers[y]);
  }
  free(row_pointers);

  if (width) *width = image_width;
  if (height) *height = image_height;
    
  return pixels;
}

GLuint readpng_bindgl2d(char* pixels, int width, int height)
{
  GLuint texture;
  
  glGenTextures (1, &texture);
  glBindTexture (GL_TEXTURE_2D, texture);
  
  glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  
  glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  return texture;
}

