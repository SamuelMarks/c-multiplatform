/**
 * @file ui_audio_wasapi.c
 * @brief WASAPI implementation for audio sink.
 */

/* clang-format off */
#include "ui_audio_sink.h"
#include "ui_error.h"
#include "ui_types.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#endif
/* clang-format on */

#if defined(_WIN32) || defined(__CYGWIN__)

typedef long HRESULT;
typedef unsigned long ULONG;
typedef unsigned int UINT32;
typedef unsigned long DWORD;
typedef long long REFERENCE_TIME;
typedef unsigned short WORD;
typedef void *LPVOID;

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef AUDCLNT_E_DEVICE_INVALIDATED
#define AUDCLNT_E_DEVICE_INVALIDATED ((HRESULT)0x88890004L)
#endif

typedef struct _GUID {
  unsigned long Data1;
  unsigned short Data2;
  unsigned short Data3;
  unsigned char Data4[8];
} GUID;
typedef GUID IID;
typedef GUID CLSID;

#define REFIID const IID *
#define REFCLSID const CLSID *

typedef enum _AUDCLNT_SHAREMODE {
  AUDCLNT_SHAREMODE_SHARED = 0,
  AUDCLNT_SHAREMODE_EXCLUSIVE = 1
} AUDCLNT_SHAREMODE;

typedef struct tWAVEFORMATEX {
  WORD wFormatTag;
  WORD nChannels;
  DWORD nSamplesPerSec;
  DWORD nAvgBytesPerSec;
  WORD nBlockAlign;
  WORD wBitsPerSample;
  WORD cbSize;
} WAVEFORMATEX;

typedef struct tWAVEFORMATEXTENSIBLE {
  WAVEFORMATEX Format;
  union {
    WORD wValidBitsPerSample;
    WORD wSamplesPerBlock;
    WORD wReserved;
  } Samples;
  DWORD dwChannelMask;
  GUID SubFormat;
} WAVEFORMATEXTENSIBLE;

#define WAVE_FORMAT_EXTENSIBLE 0xFFFE
#define AUDCLNT_STREAMFLAGS_EVENTCALLBACK 0x00040000

/* COM interface forward declarations */
typedef struct IUnknown IUnknown;
typedef struct IUnknownVtbl {
  HRESULT(__stdcall *QueryInterface)
  (IUnknown *This, REFIID riid, void **ppvObject);
  ULONG(__stdcall *AddRef)(IUnknown *This);
  ULONG(__stdcall *Release)(IUnknown *This);
} IUnknownVtbl;
struct IUnknown {
  IUnknownVtbl *lpVtbl;
};

typedef struct IAudioRenderClient IAudioRenderClient;
typedef struct IAudioRenderClientVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *GetBuffer)
  (IAudioRenderClient *This, UINT32 NumFramesRequested, unsigned char **ppData);
  HRESULT(__stdcall *ReleaseBuffer)
  (IAudioRenderClient *This, UINT32 NumFramesWritten, DWORD dwFlags);
} IAudioRenderClientVtbl;
struct IAudioRenderClient {
  IAudioRenderClientVtbl *lpVtbl;
};

typedef struct IAudioClient IAudioClient;
typedef struct IAudioClientVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *Initialize)
  (IAudioClient *This, AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags,
   REFERENCE_TIME hnsBufferDuration, REFERENCE_TIME hnsPeriodicity,
   const WAVEFORMATEX *pFormat, const GUID *AudioSessionGuid);
  HRESULT(__stdcall *GetBufferSize)
  (IAudioClient *This, UINT32 *pNumBufferFrames);
  HRESULT(__stdcall *GetStreamLatency)
  (IAudioClient *This, REFERENCE_TIME *phnsLatency);
  HRESULT(__stdcall *GetCurrentPadding)
  (IAudioClient *This, UINT32 *pNumPaddingFrames);
  HRESULT(__stdcall *IsFormatSupported)
  (IAudioClient *This, AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX *pFormat,
   WAVEFORMATEX **ppClosestMatch);
  HRESULT(__stdcall *GetMixFormat)
  (IAudioClient *This, WAVEFORMATEX **ppDeviceFormat);
  HRESULT(__stdcall *GetDevicePeriod)
  (IAudioClient *This, REFERENCE_TIME *phnsDefaultDevicePeriod,
   REFERENCE_TIME *phnsMinimumDevicePeriod);
  HRESULT(__stdcall *Start)(IAudioClient *This);
  HRESULT(__stdcall *Stop)(IAudioClient *This);
  HRESULT(__stdcall *Reset)(IAudioClient *This);
  HRESULT(__stdcall *SetEventHandle)(IAudioClient *This, void *eventHandle);
  HRESULT(__stdcall *GetService)(IAudioClient *This, REFIID riid, void **ppv);
} IAudioClientVtbl;
struct IAudioClient {
  IAudioClientVtbl *lpVtbl;
};

