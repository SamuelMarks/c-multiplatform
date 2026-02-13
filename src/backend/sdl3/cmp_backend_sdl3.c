#include "cmpc/cmp_backend_sdl3.h"

#include "cmpc/cmp_log.h"
#include "cmpc/cmp_object.h"
#include "cmpc/cmp_tasks.h"

#include <limits.h>
#include <string.h>
#if defined(CMP_LIBCURL_AVAILABLE)
#include <curl/curl.h>
#endif

#if defined(CMP_SDL3_AVAILABLE)
#include <SDL3/SDL.h>
#if defined(CMP_SDL3_TTF_AVAILABLE)
#include <SDL3_ttf/SDL_ttf.h>
#endif
#endif

#define CMP_SDL3_RETURN_IF_ERROR(rc)                                           \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_SDL3_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                          \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_SDL3_DEFAULT_HANDLE_CAPACITY 64u
#define CMP_SDL3_DEFAULT_RENDERER_FLAGS                                        \
  (CMP_SDL3_RENDERER_ACCELERATED | CMP_SDL3_RENDERER_PRESENTVSYNC)

#define CMP_SDL3_RENDERER_FLAG_MASK                                            \
  (CMP_SDL3_RENDERER_ACCELERATED | CMP_SDL3_RENDERER_PRESENTVSYNC |            \
   CMP_SDL3_RENDERER_TARGETTEXTURE)

static int
cmp_sdl3_backend_validate_config(const CMPSDL3BackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if ((config->renderer_flags & ~CMP_SDL3_RENDERER_FLAG_MASK) != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_sdl3_backend_test_validate_config(const CMPSDL3BackendConfig *config) {
  return cmp_sdl3_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_sdl3_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_SDL3_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_sdl3_backend_config_init(CMPSDL3BackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_SDL3_DEFAULT_HANDLE_CAPACITY;
  config->renderer_flags = CMP_SDL3_DEFAULT_RENDERER_FLAGS;
  config->enable_logging = CMP_TRUE;
  config->enable_tasks = CMP_TRUE;
  return CMP_OK;
}

/* GCOVR_EXCL_START */
#if defined(CMP_SDL3_AVAILABLE)

#define CMP_SDL3_TYPE_WINDOW 1
#define CMP_SDL3_TYPE_TEXTURE 2
#define CMP_SDL3_TYPE_FONT 3

#define CMP_SDL3_CLIP_STACK_CAPACITY 32u

typedef struct CMPSDL3Window {
  CMPObjectHeader header;
  struct CMPSDL3Backend *backend;
  SDL_Window *window;
  SDL_Renderer *renderer;
  cmp_i32 width;
  cmp_i32 height;
  CMPScalar dpi_scale;
  cmp_u32 flags;
  CMPBool visible;
} CMPSDL3Window;

typedef struct CMPSDL3Texture {
  CMPObjectHeader header;
  struct CMPSDL3Backend *backend;
  void *texture;
  void *renderer;
  cmp_i32 width;
  cmp_i32 height;
  cmp_u32 format;
  cmp_u32 bytes_per_pixel;
} CMPSDL3Texture;

typedef struct CMPSDL3Font {
  CMPObjectHeader header;
  struct CMPSDL3Backend *backend;
  cmp_i32 size_px;
  cmp_i32 weight;
  CMPBool italic;
  void *font;
} CMPSDL3Font;

struct CMPSDL3Backend {
  CMPAllocator allocator;
  CMPHandleSystem handles;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPIO io;
  CMPSensors sensors;
  CMPCamera camera;
  CMPImage image;
  CMPVideo video;
  CMPAudio audio;
  CMPNetwork network;
  CMPTasks tasks;
  CMPBool tasks_owned;
  CMPBool tasks_enabled;
  CMPBool initialized;
  CMPBool log_enabled;
  CMPBool log_owner;
  CMPBool sdl_initialized;
  CMPBool ttf_initialized;
  cmp_u32 renderer_flags;
  CMPSDL3Window *active_window;
  CMPRect clip_stack[CMP_SDL3_CLIP_STACK_CAPACITY];
  cmp_usize clip_depth;
  CMPMat3 transform;
  CMPBool has_transform;
};

static cmp_u32 g_cmp_sdl3_refcount = 0u;
#if defined(CMP_SDL3_TTF_AVAILABLE)
static cmp_u32 g_cmp_sdl3_ttf_refcount = 0u;
#endif

static int cmp_sdl3_backend_log(struct CMPSDL3Backend *backend,
                                CMPLogLevel level, const char *message) {
  if (backend == NULL || message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->log_enabled) {
    return CMP_OK;
  }
  return cmp_log_write(level, "m3.sdl3", message);
}

static int cmp_sdl3_backend_log_sdl_error(struct CMPSDL3Backend *backend,
                                          const char *message) {
  const char *sdl_error;
  int rc;

  if (backend == NULL || message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_ERROR, message);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  sdl_error = SDL_GetError();
  if (sdl_error != NULL && sdl_error[0] != '\0') {
    rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_ERROR, sdl_error);
    CMP_SDL3_RETURN_IF_ERROR(rc);
  }
  return CMP_OK;
}

#if defined(CMP_SDL3_TTF_AVAILABLE)
static int cmp_sdl3_backend_log_ttf_error(struct CMPSDL3Backend *backend,
                                          const char *message) {
  const char *ttf_error;
  int rc;

  if (backend == NULL || message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_ERROR, message);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  ttf_error = TTF_GetError();
  if (ttf_error != NULL && ttf_error[0] != '\0') {
    rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_ERROR, ttf_error);
    CMP_SDL3_RETURN_IF_ERROR(rc);
  }
  return CMP_OK;
}
#endif

static int cmp_sdl3_global_init(struct CMPSDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (g_cmp_sdl3_refcount == 0u) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      cmp_sdl3_backend_log_sdl_error(backend, "sdl.init");
      return CMP_ERR_UNKNOWN;
    }
  }

  g_cmp_sdl3_refcount += 1u;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "sdl.init");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_sdl3_global_shutdown(struct CMPSDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_cmp_sdl3_refcount == 0u) {
    return CMP_ERR_STATE;
  }

  g_cmp_sdl3_refcount -= 1u;
  if (g_cmp_sdl3_refcount == 0u) {
    SDL_Quit();
  }

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "sdl.shutdown");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

#if defined(CMP_SDL3_TTF_AVAILABLE)
static int cmp_sdl3_ttf_init(struct CMPSDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (g_cmp_sdl3_ttf_refcount == 0u) {
    if (TTF_Init() != 0) {
      cmp_sdl3_backend_log_ttf_error(backend, "ttf.init");
      return CMP_ERR_UNKNOWN;
    }
  }

  g_cmp_sdl3_ttf_refcount += 1u;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ttf.init");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_sdl3_ttf_shutdown(struct CMPSDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_cmp_sdl3_ttf_refcount == 0u) {
    return CMP_ERR_STATE;
  }

  g_cmp_sdl3_ttf_refcount -= 1u;
  if (g_cmp_sdl3_ttf_refcount == 0u) {
    TTF_Quit();
  }

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ttf.shutdown");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}
#endif

