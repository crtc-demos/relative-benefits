#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "adpcm.h"

typedef struct
{
  uint32_t aram_handle;
  uint32_t block_idx[2];
  uint32_t block_valid[2];
  unsigned char *block[2];
  int front_buf;
} aram_buffer_info;

#define ARAM_CACHE_BLOCKSIZE 16384

static aram_buffer_info aram_buffer;

void
aram_cache_init (aram_buffer_info *abuf __attribute__((unused)),
		 uint32_t aram_handle __attribute__((unused)),
		 uint32_t start_idx __attribute__((unused)))
{
}

static unsigned char *whole_song;

/* The ARAM buffering doesn't work on a Wii! Just load the file into the
   (larger amount of) RAM instead.  */

static unsigned char
aram_read (aram_buffer_info *abuf __attribute__((unused)), int offset)
{
  return whole_song[offset];
}

typedef struct
{
  int format;
  int channels;
  int sample_rate;
  int avg_bytes_per_sec;
  int block_align;
  int bits_per_sample;
  int data_length;
} waveformatex;

typedef struct
{
  int block_predictor;
  int idelta;
  int sample1;
  int sample2;
} adpcm_state;

typedef struct
{
  waveformatex header;
  adpcm_state left, right;
  int idx;
  int block_remaining;
  int length;
} playback_state;

static playback_state playback_state_0;

static int
get_halfword (aram_buffer_info *abuf, int offset)
{
  unsigned int b1 = aram_read (abuf, offset);
  unsigned int b2 = aram_read (abuf, offset + 1);
  return b1 + b2 * 256;
}

static int
get_word (aram_buffer_info *abuf, int offset)
{
  unsigned int b1 = aram_read (abuf, offset);
  unsigned int b2 = aram_read (abuf, offset + 1);
  unsigned int b3 = aram_read (abuf, offset + 2);
  unsigned int b4 = aram_read (abuf, offset + 3);
  return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24);
}

static uint32_t
read_header (aram_buffer_info *abuf, playback_state *ps, uint32_t skip_millis)
{
  unsigned int ssize;
  int idx;
  waveformatex header;
  uint32_t frames;

  idx = 12;

  printf ("subchunk 1 id: %c %c %c %c\n", aram_read (abuf, idx),
	      aram_read (abuf, idx + 1), aram_read (abuf, idx + 2),
	      aram_read (abuf, idx + 3));
  ssize = get_word (abuf, idx + 4);
  printf ("subchunk 1 size: %d\n", ssize);
  header.format = get_halfword (abuf, idx + 8);
  printf ("format: %d\n", header.format);
  header.channels = get_halfword (abuf, idx + 10);
  printf ("num channels: %d\n", header.channels);
  header.sample_rate = get_word (abuf, idx + 12);
  printf ("sample rate: %d\n", header.sample_rate);
  header.avg_bytes_per_sec = get_word (abuf, idx + 16);
  printf ("average bytes per sec: %d\n", header.avg_bytes_per_sec);
  header.block_align = get_halfword (abuf, idx + 20);
  printf ("block align (adpcm block size): %d\n", header.block_align);
  header.bits_per_sample = get_halfword (abuf, idx + 22);
  printf ("bits per sample: %d\n", header.bits_per_sample);
  idx += ssize + 8;
  
  printf ("subchunk 2 id: %c %c %c %c\n", aram_read (abuf, idx),
	      aram_read (abuf, idx + 1), aram_read (abuf, idx + 2),
	      aram_read (abuf, idx + 3));
  ssize = get_word (abuf, idx + 4);
  printf ("subchunk 2 size: %d\n", ssize);
  printf ("value: %x (\?\?\?)\n", get_word (abuf, idx + 8));
  idx += ssize + 8;
  
  printf ("subchunk 3 id: %c %c %c %c\n", aram_read (abuf, idx),
	      aram_read (abuf, idx + 1), aram_read (abuf, idx + 2),
	      aram_read (abuf, idx + 3));
  header.data_length = get_word (abuf, idx + 4);
  printf ("subchunk 3 size: %d\n", header.data_length);
  idx += 8;
  
  /* Assumes one byte per (left+right) sample...  */
  frames = (header.sample_rate * skip_millis)
	   / (1000 * (header.block_align - 14));
  
  if (frames > 0)
    printf ("Skipping %u frames\n", frames);
  
  ps->idx = idx + header.block_align * frames;
  ps->header = header;
  
  return skip_millis;
}