typedef struct IMMDevice IMMDevice;
typedef struct IMMDeviceVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *Activate)
  (IMMDevice *This, REFIID iid, DWORD dwClsCtx, void *pActivationParams,
   void **ppInterface);
  HRESULT(__stdcall *OpenPropertyStore)
  (IMMDevice *This, DWORD stgmAccess, void **ppProperties);
  HRESULT(__stdcall *GetId)(IMMDevice *This, WORD **ppstrId);
  HRESULT(__stdcall *GetState)(IMMDevice *This, DWORD *pdwState);
} IMMDeviceVtbl;
struct IMMDevice {
  IMMDeviceVtbl *lpVtbl;
};

typedef struct IMMDeviceEnumerator IMMDeviceEnumerator;
typedef struct IMMDeviceEnumeratorVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *EnumAudioEndpoints)
  (IMMDeviceEnumerator *This, DWORD dataFlow, DWORD dwStateMask,
   void **ppDevices);
  HRESULT(__stdcall *GetDefaultAudioEndpoint)
  (IMMDeviceEnumerator *This, DWORD dataFlow, DWORD role,
   IMMDevice **ppEndpoint);
  HRESULT(__stdcall *GetDevice)
  (IMMDeviceEnumerator *This, const WORD *pwstrId, IMMDevice **ppDevice);
  HRESULT(__stdcall *RegisterEndpointNotificationCallback)
  (IMMDeviceEnumerator *This, void *pClient);
  HRESULT(__stdcall *UnregisterEndpointNotificationCallback)
  (IMMDeviceEnumerator *This, void *pClient);
} IMMDeviceEnumeratorVtbl;
struct IMMDeviceEnumerator {
  IMMDeviceEnumeratorVtbl *lpVtbl;
};

/* eRender = 0, eConsole = 0 */
#define CLSCTX_ALL (1 | 2 | 4 | 16)

#if defined(__cplusplus)
extern "C" {
#endif
HRESULT __stdcall CoCreateInstance(REFCLSID rclsid, LPVOID pUnkOuter,
                                   DWORD dwClsContext, REFIID riid,
                                   LPVOID *ppv);
HRESULT __stdcall CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
void __stdcall CoUninitialize(void);
void __stdcall CoTaskMemFree(LPVOID pv);
#if defined(__cplusplus)
}
#endif

static const CLSID UI_CLSID_MMDeviceEnumerator = {
    0xBCDE0395,
    0xE52F,
    0x467C,
    {0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E}};
static const IID UI_IID_IMMDeviceEnumerator = {
    0xA95664D2,
    0x9614,
    0x4F35,
    {0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6}};
static const IID UI_IID_IAudioClient = {
    0x1CB9AD4C,
    0xDBFA,
    0x4c32,
    {0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2}};
static const IID UI_IID_IAudioRenderClient = {
    0xF294ACFC,
    0x3146,
    0x4483,
    {0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2}};
