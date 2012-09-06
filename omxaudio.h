#ifndef OMXAUDIO_H
#define OMXAUDIO_H 1

extern void omxaudio_init (void);
extern int omxaudio_load (const char *filename);
extern void omxaudio_deinit (void);
extern void omxaudio_play (uint32_t);

#endif
