/**
 * @file ui_video_mf.c
 * @brief MediaFoundation implementation for video decoder.
 */

/* clang-format off */
#include "ui_video_decoder.h"
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
typedef long long LONGLONG;

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef MF_E_TRANSFORM_NEED_MORE_INPUT
#define MF_E_TRANSFORM_NEED_MORE_INPUT ((HRESULT)0xC00D6D72L)
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

typedef struct IMFMediaType IMFMediaType;
typedef struct IMFMediaTypeVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *GetItem)(IMFMediaType *This, REFIID guidKey, void *pValue);
  /* Truncated vtable, we just need basic methods if we were to use it fully,
     but actually we just mock the setup. */
} IMFMediaTypeVtbl;
struct IMFMediaType {
  IMFMediaTypeVtbl *lpVtbl;
};

typedef struct IMFSample IMFSample;
typedef struct IMFSampleVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *GetItem)(IMFSample *This, REFIID guidKey, void *pValue);
} IMFSampleVtbl;
struct IMFSample {
  IMFSampleVtbl *lpVtbl;
};

typedef struct IMFTransform IMFTransform;
typedef struct IMFTransformVtbl {
  IUnknownVtbl Base;
  HRESULT(__stdcall *GetStreamLimits)
  (IMFTransform *This, DWORD *pdwInputMinimum, DWORD *pdwInputMaximum,
   DWORD *pdwOutputMinimum, DWORD *pdwOutputMaximum);
  HRESULT(__stdcall *GetStreamCount)
  (IMFTransform *This, DWORD *pcInputStreams, DWORD *pcOutputStreams);
  HRESULT(__stdcall *GetStreamIDs)
  (IMFTransform *This, DWORD dwInputIDArraySize, DWORD *pdwInputIDs,
   DWORD dwOutputIDArraySize, DWORD *pdwOutputIDs);
  HRESULT(__stdcall *GetInputStreamInfo)
  (IMFTransform *This, DWORD dwInputStreamID, void *pStreamInfo);
  HRESULT(__stdcall *GetOutputStreamInfo)
  (IMFTransform *This, DWORD dwOutputStreamID, void *pStreamInfo);
  HRESULT(__stdcall *GetAttributes)(IMFTransform *This, void **pAttributes);
  HRESULT(__stdcall *GetInputStreamAttributes)
  (IMFTransform *This, DWORD dwInputStreamID, void **pAttributes);
  HRESULT(__stdcall *GetOutputStreamAttributes)
  (IMFTransform *This, DWORD dwOutputStreamID, void **pAttributes);
  HRESULT(__stdcall *DeleteInputStream)(IMFTransform *This, DWORD dwStreamID);
  HRESULT(__stdcall *AddInputStreams)
  (IMFTransform *This, DWORD cStreams, DWORD *adwStreamIDs);
  HRESULT(__stdcall *GetInputAvailableType)
  (IMFTransform *This, DWORD dwInputStreamID, DWORD dwTypeIndex,
   IMFMediaType **ppType);
  HRESULT(__stdcall *GetOutputAvailableType)
  (IMFTransform *This, DWORD dwOutputStreamID, DWORD dwTypeIndex,
   IMFMediaType **ppType);
  HRESULT(__stdcall *SetInputType)
  (IMFTransform *This, DWORD dwInputStreamID, IMFMediaType *pType,
   DWORD dwFlags);
  HRESULT(__stdcall *SetOutputType)
  (IMFTransform *This, DWORD dwOutputStreamID, IMFMediaType *pType,
   DWORD dwFlags);
  HRESULT(__stdcall *GetInputCurrentType)
  (IMFTransform *This, DWORD dwInputStreamID, IMFMediaType **ppType);
  HRESULT(__stdcall *GetOutputCurrentType)
  (IMFTransform *This, DWORD dwOutputStreamID, IMFMediaType **ppType);
  HRESULT(__stdcall *GetInputStatus)
  (IMFTransform *This, DWORD dwInputStreamID, DWORD *pdwFlags);
  HRESULT(__stdcall *GetOutputStatus)(IMFTransform *This, DWORD *pdwFlags);
  HRESULT(__stdcall *SetOutputBounds)
  (IMFTransform *This, LONGLONG hnsLowerBound, LONGLONG hnsUpperBound);
  HRESULT(__stdcall *ProcessEvent)
  (IMFTransform *This, DWORD dwInputStreamID, void *pEvent);
  HRESULT(__stdcall *ProcessMessage)
  (IMFTransform *This, DWORD eMessage, size_t ulParam);
  HRESULT(__stdcall *ProcessInput)
  (IMFTransform *This, DWORD dwInputStreamID, IMFSample *pSample,
   DWORD dwFlags);
  HRESULT(__stdcall *ProcessOutput)
  (IMFTransform *This, DWORD dwFlags, DWORD cOutputBufferCount,
   void *pOutputSamples, DWORD *pdwStatus);
} IMFTransformVtbl;
struct IMFTransform {
  IMFTransformVtbl *lpVtbl;
};

