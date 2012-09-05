#ifndef ADPCM_H
#define ADPCM_H 1

#include <stdint.h>

extern void adpcm_init (void);
extern uint32_t adpcm_load_file (const char *filename);
extern uint32_t adpcm_play (uint32_t handle, uint32_t skip_to_millis);
extern void adpcm_fill (char *buf, int nsamps);
extern void adpcm_stop (void);

#endif