static int cmp_sdl3_renderer_flags_to_sdl(cmp_u32 flags, Uint32 *out_flags) {
  Uint32 sdl_flags;

  if (out_flags == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if ((flags & ~CMP_SDL3_RENDERER_FLAG_MASK) != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sdl_flags = 0u;
#if defined(SDL_RENDERER_ACCELERATED)
  if ((flags & CMP_SDL3_RENDERER_ACCELERATED) != 0u) {
    sdl_flags |= SDL_RENDERER_ACCELERATED;
  }
#else
  if ((flags & CMP_SDL3_RENDERER_ACCELERATED) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_RENDERER_PRESENTVSYNC)
  if ((flags & CMP_SDL3_RENDERER_PRESENTVSYNC) != 0u) {
    sdl_flags |= SDL_RENDERER_PRESENTVSYNC;
  }
#else
  if ((flags & CMP_SDL3_RENDERER_PRESENTVSYNC) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_RENDERER_TARGETTEXTURE)
  if ((flags & CMP_SDL3_RENDERER_TARGETTEXTURE) != 0u) {
    sdl_flags |= SDL_RENDERER_TARGETTEXTURE;
  }
#else
  if ((flags & CMP_SDL3_RENDERER_TARGETTEXTURE) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif
  *out_flags = sdl_flags;
  return CMP_OK;
}

static int cmp_sdl3_texture_format_to_sdl(cmp_u32 format, Uint32 *out_format,
                                          cmp_u32 *out_bpp) {
  if (out_format == NULL || out_bpp == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (format == CMP_TEX_FORMAT_RGBA8) {
#if defined(SDL_PIXELFORMAT_RGBA32)
    *out_format = SDL_PIXELFORMAT_RGBA32;
#elif defined(SDL_PIXELFORMAT_RGBA8888)
    *out_format = SDL_PIXELFORMAT_RGBA8888;
#else
    return CMP_ERR_UNSUPPORTED;
#endif
    *out_bpp = 4u;
    return CMP_OK;
  }

  if (format == CMP_TEX_FORMAT_BGRA8) {
#if defined(SDL_PIXELFORMAT_BGRA32)
    *out_format = SDL_PIXELFORMAT_BGRA32;
#elif defined(SDL_PIXELFORMAT_BGRA8888)
    *out_format = SDL_PIXELFORMAT_BGRA8888;
#else
    return CMP_ERR_UNSUPPORTED;
#endif
    *out_bpp = 4u;
    return CMP_OK;
  }

  if (format == CMP_TEX_FORMAT_A8) {
#if defined(SDL_PIXELFORMAT_A8)
    *out_format = SDL_PIXELFORMAT_A8;
    *out_bpp = 1u;
    return CMP_OK;
#else
    return CMP_ERR_UNSUPPORTED;
#endif
  }

  return CMP_ERR_INVALID_ARGUMENT;
}

static int cmp_sdl3_mul_usize(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (a != 0 && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}

static int cmp_sdl3_window_flags_to_sdl(cmp_u32 flags, Uint32 *out_flags) {
  Uint32 sdl_flags;
  cmp_u32 allowed;

  if (out_flags == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  allowed = CMP_WS_WINDOW_RESIZABLE | CMP_WS_WINDOW_BORDERLESS |
            CMP_WS_WINDOW_FULLSCREEN | CMP_WS_WINDOW_HIGH_DPI;
  if ((flags & ~allowed) != 0u) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sdl_flags = 0u;
#if defined(SDL_WINDOW_RESIZABLE)
  if ((flags & CMP_WS_WINDOW_RESIZABLE) != 0u) {
    sdl_flags |= SDL_WINDOW_RESIZABLE;
  }
#else
  if ((flags & CMP_WS_WINDOW_RESIZABLE) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_WINDOW_BORDERLESS)
  if ((flags & CMP_WS_WINDOW_BORDERLESS) != 0u) {
    sdl_flags |= SDL_WINDOW_BORDERLESS;
  }
#else
  if ((flags & CMP_WS_WINDOW_BORDERLESS) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_WINDOW_FULLSCREEN)
  if ((flags & CMP_WS_WINDOW_FULLSCREEN) != 0u) {
    sdl_flags |= SDL_WINDOW_FULLSCREEN;
  }
#elif defined(SDL_WINDOW_FULLSCREEN_DESKTOP)
  if ((flags & CMP_WS_WINDOW_FULLSCREEN) != 0u) {
    sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
#else
  if ((flags & CMP_WS_WINDOW_FULLSCREEN) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_WINDOW_HIGH_PIXEL_DENSITY)
  if ((flags & CMP_WS_WINDOW_HIGH_DPI) != 0u) {
    sdl_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
  }
#elif defined(SDL_WINDOW_ALLOW_HIGHDPI)
  if ((flags & CMP_WS_WINDOW_HIGH_DPI) != 0u) {
    sdl_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
  }
#else
  if ((flags & CMP_WS_WINDOW_HIGH_DPI) != 0u) {
    return CMP_ERR_UNSUPPORTED;
  }
#endif

  *out_flags = sdl_flags;
  return CMP_OK;
}

static int cmp_sdl3_color_to_u8(CMPScalar value, cmp_u8 *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (value < 0.0f) {
    value = 0.0f;
  } else if (value > 1.0f) {
    value = 1.0f;
  }

  *out_value = (cmp_u8)(value * 255.0f + 0.5f);
  return CMP_OK;
}

static int cmp_sdl3_backend_resolve(struct CMPSDL3Backend *backend,
                                    CMPHandle handle, cmp_u32 type_id,
                                    void **out_obj) {
  void *resolved;
  cmp_u32 actual_type;
  int rc;

  if (out_obj != NULL) {
    *out_obj = NULL;
  }

  rc =
      backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_object_get_type_id((const CMPObjectHeader *)resolved, &actual_type);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return CMP_OK;
}

static int cmp_sdl3_object_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}

static int cmp_sdl3_object_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}

static int cmp_sdl3_object_get_type_id(void *obj, cmp_u32 *out_type_id) {
  return cmp_object_get_type_id((const CMPObjectHeader *)obj, out_type_id);
}

static int cmp_sdl3_window_destroy(void *obj) {
  CMPSDL3Window *window;
  struct CMPSDL3Backend *backend;
  int rc;

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  window = (CMPSDL3Window *)obj;
  backend = window->backend;

  if (backend != NULL && backend->active_window == window) {
    backend->active_window = NULL;
  }

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (window->renderer != NULL) {
    SDL_DestroyRenderer(window->renderer);
    window->renderer = NULL;
  }
  if (window->window != NULL) {
    SDL_DestroyWindow(window->window);
    window->window = NULL;
  }

  rc = backend->allocator.free(backend->allocator.ctx, window);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_sdl3_texture_destroy(void *obj) {
  CMPSDL3Texture *texture;
  struct CMPSDL3Backend *backend;
  int rc;

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  texture = (CMPSDL3Texture *)obj;
  backend = texture->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  texture->header.handle);
  CMP_SDL3_RETURN_IF_ERROR(rc);

#if defined(CMP_SDL3_AVAILABLE)
  if (texture->texture != NULL) {
    SDL_DestroyTexture((SDL_Texture *)texture->texture);
    texture->texture = NULL;
  }
#endif

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_sdl3_font_destroy(void *obj) {
  CMPSDL3Font *font;
  struct CMPSDL3Backend *backend;
  int rc;

  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  font = (CMPSDL3Font *)obj;
  backend = font->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  CMP_SDL3_RETURN_IF_ERROR(rc);

#if defined(CMP_SDL3_TTF_AVAILABLE)
  if (font->font != NULL) {
    TTF_CloseFont((TTF_Font *)font->font);
    font->font = NULL;
  }
#endif

  rc = backend->allocator.free(backend->allocator.ctx, font);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static const CMPObjectVTable g_cmp_sdl3_window_vtable = {
    cmp_sdl3_object_retain, cmp_sdl3_object_release, cmp_sdl3_window_destroy,
    cmp_sdl3_object_get_type_id};

static const CMPObjectVTable g_cmp_sdl3_texture_vtable = {
    cmp_sdl3_object_retain, cmp_sdl3_object_release, cmp_sdl3_texture_destroy,
    cmp_sdl3_object_get_type_id};

static const CMPObjectVTable g_cmp_sdl3_font_vtable = {
    cmp_sdl3_object_retain, cmp_sdl3_object_release, cmp_sdl3_font_destroy,
    cmp_sdl3_object_get_type_id};

static int cmp_sdl3_ws_init(void *ws, const CMPWSConfig *config) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.init");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_sdl3_ws_shutdown(void *ws) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.shutdown");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_sdl3_ws_create_window(void *ws, const CMPWSWindowConfig *config,
                                     CMPHandle *out_window) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *window;
  Uint32 sdl_flags;
  Uint32 renderer_flags;
  int rc;

  if (ws == NULL || config == NULL || out_window == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->width <= 0 || config->height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.create_window");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_window_flags_to_sdl(config->flags, &sdl_flags);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_renderer_flags_to_sdl(backend->renderer_flags, &renderer_flags);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPSDL3Window),
                                (void **)&window);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->dpi_scale = 1.0f;
  window->flags = config->flags;
  window->visible = CMP_FALSE;

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  window->window = SDL_CreateWindow(config->utf8_title, config->width,
                                    config->height, sdl_flags);
#else
  window->window = SDL_CreateWindow(config->utf8_title, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, config->width,
                                    config->height, sdl_flags);
#endif
  if (window->window == NULL) {
    cmp_sdl3_backend_log_sdl_error(backend, "ws.create_window");
    backend->allocator.free(backend->allocator.ctx, window);
    return CMP_ERR_UNKNOWN;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  window->renderer = SDL_CreateRenderer(window->window, NULL, renderer_flags);
#else
  window->renderer = SDL_CreateRenderer(window->window, -1, renderer_flags);
#endif
  if (window->renderer == NULL) {
    cmp_sdl3_backend_log_sdl_error(backend, "ws.create_renderer");
    SDL_DestroyWindow(window->window);
    backend->allocator.free(backend->allocator.ctx, window);
    return CMP_ERR_UNKNOWN;
  }

  rc = cmp_object_header_init(&window->header, CMP_SDL3_TYPE_WINDOW, 0,
                              &g_cmp_sdl3_window_vtable);
  if (rc != CMP_OK) {
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  if (rc != CMP_OK) {
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  *out_window = window->header.handle;
  return CMP_OK;
}

static int cmp_sdl3_ws_destroy_window(void *ws, CMPHandle window) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.destroy_window");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_sdl3_ws_show_window(void *ws, CMPHandle window) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.show_window");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_ShowWindow(resolved->window);
  }
  resolved->visible = CMP_TRUE;
  return CMP_OK;
}

static int cmp_sdl3_ws_hide_window(void *ws, CMPHandle window) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.hide_window");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_HideWindow(resolved->window);
  }
  resolved->visible = CMP_FALSE;
  return CMP_OK;
}

static int cmp_sdl3_ws_set_window_title(void *ws, CMPHandle window,
                                        const char *utf8_title) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_title");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_SetWindowTitle(resolved->window, utf8_title);
  }
  return CMP_OK;
}

static int cmp_sdl3_ws_set_window_size(void *ws, CMPHandle window,
                                       cmp_i32 width, cmp_i32 height) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_size");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_SetWindowSize(resolved->window, width, height);
  }
  resolved->width = width;
  resolved->height = height;
  return CMP_OK;
}

static int cmp_sdl3_ws_get_window_size(void *ws, CMPHandle window,
                                       cmp_i32 *out_width,
                                       cmp_i32 *out_height) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_size");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(resolved->window, &w, &h);
    resolved->width = (cmp_i32)w;
    resolved->height = (cmp_i32)h;
  }

  *out_width = resolved->width;
  *out_height = resolved->height;
  return CMP_OK;
}

static int cmp_sdl3_ws_set_window_dpi_scale(void *ws, CMPHandle window,
                                            CMPScalar scale) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.set_window_dpi_scale");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  resolved->dpi_scale = scale;
  return CMP_OK;
}

static int cmp_sdl3_ws_get_window_dpi_scale(void *ws, CMPHandle window,
                                            CMPScalar *out_scale) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.get_window_dpi_scale");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_scale = resolved->dpi_scale;
  return CMP_OK;
}