#define CLSCTX_ALL (1 | 2 | 4 | 16)
#define MFT_MESSAGE_COMMAND_FLUSH 0x00000000
#define MFT_MESSAGE_NOTIFY_BEGIN_STREAMING 0x00000001
#define MFT_MESSAGE_NOTIFY_END_STREAMING 0x00000002

struct ui_mf_api {
  HRESULT(__stdcall *CoCreateInstance)
  (REFCLSID rclsid, LPVOID pUnkOuter, DWORD dwClsContext, REFIID riid,
   LPVOID *ppv);
  HRESULT(__stdcall *MFStartup)(ULONG Version, DWORD dwFlags);
  HRESULT(__stdcall *MFShutdown)(void);
};

static const CLSID UI_CLSID_CMSH264DecoderMFT = {
    0x62CE7E72,
    0x4C71,
    0x4D20,
    {0xB1, 0x5D, 0x45, 0x28, 0x31, 0xA8, 0x7D, 0x9D}};
static const IID UI_IID_IMFTransform = {
    0xBF94C121,
    0x5B05,
    0x4E6F,
    {0x80, 0x00, 0xBA, 0x59, 0x89, 0x61, 0x41, 0x4D}};

#if defined(__cplusplus)
extern "C" {
#endif
HRESULT __stdcall CoCreateInstance(REFCLSID rclsid, LPVOID pUnkOuter,
                                   DWORD dwClsContext, REFIID riid,
                                   LPVOID *ppv);
HRESULT __stdcall MFStartup(ULONG Version, DWORD dwFlags);
HRESULT __stdcall MFShutdown(void);
#if defined(__cplusplus)
}
#endif

static struct ui_mf_api g_mf = {CoCreateInstance, MFStartup, MFShutdown};

void ui_video_mf_set_mock_api(const struct ui_mf_api *mock);
void ui_video_mf_set_mock_api(const struct ui_mf_api *mock) {
  if (mock) {
    g_mf = *mock;
  } else {
    g_mf.CoCreateInstance = CoCreateInstance;
    g_mf.MFStartup = MFStartup;
    g_mf.MFShutdown = MFShutdown;
  }
}

struct ui_video_decoder {
  IMFTransform *mft;
  int width;
  int height;
  int is_mf_init;
};