static const GUID UI_KSDATAFORMAT_SUBTYPE_PCM = {
    0x00000001,
    0x0000,
    0x0010,
    {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
static const GUID UI_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {
    0x00000003,
    0x0000,
    0x0010,
    {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

struct ui_wasapi_api {
  HRESULT(__stdcall *CoCreateInstance)
  (REFCLSID rclsid, LPVOID pUnkOuter, DWORD dwClsContext, REFIID riid,
   LPVOID *ppv);
  HRESULT(__stdcall *CoInitializeEx)(LPVOID pvReserved, DWORD dwCoInit);
  void(__stdcall *CoUninitialize)(void);
  void(__stdcall *CoTaskMemFree)(LPVOID pv);
};

static struct ui_wasapi_api g_wasapi = {CoCreateInstance, CoInitializeEx,
                                        CoUninitialize, CoTaskMemFree};

void ui_audio_wasapi_set_mock_api(const struct ui_wasapi_api *mock);
void ui_audio_wasapi_set_mock_api(const struct ui_wasapi_api *mock) {
  if (mock) {
    g_wasapi = *mock;
  } else {
    g_wasapi.CoCreateInstance = CoCreateInstance;
    g_wasapi.CoInitializeEx = CoInitializeEx;
    g_wasapi.CoUninitialize = CoUninitialize;
    g_wasapi.CoTaskMemFree = CoTaskMemFree;
  }
}

struct ui_audio_sink {
  IMMDeviceEnumerator *enumerator;
  IMMDevice *device;
  IAudioClient *client;
  IAudioRenderClient *render_client;
  UINT32 buffer_frame_count;
  int sample_rate;
  int frame_size;
  int is_co_init;
};

static ui_error_t wasapi_create_sink(struct ui_audio_sink_backend *backend,
                                     const struct ui_audio_sink_config *config,
                                     struct ui_audio_sink **out_sink) {
  struct ui_audio_sink *sink = NULL;
  HRESULT hr;
  WAVEFORMATEXTENSIBLE wf;

  if (!backend || !config || !out_sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  sink = (struct ui_audio_sink *)malloc(sizeof(struct ui_audio_sink));
  if (!sink) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  sink->enumerator = NULL;
  sink->device = NULL;
  sink->client = NULL;
  sink->render_client = NULL;
  sink->buffer_frame_count = 0;
  sink->sample_rate = config->sample_rate;
  sink->frame_size = config->frame_size;
  sink->is_co_init = 0;

  hr = g_wasapi.CoInitializeEx(NULL, 0); /* COINIT_MULTITHREADED */
  if (hr == S_OK || hr == 0x80010106L) { /* S_OK or RPC_E_CHANGED_MODE */
    sink->is_co_init = 1;
  }

  hr = g_wasapi.CoCreateInstance(&UI_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
                                 &UI_IID_IMMDeviceEnumerator,
                                 (void **)&sink->enumerator);
  if (hr != S_OK) {
    goto fail;
  }

  hr = sink->enumerator->lpVtbl->GetDefaultAudioEndpoint(sink->enumerator, 0, 0,
                                                         &sink->device);
  if (hr != S_OK) {
    goto fail;
  }

  hr = sink->device->lpVtbl->Activate(sink->device, &UI_IID_IAudioClient,
                                      CLSCTX_ALL, NULL, (void **)&sink->client);
  if (hr != S_OK) {
    goto fail;
  }

  memset(&wf, 0, sizeof(wf));
  wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
  wf.Format.nChannels = (WORD)config->channels;
  wf.Format.nSamplesPerSec = (DWORD)config->sample_rate;
  wf.Format.wBitsPerSample =
      (WORD)((config->frame_size / config->channels) * 8);
  wf.Format.nBlockAlign = (WORD)config->frame_size;
  wf.Format.nAvgBytesPerSec = wf.Format.nSamplesPerSec * wf.Format.nBlockAlign;
  wf.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
  wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
  wf.dwChannelMask =
      (config->channels == 2) ? 3 : 1; /* FRONT_LEFT | FRONT_RIGHT */

  if (wf.Format.wBitsPerSample == 32) {
    wf.SubFormat = UI_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
  } else {
    wf.SubFormat = UI_KSDATAFORMAT_SUBTYPE_PCM;
  }

  hr = sink->client->lpVtbl->Initialize(sink->client, AUDCLNT_SHAREMODE_SHARED,
                                        0, 10000000, 0, (WAVEFORMATEX *)&wf,
                                        NULL);
  if (hr != S_OK) {
    goto fail;
  }

  hr = sink->client->lpVtbl->GetBufferSize(sink->client,
                                           &sink->buffer_frame_count);
  if (hr != S_OK) {
    goto fail;
  }

  hr = sink->client->lpVtbl->GetService(
      sink->client, &UI_IID_IAudioRenderClient, (void **)&sink->render_client);
  if (hr != S_OK) {
    goto fail;
  }

  *out_sink = sink;
  return UI_ERROR_NONE;

fail:
  if (sink->render_client) {
    sink->render_client->lpVtbl->Base.Release((IUnknown *)sink->render_client);
  }
  if (sink->client) {
    sink->client->lpVtbl->Base.Release((IUnknown *)sink->client);
  }
  if (sink->device) {
    sink->device->lpVtbl->Base.Release((IUnknown *)sink->device);
  }
  if (sink->enumerator) {
    sink->enumerator->lpVtbl->Base.Release((IUnknown *)sink->enumerator);
  }
  if (sink->is_co_init) {
    g_wasapi.CoUninitialize();
  }
  free(sink);
  return UI_ERROR_IO_FAILED;
}

static ui_error_t wasapi_destroy_sink(struct ui_audio_sink_backend *backend,
                                      struct ui_audio_sink *sink) {
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (sink->client) {
    sink->client->lpVtbl->Stop(sink->client);
  }
  if (sink->render_client) {
    sink->render_client->lpVtbl->Base.Release((IUnknown *)sink->render_client);
  }
  if (sink->client) {
    sink->client->lpVtbl->Base.Release((IUnknown *)sink->client);
  }
  if (sink->device) {
    sink->device->lpVtbl->Base.Release((IUnknown *)sink->device);
  }
  if (sink->enumerator) {
    sink->enumerator->lpVtbl->Base.Release((IUnknown *)sink->enumerator);
  }
  if (sink->is_co_init) {
    g_wasapi.CoUninitialize();
  }
  free(sink);
  return UI_ERROR_NONE;
}

static ui_error_t wasapi_write_frames(struct ui_audio_sink_backend *backend,
                                      struct ui_audio_sink *sink,
                                      const void *frames, int num_frames,
                                      int *out_frames_written) {
  HRESULT hr;
  UINT32 padding;
  UINT32 available;
  unsigned char *data;

  if (!backend || !sink || !frames || !out_frames_written) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_frames_written = 0;

  hr = sink->client->lpVtbl->GetCurrentPadding(sink->client, &padding);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }

  available = sink->buffer_frame_count - padding;
  if (available == 0) {
    return UI_ERROR_NONE;
  }

  if ((UINT32)num_frames > available) {
    num_frames = (int)available;
  }

  hr = sink->render_client->lpVtbl->GetBuffer(sink->render_client, num_frames,
                                              &data);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }

  memcpy(data, frames, num_frames * sink->frame_size);

  hr = sink->render_client->lpVtbl->ReleaseBuffer(sink->render_client,
                                                  num_frames, 0);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }

  *out_frames_written = num_frames;
  return UI_ERROR_NONE;
}

static ui_error_t wasapi_get_delay(struct ui_audio_sink_backend *backend,
                                   struct ui_audio_sink *sink,
                                   ui_int64 *out_delay_us) {
  HRESULT hr;
  UINT32 padding;
  if (!backend || !sink || !out_delay_us) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hr = sink->client->lpVtbl->GetCurrentPadding(sink->client, &padding);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }

  if (sink->sample_rate > 0) {
    *out_delay_us = (ui_int64)padding * 1000000 / sink->sample_rate;
  } else {
    *out_delay_us = 0;
  }
  return UI_ERROR_NONE;
}

static ui_error_t wasapi_start(struct ui_audio_sink_backend *backend,
                               struct ui_audio_sink *sink) {
  HRESULT hr;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hr = sink->client->lpVtbl->Start(sink->client);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

static ui_error_t wasapi_stop(struct ui_audio_sink_backend *backend,
                              struct ui_audio_sink *sink) {
  HRESULT hr;
  if (!backend || !sink) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hr = sink->client->lpVtbl->Stop(sink->client);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_wasapi_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = wasapi_create_sink;
  out_backend->destroy_sink = wasapi_destroy_sink;
  out_backend->write_frames = wasapi_write_frames;
  out_backend->get_delay = wasapi_get_delay;
  out_backend->start = wasapi_start;
  out_backend->stop = wasapi_stop;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t
ui_audio_sink_get_default_backend(struct ui_audio_sink_backend *out_backend) {
  return ui_audio_sink_wasapi_get_backend(out_backend);
}

#else

ui_error_t
ui_audio_sink_wasapi_get_backend(struct ui_audio_sink_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_sink = NULL;
  out_backend->destroy_sink = NULL;
  out_backend->write_frames = NULL;
  out_backend->get_delay = NULL;
  out_backend->start = NULL;
  out_backend->stop = NULL;
  out_backend->user_data = NULL;

  return UI_ERROR_UNSUPPORTED;
}

#endif /* defined(_WIN32) || defined(__CYGWIN__) */
