#ifndef SHADER_H
#define SHADER_H 1

extern GLuint load_shader (GLenum type, const char *filename);

extern GLuint create_program_with_shaders (const char *, const char *);

extern GLuint create_program_with_binary_shaders (const char *, const char *);

extern GLint get_uniform_location (GLuint program, const char *name);

extern void link_and_check (GLuint program);

#endif