static int cmp_sdl3_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct CMPSDL3Backend *backend;
  int rc;
  int sdl_rc;

  if (ws == NULL || utf8_text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.set_clipboard_text");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  sdl_rc = SDL_SetClipboardText(utf8_text);
  if (sdl_rc != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "ws.set_clipboard_text");
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int cmp_sdl3_ws_get_clipboard_text(void *ws, char *buffer,
                                          cmp_usize buffer_size,
                                          cmp_usize *out_length) {
  struct CMPSDL3Backend *backend;
  char *text;
  cmp_usize length;
  cmp_usize required;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.get_clipboard_text");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  text = SDL_GetClipboardText();
  if (text == NULL) {
    cmp_sdl3_backend_log_sdl_error(backend, "ws.get_clipboard_text");
    return CMP_ERR_UNKNOWN;
  }

  length = 0;
  while (text[length] != '\0') {
    length += 1;
  }

  required = length + 1u;
  if (buffer_size < required) {
    *out_length = length;
    SDL_free(text);
    return CMP_ERR_RANGE;
  }

  if (length == 0) {
    buffer[0] = '\0';
  } else {
    memcpy(buffer, text, required);
  }
  *out_length = length;
  SDL_free(text);
  return CMP_OK;
}

static int cmp_sdl3_get_modifiers(cmp_u32 *out_modifiers) {
  SDL_Keymod mods;
  cmp_u32 out;

  if (out_modifiers == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mods = SDL_GetModState();
  out = 0u;
#if defined(SDL_KMOD_SHIFT)
  if ((mods & SDL_KMOD_SHIFT) != 0) {
    out |= CMP_MOD_SHIFT;
  }
#endif
#if defined(SDL_KMOD_CTRL)
  if ((mods & SDL_KMOD_CTRL) != 0) {
    out |= CMP_MOD_CTRL;
  }
#endif
#if defined(SDL_KMOD_ALT)
  if ((mods & SDL_KMOD_ALT) != 0) {
    out |= CMP_MOD_ALT;
  }
#endif
#if defined(SDL_KMOD_GUI)
  if ((mods & SDL_KMOD_GUI) != 0) {
    out |= CMP_MOD_META;
  }
#endif
#if defined(SDL_KMOD_CAPS)
  if ((mods & SDL_KMOD_CAPS) != 0) {
    out |= CMP_MOD_CAPS;
  }
#endif
#if defined(SDL_KMOD_NUM)
  if ((mods & SDL_KMOD_NUM) != 0) {
    out |= CMP_MOD_NUM;
  }
#endif

  *out_modifiers = out;
  return CMP_OK;
}

static int cmp_sdl3_fill_pointer_state(CMPPointerEvent *pointer) {
  int x = 0;
  int y = 0;
  Uint32 mask;
  cmp_i32 buttons;

  if (pointer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  mask = SDL_GetMouseState(&x, &y);
  buttons = 0;
#if defined(SDL_BUTTON_LMASK)
  if ((mask & SDL_BUTTON_LMASK) != 0u) {
    buttons |= 1;
  }
#endif
#if defined(SDL_BUTTON_RMASK)
  if ((mask & SDL_BUTTON_RMASK) != 0u) {
    buttons |= 2;
  }
#endif
#if defined(SDL_BUTTON_MMASK)
  if ((mask & SDL_BUTTON_MMASK) != 0u) {
    buttons |= 4;
  }
#endif
#if defined(SDL_BUTTON_X1MASK)
  if ((mask & SDL_BUTTON_X1MASK) != 0u) {
    buttons |= 8;
  }
#endif
#if defined(SDL_BUTTON_X2MASK)
  if ((mask & SDL_BUTTON_X2MASK) != 0u) {
    buttons |= 16;
  }
#endif

  pointer->pointer_id = 0;
  pointer->x = (cmp_i32)x;
  pointer->y = (cmp_i32)y;
  pointer->buttons = buttons;
  pointer->scroll_x = 0;
  pointer->scroll_y = 0;
  return CMP_OK;
}

static int cmp_sdl3_fill_key_event(const SDL_Event *event, CMPKeyEvent *key) {
  if (event == NULL || key == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  key->key_code = (cmp_u32)event->key.key;
  key->native_code = (cmp_u32)event->key.scancode;
  key->is_repeat = event->key.repeat ? CMP_TRUE : CMP_FALSE;
#else
  key->key_code = (cmp_u32)event->key.keysym.sym;
  key->native_code = (cmp_u32)event->key.keysym.scancode;
  key->is_repeat = event->key.repeat ? CMP_TRUE : CMP_FALSE;
#endif
  return CMP_OK;
}

static int cmp_sdl3_copy_text(const char *text, CMPTextEvent *out_text) {
  cmp_usize i;
  cmp_usize max_len;

  if (out_text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_text->utf8[0] = '\0';
  out_text->length = 0u;
  if (text == NULL) {
    return CMP_OK;
  }

  max_len = (cmp_usize)sizeof(out_text->utf8) - 1u;
  i = 0;
  while (text[i] != '\0' && i < max_len) {
    out_text->utf8[i] = text[i];
    i += 1u;
  }
  out_text->utf8[i] = '\0';
  out_text->length = (cmp_u32)i;
  return CMP_OK;
}

static int cmp_sdl3_text_length(const char *text, cmp_usize *out_length) {
  cmp_usize i;

  if (out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (text == NULL) {
    *out_length = 0;
    return CMP_OK;
  }

  i = 0;
  while (text[i] != '\0') {
    i += 1u;
  }
  *out_length = i;
  return CMP_OK;
}

static int cmp_sdl3_fill_scroll_event(const SDL_Event *event,
                                      CMPPointerEvent *pointer) {
  cmp_i32 scroll_x;
  cmp_i32 scroll_y;

  if (event == NULL || pointer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  scroll_x = (cmp_i32)event->wheel.x;
  scroll_y = (cmp_i32)event->wheel.y;
#if defined(SDL_MOUSEWHEEL_FLIPPED)
  if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
    scroll_x = -scroll_x;
    scroll_y = -scroll_y;
  }
#endif

  pointer->scroll_x = scroll_x;
  pointer->scroll_y = scroll_y;
  return CMP_OK;
}

static int cmp_sdl3_translate_event(struct CMPSDL3Backend *backend,
                                    const SDL_Event *sdl_event,
                                    CMPInputEvent *out_event,
                                    CMPBool *out_has_event) {
  cmp_u32 modifiers;
  CMPHandle window_handle;
  int rc;

  if (backend == NULL || sdl_event == NULL || out_event == NULL ||
      out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(out_event, 0, sizeof(*out_event));
  out_event->time_ms = (cmp_u32)(SDL_GetTicks() & 0xFFFFFFFFu);
  *out_has_event = CMP_FALSE;

  rc = cmp_sdl3_get_modifiers(&modifiers);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  window_handle.id = 0u;
  window_handle.generation = 0u;
  if (backend->active_window != NULL) {
    window_handle = backend->active_window->header.handle;
  }

#if defined(SDL_EVENT_QUIT)
  if (sdl_event->type == SDL_EVENT_QUIT) {
    out_event->type = CMP_INPUT_WINDOW_CLOSE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_QUIT)
  if (sdl_event->type == SDL_QUIT) {
    out_event->type = CMP_INPUT_WINDOW_CLOSE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif

#if defined(SDL_EVENT_KEY_DOWN)
  if (sdl_event->type == SDL_EVENT_KEY_DOWN) {
    out_event->type = CMP_INPUT_KEY_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_EVENT_KEY_UP)
  if (sdl_event->type == SDL_EVENT_KEY_UP) {
    out_event->type = CMP_INPUT_KEY_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_KEYDOWN)
  if (sdl_event->type == SDL_KEYDOWN) {
    out_event->type = CMP_INPUT_KEY_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_KEYUP)
  if (sdl_event->type == SDL_KEYUP) {
    out_event->type = CMP_INPUT_KEY_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif

#if defined(SDL_EVENT_TEXT_INPUT)
  if (sdl_event->type == SDL_EVENT_TEXT_INPUT) {
    cmp_usize text_len;
    const char *text;

    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->text.text;
    rc = cmp_sdl3_text_length(text, &text_len);
    CMP_SDL3_RETURN_IF_ERROR(rc);

    if (text_len <= (cmp_usize)(sizeof(out_event->data.text.utf8) - 1u)) {
      out_event->type = CMP_INPUT_TEXT;
      rc = cmp_sdl3_copy_text(text, &out_event->data.text);
      CMP_SDL3_RETURN_IF_ERROR(rc);
    } else {
      out_event->type = CMP_INPUT_TEXT_UTF8;
      out_event->data.text_utf8.utf8 = text;
      out_event->data.text_utf8.length = text_len;
    }

    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_TEXTINPUT)
  if (sdl_event->type == SDL_TEXTINPUT) {
    cmp_usize text_len;
    const char *text;

    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->text.text;
    rc = cmp_sdl3_text_length(text, &text_len);
    CMP_SDL3_RETURN_IF_ERROR(rc);

    if (text_len <= (cmp_usize)(sizeof(out_event->data.text.utf8) - 1u)) {
      out_event->type = CMP_INPUT_TEXT;
      rc = cmp_sdl3_copy_text(text, &out_event->data.text);
      CMP_SDL3_RETURN_IF_ERROR(rc);
    } else {
      out_event->type = CMP_INPUT_TEXT_UTF8;
      out_event->data.text_utf8.utf8 = text;
      out_event->data.text_utf8.length = text_len;
    }

    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif

#if defined(SDL_EVENT_TEXT_EDITING)
  if (sdl_event->type == SDL_EVENT_TEXT_EDITING) {
    cmp_usize text_len;
    const char *text;

    out_event->type = CMP_INPUT_TEXT_EDIT;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->edit.text;
    rc = cmp_sdl3_text_length(text, &text_len);
    CMP_SDL3_RETURN_IF_ERROR(rc);

    out_event->data.text_edit.utf8 = text;
    out_event->data.text_edit.length = text_len;
    out_event->data.text_edit.cursor = (cmp_i32)sdl_event->edit.start;
    out_event->data.text_edit.selection_length =
        (cmp_i32)sdl_event->edit.length;

    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_TEXTEDITING)
  if (sdl_event->type == SDL_TEXTEDITING) {
    cmp_usize text_len;
    const char *text;

    out_event->type = CMP_INPUT_TEXT_EDIT;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->edit.text;
    rc = cmp_sdl3_text_length(text, &text_len);
    CMP_SDL3_RETURN_IF_ERROR(rc);

    out_event->data.text_edit.utf8 = text;
    out_event->data.text_edit.length = text_len;
    out_event->data.text_edit.cursor = (cmp_i32)sdl_event->edit.start;
    out_event->data.text_edit.selection_length =
        (cmp_i32)sdl_event->edit.length;

    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif

#if defined(SDL_EVENT_MOUSE_BUTTON_DOWN)
  if (sdl_event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    out_event->type = CMP_INPUT_POINTER_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_EVENT_MOUSE_BUTTON_UP)
  if (sdl_event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
    out_event->type = CMP_INPUT_POINTER_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_EVENT_MOUSE_MOTION)
  if (sdl_event->type == SDL_EVENT_MOUSE_MOTION) {
    out_event->type = CMP_INPUT_POINTER_MOVE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_EVENT_MOUSE_WHEEL)
  if (sdl_event->type == SDL_EVENT_MOUSE_WHEEL) {
    out_event->type = CMP_INPUT_POINTER_SCROLL;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    rc = cmp_sdl3_fill_scroll_event(sdl_event, &out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_MOUSEBUTTONDOWN)
  if (sdl_event->type == SDL_MOUSEBUTTONDOWN) {
    out_event->type = CMP_INPUT_POINTER_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_MOUSEBUTTONUP)
  if (sdl_event->type == SDL_MOUSEBUTTONUP) {
    out_event->type = CMP_INPUT_POINTER_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_MOUSEMOTION)
  if (sdl_event->type == SDL_MOUSEMOTION) {
    out_event->type = CMP_INPUT_POINTER_MOVE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_MOUSEWHEEL)
  if (sdl_event->type == SDL_MOUSEWHEEL) {
    out_event->type = CMP_INPUT_POINTER_SCROLL;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = cmp_sdl3_fill_pointer_state(&out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    rc = cmp_sdl3_fill_scroll_event(sdl_event, &out_event->data.pointer);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif

#if defined(SDL_EVENT_WINDOW_RESIZED)
  if (sdl_event->type == SDL_EVENT_WINDOW_RESIZED) {
    out_event->type = CMP_INPUT_WINDOW_RESIZE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    if (backend->active_window != NULL &&
        backend->active_window->window != NULL) {
      int w = 0;
      int h = 0;
      SDL_GetWindowSize(backend->active_window->window, &w, &h);
      out_event->data.window.width = (cmp_i32)w;
      out_event->data.window.height = (cmp_i32)h;
    }
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif
#if defined(SDL_WINDOWEVENT)
  if (sdl_event->type == SDL_WINDOWEVENT) {
    out_event->type = CMP_INPUT_WINDOW_RESIZE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    if (backend->active_window != NULL &&
        backend->active_window->window != NULL) {
      int w = 0;
      int h = 0;
      SDL_GetWindowSize(backend->active_window->window, &w, &h);
      out_event->data.window.width = (cmp_i32)w;
      out_event->data.window.height = (cmp_i32)h;
    }
    *out_has_event = CMP_TRUE;
    return CMP_OK;
  }
#endif

  return CMP_OK;
}

static int cmp_sdl3_ws_poll_event(void *ws, CMPInputEvent *out_event,
                                  CMPBool *out_has_event) {
  struct CMPSDL3Backend *backend;
  SDL_Event event;
  CMPBool has_event;
  int rc;
  int sdl_rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.poll_event");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  has_event = CMP_FALSE;
  sdl_rc = SDL_PollEvent(&event);
  while (sdl_rc != 0) {
    rc = cmp_sdl3_translate_event(backend, &event, out_event, &has_event);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    if (has_event) {
      *out_has_event = CMP_TRUE;
      return CMP_OK;
    }
    sdl_rc = SDL_PollEvent(&event);
  }

  memset(out_event, 0, sizeof(*out_event));
  *out_has_event = CMP_FALSE;
  return CMP_OK;
}

static int cmp_sdl3_ws_pump_events(void *ws) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.pump_events");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  SDL_PumpEvents();
  return CMP_OK;
}

static int cmp_sdl3_time_ms(cmp_u32 *out_time_ms) {
  Uint64 ticks;

  if (out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  ticks = (Uint64)SDL_GetTicks();
  *out_time_ms = (cmp_u32)(ticks & 0xFFFFFFFFu);
  return CMP_OK;
}

static int cmp_sdl3_ws_get_time_ms(void *ws, cmp_u32 *out_time_ms) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (ws == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)ws;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return cmp_sdl3_time_ms(out_time_ms);
}

static const CMPWSVTable g_cmp_sdl3_ws_vtable = {
    cmp_sdl3_ws_init,
    cmp_sdl3_ws_shutdown,
    cmp_sdl3_ws_create_window,
    cmp_sdl3_ws_destroy_window,
    cmp_sdl3_ws_show_window,
    cmp_sdl3_ws_hide_window,
    cmp_sdl3_ws_set_window_title,
    cmp_sdl3_ws_set_window_size,
    cmp_sdl3_ws_get_window_size,
    cmp_sdl3_ws_set_window_dpi_scale,
    cmp_sdl3_ws_get_window_dpi_scale,
    cmp_sdl3_ws_set_clipboard_text,
    cmp_sdl3_ws_get_clipboard_text,
    cmp_sdl3_ws_poll_event,
    cmp_sdl3_ws_pump_events,
    cmp_sdl3_ws_get_time_ms};

static int cmp_sdl3_get_active_renderer(struct CMPSDL3Backend *backend,
                                        SDL_Renderer **out_renderer) {
  if (backend == NULL || out_renderer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (backend->active_window == NULL ||
      backend->active_window->renderer == NULL) {
    return CMP_ERR_STATE;
  }

  *out_renderer = backend->active_window->renderer;
  return CMP_OK;
}

static int cmp_sdl3_render_fill_rect(SDL_Renderer *renderer,
                                     const CMPRect *rect) {
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_FRect sdl_rect;
#else
  SDL_Rect sdl_rect;
#endif
  int rc;

  if (renderer == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  sdl_rect.x = rect->x;
  sdl_rect.y = rect->y;
  sdl_rect.w = rect->width;
  sdl_rect.h = rect->height;
  rc = SDL_RenderFillRect(renderer, &sdl_rect);
#else
  sdl_rect.x = (int)rect->x;
  sdl_rect.y = (int)rect->y;
  sdl_rect.w = (int)rect->width;
  sdl_rect.h = (int)rect->height;
  rc = SDL_RenderFillRect(renderer, &sdl_rect);
#endif

  if (rc != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int cmp_sdl3_render_line(SDL_Renderer *renderer, CMPScalar x0,
                                CMPScalar y0, CMPScalar x1, CMPScalar y1) {
  int rc;

  if (renderer == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  rc = SDL_RenderLine(renderer, x0, y0, x1, y1);
#else
  rc = SDL_RenderDrawLine(renderer, (int)x0, (int)y0, (int)x1, (int)y1);
#endif

  if (rc != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int cmp_sdl3_gfx_begin_frame(void *gfx, CMPHandle window, cmp_i32 width,
                                    cmp_i32 height, CMPScalar dpi_scale) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  if (backend->active_window != NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  resolved->width = width;
  resolved->height = height;
  resolved->dpi_scale = dpi_scale;
  backend->active_window = resolved;
  backend->clip_depth = 0u;
  backend->has_transform = CMP_FALSE;
  return CMP_OK;
}

static int cmp_sdl3_gfx_end_frame(void *gfx, CMPHandle window) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Window *resolved;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_resolve(backend, window, CMP_SDL3_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (backend->active_window != resolved) {
    return CMP_ERR_STATE;
  }

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.end_frame");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->renderer != NULL) {
    SDL_RenderPresent(resolved->renderer);
  }
  backend->active_window = NULL;
  return CMP_OK;
}

static int cmp_sdl3_gfx_clear(void *gfx, CMPColor color) {
  struct CMPSDL3Backend *backend;
  SDL_Renderer *renderer;
  cmp_u8 r;
  cmp_u8 g;
  cmp_u8 b;
  cmp_u8 a;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.clear");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_color_to_u8(color.r, &r);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.g, &g);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.b, &b);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.a, &a);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (rc != 0) {
    return CMP_ERR_UNKNOWN;
  }
  rc = SDL_RenderClear(renderer);
  if (rc != 0) {
    return CMP_ERR_UNKNOWN;
  }

  return CMP_OK;
}

static int cmp_sdl3_gfx_draw_rect(void *gfx, const CMPRect *rect,
                                  CMPColor color, CMPScalar corner_radius) {
  struct CMPSDL3Backend *backend;
  SDL_Renderer *renderer;
  cmp_u8 r;
  cmp_u8 g;
  cmp_u8 b;
  cmp_u8 a;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_color_to_u8(color.r, &r);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.g, &g);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.b, &b);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.a, &a);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (rc != 0) {
    return CMP_ERR_UNKNOWN;
  }

  rc = cmp_sdl3_render_fill_rect(renderer, rect);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_sdl3_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                  CMPScalar x1, CMPScalar y1, CMPColor color,
                                  CMPScalar thickness) {
  struct CMPSDL3Backend *backend;
  SDL_Renderer *renderer;
  cmp_u8 r;
  cmp_u8 g;
  cmp_u8 b;
  cmp_u8 a;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_line");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_color_to_u8(color.r, &r);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.g, &g);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.b, &b);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_color_to_u8(color.a, &a);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (rc != 0) {
    return CMP_ERR_UNKNOWN;
  }

  rc = cmp_sdl3_render_line(renderer, x0, y0, x1, y1);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_sdl3_gfx_draw_path(void *gfx, const CMPPath *path,
                                  CMPColor color) {
  struct CMPSDL3Backend *backend;
  SDL_Renderer *renderer;
  int rc;

  CMP_UNUSED(color);

  if (gfx == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return CMP_ERR_STATE;
  }
  if (path->count == 0) {
    return CMP_OK;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_path");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  CMP_UNUSED(renderer);

  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_gfx_push_clip(void *gfx, const CMPRect *rect) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.push_clip");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (backend->clip_depth >= CMP_SDL3_CLIP_STACK_CAPACITY) {
    return CMP_ERR_OVERFLOW;
  }

  backend->clip_stack[backend->clip_depth] = *rect;
  backend->clip_depth += 1u;
  return CMP_OK;
}

static int cmp_sdl3_gfx_pop_clip(void *gfx) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (backend->clip_depth == 0u) {
    return CMP_ERR_STATE;
  }

  backend->clip_depth -= 1u;
  return CMP_OK;
}

static int cmp_sdl3_gfx_set_transform(void *gfx, const CMPMat3 *transform) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (gfx == NULL || transform == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.set_transform");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  backend->transform = *transform;
  backend->has_transform = CMP_TRUE;
  return CMP_OK;
}

static int cmp_sdl3_gfx_create_texture(void *gfx, cmp_i32 width, cmp_i32 height,
                                       cmp_u32 format, const void *pixels,
                                       cmp_usize size, CMPHandle *out_texture) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Texture *texture;
  SDL_Renderer *renderer;
  SDL_Texture *sdl_texture;
  Uint32 sdl_format;
  cmp_usize required;
  cmp_usize row_bytes;
  cmp_u32 bpp;
  int rc;

  if (gfx == NULL || out_texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.create_texture");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_texture_format_to_sdl(format, &sdl_format, &bpp);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_mul_usize((cmp_usize)width, (cmp_usize)height, &required);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_mul_usize(required, (cmp_usize)bpp, &required);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (pixels != NULL && size < required) {
    return CMP_ERR_RANGE;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPSDL3Texture),
                                (void **)&texture);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->bytes_per_pixel = bpp;
  texture->renderer = renderer;

  sdl_texture = SDL_CreateTexture(renderer, sdl_format,
                                  SDL_TEXTUREACCESS_STREAMING, width, height);
  if (sdl_texture == NULL) {
    cmp_sdl3_backend_log_sdl_error(backend, "gfx.create_texture");
    backend->allocator.free(backend->allocator.ctx, texture);
    return CMP_ERR_UNKNOWN;
  }
  texture->texture = sdl_texture;

  if (pixels != NULL && size > 0) {
    rc = cmp_sdl3_mul_usize((cmp_usize)width, (cmp_usize)bpp, &row_bytes);
    if (rc != CMP_OK) {
      SDL_DestroyTexture(sdl_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return rc;
    }
    if (SDL_UpdateTexture(sdl_texture, NULL, pixels, (int)row_bytes) != 0) {
      cmp_sdl3_backend_log_sdl_error(backend, "gfx.create_texture");
      SDL_DestroyTexture(sdl_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return CMP_ERR_UNKNOWN;
    }
  }

  rc = cmp_object_header_init(&texture->header, CMP_SDL3_TYPE_TEXTURE, 0,
                              &g_cmp_sdl3_texture_vtable);
  if (rc != CMP_OK) {
    SDL_DestroyTexture(sdl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != CMP_OK) {
    SDL_DestroyTexture(sdl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return CMP_OK;
}

static int cmp_sdl3_gfx_update_texture(void *gfx, CMPHandle texture, cmp_i32 x,
                                       cmp_i32 y, cmp_i32 width, cmp_i32 height,
                                       const void *pixels, cmp_usize size) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Texture *resolved;
  SDL_Rect rect;
  cmp_usize required;
  cmp_usize row_bytes;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_resolve(backend, texture, CMP_SDL3_TYPE_TEXTURE,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.update_texture");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->texture == NULL) {
    return CMP_ERR_STATE;
  }
  if (x + width > resolved->width || y + height > resolved->height) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_sdl3_mul_usize((cmp_usize)width, (cmp_usize)height, &required);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  rc = cmp_sdl3_mul_usize(required, (cmp_usize)resolved->bytes_per_pixel,
                          &required);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (pixels != NULL && size < required) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL || size == 0) {
    return CMP_OK;
  }

  rc = cmp_sdl3_mul_usize((cmp_usize)width,
                          (cmp_usize)resolved->bytes_per_pixel, &row_bytes);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rect.x = (int)x;
  rect.y = (int)y;
  rect.w = (int)width;
  rect.h = (int)height;

  if (SDL_UpdateTexture((SDL_Texture *)resolved->texture, &rect, pixels,
                        (int)row_bytes) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "gfx.update_texture");
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int cmp_sdl3_gfx_destroy_texture(void *gfx, CMPHandle texture) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Texture *resolved;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_resolve(backend, texture, CMP_SDL3_TYPE_TEXTURE,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc =
      cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_sdl3_gfx_draw_texture(void *gfx, CMPHandle texture,
                                     const CMPRect *src, const CMPRect *dst,
                                     CMPScalar opacity) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Texture *resolved;
  SDL_Renderer *renderer;
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_FRect sdl_src;
  SDL_FRect sdl_dst;
#else
  SDL_Rect sdl_src;
  SDL_Rect sdl_dst;
#endif
  cmp_u8 alpha;
  int rc;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)gfx;
  rc = cmp_sdl3_backend_resolve(backend, texture, CMP_SDL3_TYPE_TEXTURE,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->texture == NULL) {
    return CMP_ERR_STATE;
  }
  if (src->width < 0.0f || src->height < 0.0f || dst->width < 0.0f ||
      dst->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  if ((SDL_Renderer *)resolved->renderer != renderer) {
    return CMP_ERR_STATE;
  }

  rc = cmp_sdl3_color_to_u8(opacity, &alpha);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (SDL_SetTextureBlendMode((SDL_Texture *)resolved->texture,
                              SDL_BLENDMODE_BLEND) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return CMP_ERR_UNKNOWN;
  }
  if (SDL_SetTextureAlphaMod((SDL_Texture *)resolved->texture, alpha) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return CMP_ERR_UNKNOWN;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  sdl_src.x = src->x;
  sdl_src.y = src->y;
  sdl_src.w = src->width;
  sdl_src.h = src->height;
  sdl_dst.x = dst->x;
  sdl_dst.y = dst->y;
  sdl_dst.w = dst->width;
  sdl_dst.h = dst->height;
  if (SDL_RenderTexture(renderer, (SDL_Texture *)resolved->texture, &sdl_src,
                        &sdl_dst) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return CMP_ERR_UNKNOWN;
  }
#else
  sdl_src.x = (int)src->x;
  sdl_src.y = (int)src->y;
  sdl_src.w = (int)src->width;
  sdl_src.h = (int)src->height;
  sdl_dst.x = (int)dst->x;
  sdl_dst.y = (int)dst->y;
  sdl_dst.w = (int)dst->width;
  sdl_dst.h = (int)dst->height;
  if (SDL_RenderCopy(renderer, (SDL_Texture *)resolved->texture, &sdl_src,
                     &sdl_dst) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return CMP_ERR_UNKNOWN;
  }
#endif
  return CMP_OK;
}

static const CMPGfxVTable g_cmp_sdl3_gfx_vtable = {
    cmp_sdl3_gfx_begin_frame,    cmp_sdl3_gfx_end_frame,
    cmp_sdl3_gfx_clear,          cmp_sdl3_gfx_draw_rect,
    cmp_sdl3_gfx_draw_line,      cmp_sdl3_gfx_draw_path,
    cmp_sdl3_gfx_push_clip,      cmp_sdl3_gfx_pop_clip,
    cmp_sdl3_gfx_set_transform,  cmp_sdl3_gfx_create_texture,
    cmp_sdl3_gfx_update_texture, cmp_sdl3_gfx_destroy_texture,
    cmp_sdl3_gfx_draw_texture};

static int cmp_sdl3_text_make_cstr(struct CMPSDL3Backend *backend,
                                   const char *utf8, cmp_usize utf8_len,
                                   char **out_text) {
  cmp_usize max_value;
  cmp_usize required;
  char *buffer;
  int rc;

  if (backend == NULL || out_text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  *out_text = NULL;
  if (utf8_len == 0) {
    if (utf8 != NULL && utf8[0] != '\0') {
      return CMP_ERR_INVALID_ARGUMENT;
    }
    return CMP_OK;
  }
  if (utf8 == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (utf8_len > max_value - 1u) {
    return CMP_ERR_OVERFLOW;
  }
  required = utf8_len + 1u;

  rc = backend->allocator.alloc(backend->allocator.ctx, required,
                                (void **)&buffer);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memcpy(buffer, utf8, utf8_len);
  buffer[utf8_len] = '\0';
  *out_text = buffer;
  return CMP_OK;
}

static int cmp_sdl3_text_free_cstr(struct CMPSDL3Backend *backend, char *text) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (text == NULL) {
    return CMP_OK;
  }
  return backend->allocator.free(backend->allocator.ctx, text);
}

static int cmp_sdl3_text_create_font(void *text, const char *utf8_family,
                                     cmp_i32 size_px, cmp_i32 weight,
                                     CMPBool italic, CMPHandle *out_font) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Font *font;
#if defined(CMP_SDL3_TTF_AVAILABLE)
  TTF_Font *ttf_font;
  int style;
#endif
  int rc;

  if (text == NULL || out_font == NULL || utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPSDL3Backend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.create_font");
  CMP_SDL3_RETURN_IF_ERROR(rc);

#if !defined(CMP_SDL3_TTF_AVAILABLE)
  CMP_UNUSED(font);
  return CMP_ERR_UNSUPPORTED;
#else
  ttf_font = TTF_OpenFont(utf8_family, size_px);
  if (ttf_font == NULL) {
    cmp_sdl3_backend_log_ttf_error(backend, "text.create_font");
    return CMP_ERR_NOT_FOUND;
  }

  style = TTF_STYLE_NORMAL;
#if defined(TTF_STYLE_BOLD)
  if (weight >= 600) {
    style |= TTF_STYLE_BOLD;
  }
#endif
#if defined(TTF_STYLE_ITALIC)
  if (italic) {
    style |= TTF_STYLE_ITALIC;
  }
#endif
  TTF_SetFontStyle(ttf_font, style);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPSDL3Font),
                                (void **)&font);
  if (rc != CMP_OK) {
    TTF_CloseFont(ttf_font);
    return rc;
  }

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? CMP_TRUE : CMP_FALSE;
  font->font = ttf_font;

  rc = cmp_object_header_init(&font->header, CMP_SDL3_TYPE_FONT, 0,
                              &g_cmp_sdl3_font_vtable);
  if (rc != CMP_OK) {
    TTF_CloseFont(ttf_font);
    backend->allocator.free(backend->allocator.ctx, font);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  if (rc != CMP_OK) {
    TTF_CloseFont(ttf_font);
    backend->allocator.free(backend->allocator.ctx, font);
    return rc;
  }

  *out_font = font->header.handle;
  return CMP_OK;
#endif
}

static int cmp_sdl3_text_destroy_font(void *text, CMPHandle font) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Font *resolved;
  int rc;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)text;
  rc = cmp_sdl3_backend_resolve(backend, font, CMP_SDL3_TYPE_FONT,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.destroy_font");
  CMP_SDL3_RETURN_IF_ERROR(rc);

#if !defined(CMP_SDL3_TTF_AVAILABLE)
  return CMP_ERR_UNSUPPORTED;
#else
  return cmp_object_release(&resolved->header);
#endif
}

static int cmp_sdl3_text_measure_text(void *text, CMPHandle font,
                                      const char *utf8, cmp_usize utf8_len,
                                      CMPScalar *out_width,
                                      CMPScalar *out_height,
                                      CMPScalar *out_baseline) {
  struct CMPSDL3Backend *backend;
  CMPSDL3Font *resolved;
#if defined(CMP_SDL3_TTF_AVAILABLE)
  char *temp;
  int text_width;
  int text_height;
  int rc_free;
#endif
  CMPScalar size;
  int rc;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)text;
  rc = cmp_sdl3_backend_resolve(backend, font, CMP_SDL3_TYPE_FONT,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.measure_text");
  CMP_SDL3_RETURN_IF_ERROR(rc);

#if !defined(CMP_SDL3_TTF_AVAILABLE)
  CMP_UNUSED(size);
  return CMP_ERR_UNSUPPORTED;
#else
  if (resolved->font == NULL) {
    return CMP_ERR_STATE;
  }

  temp = NULL;
  if (utf8_len == 0) {
    text_width = 0;
    text_height = 0;
  } else {
    rc = cmp_sdl3_text_make_cstr(backend, utf8, utf8_len, &temp);
    CMP_SDL3_RETURN_IF_ERROR(rc);

    if (TTF_SizeUTF8((TTF_Font *)resolved->font, temp, &text_width,
                     &text_height) != 0) {
      cmp_sdl3_backend_log_ttf_error(backend, "text.measure_text");
      rc_free = cmp_sdl3_text_free_cstr(backend, temp);
      if (rc_free != CMP_OK) {
        return rc_free;
      }
      return CMP_ERR_UNKNOWN;
    }
    rc = cmp_sdl3_text_free_cstr(backend, temp);
    CMP_SDL3_RETURN_IF_ERROR(rc);
  }

  size = (CMPScalar)TTF_FontHeight((TTF_Font *)resolved->font);
  *out_width = (CMPScalar)text_width;
  if (text_height == 0) {
    *out_height = size;
  } else {
    *out_height = (CMPScalar)text_height;
  }
  *out_baseline = (CMPScalar)TTF_FontAscent((TTF_Font *)resolved->font);
  return CMP_OK;
#endif
}

static int cmp_sdl3_text_draw_text(void *text, CMPHandle font, const char *utf8,
                                   cmp_usize utf8_len, CMPScalar x, CMPScalar y,
                                   CMPColor color) {
  struct CMPSDL3Backend *backend;
#if defined(CMP_SDL3_TTF_AVAILABLE)
  CMPSDL3Font *resolved;
  SDL_Renderer *renderer;
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Color sdl_color;
  char *temp;
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_FRect dst_rect;
#else
  SDL_Rect dst_rect;
#endif
  cmp_u8 r;
  cmp_u8 g;
  cmp_u8 b;
  cmp_u8 a;
  int rc_free;
#endif
  int rc;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)text;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.draw_text");
  CMP_SDL3_RETURN_IF_ERROR(rc);

#if !defined(CMP_SDL3_TTF_AVAILABLE)
  CMP_UNUSED(font);
  CMP_UNUSED(x);
  CMP_UNUSED(y);
  CMP_UNUSED(color);
  return CMP_ERR_UNSUPPORTED;
#else
  rc = cmp_sdl3_backend_resolve(backend, font, CMP_SDL3_TYPE_FONT,
                                (void **)&resolved);
  CMP_SDL3_RETURN_IF_ERROR(rc);
  if (resolved->font == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_sdl3_get_active_renderer(backend, &renderer);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  temp = NULL;
  if (utf8_len == 0) {
    return CMP_OK;
  }

  rc = cmp_sdl3_text_make_cstr(backend, utf8, utf8_len, &temp);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  rc = cmp_sdl3_color_to_u8(color.r, &r);
  if (rc != CMP_OK) {
    rc_free = cmp_sdl3_text_free_cstr(backend, temp);
    if (rc_free != CMP_OK) {
      return rc_free;
    }
    return rc;
  }
  rc = cmp_sdl3_color_to_u8(color.g, &g);
  if (rc != CMP_OK) {
    rc_free = cmp_sdl3_text_free_cstr(backend, temp);
    if (rc_free != CMP_OK) {
      return rc_free;
    }
    return rc;
  }
  rc = cmp_sdl3_color_to_u8(color.b, &b);
  if (rc != CMP_OK) {
    rc_free = cmp_sdl3_text_free_cstr(backend, temp);
    if (rc_free != CMP_OK) {
      return rc_free;
    }
    return rc;
  }
  rc = cmp_sdl3_color_to_u8(color.a, &a);
  if (rc != CMP_OK) {
    rc_free = cmp_sdl3_text_free_cstr(backend, temp);
    if (rc_free != CMP_OK) {
      return rc_free;
    }
    return rc;
  }

  sdl_color.r = r;
  sdl_color.g = g;
  sdl_color.b = b;
  sdl_color.a = a;

  surface = TTF_RenderUTF8_Blended((TTF_Font *)resolved->font, temp, sdl_color);
  rc = cmp_sdl3_text_free_cstr(backend, temp);
  if (rc != CMP_OK) {
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
    SDL_DestroySurface(surface);
#else
    SDL_FreeSurface(surface);
#endif
    return rc;
  }
  if (surface == NULL) {
    cmp_sdl3_backend_log_ttf_error(backend, "text.draw_text");
    return CMP_ERR_UNKNOWN;
  }

  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    cmp_sdl3_backend_log_sdl_error(backend, "text.draw_text");
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
    SDL_DestroySurface(surface);
#else
    SDL_FreeSurface(surface);
#endif
    return CMP_ERR_UNKNOWN;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.w = (float)surface->w;
  dst_rect.h = (float)surface->h;
  if (SDL_RenderTexture(renderer, texture, NULL, &dst_rect) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "text.draw_text");
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    return CMP_ERR_UNKNOWN;
  }
#else
  dst_rect.x = (int)x;
  dst_rect.y = (int)y;
  dst_rect.w = surface->w;
  dst_rect.h = surface->h;
  if (SDL_RenderCopy(renderer, texture, NULL, &dst_rect) != 0) {
    cmp_sdl3_backend_log_sdl_error(backend, "text.draw_text");
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return CMP_ERR_UNKNOWN;
  }
#endif

  SDL_DestroyTexture(texture);
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_DestroySurface(surface);
#else
  SDL_FreeSurface(surface);
#endif

  return CMP_OK;
#endif
}

static const CMPTextVTable g_cmp_sdl3_text_vtable = {
    cmp_sdl3_text_create_font, cmp_sdl3_text_destroy_font,
    cmp_sdl3_text_measure_text, cmp_sdl3_text_draw_text};

static int cmp_sdl3_io_read_file(void *io, const char *utf8_path, void *buffer,
                                 cmp_usize buffer_size, cmp_usize *out_read) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)io;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_io_read_file_alloc(void *io, const char *utf8_path,
                                       const CMPAllocator *allocator,
                                       void **out_data, cmp_usize *out_size) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)io;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_io_write_file(void *io, const char *utf8_path,
                                  const void *data, cmp_usize size,
                                  CMPBool overwrite) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)io;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.write_file");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_io_file_exists(void *io, const char *utf8_path,
                                   CMPBool *out_exists) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)io;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.file_exists");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_exists = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_io_delete_file(void *io, const char *utf8_path) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)io;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.delete_file");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_io_stat_file(void *io, const char *utf8_path,
                                 CMPFileInfo *out_info) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)io;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.stat_file");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return CMP_ERR_UNSUPPORTED;
}

static const CMPIOVTable g_cmp_sdl3_io_vtable = {
    cmp_sdl3_io_read_file,   cmp_sdl3_io_read_file_alloc,
    cmp_sdl3_io_write_file,  cmp_sdl3_io_file_exists,
    cmp_sdl3_io_delete_file, cmp_sdl3_io_stat_file};

static int cmp_sdl3_sensors_is_available(void *sensors, cmp_u32 type,
                                         CMPBool *out_available) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)sensors;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                            "sensors.is_available");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_available = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_sensors_start(void *sensors, cmp_u32 type) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)sensors;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.start");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_sensors_stop(void *sensors, cmp_u32 type) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)sensors;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.stop");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_sensors_read(void *sensors, cmp_u32 type,
                                 CMPSensorReading *out_reading,
                                 CMPBool *out_has_reading) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)sensors;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.read");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static const CMPSensorsVTable g_cmp_sdl3_sensors_vtable = {
    cmp_sdl3_sensors_is_available, cmp_sdl3_sensors_start,
    cmp_sdl3_sensors_stop, cmp_sdl3_sensors_read};

static int cmp_sdl3_camera_open(void *camera, cmp_u32 camera_id) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(camera_id);

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)camera;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.open");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_camera_open_with_config(void *camera,
                                            const CMPCameraConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_sdl3_camera_open(camera, config->camera_id);
}

