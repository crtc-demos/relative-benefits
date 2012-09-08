#ifndef TIMING_H
#define TIMING_H 1

#include <stdint.h>
#include <math.h>

#define MAX_BACKBUFFERS 1

typedef enum {
  MAIN_BUFFER
} display_target;

/* These are the elapsed time since the effect in question started, and some
   global parameters which can be used to control stuff.  */

typedef struct {
  uint32_t time_offset;
  float param1;
  float param2;
  float param3;
  float bar_pos;
  int bar;
} sync_info;

typedef struct {
  int width;
  int height;
  int borders;
} display_info;

typedef struct {
  void (*preinit_assets) (void);
  void (*init_effect) (void *params, display_info *disp);
  display_target (*prepare_frame) (sync_info *sync, void *params,
				   int iparam);
  void (*display_effect) (sync_info *sync, void *params, int iparam,
			  display_info *disp);
  void (*composite_effect) (sync_info *sync, void *params, int iparam,
			    display_info *);
  void (*uninit_effect) (void *params, display_info *bbufs);
  void (*finalize) (void *params);
} effect_methods;

typedef struct {
  uint64_t start_time;
  uint64_t end_time;
  effect_methods *methods;
  void *params;
  int iparam;
  int finalize;
} do_thing_at;

static __inline__ float
impulse (float k, float x)
{
  float h = k * x;
  return h * expf (1.0f - h);
}

#ifndef NULL
#define NULL 0
#endif

#endif
