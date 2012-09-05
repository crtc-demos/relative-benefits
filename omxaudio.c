#include <assert.h>
#include <stdio.h>

#include "bcm_host.h"
#include "IL/OMX_Broadcom.h"
#include "interface/vcos/vcos.h"

#include "ilclient.h"

#include "omxaudio.h"
#include "adpcm.h"

/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Audio output demo using OpenMAX IL though the ilcient helper library

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>

#include "bcm_host.h"
#include "ilclient.h"

#define N_WAVE          1024    /* dimension of Sinewave[] */
#define PI (1<<16>>1)
#define SIN(x) Sinewave[((x)>>6) & (N_WAVE-1)]
#define COS(x) SIN((x)+(PI>>1))
extern short Sinewave[];

#ifndef countof
   #define countof(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

#define BUFFER_SIZE_SAMPLES 1024

typedef int int32_t;

typedef struct {
   sem_t sema;
   ILCLIENT_T *client;
   COMPONENT_T *audio_render;
   COMPONENT_T *list[2];
   OMX_BUFFERHEADERTYPE *user_buffer_list; // buffers owned by the client
   uint32_t num_buffers;
   uint32_t bytes_per_sample;
} AUDIOPLAY_STATE_T;

static void input_buffer_callback(void *data, COMPONENT_T *comp)
{
   // do nothing - could add a callback to the user
   // to indicate more buffers may be available.
}