static int cmp_sdl3_camera_close(void *camera) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)camera;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.close");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_camera_start(void *camera) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)camera;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.start");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_camera_stop(void *camera) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)camera;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.stop");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_camera_read_frame(void *camera, CMPCameraFrame *out_frame,
                                      CMPBool *out_has_frame) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)camera;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.read_frame");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static const CMPCameraVTable g_cmp_sdl3_camera_vtable = {
    cmp_sdl3_camera_open,  cmp_sdl3_camera_open_with_config,
    cmp_sdl3_camera_close, cmp_sdl3_camera_start,
    cmp_sdl3_camera_stop,  cmp_sdl3_camera_read_frame};

static int cmp_sdl3_image_decode(void *image,
                                 const CMPImageDecodeRequest *request,
                                 const CMPAllocator *allocator,
                                 CMPImageData *out_image) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (image == NULL || request == NULL || allocator == NULL ||
      out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)image;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "image.decode");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_image, 0, sizeof(*out_image));
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_image_free(void *image, const CMPAllocator *allocator,
                               CMPImageData *image_data) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (image == NULL || allocator == NULL || image_data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)image;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "image.free");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPImageVTable g_cmp_sdl3_image_vtable = {cmp_sdl3_image_decode,
                                                       cmp_sdl3_image_free};

