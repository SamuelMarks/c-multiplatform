#include "m3/m3_backend_sdl3.h"

#include "m3/m3_log.h"
#include "m3/m3_object.h"
#include "m3/m3_tasks.h"

#include <limits.h>
#include <string.h>
#if defined(M3_LIBCURL_AVAILABLE)
#include <curl/curl.h>
#endif

#if defined(M3_SDL3_AVAILABLE)
#include <SDL3/SDL.h>
#if defined(M3_SDL3_TTF_AVAILABLE)
#include <SDL3_ttf/SDL_ttf.h>
#endif
#endif

#define M3_SDL3_RETURN_IF_ERROR(rc)                                            \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_SDL3_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                           \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_SDL3_DEFAULT_HANDLE_CAPACITY 64u
#define M3_SDL3_DEFAULT_RENDERER_FLAGS                                         \
  (M3_SDL3_RENDERER_ACCELERATED | M3_SDL3_RENDERER_PRESENTVSYNC)

#define M3_SDL3_RENDERER_FLAG_MASK                                             \
  (M3_SDL3_RENDERER_ACCELERATED | M3_SDL3_RENDERER_PRESENTVSYNC |              \
   M3_SDL3_RENDERER_TARGETTEXTURE)

static int m3_sdl3_backend_validate_config(const M3SDL3BackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if ((config->renderer_flags & ~M3_SDL3_RENDERER_FLAG_MASK) != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL
m3_sdl3_backend_test_validate_config(const M3SDL3BackendConfig *config) {
  return m3_sdl3_backend_validate_config(config);
}
#endif

int M3_CALL m3_sdl3_backend_is_available(M3Bool *out_available) {
  if (out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#if defined(M3_SDL3_AVAILABLE)
  *out_available = M3_TRUE;
#else
  *out_available = M3_FALSE;
#endif
  return M3_OK;
}

int M3_CALL m3_sdl3_backend_config_init(M3SDL3BackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = M3_SDL3_DEFAULT_HANDLE_CAPACITY;
  config->renderer_flags = M3_SDL3_DEFAULT_RENDERER_FLAGS;
  config->enable_logging = M3_TRUE;
  config->enable_tasks = M3_TRUE;
  return M3_OK;
}

#if defined(M3_SDL3_AVAILABLE)

#define M3_SDL3_TYPE_WINDOW 1
#define M3_SDL3_TYPE_TEXTURE 2
#define M3_SDL3_TYPE_FONT 3

#define M3_SDL3_CLIP_STACK_CAPACITY 32u

typedef struct M3SDL3Window {
  M3ObjectHeader header;
  struct M3SDL3Backend *backend;
  SDL_Window *window;
  SDL_Renderer *renderer;
  m3_i32 width;
  m3_i32 height;
  M3Scalar dpi_scale;
  m3_u32 flags;
  M3Bool visible;
} M3SDL3Window;

typedef struct M3SDL3Texture {
  M3ObjectHeader header;
  struct M3SDL3Backend *backend;
  void *texture;
  void *renderer;
  m3_i32 width;
  m3_i32 height;
  m3_u32 format;
  m3_u32 bytes_per_pixel;
} M3SDL3Texture;

typedef struct M3SDL3Font {
  M3ObjectHeader header;
  struct M3SDL3Backend *backend;
  m3_i32 size_px;
  m3_i32 weight;
  M3Bool italic;
  void *font;
} M3SDL3Font;

struct M3SDL3Backend {
  M3Allocator allocator;
  M3HandleSystem handles;
  M3WS ws;
  M3Gfx gfx;
  M3Env env;
  M3IO io;
  M3Sensors sensors;
  M3Camera camera;
  M3Network network;
  M3Tasks tasks;
  M3Bool tasks_owned;
  M3Bool tasks_enabled;
  M3Bool initialized;
  M3Bool log_enabled;
  M3Bool log_owner;
  M3Bool sdl_initialized;
  M3Bool ttf_initialized;
  m3_u32 renderer_flags;
  M3SDL3Window *active_window;
  M3Rect clip_stack[M3_SDL3_CLIP_STACK_CAPACITY];
  m3_usize clip_depth;
  M3Mat3 transform;
  M3Bool has_transform;
};

static m3_u32 g_m3_sdl3_refcount = 0u;
#if defined(M3_SDL3_TTF_AVAILABLE)
static m3_u32 g_m3_sdl3_ttf_refcount = 0u;
#endif

static int m3_sdl3_backend_log(struct M3SDL3Backend *backend, M3LogLevel level,
                               const char *message) {
  if (backend == NULL || message == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->log_enabled) {
    return M3_OK;
  }
  return m3_log_write(level, "m3.sdl3", message);
}

static int m3_sdl3_backend_log_sdl_error(struct M3SDL3Backend *backend,
                                         const char *message) {
  const char *sdl_error;
  int rc;

  if (backend == NULL || message == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_ERROR, message);
  M3_SDL3_RETURN_IF_ERROR(rc);

  sdl_error = SDL_GetError();
  if (sdl_error != NULL && sdl_error[0] != '\0') {
    rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_ERROR, sdl_error);
    M3_SDL3_RETURN_IF_ERROR(rc);
  }
  return M3_OK;
}

#if defined(M3_SDL3_TTF_AVAILABLE)
static int m3_sdl3_backend_log_ttf_error(struct M3SDL3Backend *backend,
                                         const char *message) {
  const char *ttf_error;
  int rc;

  if (backend == NULL || message == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_ERROR, message);
  M3_SDL3_RETURN_IF_ERROR(rc);

  ttf_error = TTF_GetError();
  if (ttf_error != NULL && ttf_error[0] != '\0') {
    rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_ERROR, ttf_error);
    M3_SDL3_RETURN_IF_ERROR(rc);
  }
  return M3_OK;
}
#endif

static int m3_sdl3_global_init(struct M3SDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (g_m3_sdl3_refcount == 0u) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
      m3_sdl3_backend_log_sdl_error(backend, "sdl.init");
      return M3_ERR_UNKNOWN;
    }
  }

  g_m3_sdl3_refcount += 1u;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "sdl.init");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_sdl3_global_shutdown(struct M3SDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_sdl3_refcount == 0u) {
    return M3_ERR_STATE;
  }

  g_m3_sdl3_refcount -= 1u;
  if (g_m3_sdl3_refcount == 0u) {
    SDL_Quit();
  }

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "sdl.shutdown");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

#if defined(M3_SDL3_TTF_AVAILABLE)
static int m3_sdl3_ttf_init(struct M3SDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (g_m3_sdl3_ttf_refcount == 0u) {
    if (TTF_Init() != 0) {
      m3_sdl3_backend_log_ttf_error(backend, "ttf.init");
      return M3_ERR_UNKNOWN;
    }
  }

  g_m3_sdl3_ttf_refcount += 1u;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ttf.init");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_sdl3_ttf_shutdown(struct M3SDL3Backend *backend) {
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_sdl3_ttf_refcount == 0u) {
    return M3_ERR_STATE;
  }

  g_m3_sdl3_ttf_refcount -= 1u;
  if (g_m3_sdl3_ttf_refcount == 0u) {
    TTF_Quit();
  }

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ttf.shutdown");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}
#endif