int32_t audioplay_create(AUDIOPLAY_STATE_T **handle,
                         uint32_t sample_rate,
                         uint32_t num_channels,
                         uint32_t bit_depth,
                         uint32_t num_buffers,
                         uint32_t buffer_size)
{
   uint32_t bytes_per_sample = (bit_depth * num_channels) >> 3;
   int32_t ret = -1;

   *handle = NULL;

   // basic sanity check on arguments
   if(sample_rate >= 8000 && sample_rate <= 96000 &&
      (num_channels == 1 || num_channels == 2 || num_channels == 4 || num_channels == 8) &&
      (bit_depth == 16 || bit_depth == 32) &&
      num_buffers > 0 &&
      buffer_size >= bytes_per_sample)
   {
      // buffer lengths must be 16 byte aligned for VCHI
      int size = (buffer_size + 15) & ~15;
      AUDIOPLAY_STATE_T *st;

      // buffer offsets must also be 16 byte aligned for VCHI
      st = calloc(1, sizeof(AUDIOPLAY_STATE_T));

      if(st)
      {
         OMX_ERRORTYPE error;
         OMX_PARAM_PORTDEFINITIONTYPE param;
         OMX_AUDIO_PARAM_PCMMODETYPE pcm;
         int32_t s;

         ret = 0;
         *handle = st;

         // create and start up everything
         s = sem_init(&st->sema, 0, 1);
         assert(s == 0);

         st->bytes_per_sample = bytes_per_sample;
         st->num_buffers = num_buffers;

         st->client = ilclient_init();
         assert(st->client != NULL);

         ilclient_set_empty_buffer_done_callback(st->client, input_buffer_callback, st);

         error = OMX_Init();
         assert(error == OMX_ErrorNone);

         ilclient_create_component(st->client, &st->audio_render, "audio_render", ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_DISABLE_ALL_PORTS);
         assert(st->audio_render != NULL);

         st->list[0] = st->audio_render;

         // set up the number/size of buffers
         memset(&param, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
         param.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
         param.nVersion.nVersion = OMX_VERSION;
         param.nPortIndex = 100;

         error = OMX_GetParameter(ILC_GET_HANDLE(st->audio_render), OMX_IndexParamPortDefinition, &param);
         assert(error == OMX_ErrorNone);

         param.nBufferSize = size;
         param.nBufferCountActual = num_buffers;

         error = OMX_SetParameter(ILC_GET_HANDLE(st->audio_render), OMX_IndexParamPortDefinition, &param);
         assert(error == OMX_ErrorNone);

         // set the pcm parameters
         memset(&pcm, 0, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
         pcm.nSize = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
         pcm.nVersion.nVersion = OMX_VERSION;
         pcm.nPortIndex = 100;
         pcm.nChannels = num_channels;
         pcm.eNumData = OMX_NumericalDataSigned;
         pcm.eEndian = OMX_EndianLittle;
         pcm.nSamplingRate = sample_rate;
         pcm.bInterleaved = OMX_TRUE;
         pcm.nBitPerSample = bit_depth;
         pcm.ePCMMode = OMX_AUDIO_PCMModeLinear;

         switch(num_channels) {
         case 1:
            pcm.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
            break;
         case 8:
            pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
            pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
            pcm.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
            pcm.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
            pcm.eChannelMapping[4] = OMX_AUDIO_ChannelLR;
            pcm.eChannelMapping[5] = OMX_AUDIO_ChannelRR;
            pcm.eChannelMapping[6] = OMX_AUDIO_ChannelLS;
            pcm.eChannelMapping[7] = OMX_AUDIO_ChannelRS;
            break;
         case 4:
            pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
            pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
            pcm.eChannelMapping[2] = OMX_AUDIO_ChannelLR;
            pcm.eChannelMapping[3] = OMX_AUDIO_ChannelRR;
            break;
         case 2:
            pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
            pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
            break;
         }

         error = OMX_SetParameter(ILC_GET_HANDLE(st->audio_render), OMX_IndexParamAudioPcm, &pcm);
         assert(error == OMX_ErrorNone);

         ilclient_change_component_state(st->audio_render, OMX_StateIdle);
         if(ilclient_enable_port_buffers(st->audio_render, 100, NULL, NULL, NULL) < 0)
         {
            // error
            ilclient_change_component_state(st->audio_render, OMX_StateLoaded);
            ilclient_cleanup_components(st->list);

            error = OMX_Deinit();
            assert(error == OMX_ErrorNone);

            ilclient_destroy(st->client);

            sem_destroy(&st->sema);
            free(st);
            *handle = NULL;
            return -1;
         }

         ilclient_change_component_state(st->audio_render, OMX_StateExecuting);
      }
   }

   return ret;
}

int32_t audioplay_delete(AUDIOPLAY_STATE_T *st)
{
   OMX_ERRORTYPE error;

   ilclient_change_component_state(st->audio_render, OMX_StateIdle);

   error = OMX_SendCommand(ILC_GET_HANDLE(st->audio_render), OMX_CommandStateSet, OMX_StateLoaded, NULL);
   assert(error == OMX_ErrorNone);

   ilclient_disable_port_buffers(st->audio_render, 100, st->user_buffer_list, NULL, NULL);
   ilclient_change_component_state(st->audio_render, OMX_StateLoaded);
   ilclient_cleanup_components(st->list);

   error = OMX_Deinit();
   assert(error == OMX_ErrorNone);

   ilclient_destroy(st->client);

   sem_destroy(&st->sema);
   free(st);

   return 0;
}

uint8_t *audioplay_get_buffer(AUDIOPLAY_STATE_T *st)
{
   OMX_BUFFERHEADERTYPE *hdr = NULL;

   hdr = ilclient_get_input_buffer(st->audio_render, 100, 0);

   if(hdr)
   {
      // put on the user list
      sem_wait(&st->sema);

      hdr->pAppPrivate = st->user_buffer_list;
      st->user_buffer_list = hdr;

      sem_post(&st->sema);
   }

   return hdr ? hdr->pBuffer : NULL;
}

int32_t audioplay_play_buffer(AUDIOPLAY_STATE_T *st,
                              uint8_t *buffer,
                              uint32_t length)
{
   OMX_BUFFERHEADERTYPE *hdr = NULL, *prev = NULL;
   int32_t ret = -1;

   if(length % st->bytes_per_sample)
      return ret;

   sem_wait(&st->sema);

   // search through user list for the right buffer header
   hdr = st->user_buffer_list;
   while(hdr != NULL && hdr->pBuffer != buffer && hdr->nAllocLen < length)
   {
      prev = hdr;
      hdr = hdr->pAppPrivate;
   }

   if(hdr) // we found it, remove from list
   {
      ret = 0;
      if(prev)
         prev->pAppPrivate = hdr->pAppPrivate;
      else
         st->user_buffer_list = hdr->pAppPrivate;
   }

   sem_post(&st->sema);

   if(hdr)
   {
      OMX_ERRORTYPE error;

      hdr->pAppPrivate = NULL;
      hdr->nOffset = 0;
      hdr->nFilledLen = length;

      error = OMX_EmptyThisBuffer(ILC_GET_HANDLE(st->audio_render), hdr);
      assert(error == OMX_ErrorNone);
   }

   return ret;
}

int32_t audioplay_set_dest(AUDIOPLAY_STATE_T *st, const char *name)
{
   int32_t success = -1;
   OMX_CONFIG_BRCMAUDIODESTINATIONTYPE ar_dest;

   if (name && strlen(name) < sizeof(ar_dest.sName))
   {
      OMX_ERRORTYPE error;
      memset(&ar_dest, 0, sizeof(ar_dest));
      ar_dest.nSize = sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE);
      ar_dest.nVersion.nVersion = OMX_VERSION;
      strcpy((char *)ar_dest.sName, name);

      error = OMX_SetConfig(ILC_GET_HANDLE(st->audio_render), OMX_IndexConfigBrcmAudioDestination, &ar_dest);
      assert(error == OMX_ErrorNone);
      success = 0;
   }

   return success;
}


uint32_t audioplay_get_latency(AUDIOPLAY_STATE_T *st)
{
   OMX_PARAM_U32TYPE param;
   OMX_ERRORTYPE error;

   memset(&param, 0, sizeof(OMX_PARAM_U32TYPE));
   param.nSize = sizeof(OMX_PARAM_U32TYPE);
   param.nVersion.nVersion = OMX_VERSION;
   param.nPortIndex = 100;

   error = OMX_GetConfig(ILC_GET_HANDLE(st->audio_render), OMX_IndexConfigAudioRenderingLatency, &param);
   assert(error == OMX_ErrorNone);

   return param.nU32;
}

#define CTTW_SLEEP_TIME 10
#define MIN_LATENCY_TIME 20

static AUDIOPLAY_STATE_T *st;
static uint32_t player_handle;
static const int nchannels = 2;
static const int bitdepth = 16;
static const int samplerate = 44100;

void
omxaudio_init (void)
{
}

void
omxaudio_deinit (void)
{
}

int
omxaudio_load (const char *filename)
{
  int buffer_size = (BUFFER_SIZE_SAMPLES * bitdepth * nchannels) >> 3;
  int32_t ret = audioplay_create (&st, samplerate, nchannels, bitdepth, 10,
    buffer_size);

  player_handle = adpcm_load_file (filename);
  assert (ret == 0);

  return 0;
}

static void *
player_thread (void *arg)
{
  unsigned int i, j, n;
  int buffer_size = (BUFFER_SIZE_SAMPLES * bitdepth * nchannels) >> 3;
  int32_t ret;

  for (n = 0; n < (samplerate * 66) / BUFFER_SIZE_SAMPLES; n++)
    {
      uint8_t *buf;
      uint32_t latency;
      
      while ((buf = audioplay_get_buffer (st)) == NULL)
        usleep (10 * 1000);

      adpcm_fill (buf, BUFFER_SIZE_SAMPLES);

      while ((latency = audioplay_get_latency (st))
	     > (samplerate * (MIN_LATENCY_TIME + CTTW_SLEEP_TIME) / 1000))
        usleep (CTTW_SLEEP_TIME * 1000);

      ret = audioplay_play_buffer (st, buf, buffer_size);
      assert (ret == 0);
    }
  
  audioplay_delete (st);
}

void
omxaudio_play (void)
{
  pthread_t thread;
  int rc;
  int32_t ret;

  ret = audioplay_set_dest (st, "local");
  assert (ret == 0);
  
  adpcm_play (player_handle, 0);

  rc = pthread_create (&thread, NULL, player_thread, NULL);
}


#if 0

void
omxaudio_init (void)
{
  /*OMX_ERRORTYPE error;
  
  error = OMX_Init ();
  assert (error == OMX_ErrorNone);*/
}

ILCLIENT_T *omxaudio;
/*COMPONENT_T *filereader_handle;*/
COMPONENT_T *decode_handle;
COMPONENT_T *render_handle;

/*TUNNEL_T file_to_decode;*/
TUNNEL_T decode_to_render;

void
omxaudio_deinit (void)
{
  OMX_ERRORTYPE error;
  
  ilclient_destroy (omxaudio);
  error = OMX_Deinit ();
  assert (error == OMX_ErrorNone);
}

int
omxaudio_load (const char *filename)
{
  int ret;
  COMPONENT_T *complist[3];
  OMX_ERRORTYPE error;
  /*unsigned int data_len;
  unsigned int packet_size = 65536;*/
  
  omxaudio = ilclient_init ();
  
  OMX_Init ();
  
  /*ret = ilclient_create_component (omxaudio, &filereader_handle, "read_media",
				   ILCLIENT_DISABLE_ALL_PORTS);
  if (ret != 0)
    {
      fprintf (stderr, "couldn't create read_media component\n");
      return 1;
    }*/
  
  ret = ilclient_create_component (omxaudio, &decode_handle, "audio_decode",
				   ILCLIENT_DISABLE_ALL_PORTS
				   | ILCLIENT_ENABLE_INPUT_BUFFERS);
  if (ret != 0)
    {
      fprintf (stderr, "couldn't create audio_decode component\n");
      return 1;
    }

  ret = ilclient_create_component (omxaudio, &render_handle, "audio_render",
				   ILCLIENT_DISABLE_ALL_PORTS);
  if (ret != 0)
    {
      fprintf (stderr, "couldn't create audio_render component\n");
      return 1;
    }
  
  /*{
    size_t ssize = offsetof (OMX_PARAM_CONTENTURITYPE, contentURI)
		   + strlen (filename) + 1;
    OMX_PARAM_CONTENTURITYPE *uriparam = malloc (ssize);
    uriparam->nSize = ssize;
    uriparam->nVersion.nVersion = OMX_VERSION;
    strcpy ((char *) &uriparam->contentURI[0], filename);
    error = OMX_SetParameter (ILC_GET_HANDLE (filereader_handle),
			      OMX_IndexParamContentURI, uriparam);
    free (uriparam);
    if (error != OMX_ErrorNone)
      {
        fprintf (stderr, "Error setting URI\n");
	return 1;
      }
  }*/

  /*{
    OMX_CONFIG_METADATAITEMCOUNTTYPE param;
    param.nSize = sizeof (param);
    param.nVersion.nVersion = OMX_VERSION;
    param.eScopeMode = OMX_MetadataScopeTopLevel;
    param.nScopeSpecifier = 0;
    error = OMX_GetParameter (ILC_GET_HANDLE (filereader_handle),
			      OMX_IndexConfigMetadataItemCount, &param);
    if (error != OMX_ErrorNone)
      {
        fprintf (stderr, "Error getting metadata item count (%x)\n", error);
	return 1;
      }
    fprintf (stderr, "Metadata item count: %d\n", (int)
	     param.nMetadataItemCount);
    fprintf (stderr, "Metadata scope specifier: %d\n", (int)
	     param.nScopeSpecifier);
  }*/

  /*{
    OMX_AUDIO_PARAM_PORTFORMATTYPE param;
    param.nSize = sizeof (param);
    param.nVersion.nVersion = OMX_VERSION;
    param.nIndex = 110;
    param.eEncoding = OMX_AUDIO_CodingMP3;
    error = OMX_SetParameter (ILC_GET_HANDLE (filereader_handle),
			      OMX_IndexParamAudioPortFormat, &param);
    if (error != OMX_ErrorNone)
      {
        fprintf (stderr, "Error setting type to MP3 (%x)\n", error);
	return 1;
      }
  }*/

  complist[0] = decode_handle;
  complist[1] = render_handle;
  complist[2] = 0;
  ilclient_state_transition (complist, OMX_StateIdle);

  {
    OMX_AUDIO_PARAM_PORTFORMATTYPE param;
    
    param.nSize = sizeof (param);
    param.nVersion.nVersion = OMX_VERSION;
    param.nPortIndex = 120;
    param.eEncoding = OMX_AUDIO_CodingMP3;
    error = OMX_SetParameter (ILC_GET_HANDLE (decode_handle),
			      OMX_IndexParamAudioPortFormat, &param);
    if (error != OMX_ErrorNone)
      {
        fprintf (stderr, "Can't set decoder to MP3 input format (%x)\n", error);
	return 1;
      }
  }

  ret = ilclient_enable_port_buffers (decode_handle, 120, NULL, NULL, NULL);
  if (ret != 0)
    {
      fprintf (stderr, "Error enabling port buffers (%d)\n", ret);
      return 1;
    }

  decode_to_render.source = decode_handle;
  decode_to_render.source_port = 121;
  decode_to_render.sink = render_handle;
  decode_to_render.sink_port = 100;

  /*{
    OMX_AUDIO_PARAM_MP3TYPE param;
    param.nSize = sizeof (param);
    param.nVersion.nVersion = OMX_VERSION;
    param.nPortIndex = 110;
    error = OMX_GetParameter (ILC_GET_HANDLE (filereader_handle),
			      OMX_IndexParamAudioMp3, &param);
    if (error != OMX_ErrorNone)
      {
        fprintf (stderr, "error querying MP3 parameters\n");
        return 1;
      }
    fprintf (stderr, "channels: %d\n", (int) param.nChannels);
    fprintf (stderr, "bitrate: %d\n", (int) param.nBitRate);
    fprintf (stderr, "samplerate: %d\n", (int) param.nSampleRate);
    fprintf (stderr, "audiobandwidth: %d\n", (int) param.nAudioBandWidth);
  }*/

  /*file_to_decode.source = filereader_handle;
  file_to_decode.source_port = 110;
  file_to_decode.sink = decode_handle;
  file_to_decode.sink_port = 120;
  
  ret = ilclient_setup_tunnel (&file_to_decode, 0, 0);
  if (ret != 0)
    {
      fprintf (stderr, "couldn't create file-to-decode tunnel (%d)\n", ret);
      return 1;
    }*/
  
  {
    FILE *f = fopen (filename, "r");
    OMX_BUFFERHEADERTYPE *buf;
    int port_settings_changed = 0;
    
    ilclient_change_component_state (decode_handle, OMX_StateExecuting);
    
    while ((buf = ilclient_get_input_buffer (decode_handle, 130, 1)) != NULL)
      {
        fread (buf->pBuffer, 1, buf->nFilledLen, f);
	if (!port_settings_changed)
	  {
	    ret = ilclient_remove_event (decode_handle,
					 OMX_EventPortSettingsChanged, 130,
					 0, 0, 1);
	    if (ret != 0)
	      continue;

	    ret = ilclient_setup_tunnel (&decode_to_render, 0, 0);
	    if (ret != 0)
	      {
		fprintf (stderr, "couldn't create decode-to-render tunnel\n");
		return 1;
	      }

	    port_settings_changed = 1;

	    ilclient_change_component_state (render_handle, OMX_StateExecuting);
	  }
      }
    
    fprintf (stderr, "Wait for end of stream\n");
    ilclient_wait_for_event (render_handle, OMX_EventBufferFlag, 90, 0,
			     OMX_BUFFERFLAG_EOS, 0, ILCLIENT_BUFFER_FLAG_EOS,
			     10000);
    
    fclose (f);
  }
  
  return 0;
}

#endif

#if 0
typedef struct
{
  VCOS_EVENT_FLAGS_T event;
  OMX_HANDLETYPE comp;
  VCOS_SEMAPHORE_T sema;
} component_info;

void
omxaudio_init (void)
{
  OMX_ERRORTYPE error;
  
  error = OMX_Init ();
  assert (error == OMX_ErrorNone);
}

static OMX_ERRORTYPE
omxaudio_eventhandler (OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
		       OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2,
		       OMX_PTR pEventData)
{
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
omxaudio_emptybufferdone (OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
			  OMX_BUFFERHEADERTYPE *pBuffer)
{
  return OMX_ErrorNone;
}

static OMX_ERRORTYPE
omxaudio_fillbufferdone (OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
			 OMX_BUFFERHEADERTYPE *pBuffer)
{
  return OMX_ErrorNone;
}

void
set_version (OMX_VERSIONTYPE *vsn)
{
  vsn->nVersion = OMX_VERSION;
}

void
print_state (char *name, OMX_HANDLETYPE handle)
{
  OMX_STATETYPE eState;

  OMX_GetState (handle, &eState);

  fprintf (stderr, "%s: ", name);

  switch (eState)
    {
    case OMX_StateInvalid:
      fprintf (stderr, "OMX_StateInvalid\n");
      break;
    case OMX_StateLoaded:
      fprintf (stderr, "OMX_StateLoaded\n");
      break;
    case OMX_StateIdle:
      fprintf (stderr, "OMX_StateIdle\n");
      break;
    case OMX_StateExecuting:
      fprintf (stderr, "OMX_StateExecuting\n");
      break;
    case OMX_StatePause:
      fprintf (stderr, "OMX_StatePause\n");
      break;
    case OMX_StateWaitForResources:
      fprintf (stderr, "OMX_StateWaitForResources\n");
      break;
    default:
      ;
    }
}


OMX_ERRORTYPE
omxaudio_load (const char *filename)
{
  component_info *comp;
  OMX_ERRORTYPE error;
  OMX_CALLBACKTYPE callbacks;
  OMX_HANDLETYPE render_handle;
  OMX_HANDLETYPE decode_handle;
  OMX_HANDLETYPE filereader_handle;
  int32_t status;
  OMX_STATETYPE eState;
  char *what = "";
  
  comp = vcos_malloc (sizeof (component_info), "il:comp");
  memset (comp, 0, sizeof (component_info));
  
  status = vcos_event_flags_create (&comp->event, "il:comp");
  vc_assert (status == VCOS_SUCCESS);
  status = vcos_semaphore_create (&comp->sema, "il:comp", 1);
  vc_assert (status == VCOS_SUCCESS);
  
  callbacks.EventHandler = &omxaudio_eventhandler;
  callbacks.EmptyBufferDone = &omxaudio_emptybufferdone;
  callbacks.FillBufferDone = &omxaudio_fillbufferdone;
  
  what = "get handle audio_render";
  error = OMX_GetHandle (&render_handle, "OMX.broadcom.audio_render", comp,
			 &callbacks);
  if (error != OMX_ErrorNone)
    goto error_out;
  
  what = "get handle audio_decode";
  error = OMX_GetHandle (&decode_handle, "OMX.broadcom.audio_decode", comp,
			 &callbacks);
  if (error != OMX_ErrorNone)
    goto error_out;
  
  what = "get handle read_media";
  error = OMX_GetHandle (&filereader_handle, "OMX.broadcom.read_media", comp,
			 &callbacks);
  if (error != OMX_ErrorNone)
    goto error_out;
  
  {
    size_t ssize = offsetof (OMX_PARAM_CONTENTURITYPE, contentURI)
		   + strlen (filename) + 1;
    OMX_PARAM_CONTENTURITYPE *uriparam = malloc (ssize);
    uriparam->nSize = ssize;
    set_version (&uriparam->nVersion);
    strcpy ((char *) &uriparam->contentURI[0], filename);
    error = OMX_SetParameter (filereader_handle, OMX_IndexParamContentURI,
			      uriparam);
    free (uriparam);
    what = "set filename";
    if (error != OMX_ErrorNone)
      goto error_out;
  }
  
  {
    OMX_PORT_PARAM_TYPE param;
    int j;
    
    param.nSize = sizeof (param);
    set_version (&param.nVersion);
    error = OMX_GetParameter (filereader_handle, OMX_IndexParamAudioInit,
			      &param);
    what = "IndexParamAudioInit";
    if (error != OMX_ErrorNone)
      goto error_out;
    
    for (j = 0; j < param.nPorts; j++)
      {
	OMX_PARAM_PORTDEFINITIONTYPE portdef;
	int port = param.nStartPortNumber + j;
	
	portdef.nSize = sizeof (portdef);
	set_version (&portdef.nVersion);
	portdef.nPortIndex = port;
	error = OMX_GetParameter (filereader_handle,
				  OMX_IndexParamPortDefinition, &portdef);
	what = "IndexParamPortDefinition";
	if (error != OMX_ErrorNone)
	  goto error_out;
	
	fprintf (stderr, "port %d: direction %s\n", port,
		 portdef.eDir == OMX_DirInput ? "input" : "output");
        fprintf (stderr, "actual buffer count: %d\n",
		 (int) portdef.nBufferCountActual);
        fprintf (stderr, "minimum buffer count: %d\n",
		 (int) portdef.nBufferCountMin);
        fprintf (stderr, "size for buffers: %d\n",
		 (int) portdef.nBufferSize);
        fprintf (stderr, "enabled: %s\n",
		 portdef.bEnabled ? "yes" : "no");
      }
  }
  
  print_state ("read_media", filereader_handle);
  print_state ("audio_decode", decode_handle);
  print_state ("audio_render", render_handle);

  {
    OMX_AUDIO_PARAM_MP3TYPE param;
    param.nSize = sizeof (param);
    set_version (&param.nVersion);
    param.nPortIndex = 110;
    error = OMX_GetParameter (filereader_handle, OMX_IndexParamAudioMp3,
			      &param);
    what = "query MP3";
    if (error != OMX_ErrorNone)
      goto error_out;
    fprintf (stderr, "channels: %d\n", (int) param.nChannels);
    fprintf (stderr, "bitrate: %d\n", (int) param.nBitRate);
    fprintf (stderr, "samplerate: %d\n", (int) param.nSampleRate);
    fprintf (stderr, "audiobandwidth: %d\n", (int) param.nAudioBandWidth);
  }

  what = "send command filereader idle";
  error = OMX_SendCommand (filereader_handle, OMX_CommandStateSet,
			   OMX_StateIdle, NULL);
  if (error != OMX_ErrorNone)
    goto error_out;

  printf ("wait for media reader to become idle...\n");
  do
    OMX_GetState (filereader_handle, &eState);
  while (eState != OMX_StateIdle);

  print_state ("read_media", filereader_handle);
  print_state ("audio_decode", decode_handle);
  print_state ("audio_render", render_handle);

  {
    OMX_PARAM_U32TYPE param;
    param.nSize = sizeof (param);
    set_version (&param.nVersion);
    param.nPortIndex = 110;
    error = OMX_GetParameter (filereader_handle,
			      OMX_IndexParamNumAvailableStreams, &param);
    fprintf (stderr, "Number of streams from input: %d\n", (int)param.nU32);
    what = "query streams from input";
    if (error != OMX_ErrorNone)
      goto error_out;
  }
  
  /*{
    OMX_CONFIG_PLAYMODETYPE playmode;
    playmode.nSize = sizeof (playmode);
    set_version (&playmode.nVersion);
    playmode.eMode = OMX_PLAYMODE_NORMAL;
    error = OMX_SetParameter (filereader_handle, OMX_IndexConfigPlayMode,
			      &playmode);
    what = "playback mode";
    if (error != OMX_ErrorNone)
      goto error_out;
  }*/
  
  what = "tunnel decode->render";
  error = OMX_SetupTunnel (decode_handle, 121, render_handle, 100);
  if (error != OMX_ErrorNone)
    goto error_out;

  what = "tunnel filereader->decode";
  error = OMX_SetupTunnel (filereader_handle, 110, decode_handle, 120);
  if (error != OMX_ErrorNone)
    goto error_out;

  what = "send command filereader execute";
  error = OMX_SendCommand (filereader_handle, OMX_CommandStateSet,
			   OMX_StateExecuting, NULL);
  if (error != OMX_ErrorNone)
    goto error_out;

  what = "send command decode execute";
  error = OMX_SendCommand (decode_handle, OMX_CommandStateSet,
			   OMX_StateExecuting, NULL);
  if (error != OMX_ErrorNone)
    goto error_out;

  what = "send command render execute";
  error = OMX_SendCommand (render_handle, OMX_CommandStateSet,
			   OMX_StateExecuting, NULL);
  if (error != OMX_ErrorNone)
    goto error_out;
  
  printf ("wait for media reader to become ready...\n");
  do
    OMX_GetState (filereader_handle, &eState);
  while (eState != OMX_StateExecuting);
  
  return OMX_ErrorNone;

error_out:
  fprintf (stderr, "Error in: %s\n", what);
  vcos_event_flags_delete (&comp->event);
  vcos_semaphore_delete (&comp->sema);
  vcos_free (comp);
  return error;
}

void
omxaudio_deinit (void)
{
  OMX_ERRORTYPE error;
  
  error = OMX_Deinit ();
  assert (error == OMX_ErrorNone);
}
#endif
