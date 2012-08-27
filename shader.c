#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
//#include <GLES2/gl2extimg.h>

#include "shader.h"

/* Pass GL_COMPILE_STATUS or GL_LINK_STATUS for stage_status.  */

static void
print_info_log (GLuint object, GLenum stage_status)
{
  GLint length;
  GLint success;

  if (stage_status == GL_COMPILE_STATUS)
    glGetShaderiv (object, stage_status, &success);
  else
    glGetProgramiv (object, stage_status, &success);

  if (success != GL_TRUE)
    {
      fprintf (stderr, "%s unsuccessful.\n",
        (stage_status == GL_COMPILE_STATUS) ? "Compile"
	: (stage_status == GL_LINK_STATUS) ? "Link" : "Unknown thing");
      
      if (stage_status == GL_COMPILE_STATUS)
	glGetShaderiv (object, GL_INFO_LOG_LENGTH, &length);
      else
        glGetProgramiv (object, GL_INFO_LOG_LENGTH, &length);

      if (length)
	{
	  char* buffer = malloc (length);

          if (stage_status == GL_COMPILE_STATUS)
	    glGetShaderInfoLog (object, length, 0, buffer);
	  else
	    glGetProgramInfoLog (object, length, 0, buffer);

	  fprintf (stderr, "%s", buffer);

	  free (buffer);
	}
      exit(1);
    }
}

GLuint
createShader (GLenum type, const char *pSource)
{
  GLuint shader = glCreateShader (type);

  glShaderSource (shader, 1, &pSource, NULL);
  glCompileShader (shader);
  print_info_log (shader, GL_COMPILE_STATUS);

  return shader;
}

GLuint
load_shader (GLenum type, const char *filename)
{
  struct stat buf;
  int rc;
  char *srcbuf;
  FILE *f;
  GLuint shader;
  
  rc = stat (filename, &buf);
  
  if (rc != 0)
    {
      perror ("load_shader");
      exit (1);
    }
  
  srcbuf = malloc (buf.st_size + 1);
  
  f = fopen (filename, "r");

  if (!f)
    {
      perror ("load_shader");
      exit (1);
    }
  
  fread (srcbuf, 1, buf.st_size, f);

  /* Null-terminating is a really good idea.  */
  srcbuf[buf.st_size] = 0;

  fclose (f);

  shader = createShader (type, srcbuf);
  
  /* Maybe we can't free the source until after linking?  */
  // free (srcbuf);
  
  return shader;
}

char *
load_binary (const char *filename, off_t *size)
{
  struct stat buf;
  FILE *f;
  int rc;
  char *binbuf;
  
  rc = stat (filename, &buf);
  
  if (rc != 0)
    {
      perror ("load_binary");
      exit (1);
    }
  
  fprintf (stderr, "shader '%s' size %d\n", filename, (int) buf.st_size);
  
  binbuf = malloc (buf.st_size);
  
  f = fopen (filename, "r");
  
  if (!f)
    {
      perror ("load_binary");
      exit (1);
    }
  
  fread (binbuf, 1, buf.st_size, f);
  
  fclose (f);
  
  *size = buf.st_size;
  
  return binbuf;
}

/* Load vertex & fragment programs for the "easy" case where they're just used
   by a single program.  */

GLuint
create_program_with_shaders (const char *vertex_prog, const char *fragment_prog)
{
  GLuint program = glCreateProgram ();
  GLuint vertex_shader = load_shader (GL_VERTEX_SHADER, vertex_prog);
  GLuint fragment_shader = load_shader (GL_FRAGMENT_SHADER, fragment_prog);
  
  glAttachShader (program, vertex_shader);
  glAttachShader (program, fragment_shader);
  
  return program;
}

static void
iterate_binary_formats (GLenum format, GLuint *shader, const char *data,
			off_t size)
{
  GLint num_formats = 0;
  GLint *formats_array;
  unsigned int i;

  glGetIntegerv (GL_NUM_SHADER_BINARY_FORMATS, &num_formats);
  
  if (num_formats == 0)
    {
      fprintf (stderr, "Need a binary shader format!\n");
      exit (1);
    }
  
  formats_array = malloc (sizeof (GLint) * num_formats);
  
  glGetIntegerv (GL_SHADER_BINARY_FORMATS, formats_array);
  
  for (i = 0; i < num_formats; i++)
    {
      if (format == formats_array[i])
        {
	  GLenum err;

	  glShaderBinary (1, shader, format, data, size);

	  err = glGetError ();

	  if (err != GL_NO_ERROR)
	    {
	      fprintf (stderr, "Can't load shader (%d)!\n", (int) err);
	      exit (1);
	    }

	  goto success;
	}
    }

  fprintf (stderr, "Couldn't find a suitable shader format\n");

success:  
  free (formats_array);
}

GLuint
create_program_with_binary_shaders (const char *vertex_bin,
				    const char *fragment_bin)
{
  char *bin;
  GLuint vtx_shader, frag_shader;
  off_t size;
  GLuint program;
  
  vtx_shader = glCreateShader (GL_VERTEX_SHADER);
  frag_shader = glCreateShader (GL_FRAGMENT_SHADER);
  
  bin = load_binary (vertex_bin, &size);
  iterate_binary_formats (GL_SGX_BINARY_IMG, &vtx_shader, bin, size);
  free (bin);
  
  bin = load_binary (fragment_bin, &size);
  iterate_binary_formats (GL_SGX_BINARY_IMG, &frag_shader, bin, size);
  free (bin);
  
  program = glCreateProgram ();
  glAttachShader (program, vtx_shader);
  glAttachShader (program, frag_shader);
  
  return program;
}

GLint
get_uniform_location (GLuint program, const char *name)
{
  GLint loc = glGetUniformLocation (program, name);
  
  if (loc == -1)
    {
      fprintf (stderr, "Can't get uniform '%s'\n", name);
      exit (1);
    }
  
  return loc;
}

void
link_and_check (GLuint program)
{
  glLinkProgram (program);
  print_info_log (program, GL_LINK_STATUS);
}