static int cmp_sdl3_video_open(void *video,
                               const CMPVideoOpenRequest *request) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (video == NULL || request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)video;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "video.open");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_video_close(void *video) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)video;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "video.close");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_video_read_frame(void *video, CMPVideoFrame *out_frame,
                                     CMPBool *out_has_frame) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (video == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)video;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "video.read_frame");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static const CMPVideoVTable g_cmp_sdl3_video_vtable = {
    cmp_sdl3_video_open, cmp_sdl3_video_close, cmp_sdl3_video_read_frame};

static int cmp_sdl3_audio_decode(void *audio,
                                 const CMPAudioDecodeRequest *request,
                                 const CMPAllocator *allocator,
                                 CMPAudioData *out_audio) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (audio == NULL || request == NULL || allocator == NULL ||
      out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)audio;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "audio.decode");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_audio, 0, sizeof(*out_audio));
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_audio_free(void *audio, const CMPAllocator *allocator,
                               CMPAudioData *audio_data) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (audio == NULL || allocator == NULL || audio_data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)audio;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "audio.free");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPAudioVTable g_cmp_sdl3_audio_vtable = {cmp_sdl3_audio_decode,
                                                       cmp_sdl3_audio_free};

#if defined(CMP_LIBCURL_AVAILABLE)
typedef struct CMPSDL3CurlBuffer {
  const CMPAllocator *allocator;
  void *data;
  cmp_usize size;
  cmp_usize capacity;
  int error;
} CMPSDL3CurlBuffer;