static int
extend_16bits (unsigned int in)
{
  if (in < 32768)
    return in;
  else
    return (int) in - 65536;
}

static int
get_signed_halfword (aram_buffer_info *abuf, int offset)
{
  return extend_16bits (get_halfword (abuf, offset));
}

static void
decode_preamble (aram_buffer_info *abuf, playback_state *ps)
{
  int idx = ps->idx;

  ps->left.block_predictor = aram_read (abuf, idx);
  ps->right.block_predictor = aram_read (abuf, idx + 1);
  ps->left.idelta = get_signed_halfword (abuf, idx + 2);
  ps->right.idelta = get_signed_halfword (abuf, idx + 4);
  ps->left.sample1 = get_signed_halfword (abuf, idx + 6);
  ps->right.sample1 = get_signed_halfword (abuf, idx + 8);
  ps->left.sample2 = get_signed_halfword (abuf, idx + 10);
  ps->right.sample2 = get_signed_halfword (abuf, idx + 12);

  ps->idx = idx + 14;
}

const static int adaptation_table[] = {
  230, 230, 230, 230, 307, 409, 512, 614,
  768, 614, 512, 409, 307, 230, 230, 230
};

const static int adapt_coeff_1[] = {
  256, 512, 0, 192, 240, 460, 392
};

const static int adapt_coeff_2[] = {
  0, -256, 0, 64, 0, -208, -232
};

static int
get_sample (adpcm_state *state, int in_sample)
{
  int coeff1 = adapt_coeff_1[state->block_predictor];
  int coeff2 = adapt_coeff_2[state->block_predictor];
  int out_sample = state->sample2;
  int predictor = (state->sample1 * coeff1 + state->sample2 * coeff2) / 256;
  unsigned int in_sample_uns = in_sample >= 0 ? in_sample : in_sample + 16;
  
  predictor += in_sample * state->idelta;
  
  if (predictor < -32768)
    predictor = -32768;
  else if (predictor > 32767)
    predictor = 32767;
  
  state->sample2 = state->sample1;
  state->sample1 = predictor;
  
  state->idelta = (adaptation_table[in_sample_uns] * state->idelta) / 256;
  
  if (state->idelta < 16)
    state->idelta = 16;
  
  return out_sample;
}

static int
extend_4bits (unsigned int in)
{
  if (in < 8)
    return in;
  else
    return (int) in - 16;
}

#define MAX_NUM_BLOCKS 1

static uint32_t aram_blocks[MAX_NUM_BLOCKS];
static int sndPlaying = 0;
static int thr_running = 0;

void
adpcm_init (void)
{
  /*AR_Init (aram_blocks, MAX_NUM_BLOCKS);*/
  sndPlaying = 0;
  thr_running = 0;
}

#define LOAD_BLOCKSIZE 1024

static int adpcm_next_sample (aram_buffer_info *abuf, playback_state *ps,
			      int *lsamp_out, int *rsamp_out);

uint32_t
adpcm_load_file (const char *filename)
{
  struct stat buf;
  FILE *fh;
  uint32_t song;
  unsigned char *mem = memalign (32, LOAD_BLOCKSIZE);
  size_t nread;
  unsigned int song_length;

  if (stat (filename, &buf) != 0)
    {
      printf ("Can't stat songfile\n");
      return 0;
    }

  song_length = buf.st_size;

  
  fh = fopen (filename, "r");
  
  if (!fh)
    {
      printf ("Can't load song!\n");
      return 0;
    }

  whole_song = malloc (song_length);
  
  if (!whole_song)
    {
      printf ("Can't allocate memory\n");
      return 0;
    }
  
  printf ("Loading '%s' into RAM (%d bytes)", filename, song_length);
  
  nread = fread (whole_song, 1, song_length, fh);
  if (nread != song_length)
    printf ("failed!\n");

  fclose (fh);
  
  free (mem);
  
  printf ("\nDone.\n");

  playback_state_0.length = song_length;
  playback_state_0.block_remaining = 0;
  playback_state_0.idx = 0;

  return song;
}

#define STACKSIZE 8192
#define SNDBUFFERSIZE 5760