static int m3_sdl3_renderer_flags_to_sdl(m3_u32 flags, Uint32 *out_flags) {
  Uint32 sdl_flags;

  if (out_flags == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if ((flags & ~M3_SDL3_RENDERER_FLAG_MASK) != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  sdl_flags = 0u;
#if defined(SDL_RENDERER_ACCELERATED)
  if ((flags & M3_SDL3_RENDERER_ACCELERATED) != 0u) {
    sdl_flags |= SDL_RENDERER_ACCELERATED;
  }
#else
  if ((flags & M3_SDL3_RENDERER_ACCELERATED) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_RENDERER_PRESENTVSYNC)
  if ((flags & M3_SDL3_RENDERER_PRESENTVSYNC) != 0u) {
    sdl_flags |= SDL_RENDERER_PRESENTVSYNC;
  }
#else
  if ((flags & M3_SDL3_RENDERER_PRESENTVSYNC) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_RENDERER_TARGETTEXTURE)
  if ((flags & M3_SDL3_RENDERER_TARGETTEXTURE) != 0u) {
    sdl_flags |= SDL_RENDERER_TARGETTEXTURE;
  }
#else
  if ((flags & M3_SDL3_RENDERER_TARGETTEXTURE) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif
  *out_flags = sdl_flags;
  return M3_OK;
}

static int m3_sdl3_texture_format_to_sdl(m3_u32 format, Uint32 *out_format,
                                         m3_u32 *out_bpp) {
  if (out_format == NULL || out_bpp == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (format == M3_TEX_FORMAT_RGBA8) {
#if defined(SDL_PIXELFORMAT_RGBA32)
    *out_format = SDL_PIXELFORMAT_RGBA32;
#elif defined(SDL_PIXELFORMAT_RGBA8888)
    *out_format = SDL_PIXELFORMAT_RGBA8888;
#else
    return M3_ERR_UNSUPPORTED;
#endif
    *out_bpp = 4u;
    return M3_OK;
  }

  if (format == M3_TEX_FORMAT_BGRA8) {
#if defined(SDL_PIXELFORMAT_BGRA32)
    *out_format = SDL_PIXELFORMAT_BGRA32;
#elif defined(SDL_PIXELFORMAT_BGRA8888)
    *out_format = SDL_PIXELFORMAT_BGRA8888;
#else
    return M3_ERR_UNSUPPORTED;
#endif
    *out_bpp = 4u;
    return M3_OK;
  }

  if (format == M3_TEX_FORMAT_A8) {
#if defined(SDL_PIXELFORMAT_A8)
    *out_format = SDL_PIXELFORMAT_A8;
    *out_bpp = 1u;
    return M3_OK;
#else
    return M3_ERR_UNSUPPORTED;
#endif
  }

  return M3_ERR_INVALID_ARGUMENT;
}

static int m3_sdl3_mul_usize(m3_usize a, m3_usize b, m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = (m3_usize) ~(m3_usize)0;
  if (a != 0 && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}

static int m3_sdl3_window_flags_to_sdl(m3_u32 flags, Uint32 *out_flags) {
  Uint32 sdl_flags;
  m3_u32 allowed;

  if (out_flags == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  allowed = M3_WS_WINDOW_RESIZABLE | M3_WS_WINDOW_BORDERLESS |
            M3_WS_WINDOW_FULLSCREEN | M3_WS_WINDOW_HIGH_DPI;
  if ((flags & ~allowed) != 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  sdl_flags = 0u;
#if defined(SDL_WINDOW_RESIZABLE)
  if ((flags & M3_WS_WINDOW_RESIZABLE) != 0u) {
    sdl_flags |= SDL_WINDOW_RESIZABLE;
  }
#else
  if ((flags & M3_WS_WINDOW_RESIZABLE) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_WINDOW_BORDERLESS)
  if ((flags & M3_WS_WINDOW_BORDERLESS) != 0u) {
    sdl_flags |= SDL_WINDOW_BORDERLESS;
  }
#else
  if ((flags & M3_WS_WINDOW_BORDERLESS) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_WINDOW_FULLSCREEN)
  if ((flags & M3_WS_WINDOW_FULLSCREEN) != 0u) {
    sdl_flags |= SDL_WINDOW_FULLSCREEN;
  }
#elif defined(SDL_WINDOW_FULLSCREEN_DESKTOP)
  if ((flags & M3_WS_WINDOW_FULLSCREEN) != 0u) {
    sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
#else
  if ((flags & M3_WS_WINDOW_FULLSCREEN) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif
#if defined(SDL_WINDOW_HIGH_PIXEL_DENSITY)
  if ((flags & M3_WS_WINDOW_HIGH_DPI) != 0u) {
    sdl_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
  }
#elif defined(SDL_WINDOW_ALLOW_HIGHDPI)
  if ((flags & M3_WS_WINDOW_HIGH_DPI) != 0u) {
    sdl_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
  }
#else
  if ((flags & M3_WS_WINDOW_HIGH_DPI) != 0u) {
    return M3_ERR_UNSUPPORTED;
  }
#endif

  *out_flags = sdl_flags;
  return M3_OK;
}

static int m3_sdl3_color_to_u8(M3Scalar value, m3_u8 *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (value < 0.0f) {
    value = 0.0f;
  } else if (value > 1.0f) {
    value = 1.0f;
  }

  *out_value = (m3_u8)(value * 255.0f + 0.5f);
  return M3_OK;
}

static int m3_sdl3_backend_resolve(struct M3SDL3Backend *backend,
                                   M3Handle handle, m3_u32 type_id,
                                   void **out_obj) {
  void *resolved;
  m3_u32 actual_type;
  int rc;

  if (out_obj != NULL) {
    *out_obj = NULL;
  }

  rc =
      backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_object_get_type_id((const M3ObjectHeader *)resolved, &actual_type);
  M3_SDL3_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return M3_OK;
}

static int m3_sdl3_object_retain(void *obj) {
  return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_sdl3_object_release(void *obj) {
  return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_sdl3_object_get_type_id(void *obj, m3_u32 *out_type_id) {
  return m3_object_get_type_id((const M3ObjectHeader *)obj, out_type_id);
}

static int m3_sdl3_window_destroy(void *obj) {
  M3SDL3Window *window;
  struct M3SDL3Backend *backend;
  int rc;

  if (obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  window = (M3SDL3Window *)obj;
  backend = window->backend;

  if (backend != NULL && backend->active_window == window) {
    backend->active_window = NULL;
  }

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (window->renderer != NULL) {
    SDL_DestroyRenderer(window->renderer);
    window->renderer = NULL;
  }
  if (window->window != NULL) {
    SDL_DestroyWindow(window->window);
    window->window = NULL;
  }

  rc = backend->allocator.free(backend->allocator.ctx, window);
  M3_SDL3_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_sdl3_texture_destroy(void *obj) {
  M3SDL3Texture *texture;
  struct M3SDL3Backend *backend;
  int rc;

  if (obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  texture = (M3SDL3Texture *)obj;
  backend = texture->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  texture->header.handle);
  M3_SDL3_RETURN_IF_ERROR(rc);

#if defined(M3_SDL3_AVAILABLE)
  if (texture->texture != NULL) {
    SDL_DestroyTexture((SDL_Texture *)texture->texture);
    texture->texture = NULL;
  }
#endif

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  M3_SDL3_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_sdl3_font_destroy(void *obj) {
  M3SDL3Font *font;
  struct M3SDL3Backend *backend;
  int rc;

  if (obj == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  font = (M3SDL3Font *)obj;
  backend = font->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  M3_SDL3_RETURN_IF_ERROR(rc);

#if defined(M3_SDL3_TTF_AVAILABLE)
  if (font->font != NULL) {
    TTF_CloseFont((TTF_Font *)font->font);
    font->font = NULL;
  }
#endif

  rc = backend->allocator.free(backend->allocator.ctx, font);
  M3_SDL3_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static const M3ObjectVTable g_m3_sdl3_window_vtable = {
    m3_sdl3_object_retain, m3_sdl3_object_release, m3_sdl3_window_destroy,
    m3_sdl3_object_get_type_id};

static const M3ObjectVTable g_m3_sdl3_texture_vtable = {
    m3_sdl3_object_retain, m3_sdl3_object_release, m3_sdl3_texture_destroy,
    m3_sdl3_object_get_type_id};

static const M3ObjectVTable g_m3_sdl3_font_vtable = {
    m3_sdl3_object_retain, m3_sdl3_object_release, m3_sdl3_font_destroy,
    m3_sdl3_object_get_type_id};

static int m3_sdl3_ws_init(void *ws, const M3WSConfig *config) {
  struct M3SDL3Backend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.init");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_sdl3_ws_shutdown(void *ws) {
  struct M3SDL3Backend *backend;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.shutdown");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_sdl3_ws_create_window(void *ws, const M3WSWindowConfig *config,
                                    M3Handle *out_window) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *window;
  Uint32 sdl_flags;
  Uint32 renderer_flags;
  int rc;

  if (ws == NULL || config == NULL || out_window == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_title == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->width <= 0 || config->height <= 0) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.create_window");
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_window_flags_to_sdl(config->flags, &sdl_flags);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_renderer_flags_to_sdl(backend->renderer_flags, &renderer_flags);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3SDL3Window),
                                (void **)&window);
  M3_SDL3_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->dpi_scale = 1.0f;
  window->flags = config->flags;
  window->visible = M3_FALSE;

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  window->window = SDL_CreateWindow(config->utf8_title, config->width,
                                    config->height, sdl_flags);
#else
  window->window = SDL_CreateWindow(config->utf8_title, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, config->width,
                                    config->height, sdl_flags);
#endif
  if (window->window == NULL) {
    m3_sdl3_backend_log_sdl_error(backend, "ws.create_window");
    backend->allocator.free(backend->allocator.ctx, window);
    return M3_ERR_UNKNOWN;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  window->renderer = SDL_CreateRenderer(window->window, NULL, renderer_flags);
#else
  window->renderer = SDL_CreateRenderer(window->window, -1, renderer_flags);
#endif
  if (window->renderer == NULL) {
    m3_sdl3_backend_log_sdl_error(backend, "ws.create_renderer");
    SDL_DestroyWindow(window->window);
    backend->allocator.free(backend->allocator.ctx, window);
    return M3_ERR_UNKNOWN;
  }

  rc = m3_object_header_init(&window->header, M3_SDL3_TYPE_WINDOW, 0,
                             &g_m3_sdl3_window_vtable);
  if (rc != M3_OK) {
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  if (rc != M3_OK) {
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  *out_window = window->header.handle;
  return M3_OK;
}

static int m3_sdl3_ws_destroy_window(void *ws, M3Handle window) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.destroy_window");
  M3_SDL3_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_sdl3_ws_show_window(void *ws, M3Handle window) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.show_window");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_ShowWindow(resolved->window);
  }
  resolved->visible = M3_TRUE;
  return M3_OK;
}

static int m3_sdl3_ws_hide_window(void *ws, M3Handle window) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.hide_window");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_HideWindow(resolved->window);
  }
  resolved->visible = M3_FALSE;
  return M3_OK;
}

static int m3_sdl3_ws_set_window_title(void *ws, M3Handle window,
                                       const char *utf8_title) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_title");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_SetWindowTitle(resolved->window, utf8_title);
  }
  return M3_OK;
}

static int m3_sdl3_ws_set_window_size(void *ws, M3Handle window, m3_i32 width,
                                      m3_i32 height) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_size");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    SDL_SetWindowSize(resolved->window, width, height);
  }
  resolved->width = width;
  resolved->height = height;
  return M3_OK;
}

static int m3_sdl3_ws_get_window_size(void *ws, M3Handle window,
                                      m3_i32 *out_width, m3_i32 *out_height) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_size");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(resolved->window, &w, &h);
    resolved->width = (m3_i32)w;
    resolved->height = (m3_i32)h;
  }

  *out_width = resolved->width;
  *out_height = resolved->height;
  return M3_OK;
}

static int m3_sdl3_ws_set_window_dpi_scale(void *ws, M3Handle window,
                                           M3Scalar scale) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO,
                           "ws.set_window_dpi_scale");
  M3_SDL3_RETURN_IF_ERROR(rc);

  resolved->dpi_scale = scale;
  return M3_OK;
}

static int m3_sdl3_ws_get_window_dpi_scale(void *ws, M3Handle window,
                                           M3Scalar *out_scale) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO,
                           "ws.get_window_dpi_scale");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_scale = resolved->dpi_scale;
  return M3_OK;
}

static int m3_sdl3_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct M3SDL3Backend *backend;
  int rc;
  int sdl_rc;

  if (ws == NULL || utf8_text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_clipboard_text");
  M3_SDL3_RETURN_IF_ERROR(rc);

  sdl_rc = SDL_SetClipboardText(utf8_text);
  if (sdl_rc != 0) {
    m3_sdl3_backend_log_sdl_error(backend, "ws.set_clipboard_text");
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int m3_sdl3_ws_get_clipboard_text(void *ws, char *buffer,
                                         m3_usize buffer_size,
                                         m3_usize *out_length) {
  struct M3SDL3Backend *backend;
  char *text;
  m3_usize length;
  m3_usize required;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_clipboard_text");
  M3_SDL3_RETURN_IF_ERROR(rc);

  text = SDL_GetClipboardText();
  if (text == NULL) {
    m3_sdl3_backend_log_sdl_error(backend, "ws.get_clipboard_text");
    return M3_ERR_UNKNOWN;
  }

  length = 0;
  while (text[length] != '\0') {
    length += 1;
  }

  required = length + 1u;
  if (buffer_size < required) {
    *out_length = length;
    SDL_free(text);
    return M3_ERR_RANGE;
  }

  if (length == 0) {
    buffer[0] = '\0';
  } else {
    memcpy(buffer, text, required);
  }
  *out_length = length;
  SDL_free(text);
  return M3_OK;
}

static int m3_sdl3_get_modifiers(m3_u32 *out_modifiers) {
  SDL_Keymod mods;
  m3_u32 out;

  if (out_modifiers == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  mods = SDL_GetModState();
  out = 0u;
#if defined(SDL_KMOD_SHIFT)
  if ((mods & SDL_KMOD_SHIFT) != 0) {
    out |= M3_MOD_SHIFT;
  }
#endif
#if defined(SDL_KMOD_CTRL)
  if ((mods & SDL_KMOD_CTRL) != 0) {
    out |= M3_MOD_CTRL;
  }
#endif
#if defined(SDL_KMOD_ALT)
  if ((mods & SDL_KMOD_ALT) != 0) {
    out |= M3_MOD_ALT;
  }
#endif
#if defined(SDL_KMOD_GUI)
  if ((mods & SDL_KMOD_GUI) != 0) {
    out |= M3_MOD_META;
  }
#endif
#if defined(SDL_KMOD_CAPS)
  if ((mods & SDL_KMOD_CAPS) != 0) {
    out |= M3_MOD_CAPS;
  }
#endif
#if defined(SDL_KMOD_NUM)
  if ((mods & SDL_KMOD_NUM) != 0) {
    out |= M3_MOD_NUM;
  }
#endif

  *out_modifiers = out;
  return M3_OK;
}

static int m3_sdl3_fill_pointer_state(M3PointerEvent *pointer) {
  int x = 0;
  int y = 0;
  Uint32 mask;
  m3_i32 buttons;

  if (pointer == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
  pointer->x = (m3_i32)x;
  pointer->y = (m3_i32)y;
  pointer->buttons = buttons;
  pointer->scroll_x = 0;
  pointer->scroll_y = 0;
  return M3_OK;
}

static int m3_sdl3_fill_key_event(const SDL_Event *event, M3KeyEvent *key) {
  if (event == NULL || key == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  key->key_code = (m3_u32)event->key.key;
  key->native_code = (m3_u32)event->key.scancode;
  key->is_repeat = event->key.repeat ? M3_TRUE : M3_FALSE;
#else
  key->key_code = (m3_u32)event->key.keysym.sym;
  key->native_code = (m3_u32)event->key.keysym.scancode;
  key->is_repeat = event->key.repeat ? M3_TRUE : M3_FALSE;
#endif
  return M3_OK;
}

static int m3_sdl3_copy_text(const char *text, M3TextEvent *out_text) {
  m3_usize i;
  m3_usize max_len;

  if (out_text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_text->utf8[0] = '\0';
  out_text->length = 0u;
  if (text == NULL) {
    return M3_OK;
  }

  max_len = (m3_usize)sizeof(out_text->utf8) - 1u;
  i = 0;
  while (text[i] != '\0' && i < max_len) {
    out_text->utf8[i] = text[i];
    i += 1u;
  }
  out_text->utf8[i] = '\0';
  out_text->length = (m3_u32)i;
  return M3_OK;
}

static int m3_sdl3_text_length(const char *text, m3_usize *out_length) {
  m3_usize i;

  if (out_length == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (text == NULL) {
    *out_length = 0;
    return M3_OK;
  }

  i = 0;
  while (text[i] != '\0') {
    i += 1u;
  }
  *out_length = i;
  return M3_OK;
}

static int m3_sdl3_fill_scroll_event(const SDL_Event *event,
                                     M3PointerEvent *pointer) {
  m3_i32 scroll_x;
  m3_i32 scroll_y;

  if (event == NULL || pointer == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  scroll_x = (m3_i32)event->wheel.x;
  scroll_y = (m3_i32)event->wheel.y;
#if defined(SDL_MOUSEWHEEL_FLIPPED)
  if (event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
    scroll_x = -scroll_x;
    scroll_y = -scroll_y;
  }
#endif

  pointer->scroll_x = scroll_x;
  pointer->scroll_y = scroll_y;
  return M3_OK;
}

static int m3_sdl3_translate_event(struct M3SDL3Backend *backend,
                                   const SDL_Event *sdl_event,
                                   M3InputEvent *out_event,
                                   M3Bool *out_has_event) {
  m3_u32 modifiers;
  M3Handle window_handle;
  int rc;

  if (backend == NULL || sdl_event == NULL || out_event == NULL ||
      out_has_event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(out_event, 0, sizeof(*out_event));
  out_event->time_ms = (m3_u32)(SDL_GetTicks() & 0xFFFFFFFFu);
  *out_has_event = M3_FALSE;

  rc = m3_sdl3_get_modifiers(&modifiers);
  M3_SDL3_RETURN_IF_ERROR(rc);

  window_handle.id = 0u;
  window_handle.generation = 0u;
  if (backend->active_window != NULL) {
    window_handle = backend->active_window->header.handle;
  }

#if defined(SDL_EVENT_QUIT)
  if (sdl_event->type == SDL_EVENT_QUIT) {
    out_event->type = M3_INPUT_WINDOW_CLOSE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_QUIT)
  if (sdl_event->type == SDL_QUIT) {
    out_event->type = M3_INPUT_WINDOW_CLOSE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif

#if defined(SDL_EVENT_KEY_DOWN)
  if (sdl_event->type == SDL_EVENT_KEY_DOWN) {
    out_event->type = M3_INPUT_KEY_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_EVENT_KEY_UP)
  if (sdl_event->type == SDL_EVENT_KEY_UP) {
    out_event->type = M3_INPUT_KEY_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_KEYDOWN)
  if (sdl_event->type == SDL_KEYDOWN) {
    out_event->type = M3_INPUT_KEY_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_KEYUP)
  if (sdl_event->type == SDL_KEYUP) {
    out_event->type = M3_INPUT_KEY_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_key_event(sdl_event, &out_event->data.key);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif

#if defined(SDL_EVENT_TEXT_INPUT)
  if (sdl_event->type == SDL_EVENT_TEXT_INPUT) {
    m3_usize text_len;
    const char *text;

    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->text.text;
    rc = m3_sdl3_text_length(text, &text_len);
    M3_SDL3_RETURN_IF_ERROR(rc);

    if (text_len <= (m3_usize)(sizeof(out_event->data.text.utf8) - 1u)) {
      out_event->type = M3_INPUT_TEXT;
      rc = m3_sdl3_copy_text(text, &out_event->data.text);
      M3_SDL3_RETURN_IF_ERROR(rc);
    } else {
      out_event->type = M3_INPUT_TEXT_UTF8;
      out_event->data.text_utf8.utf8 = text;
      out_event->data.text_utf8.length = text_len;
    }

    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_TEXTINPUT)
  if (sdl_event->type == SDL_TEXTINPUT) {
    m3_usize text_len;
    const char *text;

    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->text.text;
    rc = m3_sdl3_text_length(text, &text_len);
    M3_SDL3_RETURN_IF_ERROR(rc);

    if (text_len <= (m3_usize)(sizeof(out_event->data.text.utf8) - 1u)) {
      out_event->type = M3_INPUT_TEXT;
      rc = m3_sdl3_copy_text(text, &out_event->data.text);
      M3_SDL3_RETURN_IF_ERROR(rc);
    } else {
      out_event->type = M3_INPUT_TEXT_UTF8;
      out_event->data.text_utf8.utf8 = text;
      out_event->data.text_utf8.length = text_len;
    }

    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif

#if defined(SDL_EVENT_TEXT_EDITING)
  if (sdl_event->type == SDL_EVENT_TEXT_EDITING) {
    m3_usize text_len;
    const char *text;

    out_event->type = M3_INPUT_TEXT_EDIT;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->edit.text;
    rc = m3_sdl3_text_length(text, &text_len);
    M3_SDL3_RETURN_IF_ERROR(rc);

    out_event->data.text_edit.utf8 = text;
    out_event->data.text_edit.length = text_len;
    out_event->data.text_edit.cursor = (m3_i32)sdl_event->edit.start;
    out_event->data.text_edit.selection_length = (m3_i32)sdl_event->edit.length;

    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_TEXTEDITING)
  if (sdl_event->type == SDL_TEXTEDITING) {
    m3_usize text_len;
    const char *text;

    out_event->type = M3_INPUT_TEXT_EDIT;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;

    text = sdl_event->edit.text;
    rc = m3_sdl3_text_length(text, &text_len);
    M3_SDL3_RETURN_IF_ERROR(rc);

    out_event->data.text_edit.utf8 = text;
    out_event->data.text_edit.length = text_len;
    out_event->data.text_edit.cursor = (m3_i32)sdl_event->edit.start;
    out_event->data.text_edit.selection_length = (m3_i32)sdl_event->edit.length;

    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif

#if defined(SDL_EVENT_MOUSE_BUTTON_DOWN)
  if (sdl_event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
    out_event->type = M3_INPUT_POINTER_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_EVENT_MOUSE_BUTTON_UP)
  if (sdl_event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
    out_event->type = M3_INPUT_POINTER_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_EVENT_MOUSE_MOTION)
  if (sdl_event->type == SDL_EVENT_MOUSE_MOTION) {
    out_event->type = M3_INPUT_POINTER_MOVE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_EVENT_MOUSE_WHEEL)
  if (sdl_event->type == SDL_EVENT_MOUSE_WHEEL) {
    out_event->type = M3_INPUT_POINTER_SCROLL;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    rc = m3_sdl3_fill_scroll_event(sdl_event, &out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_MOUSEBUTTONDOWN)
  if (sdl_event->type == SDL_MOUSEBUTTONDOWN) {
    out_event->type = M3_INPUT_POINTER_DOWN;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_MOUSEBUTTONUP)
  if (sdl_event->type == SDL_MOUSEBUTTONUP) {
    out_event->type = M3_INPUT_POINTER_UP;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_MOUSEMOTION)
  if (sdl_event->type == SDL_MOUSEMOTION) {
    out_event->type = M3_INPUT_POINTER_MOVE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_MOUSEWHEEL)
  if (sdl_event->type == SDL_MOUSEWHEEL) {
    out_event->type = M3_INPUT_POINTER_SCROLL;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    rc = m3_sdl3_fill_pointer_state(&out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    rc = m3_sdl3_fill_scroll_event(sdl_event, &out_event->data.pointer);
    M3_SDL3_RETURN_IF_ERROR(rc);
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif

#if defined(SDL_EVENT_WINDOW_RESIZED)
  if (sdl_event->type == SDL_EVENT_WINDOW_RESIZED) {
    out_event->type = M3_INPUT_WINDOW_RESIZE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    if (backend->active_window != NULL &&
        backend->active_window->window != NULL) {
      int w = 0;
      int h = 0;
      SDL_GetWindowSize(backend->active_window->window, &w, &h);
      out_event->data.window.width = (m3_i32)w;
      out_event->data.window.height = (m3_i32)h;
    }
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif
#if defined(SDL_WINDOWEVENT)
  if (sdl_event->type == SDL_WINDOWEVENT) {
    out_event->type = M3_INPUT_WINDOW_RESIZE;
    out_event->modifiers = modifiers;
    out_event->window = window_handle;
    if (backend->active_window != NULL &&
        backend->active_window->window != NULL) {
      int w = 0;
      int h = 0;
      SDL_GetWindowSize(backend->active_window->window, &w, &h);
      out_event->data.window.width = (m3_i32)w;
      out_event->data.window.height = (m3_i32)h;
    }
    *out_has_event = M3_TRUE;
    return M3_OK;
  }
#endif

  return M3_OK;
}

static int m3_sdl3_ws_poll_event(void *ws, M3InputEvent *out_event,
                                 M3Bool *out_has_event) {
  struct M3SDL3Backend *backend;
  SDL_Event event;
  M3Bool has_event;
  int rc;
  int sdl_rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.poll_event");
  M3_SDL3_RETURN_IF_ERROR(rc);

  has_event = M3_FALSE;
  sdl_rc = SDL_PollEvent(&event);
  while (sdl_rc != 0) {
    rc = m3_sdl3_translate_event(backend, &event, out_event, &has_event);
    M3_SDL3_RETURN_IF_ERROR(rc);
    if (has_event) {
      *out_has_event = M3_TRUE;
      return M3_OK;
    }
    sdl_rc = SDL_PollEvent(&event);
  }

  memset(out_event, 0, sizeof(*out_event));
  *out_has_event = M3_FALSE;
  return M3_OK;
}

static int m3_sdl3_ws_pump_events(void *ws) {
  struct M3SDL3Backend *backend;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.pump_events");
  M3_SDL3_RETURN_IF_ERROR(rc);

  SDL_PumpEvents();
  return M3_OK;
}

static int m3_sdl3_time_ms(m3_u32 *out_time_ms) {
  Uint64 ticks;

  if (out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  ticks = (Uint64)SDL_GetTicks();
  *out_time_ms = (m3_u32)(ticks & 0xFFFFFFFFu);
  return M3_OK;
}

static int m3_sdl3_ws_get_time_ms(void *ws, m3_u32 *out_time_ms) {
  struct M3SDL3Backend *backend;
  int rc;

  if (ws == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)ws;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  M3_SDL3_RETURN_IF_ERROR(rc);

  return m3_sdl3_time_ms(out_time_ms);
}

static const M3WSVTable g_m3_sdl3_ws_vtable = {m3_sdl3_ws_init,
                                               m3_sdl3_ws_shutdown,
                                               m3_sdl3_ws_create_window,
                                               m3_sdl3_ws_destroy_window,
                                               m3_sdl3_ws_show_window,
                                               m3_sdl3_ws_hide_window,
                                               m3_sdl3_ws_set_window_title,
                                               m3_sdl3_ws_set_window_size,
                                               m3_sdl3_ws_get_window_size,
                                               m3_sdl3_ws_set_window_dpi_scale,
                                               m3_sdl3_ws_get_window_dpi_scale,
                                               m3_sdl3_ws_set_clipboard_text,
                                               m3_sdl3_ws_get_clipboard_text,
                                               m3_sdl3_ws_poll_event,
                                               m3_sdl3_ws_pump_events,
                                               m3_sdl3_ws_get_time_ms};

static int m3_sdl3_get_active_renderer(struct M3SDL3Backend *backend,
                                       SDL_Renderer **out_renderer) {
  if (backend == NULL || out_renderer == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->active_window == NULL ||
      backend->active_window->renderer == NULL) {
    return M3_ERR_STATE;
  }

  *out_renderer = backend->active_window->renderer;
  return M3_OK;
}

static int m3_sdl3_render_fill_rect(SDL_Renderer *renderer,
                                    const M3Rect *rect) {
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_FRect sdl_rect;
#else
  SDL_Rect sdl_rect;
#endif
  int rc;

  if (renderer == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int m3_sdl3_render_line(SDL_Renderer *renderer, M3Scalar x0, M3Scalar y0,
                               M3Scalar x1, M3Scalar y1) {
  int rc;

  if (renderer == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  rc = SDL_RenderLine(renderer, x0, y0, x1, y1);
#else
  rc = SDL_RenderDrawLine(renderer, (int)x0, (int)y0, (int)x1, (int)y1);
#endif

  if (rc != 0) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int m3_sdl3_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width,
                                   m3_i32 height, M3Scalar dpi_scale) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (dpi_scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)gfx;
  if (backend->active_window != NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  M3_SDL3_RETURN_IF_ERROR(rc);

  resolved->width = width;
  resolved->height = height;
  resolved->dpi_scale = dpi_scale;
  backend->active_window = resolved;
  backend->clip_depth = 0u;
  backend->has_transform = M3_FALSE;
  return M3_OK;
}

static int m3_sdl3_gfx_end_frame(void *gfx, M3Handle window) {
  struct M3SDL3Backend *backend;
  M3SDL3Window *resolved;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_resolve(backend, window, M3_SDL3_TYPE_WINDOW,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (backend->active_window != resolved) {
    return M3_ERR_STATE;
  }

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.end_frame");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->renderer != NULL) {
    SDL_RenderPresent(resolved->renderer);
  }
  backend->active_window = NULL;
  return M3_OK;
}

static int m3_sdl3_gfx_clear(void *gfx, M3Color color) {
  struct M3SDL3Backend *backend;
  SDL_Renderer *renderer;
  m3_u8 r;
  m3_u8 g;
  m3_u8 b;
  m3_u8 a;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.clear");
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_get_active_renderer(backend, &renderer);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_color_to_u8(color.r, &r);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.g, &g);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.b, &b);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.a, &a);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (rc != 0) {
    return M3_ERR_UNKNOWN;
  }
  rc = SDL_RenderClear(renderer);
  if (rc != 0) {
    return M3_ERR_UNKNOWN;
  }

  return M3_OK;
}

static int m3_sdl3_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                                 M3Scalar corner_radius) {
  struct M3SDL3Backend *backend;
  SDL_Renderer *renderer;
  m3_u8 r;
  m3_u8 g;
  m3_u8 b;
  m3_u8 a;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }
  if (corner_radius < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_get_active_renderer(backend, &renderer);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_color_to_u8(color.r, &r);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.g, &g);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.b, &b);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.a, &a);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (rc != 0) {
    return M3_ERR_UNKNOWN;
  }

  rc = m3_sdl3_render_fill_rect(renderer, rect);
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_sdl3_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0,
                                 M3Scalar x1, M3Scalar y1, M3Color color,
                                 M3Scalar thickness) {
  struct M3SDL3Backend *backend;
  SDL_Renderer *renderer;
  m3_u8 r;
  m3_u8 g;
  m3_u8 b;
  m3_u8 a;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_line");
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_get_active_renderer(backend, &renderer);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_color_to_u8(color.r, &r);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.g, &g);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.b, &b);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_color_to_u8(color.a, &a);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (rc != 0) {
    return M3_ERR_UNKNOWN;
  }

  rc = m3_sdl3_render_line(renderer, x0, y0, x1, y1);
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_sdl3_gfx_push_clip(void *gfx, const M3Rect *rect) {
  struct M3SDL3Backend *backend;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.push_clip");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (backend->clip_depth >= M3_SDL3_CLIP_STACK_CAPACITY) {
    return M3_ERR_OVERFLOW;
  }

  backend->clip_stack[backend->clip_depth] = *rect;
  backend->clip_depth += 1u;
  return M3_OK;
}

static int m3_sdl3_gfx_pop_clip(void *gfx) {
  struct M3SDL3Backend *backend;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (backend->clip_depth == 0u) {
    return M3_ERR_STATE;
  }

  backend->clip_depth -= 1u;
  return M3_OK;
}

static int m3_sdl3_gfx_set_transform(void *gfx, const M3Mat3 *transform) {
  struct M3SDL3Backend *backend;
  int rc;

  if (gfx == NULL || transform == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.set_transform");
  M3_SDL3_RETURN_IF_ERROR(rc);

  backend->transform = *transform;
  backend->has_transform = M3_TRUE;
  return M3_OK;
}

static int m3_sdl3_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height,
                                      m3_u32 format, const void *pixels,
                                      m3_usize size, M3Handle *out_texture) {
  struct M3SDL3Backend *backend;
  M3SDL3Texture *texture;
  SDL_Renderer *renderer;
  SDL_Texture *sdl_texture;
  Uint32 sdl_format;
  m3_usize required;
  m3_usize row_bytes;
  m3_u32 bpp;
  int rc;

  if (gfx == NULL || out_texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.create_texture");
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_get_active_renderer(backend, &renderer);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_texture_format_to_sdl(format, &sdl_format, &bpp);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_mul_usize((m3_usize)width, (m3_usize)height, &required);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_mul_usize(required, (m3_usize)bpp, &required);
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (pixels != NULL && size < required) {
    return M3_ERR_RANGE;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3SDL3Texture),
                                (void **)&texture);
  M3_SDL3_RETURN_IF_ERROR(rc);

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
    m3_sdl3_backend_log_sdl_error(backend, "gfx.create_texture");
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_UNKNOWN;
  }
  texture->texture = sdl_texture;

  if (pixels != NULL && size > 0) {
    rc = m3_sdl3_mul_usize((m3_usize)width, (m3_usize)bpp, &row_bytes);
    if (rc != M3_OK) {
      SDL_DestroyTexture(sdl_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return rc;
    }
    if (SDL_UpdateTexture(sdl_texture, NULL, pixels, (int)row_bytes) != 0) {
      m3_sdl3_backend_log_sdl_error(backend, "gfx.create_texture");
      SDL_DestroyTexture(sdl_texture);
      backend->allocator.free(backend->allocator.ctx, texture);
      return M3_ERR_UNKNOWN;
    }
  }

  rc = m3_object_header_init(&texture->header, M3_SDL3_TYPE_TEXTURE, 0,
                             &g_m3_sdl3_texture_vtable);
  if (rc != M3_OK) {
    SDL_DestroyTexture(sdl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != M3_OK) {
    SDL_DestroyTexture(sdl_texture);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return M3_OK;
}

static int m3_sdl3_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x,
                                      m3_i32 y, m3_i32 width, m3_i32 height,
                                      const void *pixels, m3_usize size) {
  struct M3SDL3Backend *backend;
  M3SDL3Texture *resolved;
  SDL_Rect rect;
  m3_usize required;
  m3_usize row_bytes;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (x < 0 || y < 0 || width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_resolve(backend, texture, M3_SDL3_TYPE_TEXTURE,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.update_texture");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->texture == NULL) {
    return M3_ERR_STATE;
  }
  if (x + width > resolved->width || y + height > resolved->height) {
    return M3_ERR_RANGE;
  }

  rc = m3_sdl3_mul_usize((m3_usize)width, (m3_usize)height, &required);
  M3_SDL3_RETURN_IF_ERROR(rc);
  rc = m3_sdl3_mul_usize(required, (m3_usize)resolved->bytes_per_pixel,
                         &required);
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (pixels != NULL && size < required) {
    return M3_ERR_RANGE;
  }
  if (pixels == NULL || size == 0) {
    return M3_OK;
  }

  rc = m3_sdl3_mul_usize((m3_usize)width, (m3_usize)resolved->bytes_per_pixel,
                         &row_bytes);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rect.x = (int)x;
  rect.y = (int)y;
  rect.w = (int)width;
  rect.h = (int)height;

  if (SDL_UpdateTexture((SDL_Texture *)resolved->texture, &rect, pixels,
                        (int)row_bytes) != 0) {
    m3_sdl3_backend_log_sdl_error(backend, "gfx.update_texture");
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int m3_sdl3_gfx_destroy_texture(void *gfx, M3Handle texture) {
  struct M3SDL3Backend *backend;
  M3SDL3Texture *resolved;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_resolve(backend, texture, M3_SDL3_TYPE_TEXTURE,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
  M3_SDL3_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_sdl3_gfx_draw_texture(void *gfx, M3Handle texture,
                                    const M3Rect *src, const M3Rect *dst,
                                    M3Scalar opacity) {
  struct M3SDL3Backend *backend;
  M3SDL3Texture *resolved;
  SDL_Renderer *renderer;
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_FRect sdl_src;
  SDL_FRect sdl_dst;
#else
  SDL_Rect sdl_src;
  SDL_Rect sdl_dst;
#endif
  m3_u8 alpha;
  int rc;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)gfx;
  rc = m3_sdl3_backend_resolve(backend, texture, M3_SDL3_TYPE_TEXTURE,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (resolved->texture == NULL) {
    return M3_ERR_STATE;
  }
  if (src->width < 0.0f || src->height < 0.0f || dst->width < 0.0f ||
      dst->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  rc = m3_sdl3_get_active_renderer(backend, &renderer);
  M3_SDL3_RETURN_IF_ERROR(rc);
  if ((SDL_Renderer *)resolved->renderer != renderer) {
    return M3_ERR_STATE;
  }

  rc = m3_sdl3_color_to_u8(opacity, &alpha);
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (SDL_SetTextureBlendMode((SDL_Texture *)resolved->texture,
                              SDL_BLENDMODE_BLEND) != 0) {
    m3_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return M3_ERR_UNKNOWN;
  }
  if (SDL_SetTextureAlphaMod((SDL_Texture *)resolved->texture, alpha) != 0) {
    m3_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return M3_ERR_UNKNOWN;
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
    m3_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return M3_ERR_UNKNOWN;
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
    m3_sdl3_backend_log_sdl_error(backend, "gfx.draw_texture");
    return M3_ERR_UNKNOWN;
  }
#endif
  return M3_OK;
}

static const M3GfxVTable g_m3_sdl3_gfx_vtable = {
    m3_sdl3_gfx_begin_frame,     m3_sdl3_gfx_end_frame,
    m3_sdl3_gfx_clear,           m3_sdl3_gfx_draw_rect,
    m3_sdl3_gfx_draw_line,       m3_sdl3_gfx_push_clip,
    m3_sdl3_gfx_pop_clip,        m3_sdl3_gfx_set_transform,
    m3_sdl3_gfx_create_texture,  m3_sdl3_gfx_update_texture,
    m3_sdl3_gfx_destroy_texture, m3_sdl3_gfx_draw_texture};

static int m3_sdl3_text_make_cstr(struct M3SDL3Backend *backend,
                                  const char *utf8, m3_usize utf8_len,
                                  char **out_text) {
  m3_usize max_value;
  m3_usize required;
  char *buffer;
  int rc;

  if (backend == NULL || out_text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  *out_text = NULL;
  if (utf8_len == 0) {
    if (utf8 != NULL && utf8[0] != '\0') {
      return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_OK;
  }
  if (utf8 == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = (m3_usize) ~(m3_usize)0;
  if (utf8_len > max_value - 1u) {
    return M3_ERR_OVERFLOW;
  }
  required = utf8_len + 1u;

  rc = backend->allocator.alloc(backend->allocator.ctx, required,
                                (void **)&buffer);
  M3_SDL3_RETURN_IF_ERROR(rc);

  memcpy(buffer, utf8, utf8_len);
  buffer[utf8_len] = '\0';
  *out_text = buffer;
  return M3_OK;
}

static int m3_sdl3_text_free_cstr(struct M3SDL3Backend *backend, char *text) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (text == NULL) {
    return M3_OK;
  }
  return backend->allocator.free(backend->allocator.ctx, text);
}

static int m3_sdl3_text_create_font(void *text, const char *utf8_family,
                                    m3_i32 size_px, m3_i32 weight,
                                    M3Bool italic, M3Handle *out_font) {
  struct M3SDL3Backend *backend;
  M3SDL3Font *font;
#if defined(M3_SDL3_TTF_AVAILABLE)
  TTF_Font *ttf_font;
  int style;
#endif
  int rc;

  if (text == NULL || out_font == NULL || utf8_family == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return M3_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3SDL3Backend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.create_font");
  M3_SDL3_RETURN_IF_ERROR(rc);

#if !defined(M3_SDL3_TTF_AVAILABLE)
  M3_UNUSED(font);
  return M3_ERR_UNSUPPORTED;
#else
  ttf_font = TTF_OpenFont(utf8_family, size_px);
  if (ttf_font == NULL) {
    m3_sdl3_backend_log_ttf_error(backend, "text.create_font");
    return M3_ERR_NOT_FOUND;
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

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3SDL3Font),
                                (void **)&font);
  if (rc != M3_OK) {
    TTF_CloseFont(ttf_font);
    return rc;
  }

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? M3_TRUE : M3_FALSE;
  font->font = ttf_font;

  rc = m3_object_header_init(&font->header, M3_SDL3_TYPE_FONT, 0,
                             &g_m3_sdl3_font_vtable);
  if (rc != M3_OK) {
    TTF_CloseFont(ttf_font);
    backend->allocator.free(backend->allocator.ctx, font);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  if (rc != M3_OK) {
    TTF_CloseFont(ttf_font);
    backend->allocator.free(backend->allocator.ctx, font);
    return rc;
  }

  *out_font = font->header.handle;
  return M3_OK;
#endif
}

static int m3_sdl3_text_destroy_font(void *text, M3Handle font) {
  struct M3SDL3Backend *backend;
  M3SDL3Font *resolved;
  int rc;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)text;
  rc = m3_sdl3_backend_resolve(backend, font, M3_SDL3_TYPE_FONT,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.destroy_font");
  M3_SDL3_RETURN_IF_ERROR(rc);

#if !defined(M3_SDL3_TTF_AVAILABLE)
  return M3_ERR_UNSUPPORTED;
#else
  return m3_object_release(&resolved->header);
#endif
}

static int m3_sdl3_text_measure_text(void *text, M3Handle font,
                                     const char *utf8, m3_usize utf8_len,
                                     M3Scalar *out_width, M3Scalar *out_height,
                                     M3Scalar *out_baseline) {
  struct M3SDL3Backend *backend;
  M3SDL3Font *resolved;
#if defined(M3_SDL3_TTF_AVAILABLE)
  char *temp;
  int text_width;
  int text_height;
  int rc_free;
#endif
  M3Scalar size;
  int rc;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)text;
  rc = m3_sdl3_backend_resolve(backend, font, M3_SDL3_TYPE_FONT,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.measure_text");
  M3_SDL3_RETURN_IF_ERROR(rc);

#if !defined(M3_SDL3_TTF_AVAILABLE)
  M3_UNUSED(size);
  return M3_ERR_UNSUPPORTED;
#else
  if (resolved->font == NULL) {
    return M3_ERR_STATE;
  }

  temp = NULL;
  if (utf8_len == 0) {
    text_width = 0;
    text_height = 0;
  } else {
    rc = m3_sdl3_text_make_cstr(backend, utf8, utf8_len, &temp);
    M3_SDL3_RETURN_IF_ERROR(rc);

    if (TTF_SizeUTF8((TTF_Font *)resolved->font, temp, &text_width,
                     &text_height) != 0) {
      m3_sdl3_backend_log_ttf_error(backend, "text.measure_text");
      rc_free = m3_sdl3_text_free_cstr(backend, temp);
      if (rc_free != M3_OK) {
        return rc_free;
      }
      return M3_ERR_UNKNOWN;
    }
    rc = m3_sdl3_text_free_cstr(backend, temp);
    M3_SDL3_RETURN_IF_ERROR(rc);
  }

  size = (M3Scalar)TTF_FontHeight((TTF_Font *)resolved->font);
  *out_width = (M3Scalar)text_width;
  if (text_height == 0) {
    *out_height = size;
  } else {
    *out_height = (M3Scalar)text_height;
  }
  *out_baseline = (M3Scalar)TTF_FontAscent((TTF_Font *)resolved->font);
  return M3_OK;
#endif
}

static int m3_sdl3_text_draw_text(void *text, M3Handle font, const char *utf8,
                                  m3_usize utf8_len, M3Scalar x, M3Scalar y,
                                  M3Color color) {
  struct M3SDL3Backend *backend;
#if defined(M3_SDL3_TTF_AVAILABLE)
  M3SDL3Font *resolved;
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
  m3_u8 r;
  m3_u8 g;
  m3_u8 b;
  m3_u8 a;
  int rc_free;
#endif
  int rc;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)text;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.draw_text");
  M3_SDL3_RETURN_IF_ERROR(rc);

#if !defined(M3_SDL3_TTF_AVAILABLE)
  M3_UNUSED(font);
  M3_UNUSED(x);
  M3_UNUSED(y);
  M3_UNUSED(color);
  return M3_ERR_UNSUPPORTED;
#else
  rc = m3_sdl3_backend_resolve(backend, font, M3_SDL3_TYPE_FONT,
                               (void **)&resolved);
  M3_SDL3_RETURN_IF_ERROR(rc);
  if (resolved->font == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_sdl3_get_active_renderer(backend, &renderer);
  M3_SDL3_RETURN_IF_ERROR(rc);

  temp = NULL;
  if (utf8_len == 0) {
    return M3_OK;
  }

  rc = m3_sdl3_text_make_cstr(backend, utf8, utf8_len, &temp);
  M3_SDL3_RETURN_IF_ERROR(rc);

  rc = m3_sdl3_color_to_u8(color.r, &r);
  if (rc != M3_OK) {
    rc_free = m3_sdl3_text_free_cstr(backend, temp);
    if (rc_free != M3_OK) {
      return rc_free;
    }
    return rc;
  }
  rc = m3_sdl3_color_to_u8(color.g, &g);
  if (rc != M3_OK) {
    rc_free = m3_sdl3_text_free_cstr(backend, temp);
    if (rc_free != M3_OK) {
      return rc_free;
    }
    return rc;
  }
  rc = m3_sdl3_color_to_u8(color.b, &b);
  if (rc != M3_OK) {
    rc_free = m3_sdl3_text_free_cstr(backend, temp);
    if (rc_free != M3_OK) {
      return rc_free;
    }
    return rc;
  }
  rc = m3_sdl3_color_to_u8(color.a, &a);
  if (rc != M3_OK) {
    rc_free = m3_sdl3_text_free_cstr(backend, temp);
    if (rc_free != M3_OK) {
      return rc_free;
    }
    return rc;
  }

  sdl_color.r = r;
  sdl_color.g = g;
  sdl_color.b = b;
  sdl_color.a = a;

  surface = TTF_RenderUTF8_Blended((TTF_Font *)resolved->font, temp, sdl_color);
  rc = m3_sdl3_text_free_cstr(backend, temp);
  if (rc != M3_OK) {
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
    SDL_DestroySurface(surface);
#else
    SDL_FreeSurface(surface);
#endif
    return rc;
  }
  if (surface == NULL) {
    m3_sdl3_backend_log_ttf_error(backend, "text.draw_text");
    return M3_ERR_UNKNOWN;
  }

  texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    m3_sdl3_backend_log_sdl_error(backend, "text.draw_text");
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
    SDL_DestroySurface(surface);
#else
    SDL_FreeSurface(surface);
#endif
    return M3_ERR_UNKNOWN;
  }

#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  dst_rect.x = x;
  dst_rect.y = y;
  dst_rect.w = (float)surface->w;
  dst_rect.h = (float)surface->h;
  if (SDL_RenderTexture(renderer, texture, NULL, &dst_rect) != 0) {
    m3_sdl3_backend_log_sdl_error(backend, "text.draw_text");
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    return M3_ERR_UNKNOWN;
  }
#else
  dst_rect.x = (int)x;
  dst_rect.y = (int)y;
  dst_rect.w = surface->w;
  dst_rect.h = surface->h;
  if (SDL_RenderCopy(renderer, texture, NULL, &dst_rect) != 0) {
    m3_sdl3_backend_log_sdl_error(backend, "text.draw_text");
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return M3_ERR_UNKNOWN;
  }
#endif

  SDL_DestroyTexture(texture);
#if defined(SDL_MAJOR_VERSION) && (SDL_MAJOR_VERSION >= 3)
  SDL_DestroySurface(surface);
#else
  SDL_FreeSurface(surface);
#endif

  return M3_OK;
#endif
}

static const M3TextVTable g_m3_sdl3_text_vtable = {
    m3_sdl3_text_create_font, m3_sdl3_text_destroy_font,
    m3_sdl3_text_measure_text, m3_sdl3_text_draw_text};

static int m3_sdl3_io_read_file(void *io, const char *utf8_path, void *buffer,
                                m3_usize buffer_size, m3_usize *out_read) {
  struct M3SDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)io;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_io_read_file_alloc(void *io, const char *utf8_path,
                                      const M3Allocator *allocator,
                                      void **out_data, m3_usize *out_size) {
  struct M3SDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)io;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_io_write_file(void *io, const char *utf8_path,
                                 const void *data, m3_usize size,
                                 M3Bool overwrite) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)io;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.write_file");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_io_file_exists(void *io, const char *utf8_path,
                                  M3Bool *out_exists) {
  struct M3SDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)io;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.file_exists");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_exists = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_io_delete_file(void *io, const char *utf8_path) {
  struct M3SDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)io;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.delete_file");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_io_stat_file(void *io, const char *utf8_path,
                                M3FileInfo *out_info) {
  struct M3SDL3Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)io;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.stat_file");
  M3_SDL3_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return M3_ERR_UNSUPPORTED;
}

static const M3IOVTable g_m3_sdl3_io_vtable = {
    m3_sdl3_io_read_file,   m3_sdl3_io_read_file_alloc, m3_sdl3_io_write_file,
    m3_sdl3_io_file_exists, m3_sdl3_io_delete_file,     m3_sdl3_io_stat_file};

static int m3_sdl3_sensors_is_available(void *sensors, m3_u32 type,
                                        M3Bool *out_available) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)sensors;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.is_available");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_available = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_sensors_start(void *sensors, m3_u32 type) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)sensors;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.start");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_sensors_stop(void *sensors, m3_u32 type) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)sensors;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.stop");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_sensors_read(void *sensors, m3_u32 type,
                                M3SensorReading *out_reading,
                                M3Bool *out_has_reading) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)sensors;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.read");
  M3_SDL3_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
}

static const M3SensorsVTable g_m3_sdl3_sensors_vtable = {
    m3_sdl3_sensors_is_available, m3_sdl3_sensors_start, m3_sdl3_sensors_stop,
    m3_sdl3_sensors_read};

static int m3_sdl3_camera_open(void *camera, m3_u32 camera_id) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(camera_id);

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)camera;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.open");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_camera_open_with_config(void *camera,
                                           const M3CameraConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return m3_sdl3_camera_open(camera, config->camera_id);
}

static int m3_sdl3_camera_close(void *camera) {
  struct M3SDL3Backend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)camera;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.close");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_camera_start(void *camera) {
  struct M3SDL3Backend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)camera;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.start");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_camera_stop(void *camera) {
  struct M3SDL3Backend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)camera;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.stop");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_camera_read_frame(void *camera, M3CameraFrame *out_frame,
                                     M3Bool *out_has_frame) {
  struct M3SDL3Backend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)camera;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.read_frame");
  M3_SDL3_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
}

static const M3CameraVTable g_m3_sdl3_camera_vtable = {
    m3_sdl3_camera_open,  m3_sdl3_camera_open_with_config,
    m3_sdl3_camera_close, m3_sdl3_camera_start,
    m3_sdl3_camera_stop,  m3_sdl3_camera_read_frame};

#if defined(M3_LIBCURL_AVAILABLE)
typedef struct M3SDL3CurlBuffer {
  const M3Allocator *allocator;
  void *data;
  m3_usize size;
  m3_usize capacity;
  int error;
} M3SDL3CurlBuffer;

static m3_u32 g_m3_sdl3_curl_refcount = 0u;

static int m3_sdl3_network_add_usize(m3_usize a, m3_usize b,
                                     m3_usize *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (b > ((m3_usize) ~(m3_usize)0) - a) {
    return M3_ERR_OVERFLOW;
  }
  *out_value = a + b;
  return M3_OK;
}

static int m3_sdl3_network_mul_usize(m3_usize a, m3_usize b,
                                     m3_usize *out_value) {
  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (a != 0 && b > ((m3_usize) ~(m3_usize)0) / a) {
    return M3_ERR_OVERFLOW;
  }
  *out_value = a * b;
  return M3_OK;
}

static int m3_sdl3_network_error_from_curl(CURLcode code) {
  switch (code) {
  case CURLE_OK:
    return M3_OK;
  case CURLE_UNSUPPORTED_PROTOCOL:
    return M3_ERR_UNSUPPORTED;
  case CURLE_URL_MALFORMAT:
  case CURLE_BAD_FUNCTION_ARGUMENT:
    return M3_ERR_INVALID_ARGUMENT;
  case CURLE_OUT_OF_MEMORY:
    return M3_ERR_OUT_OF_MEMORY;
  case CURLE_OPERATION_TIMEDOUT:
    return M3_ERR_TIMEOUT;
  case CURLE_COULDNT_RESOLVE_HOST:
  case CURLE_COULDNT_RESOLVE_PROXY:
  case CURLE_REMOTE_FILE_NOT_FOUND:
    return M3_ERR_NOT_FOUND;
  case CURLE_LOGIN_DENIED:
  case CURLE_REMOTE_ACCESS_DENIED:
    return M3_ERR_PERMISSION;
  case CURLE_TOO_MANY_REDIRECTS:
  case CURLE_FILESIZE_EXCEEDED:
    return M3_ERR_RANGE;
  default:
    return M3_ERR_IO;
  }
}

static int m3_sdl3_libcurl_init(void) {
  CURLcode curl_rc;

  if (g_m3_sdl3_curl_refcount == 0u) {
    curl_rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_rc != CURLE_OK) {
      return m3_sdl3_network_error_from_curl(curl_rc);
    }
  }

  g_m3_sdl3_curl_refcount += 1u;
  return M3_OK;
}

static int m3_sdl3_libcurl_shutdown(void) {
  if (g_m3_sdl3_curl_refcount == 0u) {
    return M3_ERR_STATE;
  }

  g_m3_sdl3_curl_refcount -= 1u;
  if (g_m3_sdl3_curl_refcount == 0u) {
    curl_global_cleanup();
  }
  return M3_OK;
}

static size_t m3_sdl3_network_write_cb(char *ptr, size_t size, size_t nmemb,
                                       void *userdata) {
  M3SDL3CurlBuffer *buffer;
  m3_usize chunk;
  m3_usize new_size;
  m3_usize alloc_size;
  void *new_data;
  int rc;

  if (userdata == NULL || ptr == NULL) {
    return 0;
  }

  buffer = (M3SDL3CurlBuffer *)userdata;
  if (buffer->allocator == NULL) {
    buffer->error = M3_ERR_INVALID_ARGUMENT;
    return 0;
  }

  if (size == 0 || nmemb == 0) {
    return 0;
  }

  rc = m3_sdl3_network_mul_usize((m3_usize)size, (m3_usize)nmemb, &chunk);
  if (rc != M3_OK) {
    buffer->error = rc;
    return 0;
  }

  rc = m3_sdl3_network_add_usize(buffer->size, chunk, &new_size);
  if (rc != M3_OK) {
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
    if (rc != M3_OK) {
      buffer->error = rc;
      return 0;
    }
    buffer->data = new_data;
    buffer->capacity = alloc_size;
  }

  memcpy((m3_u8 *)buffer->data + buffer->size, ptr, (size_t)chunk);
  buffer->size = new_size;
  return (size_t)chunk;
}

static int m3_sdl3_network_append_headers(struct M3SDL3Backend *backend,
                                          const char *headers,
                                          struct curl_slist **out_list) {
  const char *cursor;
  struct curl_slist *list;

  if (backend == NULL || out_list == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  list = *out_list;
  if (headers == NULL || headers[0] == '\0') {
    *out_list = list;
    return M3_OK;
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
    m3_usize key_len;
    m3_usize value_len;
    m3_usize line_len;
    m3_usize alloc_len;
    m3_usize offset;
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
        return M3_ERR_INVALID_ARGUMENT;
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
        return M3_ERR_INVALID_ARGUMENT;
      }

      key_len = (m3_usize)(key_end - key_start);
      value_len = (m3_usize)(value_end - value_start);
      rc = m3_sdl3_network_add_usize(key_len, 1u, &line_len);
      if (rc != M3_OK) {
        *out_list = list;
        return rc;
      }
      if (value_len > 0u) {
        rc = m3_sdl3_network_add_usize(line_len, 1u, &line_len);
        if (rc != M3_OK) {
          *out_list = list;
          return rc;
        }
      }
      rc = m3_sdl3_network_add_usize(line_len, value_len, &line_len);
      if (rc != M3_OK) {
        *out_list = list;
        return rc;
      }
      rc = m3_sdl3_network_add_usize(line_len, 1u, &alloc_len);
      if (rc != M3_OK) {
        *out_list = list;
        return rc;
      }

      rc = backend->allocator.alloc(backend->allocator.ctx, alloc_len,
                                    (void **)&line);
      if (rc != M3_OK) {
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
      if (rc != M3_OK) {
        *out_list = list;
        return rc;
      }
      if (new_list == NULL) {
        *out_list = list;
        return M3_ERR_OUT_OF_MEMORY;
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
  return M3_OK;
}
#endif

static int m3_sdl3_network_request(void *net, const M3NetworkRequest *request,
                                   const M3Allocator *allocator,
                                   M3NetworkResponse *out_response) {
  struct M3SDL3Backend *backend;
#if defined(M3_LIBCURL_AVAILABLE)
  CURL *curl;
  CURLcode curl_rc;
  struct curl_slist *header_list;
  M3SDL3CurlBuffer buffer;
  long response_code;
  long timeout_ms;
  long body_len;
#endif
  int rc;

  if (net == NULL || request == NULL || allocator == NULL ||
      out_response == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)net;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
  M3_SDL3_RETURN_IF_ERROR(rc);

  memset(out_response, 0, sizeof(*out_response));
#if defined(M3_LIBCURL_AVAILABLE)
  if (request->method == NULL || request->url == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (request->method[0] == '\0' || request->url[0] == '\0') {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (request->body_size > 0 && request->body == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (request->timeout_ms > 0u && request->timeout_ms > (m3_u32)LONG_MAX) {
    return M3_ERR_RANGE;
  }
  if (request->body_size > (m3_usize)LONG_MAX) {
    return M3_ERR_RANGE;
  }

  curl = NULL;
  header_list = NULL;
  buffer.allocator = allocator;
  buffer.data = NULL;
  buffer.size = 0u;
  buffer.capacity = 0u;
  buffer.error = M3_OK;

  curl = curl_easy_init();
  if (curl == NULL) {
    rc = M3_ERR_OUT_OF_MEMORY;
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_URL, request->url);
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request->method);
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_USERAGENT, "LibM3C/1.0");
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc =
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, m3_sdl3_network_write_cb);
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  if (request->timeout_ms > 0u) {
    timeout_ms = (long)request->timeout_ms;
    curl_rc = curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    if (curl_rc != CURLE_OK) {
      rc = m3_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
    curl_rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
    if (curl_rc != CURLE_OK) {
      rc = m3_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
  }

  if (request->headers != NULL && request->headers[0] != '\0') {
    rc =
        m3_sdl3_network_append_headers(backend, request->headers, &header_list);
    if (rc != M3_OK) {
      goto cleanup;
    }
    if (header_list != NULL) {
      curl_rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
      if (curl_rc != CURLE_OK) {
        rc = m3_sdl3_network_error_from_curl(curl_rc);
        goto cleanup;
      }
    }
  }

  if (request->body_size > 0u) {
    body_len = (long)request->body_size;
    curl_rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
    if (curl_rc != CURLE_OK) {
      rc = m3_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
    curl_rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_len);
    if (curl_rc != CURLE_OK) {
      rc = m3_sdl3_network_error_from_curl(curl_rc);
      goto cleanup;
    }
  }

  curl_rc = curl_easy_perform(curl);
  if (curl_rc != CURLE_OK) {
    if (buffer.error != M3_OK) {
      rc = buffer.error;
    } else {
      rc = m3_sdl3_network_error_from_curl(curl_rc);
    }
    goto cleanup;
  }

  if (buffer.error != M3_OK) {
    rc = buffer.error;
    goto cleanup;
  }

  response_code = 0;
  curl_rc = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  if (curl_rc != CURLE_OK) {
    rc = m3_sdl3_network_error_from_curl(curl_rc);
    goto cleanup;
  }
  if (response_code < 0 ||
      (m3_usize)response_code > (m3_usize)((m3_u32) ~(m3_u32)0)) {
    rc = M3_ERR_RANGE;
    goto cleanup;
  }

  out_response->status_code = (m3_u32)response_code;
  out_response->body = buffer.data;
  out_response->body_size = buffer.size;
  buffer.data = NULL;
  rc = M3_OK;

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
  if (rc != M3_OK) {
    memset(out_response, 0, sizeof(*out_response));
  }
  return rc;
#else
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_sdl3_network_free_response(void *net,
                                         const M3Allocator *allocator,
                                         M3NetworkResponse *response) {
  struct M3SDL3Backend *backend;
  int rc;

  if (net == NULL || allocator == NULL || response == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (response->body_size > 0 && response->body == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)net;
  rc =
      m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.free_response");
  M3_SDL3_RETURN_IF_ERROR(rc);
#if defined(M3_LIBCURL_AVAILABLE)
  if (response->body != NULL) {
    rc = allocator->free(allocator->ctx, (void *)response->body);
    if (rc != M3_OK) {
      return rc;
    }
  }

  response->body = NULL;
  response->body_size = 0u;
  response->status_code = 0u;
  return M3_OK;
#else
  return M3_ERR_UNSUPPORTED;
#endif
}

static const M3NetworkVTable g_m3_sdl3_network_vtable = {
    m3_sdl3_network_request, m3_sdl3_network_free_response};

static int m3_sdl3_tasks_thread_create(void *tasks, M3ThreadFn entry,
                                       void *user, M3Handle *out_thread) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(entry);
  M3_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_create");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_thread_join(void *tasks, M3Handle thread) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(thread);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_join");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_mutex_create(void *tasks, M3Handle *out_mutex) {
  struct M3SDL3Backend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_mutex_destroy(void *tasks, M3Handle mutex) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_mutex_lock(void *tasks, M3Handle mutex) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_mutex_unlock(void *tasks, M3Handle mutex) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_sleep_ms(void *tasks, m3_u32 ms) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(ms);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_post(void *tasks, M3TaskFn fn, void *user) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(fn);
  M3_UNUSED(user);

  if (tasks == NULL || fn == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_sdl3_tasks_post_delayed(void *tasks, M3TaskFn fn, void *user,
                                      m3_u32 delay_ms) {
  struct M3SDL3Backend *backend;
  int rc;

  M3_UNUSED(fn);
  M3_UNUSED(user);
  M3_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)tasks;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  M3_SDL3_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static const M3TasksVTable g_m3_sdl3_tasks_vtable = {
    m3_sdl3_tasks_thread_create, m3_sdl3_tasks_thread_join,
    m3_sdl3_tasks_mutex_create,  m3_sdl3_tasks_mutex_destroy,
    m3_sdl3_tasks_mutex_lock,    m3_sdl3_tasks_mutex_unlock,
    m3_sdl3_tasks_sleep_ms,      m3_sdl3_tasks_post,
    m3_sdl3_tasks_post_delayed};

static int m3_sdl3_env_get_io(void *env, M3IO *out_io) {
  struct M3SDL3Backend *backend;
  int rc;

  if (env == NULL || out_io == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)env;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_io");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return M3_OK;
}

static int m3_sdl3_env_get_sensors(void *env, M3Sensors *out_sensors) {
  struct M3SDL3Backend *backend;
  int rc;

  if (env == NULL || out_sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)env;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_sensors");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return M3_OK;
}

static int m3_sdl3_env_get_camera(void *env, M3Camera *out_camera) {
  struct M3SDL3Backend *backend;
  int rc;

  if (env == NULL || out_camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)env;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_camera");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return M3_OK;
}

static int m3_sdl3_env_get_network(void *env, M3Network *out_network) {
  struct M3SDL3Backend *backend;
  int rc;

  if (env == NULL || out_network == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)env;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_network");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return M3_OK;
}

static int m3_sdl3_env_get_tasks(void *env, M3Tasks *out_tasks) {
  struct M3SDL3Backend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)env;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_tasks");
  M3_SDL3_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return M3_OK;
}

static int m3_sdl3_env_get_time_ms(void *env, m3_u32 *out_time_ms) {
  struct M3SDL3Backend *backend;
  int rc;

  if (env == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3SDL3Backend *)env;
  rc = m3_sdl3_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_time_ms");
  M3_SDL3_RETURN_IF_ERROR(rc);

  return m3_sdl3_time_ms(out_time_ms);
}

static const M3EnvVTable g_m3_sdl3_env_vtable = {
    m3_sdl3_env_get_io,      m3_sdl3_env_get_sensors, m3_sdl3_env_get_camera,
    m3_sdl3_env_get_network, m3_sdl3_env_get_tasks,   m3_sdl3_env_get_time_ms};

int M3_CALL m3_sdl3_backend_create(const M3SDL3BackendConfig *config,
                                   M3SDL3Backend **out_backend) {
  M3SDL3BackendConfig local_config;
  M3Allocator allocator;
  struct M3SDL3Backend *backend;
  M3TasksDefaultConfig tasks_config;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_sdl3_backend_config_init(&local_config);
    M3_SDL3_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_sdl3_backend_validate_config(config);
  M3_SDL3_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = m3_get_default_allocator(&allocator);
    M3_SDL3_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(M3SDL3Backend), (void **)&backend);
  M3_SDL3_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->renderer_flags = config->renderer_flags;
  backend->log_enabled = config->enable_logging ? M3_TRUE : M3_FALSE;
  backend->tasks_enabled = config->enable_tasks ? M3_TRUE : M3_FALSE;

  if (backend->log_enabled) {
    rc = m3_log_init(&allocator);
    if (rc != M3_OK && rc != M3_ERR_STATE) {
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->log_owner = (rc == M3_OK) ? M3_TRUE : M3_FALSE;
  }

  rc = m3_handle_system_default_create(config->handle_capacity, &allocator,
                                       &backend->handles);
  if (rc != M3_OK) {
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = m3_sdl3_global_init(backend);
  if (rc != M3_OK) {
    m3_handle_system_default_destroy(&backend->handles);
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
  backend->sdl_initialized = M3_TRUE;

#if defined(M3_LIBCURL_AVAILABLE)
  rc = m3_sdl3_libcurl_init();
  if (rc != M3_OK) {
    m3_sdl3_global_shutdown(backend);
    m3_handle_system_default_destroy(&backend->handles);
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
#endif

#if defined(M3_SDL3_TTF_AVAILABLE)
  rc = m3_sdl3_ttf_init(backend);
  if (rc != M3_OK) {
#if defined(M3_LIBCURL_AVAILABLE)
    m3_sdl3_libcurl_shutdown();
#endif
    m3_sdl3_global_shutdown(backend);
    m3_handle_system_default_destroy(&backend->handles);
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
  backend->ttf_initialized = M3_TRUE;
#endif

  if (backend->tasks_enabled) {
    rc = m3_tasks_default_config_init(&tasks_config);
    if (rc != M3_OK) {
#if defined(M3_SDL3_TTF_AVAILABLE)
      if (backend->ttf_initialized) {
        m3_sdl3_ttf_shutdown(backend);
        backend->ttf_initialized = M3_FALSE;
      }
#endif
#if defined(M3_LIBCURL_AVAILABLE)
      m3_sdl3_libcurl_shutdown();
#endif
      m3_sdl3_global_shutdown(backend);
      m3_handle_system_default_destroy(&backend->handles);
      if (backend->log_owner) {
        m3_log_shutdown();
      }
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    tasks_config.allocator = &backend->allocator;
    rc = m3_tasks_default_create(&tasks_config, &backend->tasks);
    if (rc != M3_OK) {
#if defined(M3_SDL3_TTF_AVAILABLE)
      if (backend->ttf_initialized) {
        m3_sdl3_ttf_shutdown(backend);
        backend->ttf_initialized = M3_FALSE;
      }
#endif
#if defined(M3_LIBCURL_AVAILABLE)
      m3_sdl3_libcurl_shutdown();
#endif
      m3_sdl3_global_shutdown(backend);
      m3_handle_system_default_destroy(&backend->handles);
      if (backend->log_owner) {
        m3_log_shutdown();
      }
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->tasks_owned = M3_TRUE;
  } else {
    backend->tasks.ctx = backend;
    backend->tasks.vtable = &g_m3_sdl3_tasks_vtable;
  }

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_m3_sdl3_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_m3_sdl3_gfx_vtable;
  backend->gfx.text_vtable = &g_m3_sdl3_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_m3_sdl3_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_m3_sdl3_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_m3_sdl3_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_m3_sdl3_camera_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_m3_sdl3_network_vtable;

  backend->initialized = M3_TRUE;

  *out_backend = backend;
  return M3_OK;
}

int M3_CALL m3_sdl3_backend_destroy(M3SDL3Backend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_handle_system_default_destroy(&backend->handles);
  M3_SDL3_RETURN_IF_ERROR(rc);

  first_error = M3_OK;

  if (backend->tasks_owned) {
    rc = m3_tasks_default_destroy(&backend->tasks);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
  }

#if defined(M3_SDL3_TTF_AVAILABLE)
  if (backend->ttf_initialized) {
    rc = m3_sdl3_ttf_shutdown(backend);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    backend->ttf_initialized = M3_FALSE;
  }
#endif

  if (backend->sdl_initialized) {
    rc = m3_sdl3_global_shutdown(backend);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    backend->sdl_initialized = M3_FALSE;
  }

#if defined(M3_LIBCURL_AVAILABLE)
  rc = m3_sdl3_libcurl_shutdown();
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }
#endif

  if (backend->log_owner) {
    rc = m3_log_shutdown();
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
  }

  backend->initialized = M3_FALSE;
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
  backend->network.ctx = NULL;
  backend->network.vtable = NULL;
  backend->tasks.ctx = NULL;
  backend->tasks.vtable = NULL;

  rc = backend->allocator.free(backend->allocator.ctx, backend);
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }

  return first_error;
}

int M3_CALL m3_sdl3_backend_get_ws(M3SDL3Backend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }

  *out_ws = backend->ws;
  return M3_OK;
}

int M3_CALL m3_sdl3_backend_get_gfx(M3SDL3Backend *backend, M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }

  *out_gfx = backend->gfx;
  return M3_OK;
}

int M3_CALL m3_sdl3_backend_get_env(M3SDL3Backend *backend, M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }

  *out_env = backend->env;
  return M3_OK;
}

#else

int M3_CALL m3_sdl3_backend_create(const M3SDL3BackendConfig *config,
                                   M3SDL3Backend **out_backend) {
  M3SDL3BackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_sdl3_backend_config_init(&local_config);
    M3_SDL3_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_sdl3_backend_validate_config(config);
  M3_SDL3_RETURN_IF_ERROR(rc);

  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_sdl3_backend_destroy(M3SDL3Backend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_sdl3_backend_get_ws(M3SDL3Backend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_sdl3_backend_get_gfx(M3SDL3Backend *backend, M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_sdl3_backend_get_env(M3SDL3Backend *backend, M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return M3_ERR_UNSUPPORTED;
}

#endif