static cmp_u32 g_cmp_sdl3_curl_refcount = 0u;

static int cmp_sdl3_network_add_usize(cmp_usize a, cmp_usize b,
                                      cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (b > ((cmp_usize) ~(cmp_usize)0) - a) {
    return CMP_ERR_OVERFLOW;
  }
  *out_value = a + b;
  return CMP_OK;
}

static int cmp_sdl3_network_mul_usize(cmp_usize a, cmp_usize b,
                                      cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (a != 0 && b > ((cmp_usize) ~(cmp_usize)0) / a) {
    return CMP_ERR_OVERFLOW;
  }
  *out_value = a * b;
  return CMP_OK;
}

static int cmp_sdl3_network_error_from_curl(CURLcode code) {
  switch (code) {
  case CURLE_OK:
    return CMP_OK;
  case CURLE_UNSUPPORTED_PROTOCOL:
    return CMP_ERR_UNSUPPORTED;
  case CURLE_URL_MALFORMAT:
  case CURLE_BAD_FUNCTION_ARGUMENT:
    return CMP_ERR_INVALID_ARGUMENT;
  case CURLE_OUT_OF_MEMORY:
    return CMP_ERR_OUT_OF_MEMORY;
  case CURLE_OPERATION_TIMEDOUT:
    return CMP_ERR_TIMEOUT;
  case CURLE_COULDNT_RESOLVE_HOST:
  case CURLE_COULDNT_RESOLVE_PROXY:
  case CURLE_REMOTE_FILE_NOT_FOUND:
    return CMP_ERR_NOT_FOUND;
  case CURLE_LOGIN_DENIED:
  case CURLE_REMOTE_ACCESS_DENIED:
    return CMP_ERR_PERMISSION;
  case CURLE_TOO_MANY_REDIRECTS:
  case CURLE_FILESIZE_EXCEEDED:
    return CMP_ERR_RANGE;
  default:
    return CMP_ERR_IO;
  }
}

static int cmp_sdl3_libcurl_init(void) {
  CURLcode curl_rc;

  if (g_cmp_sdl3_curl_refcount == 0u) {
    curl_rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_rc != CURLE_OK) {
      return cmp_sdl3_network_error_from_curl(curl_rc);
    }
  }

  g_cmp_sdl3_curl_refcount += 1u;
  return CMP_OK;
}

static int cmp_sdl3_libcurl_shutdown(void) {
  if (g_cmp_sdl3_curl_refcount == 0u) {
    return CMP_ERR_STATE;
  }

  g_cmp_sdl3_curl_refcount -= 1u;
  if (g_cmp_sdl3_curl_refcount == 0u) {
    curl_global_cleanup();
  }
  return CMP_OK;
}