static volatile uint32_t curr_audio = 0;
/*static uint8_t audioBuf[2][SNDBUFFERSIZE];*/
/*static lwpq_t player_queue;
static lwp_t hplayer;*/
static uint8_t player_stack[STACKSIZE];
static volatile uint32_t filling = 0;

static int
adpcm_next_sample (aram_buffer_info *abuf, playback_state *ps, int *lsamp_out,
		   int *rsamp_out)
{
  unsigned char samp;
  int lsamp, rsamp;

  if (ps->block_remaining == 0)
    {
      int start_idx = ps->idx;
      decode_preamble (abuf, ps);
      ps->block_remaining = ps->header.block_align - (ps->idx - start_idx);
    }
  
  if (ps->idx >= ps->length)
    return 1;
  
  samp = aram_read (abuf, ps->idx++);
  
  lsamp = extend_4bits (samp >> 4);
  rsamp = extend_4bits (samp & 15);
  
  *lsamp_out = get_sample (&ps->left, lsamp);
  *rsamp_out = get_sample (&ps->right, rsamp);
  
  ps->block_remaining--;
  
  return 0;
}

void
adpcm_fill (char *buf, int nsamps)
{
  int i;
  for (i = 0; i < nsamps; i++)
    {
      int lsamp = 0, rsamp = 0;
      adpcm_next_sample (&aram_buffer, &playback_state_0, &lsamp,
			 &rsamp);
      buf[i * 4] = lsamp & 255;
      buf[i * 4 + 1] = lsamp >> 8;
      buf[i * 4 + 2] = rsamp & 255;
      buf[i * 4 + 3] = rsamp >> 8;
    }

  return 0;
}

/*static void
adpcm_voice_callback (AESNDPB *pb, uint32_t state)
{
  switch (state)
    {
    case VOICE_STATE_STOPPED:
    case VOICE_STATE_RUNNING:
      break;
    
    case VOICE_STATE_STREAM:
      if (filling)
        printf ("Underrun detected!\n");
      AESND_SetVoiceBuffer (pb, (void*) audioBuf[curr_audio], SNDBUFFERSIZE);
      curr_audio ^= 1;
      LWP_ThreadSignal (player_queue);
    }
}

static AESNDPB *adpcm_voice;*/

uint32_t
adpcm_play (uint32_t handle, uint32_t skip_to_millis)
{
  uint32_t adjusted_millis;

  if (sndPlaying)
    return skip_to_millis;

  /*adpcm_voice = AESND_AllocateVoice (adpcm_voice_callback);
  if (adpcm_voice)
    {
      AESND_SetVoiceFormat (adpcm_voice, VOICE_STEREO16);
      AESND_SetVoiceFrequency (adpcm_voice, 44100);
      AESND_SetVoiceVolume (adpcm_voice, 255, 255);
      AESND_SetVoiceStream (adpcm_voice, 1);
    }
  else
    {
      printf ("No voice?\n");
      return skip_to_millis;
    }
  
  LWP_InitQueue (&player_queue);*/
  
 /* memset (audioBuf[0], 0, SNDBUFFERSIZE);
  memset (audioBuf[1], 0, SNDBUFFERSIZE);*/
  
  /*DCFlushRange (audioBuf[0], SNDBUFFERSIZE);
  DCFlushRange (audioBuf[1], SNDBUFFERSIZE);*/
  
  aram_cache_init (&aram_buffer, handle, 0);
  
  adjusted_millis = read_header (&aram_buffer, &playback_state_0,
				 skip_to_millis);
  
  aram_cache_init (&aram_buffer, handle, playback_state_0.idx);
  
  while (thr_running);
  
  curr_audio = 0;
  sndPlaying = 1;

  /*if (LWP_CreateThread (&hplayer, player, NULL, player_stack, STACKSIZE, 80)
      != -1)
    {
      AESND_SetVoiceStop (adpcm_voice, 0);
      return skip_to_millis;
    }*/

  sndPlaying = 0;
  
  return adjusted_millis;
}

/* Stop audio, shut up output.  */
void
adpcm_stop (void)
{
  void *thread_ret;

  /*AESND_SetVoiceVolume (adpcm_voice, 0, 0);*/
  
  sndPlaying = 0;
  
  /*LWP_JoinThread (hplayer, &thread_ret);

  AESND_SetVoiceStop (adpcm_voice, 1);
  AESND_SetVoiceStream (adpcm_voice, 0);*/
}