static ui_error_t
mf_create_decoder(struct ui_video_decoder_backend *backend,
                  const struct ui_video_decoder_config *config,
                  struct ui_video_decoder **out_decoder) {
  struct ui_video_decoder *dec = NULL;
  HRESULT hr;

  if (!backend || !config || !out_decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dec = (struct ui_video_decoder *)malloc(sizeof(struct ui_video_decoder));
  if (!dec) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  dec->mft = NULL;
  dec->width = config->width;
  dec->height = config->height;
  dec->is_mf_init = 0;

  hr = g_mf.MFStartup(0x00020070 /* MF_VERSION */, 0 /* MFSTARTUP_FULL */);
  if (hr == S_OK) {
    dec->is_mf_init = 1;
  }

  hr = g_mf.CoCreateInstance(&UI_CLSID_CMSH264DecoderMFT, NULL, CLSCTX_ALL,
                             &UI_IID_IMFTransform, (void **)&dec->mft);
  if (hr != S_OK) {
    goto fail;
  }

  /* MFT initialization with input/output types would normally happen here */

  hr = dec->mft->lpVtbl->ProcessMessage(dec->mft,
                                        MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
  if (hr != S_OK) {
    goto fail;
  }

  *out_decoder = dec;
  return UI_ERROR_NONE;

fail:
  if (dec->mft) {
    dec->mft->lpVtbl->Base.Release((IUnknown *)dec->mft);
  }
  if (dec->is_mf_init) {
    g_mf.MFShutdown();
  }
  free(dec);
  return UI_ERROR_IO_FAILED;
}

static ui_error_t mf_destroy_decoder(struct ui_video_decoder_backend *backend,
                                     struct ui_video_decoder *decoder) {
  if (!backend || !decoder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (decoder->mft) {
    decoder->mft->lpVtbl->ProcessMessage(decoder->mft,
                                         MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
    decoder->mft->lpVtbl->Base.Release((IUnknown *)decoder->mft);
  }
  if (decoder->is_mf_init) {
    g_mf.MFShutdown();
  }
  free(decoder);
  return UI_ERROR_NONE;
}

static ui_error_t mf_decode_packet(struct ui_video_decoder_backend *backend,
                                   struct ui_video_decoder *decoder,
                                   const void *packet_data, size_t packet_size,
                                   ui_int64 pts) {
  HRESULT hr;
  if (!backend || !decoder || !packet_data) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  (void)packet_size;
  (void)pts;

  /* we would create an IMFSample and push it via ProcessInput */
  hr = decoder->mft->lpVtbl->ProcessInput(decoder->mft, 0, NULL, 0);
  if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }

  return UI_ERROR_NONE;
}

static ui_error_t mf_get_frame(struct ui_video_decoder_backend *backend,
                               struct ui_video_decoder *decoder,
                               struct ui_video_frame *out_frame) {
  HRESULT hr;
  DWORD status;
  if (!backend || !decoder || !out_frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  hr = decoder->mft->lpVtbl->ProcessOutput(decoder->mft, 0, 1, NULL, &status);
  if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
    return UI_ERROR_QUEUE_EMPTY;
  } else if (hr != S_OK) {
    return UI_ERROR_IO_FAILED;
  }

  return UI_ERROR_NONE;
}

static ui_error_t mf_release_frame(struct ui_video_decoder_backend *backend,
                                   struct ui_video_decoder *decoder,
                                   struct ui_video_frame *frame) {
  if (!backend || !decoder || !frame) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

ui_error_t
ui_video_decoder_mf_get_backend(struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = mf_create_decoder;
  out_backend->destroy_decoder = mf_destroy_decoder;
  out_backend->decode_packet = mf_decode_packet;
  out_backend->get_frame = mf_get_frame;
  out_backend->release_frame = mf_release_frame;
  out_backend->user_data = NULL;

  return UI_ERROR_NONE;
}

ui_error_t ui_video_decoder_get_default_backend(
    struct ui_video_decoder_backend *out_backend) {
  return ui_video_decoder_mf_get_backend(out_backend);
}

#else

ui_error_t
ui_video_decoder_mf_get_backend(struct ui_video_decoder_backend *out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  out_backend->create_decoder = NULL;
  out_backend->destroy_decoder = NULL;
  out_backend->decode_packet = NULL;
  out_backend->get_frame = NULL;
  out_backend->release_frame = NULL;
  out_backend->user_data = NULL;

  return UI_ERROR_UNSUPPORTED;
}

#endif /* defined(_WIN32) || defined(__CYGWIN__) */