static size_t cmp_sdl3_network_write_cb(char *ptr, size_t size, size_t nmemb,
                                        void *userdata) {
  CMPSDL3CurlBuffer *buffer;
  cmp_usize chunk;
  cmp_usize new_size;
  cmp_usize alloc_size;
  void *new_data;
  int rc;

  if (userdata == NULL || ptr == NULL) {
    return 0;
  }

  buffer = (CMPSDL3CurlBuffer *)userdata;
  if (buffer->allocator == NULL) {
    buffer->error = CMP_ERR_INVALID_ARGUMENT;
    return 0;
  }

  if (size == 0 || nmemb == 0) {
    return 0;
  }

  rc = cmp_sdl3_network_mul_usize((cmp_usize)size, (cmp_usize)nmemb, &chunk);
  if (rc != CMP_OK) {
    buffer->error = rc;
    return 0;
  }

  rc = cmp_sdl3_network_add_usize(buffer->size, chunk, &new_size);
  if (rc != CMP_OK) {
    buffer->error = rc;
    return 0;
  }

  if (new_size > buffer->capacity) {
    alloc_size = new_size;
    new_data = NULL;
    if (buffer->data == NULL) {
      rc = buffer->allocator->alloc(buffer->allocator->ctx, alloc_size,
                                    &new_data);
    } else {
      rc = buffer->allocator->realloc(buffer->allocator->ctx, buffer->data,
                                      alloc_size, &new_data);
    }
    if (rc != CMP_OK) {
      buffer->error = rc;
      return 0;
    }
    buffer->data = new_data;
    buffer->capacity = alloc_size;
  }

  memcpy((cmp_u8 *)buffer->data + buffer->size, ptr, (size_t)chunk);
  buffer->size = new_size;
  return (size_t)chunk;
}

static int cmp_sdl3_network_append_headers(struct CMPSDL3Backend *backend,
                                           const char *headers,
                                           struct curl_slist **out_list) {
  const char *cursor;
  struct curl_slist *list;

  if (backend == NULL || out_list == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  list = *out_list;
  if (headers == NULL || headers[0] == '\0') {
    *out_list = list;
    return CMP_OK;
  }

  cursor = headers;
  while (*cursor != '\0') {
    const char *line_start;
    const char *line_end;
    const char *colon;
    const char *key_start;
    const char *key_end;
    const char *value_start;
    const char *value_end;
    cmp_usize key_len;
    cmp_usize value_len;
    cmp_usize line_len;
    cmp_usize alloc_len;
    cmp_usize offset;
    char *line;
    struct curl_slist *new_list;
    int rc;

    line_start = cursor;
    line_end = line_start;
    while (*line_end != '\0' && *line_end != '\r' && *line_end != '\n') {
      line_end++;
    }

    if (line_end != line_start) {
      colon = line_start;
      while (colon < line_end && *colon != ':') {
        colon++;
      }
      if (colon == line_end) {
        *out_list = list;
        return CMP_ERR_INVALID_ARGUMENT;
      }

      key_start = line_start;
      key_end = colon;
      while (key_start < key_end && (*key_start == ' ' || *key_start == '\t')) {
        key_start++;
      }
      while (key_end > key_start &&
             (key_end[-1] == ' ' || key_end[-1] == '\t')) {
        key_end--;
      }

      value_start = colon + 1;
      value_end = line_end;
      while (value_start < value_end &&
             (*value_start == ' ' || *value_start == '\t')) {
        value_start++;
      }
      while (value_end > value_start &&
             (value_end[-1] == ' ' || value_end[-1] == '\t')) {
        value_end--;
      }

      if (key_start == key_end) {
        *out_list = list;
        return CMP_ERR_INVALID_ARGUMENT;
      }

      key_len = (cmp_usize)(key_end - key_start);
      value_len = (cmp_usize)(value_end - value_start);
      rc = cmp_sdl3_network_add_usize(key_len, 1u, &line_len);
      if (rc != CMP_OK) {
        *out_list = list;
        return rc;
      }
      if (value_len > 0u) {
        rc = cmp_sdl3_network_add_usize(line_len, 1u, &line_len);
        if (rc != CMP_OK) {
          *out_list = list;
          return rc;
        }
      }
      rc = cmp_sdl3_network_add_usize(line_len, value_len, &line_len);
      if (rc != CMP_OK) {
        *out_list = list;
        return rc;
      }
      rc = cmp_sdl3_network_add_usize(line_len, 1u, &alloc_len);
      if (rc != CMP_OK) {
        *out_list = list;
        return rc;
      }

      rc = backend->allocator.alloc(backend->allocator.ctx, alloc_len,
                                    (void **)&line);
      if (rc != CMP_OK) {
        *out_list = list;
        return rc;
      }

      memcpy(line, key_start, (size_t)key_len);
      offset = key_len;
      line[offset] = ':';
      offset += 1u;
      if (value_len > 0u) {
        line[offset] = ' ';
        offset += 1u;
        memcpy(line + offset, value_start, (size_t)value_len);
        offset += value_len;
      }
      line[offset] = '\0';

      new_list = curl_slist_append(list, line);
      rc = backend->allocator.free(backend->allocator.ctx, line);
      if (rc != CMP_OK) {
        *out_list = list;
        return rc;
      }
      if (new_list == NULL) {
        *out_list = list;
        return CMP_ERR_OUT_OF_MEMORY;
      }
      list = new_list;
    }

    if (*line_end == '\r' && line_end[1] == '\n') {
      cursor = line_end + 2;
    } else if (*line_end == '\r' || *line_end == '\n') {
      cursor = line_end + 1;
    } else {
      cursor = line_end;
    }
  }

  *out_list = list;
  return CMP_OK;
}
#endif

static int cmp_sdl3_network_request(void *net, const CMPNetworkRequest *request,
                                    const CMPAllocator *allocator,
                                    CMPNetworkResponse *out_response) {
  struct CMPSDL3Backend *backend;
#if defined(CMP_LIBCURL_AVAILABLE)
  CURL *curl;
  CURLcode curl_rc;
  struct curl_slist *header_list;
  CMPSDL3CurlBuffer buffer;
  long response_code;
  long timeout_ms;
  long body_len;
#endif
  int rc;

  if (net == NULL || request == NULL || allocator == NULL ||
      out_response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)net;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.request");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(out_response, 0, sizeof(*out_response));
#if defined(CMP_LIBCURL_AVAILABLE)
  if (request->method == NULL || request->url == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->method[0] == '\0' || request->url[0] == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->body_size > 0 && request->body == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->timeout_ms > 0u && request->timeout_ms > (cmp_u32)LONG_MAX) {
    return CMP_ERR_RANGE;
  }
  if (request->body_size > (cmp_usize)LONG_MAX) {
    return CMP_ERR_RANGE;
  }

  curl = NULL;
  header_list = NULL;
  buffer.allocator = allocator;
  buffer.data = NULL;
  buffer.size = 0u;
  buffer.capacity = 0u;
  buffer.error = CMP_OK;

  curl = curl_easy_init();
  if (curl == NULL) {
    rc = CMP_ERR_OUT_OF_MEMORY;
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_URL, request->url);
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request->method);
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_USERAGENT, "LibCMPC/1.0");
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc =
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cmp_sdl3_network_write_cb);
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  if (request->timeout_ms > 0u) {
    timeout_ms = (long)request->timeout_ms;
    curl_rc = curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    if (curl_rc != CURLE_OK) {
      rc = cmp_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
    curl_rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
    if (curl_rc != CURLE_OK) {
      rc = cmp_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
  }

  if (request->headers != NULL && request->headers[0] != '\0') {
    rc = cmp_sdl3_network_append_headers(backend, request->headers,
                                         &header_list);
    if (rc != CMP_OK) {
      goto cleanup;
    }
    if (header_list != NULL) {
      curl_rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
      if (curl_rc != CURLE_OK) {
        rc = cmp_sdl3_network_error_from_curl(curl_rc);
        goto cleanup;
      }
    }
  }

  if (request->body_size > 0u) {
    body_len = (long)request->body_size;
    curl_rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
    if (curl_rc != CURLE_OK) {
      rc = cmp_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
    curl_rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_len);
    if (curl_rc != CURLE_OK) {
      rc = cmp_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
  }

  curl_rc = curl_easy_perform(curl);
  if (curl_rc != CURLE_OK) {
    if (buffer.error != CMP_OK) {
      rc = buffer.error;
    } else {
      rc = cmp_sdl3_network_error_from_curl(curl_rc);
    }
    goto cleanup;
  }

  if (buffer.error != CMP_OK) {
    rc = buffer.error;
    goto cleanup;
  }

  response_code = 0;
  curl_rc = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  if (curl_rc != CURLE_OK) {
    rc = cmp_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }
  if (response_code < 0 ||
      (cmp_usize)response_code > (cmp_usize)((cmp_u32) ~(cmp_u32)0)) {
    rc = CMP_ERR_RANGE;
    goto cleanup;
  }

  out_response->status_code = (cmp_u32)response_code;
  out_response->body = buffer.data;
  out_response->body_size = buffer.size;
  buffer.data = NULL;
  rc = CMP_OK;

cleanup:
  if (header_list != NULL) {
    curl_slist_free_all(header_list);
  }
  if (curl != NULL) {
    curl_easy_cleanup(curl);
  }
  if (buffer.data != NULL) {
    allocator->free(allocator->ctx, buffer.data);
  }
  if (rc != CMP_OK) {
    memset(out_response, 0, sizeof(*out_response));
  }
  return rc;
#else
  return CMP_ERR_UNSUPPORTED;
#endif
}

static int cmp_sdl3_network_free_response(void *net,
                                          const CMPAllocator *allocator,
                                          CMPNetworkResponse *response) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (net == NULL || allocator == NULL || response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > 0 && response->body == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)net;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                            "network.free_response");
  CMP_SDL3_RETURN_IF_ERROR(rc);
#if defined(CMP_LIBCURL_AVAILABLE)
  if (response->body != NULL) {
    rc = allocator->free(allocator->ctx, (void *)response->body);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  response->body = NULL;
  response->body_size = 0u;
  response->status_code = 0u;
  return CMP_OK;
#else
  return CMP_ERR_UNSUPPORTED;
#endif
}

static const CMPNetworkVTable g_cmp_sdl3_network_vtable = {
    cmp_sdl3_network_request, cmp_sdl3_network_free_response};

static int cmp_sdl3_tasks_thread_create(void *tasks, CMPThreadFn entry,
                                        void *user, CMPHandle *out_thread) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(entry);
  CMP_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc =
      cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_create");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_thread_join(void *tasks, CMPHandle thread) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(thread);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_join");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_mutex_create(void *tasks, CMPHandle *out_mutex) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_mutex_destroy(void *tasks, CMPHandle mutex) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc =
      cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_mutex_lock(void *tasks, CMPHandle mutex) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_mutex_unlock(void *tasks, CMPHandle mutex) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_sleep_ms(void *tasks, cmp_u32 ms) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(ms);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_post(void *tasks, CMPTaskFn fn, void *user) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(fn);
  CMP_UNUSED(user);

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_sdl3_tasks_post_delayed(void *tasks, CMPTaskFn fn, void *user,
                                       cmp_u32 delay_ms) {
  struct CMPSDL3Backend *backend;
  int rc;

  CMP_UNUSED(fn);
  CMP_UNUSED(user);
  CMP_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)tasks;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  CMP_SDL3_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPTasksVTable g_cmp_sdl3_tasks_vtable = {
    cmp_sdl3_tasks_thread_create, cmp_sdl3_tasks_thread_join,
    cmp_sdl3_tasks_mutex_create,  cmp_sdl3_tasks_mutex_destroy,
    cmp_sdl3_tasks_mutex_lock,    cmp_sdl3_tasks_mutex_unlock,
    cmp_sdl3_tasks_sleep_ms,      cmp_sdl3_tasks_post,
    cmp_sdl3_tasks_post_delayed};

static int cmp_sdl3_env_get_io(void *env, CMPIO *out_io) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (env == NULL || out_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_io");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return CMP_OK;
}

static int cmp_sdl3_env_get_sensors(void *env, CMPSensors *out_sensors) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (env == NULL || out_sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_sensors");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return CMP_OK;
}

static int cmp_sdl3_env_get_camera(void *env, CMPCamera *out_camera) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (env == NULL || out_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_camera");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return CMP_OK;
}

static int cmp_sdl3_env_get_image(void *env, CMPImage *out_image) {
  struct CMPSDL3Backend *backend;

  if (env == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  *out_image = backend->image;
  return CMP_OK;
}

static int cmp_sdl3_env_get_video(void *env, CMPVideo *out_video) {
  struct CMPSDL3Backend *backend;

  if (env == NULL || out_video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  *out_video = backend->video;
  return CMP_OK;
}

static int cmp_sdl3_env_get_audio(void *env, CMPAudio *out_audio) {
  struct CMPSDL3Backend *backend;

  if (env == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  *out_audio = backend->audio;
  return CMP_OK;
}

static int cmp_sdl3_env_get_network(void *env, CMPNetwork *out_network) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (env == NULL || out_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_network");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return CMP_OK;
}

static int cmp_sdl3_env_get_tasks(void *env, CMPTasks *out_tasks) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_tasks");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return CMP_OK;
}

static int cmp_sdl3_env_get_time_ms(void *env, cmp_u32 *out_time_ms) {
  struct CMPSDL3Backend *backend;
  int rc;

  if (env == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPSDL3Backend *)env;
  rc = cmp_sdl3_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_time_ms");
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return cmp_sdl3_time_ms(out_time_ms);
}

static const CMPEnvVTable g_cmp_sdl3_env_vtable = {
    cmp_sdl3_env_get_io,      cmp_sdl3_env_get_sensors,
    cmp_sdl3_env_get_camera,  cmp_sdl3_env_get_image,
    cmp_sdl3_env_get_video,   cmp_sdl3_env_get_audio,
    cmp_sdl3_env_get_network, cmp_sdl3_env_get_tasks,
    cmp_sdl3_env_get_time_ms};

int CMP_CALL cmp_sdl3_backend_create(const CMPSDL3BackendConfig *config,
                                     CMPSDL3Backend **out_backend) {
  CMPSDL3BackendConfig local_config;
  CMPAllocator allocator;
  struct CMPSDL3Backend *backend;
  CMPTasksDefaultConfig tasks_config;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_sdl3_backend_config_init(&local_config);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_sdl3_backend_validate_config(config);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    CMP_SDL3_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc =
      allocator.alloc(allocator.ctx, sizeof(CMPSDL3Backend), (void **)&backend);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->renderer_flags = config->renderer_flags;
  backend->log_enabled = config->enable_logging ? CMP_TRUE : CMP_FALSE;
  backend->tasks_enabled = config->enable_tasks ? CMP_TRUE : CMP_FALSE;

  if (backend->log_enabled) {
    rc = cmp_log_init(&allocator);
    if (rc != CMP_OK && rc != CMP_ERR_STATE) {
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->log_owner = (rc == CMP_OK) ? CMP_TRUE : CMP_FALSE;
  }

  rc = cmp_handle_system_default_create(config->handle_capacity, &allocator,
                                        &backend->handles);
  if (rc != CMP_OK) {
    if (backend->log_owner) {
      cmp_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = cmp_sdl3_global_init(backend);
  if (rc != CMP_OK) {
    cmp_handle_system_default_destroy(&backend->handles);
    if (backend->log_owner) {
      cmp_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
  backend->sdl_initialized = CMP_TRUE;

#if defined(CMP_LIBCURL_AVAILABLE)
  rc = cmp_sdl3_libcurl_init();
  if (rc != CMP_OK) {
    cmp_sdl3_global_shutdown(backend);
    cmp_handle_system_default_destroy(&backend->handles);
    if (backend->log_owner) {
      cmp_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
#endif

#if defined(CMP_SDL3_TTF_AVAILABLE)
  rc = cmp_sdl3_ttf_init(backend);
  if (rc != CMP_OK) {
#if defined(CMP_LIBCURL_AVAILABLE)
    cmp_sdl3_libcurl_shutdown();
#endif
    cmp_sdl3_global_shutdown(backend);
    cmp_handle_system_default_destroy(&backend->handles);
    if (backend->log_owner) {
      cmp_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
  backend->ttf_initialized = CMP_TRUE;
#endif

  if (backend->tasks_enabled) {
    rc = cmp_tasks_default_config_init(&tasks_config);
    if (rc != CMP_OK) {
#if defined(CMP_SDL3_TTF_AVAILABLE)
      if (backend->ttf_initialized) {
        cmp_sdl3_ttf_shutdown(backend);
        backend->ttf_initialized = CMP_FALSE;
      }
#endif
#if defined(CMP_LIBCURL_AVAILABLE)
      cmp_sdl3_libcurl_shutdown();
#endif
      cmp_sdl3_global_shutdown(backend);
      cmp_handle_system_default_destroy(&backend->handles);
      if (backend->log_owner) {
        cmp_log_shutdown();
      }
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    tasks_config.allocator = &backend->allocator;
    rc = cmp_tasks_default_create(&tasks_config, &backend->tasks);
    if (rc != CMP_OK) {
#if defined(CMP_SDL3_TTF_AVAILABLE)
      if (backend->ttf_initialized) {
        cmp_sdl3_ttf_shutdown(backend);
        backend->ttf_initialized = CMP_FALSE;
      }
#endif
#if defined(CMP_LIBCURL_AVAILABLE)
      cmp_sdl3_libcurl_shutdown();
#endif
      cmp_sdl3_global_shutdown(backend);
      cmp_handle_system_default_destroy(&backend->handles);
      if (backend->log_owner) {
        cmp_log_shutdown();
      }
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->tasks_owned = CMP_TRUE;
  } else {
    backend->tasks.ctx = backend;
    backend->tasks.vtable = &g_cmp_sdl3_tasks_vtable;
  }

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_cmp_sdl3_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_cmp_sdl3_gfx_vtable;
  backend->gfx.text_vtable = &g_cmp_sdl3_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_cmp_sdl3_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_cmp_sdl3_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_cmp_sdl3_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_cmp_sdl3_camera_vtable;
  backend->image.ctx = backend;
  backend->image.vtable = &g_cmp_sdl3_image_vtable;
  backend->video.ctx = backend;
  backend->video.vtable = &g_cmp_sdl3_video_vtable;
  backend->audio.ctx = backend;
  backend->audio.vtable = &g_cmp_sdl3_audio_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_cmp_sdl3_network_vtable;

  backend->initialized = CMP_TRUE;

  *out_backend = backend;
  return CMP_OK;
}

int CMP_CALL cmp_sdl3_backend_destroy(CMPSDL3Backend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_handle_system_default_destroy(&backend->handles);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  first_error = CMP_OK;

  if (backend->tasks_owned) {
    rc = cmp_tasks_default_destroy(&backend->tasks);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
  }

#if defined(CMP_SDL3_TTF_AVAILABLE)
  if (backend->ttf_initialized) {
    rc = cmp_sdl3_ttf_shutdown(backend);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    backend->ttf_initialized = CMP_FALSE;
  }
#endif

  if (backend->sdl_initialized) {
    rc = cmp_sdl3_global_shutdown(backend);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    backend->sdl_initialized = CMP_FALSE;
  }

#if defined(CMP_LIBCURL_AVAILABLE)
  rc = cmp_sdl3_libcurl_shutdown();
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }
#endif

  if (backend->log_owner) {
    rc = cmp_log_shutdown();
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
  }

  backend->initialized = CMP_FALSE;
  backend->ws.ctx = NULL;
  backend->ws.vtable = NULL;
  backend->gfx.ctx = NULL;
  backend->gfx.vtable = NULL;
  backend->gfx.text_vtable = NULL;
  backend->env.ctx = NULL;
  backend->env.vtable = NULL;
  backend->io.ctx = NULL;
  backend->io.vtable = NULL;
  backend->sensors.ctx = NULL;
  backend->sensors.vtable = NULL;
  backend->camera.ctx = NULL;
  backend->camera.vtable = NULL;
  backend->image.ctx = NULL;
  backend->image.vtable = NULL;
  backend->video.ctx = NULL;
  backend->video.vtable = NULL;
  backend->audio.ctx = NULL;
  backend->audio.vtable = NULL;
  backend->network.ctx = NULL;
  backend->network.vtable = NULL;
  backend->tasks.ctx = NULL;
  backend->tasks.vtable = NULL;

  rc = backend->allocator.free(backend->allocator.ctx, backend);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  return first_error;
}

int CMP_CALL cmp_sdl3_backend_get_ws(CMPSDL3Backend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  *out_ws = backend->ws;
  return CMP_OK;
}

int CMP_CALL cmp_sdl3_backend_get_gfx(CMPSDL3Backend *backend,
                                      CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  *out_gfx = backend->gfx;
  return CMP_OK;
}

int CMP_CALL cmp_sdl3_backend_get_env(CMPSDL3Backend *backend,
                                      CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  *out_env = backend->env;
  return CMP_OK;
}

/* GCOVR_EXCL_STOP */
#else

int CMP_CALL cmp_sdl3_backend_create(const CMPSDL3BackendConfig *config,
                                     CMPSDL3Backend **out_backend) {
  CMPSDL3BackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_sdl3_backend_config_init(&local_config);
    CMP_SDL3_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_sdl3_backend_validate_config(config);
  CMP_SDL3_RETURN_IF_ERROR(rc);

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_sdl3_backend_destroy(CMPSDL3Backend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_sdl3_backend_get_ws(CMPSDL3Backend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_sdl3_backend_get_gfx(CMPSDL3Backend *backend,
                                      CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_sdl3_backend_get_env(CMPSDL3Backend *backend,
                                      CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return CMP_ERR_UNSUPPORTED;
}

#endif
