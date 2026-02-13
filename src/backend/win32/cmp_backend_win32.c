#include "cmpc/cmp_backend_win32.h"

#include "cmpc/cmp_log.h"
#include "cmpc/cmp_object.h"

#include <limits.h>
#include <string.h>
#include <wchar.h>

#if defined(CMP_WIN32_AVAILABLE)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef WINVER
#define WINVER 0x0600
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winhttp.h>
#endif

#define CMP_WIN32_RETURN_IF_ERROR(rc)                                          \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_WIN32_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                         \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                      \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_WIN32_DEFAULT_HANDLE_CAPACITY 64u
#define CMP_WIN32_EVENT_CAPACITY 256u
#define CMP_WIN32_CLIP_STACK_CAPACITY 32u

static int
cmp_win32_backend_validate_config(const CMPWin32BackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
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
cmp_win32_backend_test_validate_config(const CMPWin32BackendConfig *config) {
  return cmp_win32_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_win32_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_WIN32_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_win32_backend_config_init(CMPWin32BackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_WIN32_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  return CMP_OK;
}

/* GCOVR_EXCL_START */
#if defined(CMP_WIN32_AVAILABLE)

#define CMP_WIN32_TYPE_WINDOW 1
#define CMP_WIN32_TYPE_TEXTURE 2
#define CMP_WIN32_TYPE_FONT 3

typedef BOOL(WINAPI *CMPWin32AlphaBlendFn)(HDC, int, int, int, int, HDC, int,
                                           int, int, int, BLENDFUNCTION);

typedef struct CMPWin32EventQueue {
  CMPInputEvent events[CMP_WIN32_EVENT_CAPACITY];
  cmp_usize head;
  cmp_usize tail;
  cmp_usize count;
} CMPWin32EventQueue;

typedef struct CMPWin32Window {
  CMPObjectHeader header;
  struct CMPWin32Backend *backend;
  HWND hwnd;
  HDC mem_dc;
  HBITMAP dib;
  void *dib_pixels;
  cmp_i32 width;
  cmp_i32 height;
  CMPScalar dpi_scale;
  CMPScalar dpi_scale_override;
  CMPBool has_scale_override;
  cmp_u32 flags;
  CMPBool visible;
} CMPWin32Window;

typedef struct CMPWin32Texture {
  CMPObjectHeader header;
  struct CMPWin32Backend *backend;
  HDC mem_dc;
  HBITMAP dib;
  void *pixels;
  cmp_i32 width;
  cmp_i32 height;
  cmp_u32 format;
  cmp_u32 bytes_per_pixel;
  cmp_i32 stride;
} CMPWin32Texture;

typedef struct CMPWin32Font {
  CMPObjectHeader header;
  struct CMPWin32Backend *backend;
  HFONT font;
  cmp_i32 size_px;
  cmp_i32 weight;
  CMPBool italic;
} CMPWin32Font;

struct CMPWin32Backend {
  CMPAllocator allocator;
  CMPHandleSystem handles;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPIO io;
  CMPSensors sensors;
  CMPCamera camera;
  CMPNetwork network;
  CMPTasks tasks;
  CMPBool initialized;
  CMPBool log_enabled;
  CMPBool log_owner;
  CMPBool inline_tasks;
  cmp_usize clipboard_limit;
  cmp_u32 time_ms;
  HINSTANCE instance;
  HMODULE msimg32;
  CMPWin32AlphaBlendFn alpha_blend;
  ATOM window_class;
  CMPBool class_registered;
  CMPWin32Window *active_window;
  CMPMat3 transform;
  CMPBool has_transform;
  cmp_usize clip_depth;
  CMPWin32EventQueue event_queue;
};

static const wchar_t g_cmp_win32_window_class_name[] = L"LibCMPCWindow";

static int cmp_win32_mul_usize(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (a != 0 && b > ((cmp_usize) ~(cmp_usize)0) / a) {
    return CMP_ERR_OVERFLOW;
  }
  *out_value = a * b;
  return CMP_OK;
}

static int cmp_win32_add_usize(cmp_usize a, cmp_usize b, cmp_usize *out_value) {
  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (b > ((cmp_usize) ~(cmp_usize)0) - a) {
    return CMP_ERR_OVERFLOW;
  }
  *out_value = a + b;
  return CMP_OK;
}

static int cmp_win32_network_error_from_winhttp(DWORD error_code) {
  switch (error_code) {
  case ERROR_WINHTTP_TIMEOUT:
    return CMP_ERR_TIMEOUT;
  case ERROR_WINHTTP_INVALID_URL:
    return CMP_ERR_INVALID_ARGUMENT;
  case ERROR_WINHTTP_NAME_NOT_RESOLVED:
    return CMP_ERR_NOT_FOUND;
  case ERROR_WINHTTP_SECURE_FAILURE:
  case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED:
  case ERROR_WINHTTP_LOGIN_FAILURE:
    return CMP_ERR_PERMISSION;
  case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:
    return CMP_ERR_UNSUPPORTED;
  case ERROR_WINHTTP_CANNOT_CONNECT:
  case ERROR_WINHTTP_CONNECTION_ERROR:
  case ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR:
#ifdef ERROR_WINHTTP_AUTO_PROXY_SERVICE_FAILED
  case ERROR_WINHTTP_AUTO_PROXY_SERVICE_FAILED:
#endif
    return CMP_ERR_IO;
  default:
    return CMP_ERR_IO;
  }
}

static int cmp_win32_network_copy_wide_range(struct CMPWin32Backend *backend,
                                             const wchar_t *src, DWORD src_len,
                                             wchar_t **out_wide) {
  cmp_usize len_usize;
  cmp_usize alloc_size;
  wchar_t *dst;
  int rc;

  if (backend == NULL || out_wide == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_wide = NULL;

  if (src_len > 0 && src == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_win32_add_usize((cmp_usize)src_len, 1u, &len_usize);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_mul_usize(len_usize, (cmp_usize)sizeof(wchar_t), &alloc_size);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, alloc_size,
                                (void **)&dst);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (src_len > 0) {
    memcpy(dst, src, (size_t)src_len * sizeof(wchar_t));
  }
  dst[src_len] = L'\0';

  *out_wide = dst;
  return CMP_OK;
}

static int cmp_win32_backend_log(struct CMPWin32Backend *backend,
                                 CMPLogLevel level, const char *message) {
  if (backend == NULL || message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->log_enabled) {
    return CMP_OK;
  }
  return cmp_log_write(level, "m3.win32", message);
}

static int cmp_win32_backend_log_last_error(struct CMPWin32Backend *backend,
                                            const char *message) {
  DWORD error_code;
  char buffer[256];
  int rc;

  if (backend == NULL || message == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_ERROR, message);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  error_code = GetLastError();
  if (error_code == 0u) {
    return CMP_OK;
  }

  buffer[0] = '\0';
  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 buffer, (DWORD)sizeof(buffer), NULL);
  if (buffer[0] != '\0') {
    rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_ERROR, buffer);
    CMP_WIN32_RETURN_IF_ERROR(rc);
  }
  return CMP_OK;
}

static void
cmp_win32_backend_init_alpha_blend(struct CMPWin32Backend *backend) {
  HMODULE module;
  CMPWin32AlphaBlendFn fn;

  if (backend == NULL) {
    return;
  }

  backend->msimg32 = NULL;
  backend->alpha_blend = NULL;

  module = LoadLibraryA("msimg32.dll");
  if (module == NULL) {
    return;
  }

  fn = (CMPWin32AlphaBlendFn)GetProcAddress(module, "AlphaBlend");
  if (fn == NULL) {
    FreeLibrary(module);
    return;
  }

  backend->msimg32 = module;
  backend->alpha_blend = fn;
}

static void cmp_win32_event_queue_init(CMPWin32EventQueue *queue) {
  if (queue == NULL) {
    return;
  }
  memset(queue, 0, sizeof(*queue));
}

static int cmp_win32_event_queue_push(CMPWin32EventQueue *queue,
                                      const CMPInputEvent *event) {
  if (queue == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (queue->count >= CMP_WIN32_EVENT_CAPACITY) {
    return CMP_ERR_OVERFLOW;
  }
  queue->events[queue->tail] = *event;
  queue->tail = (queue->tail + 1u) % CMP_WIN32_EVENT_CAPACITY;
  queue->count += 1u;
  return CMP_OK;
}

static int cmp_win32_event_queue_pop(CMPWin32EventQueue *queue,
                                     CMPInputEvent *out_event,
                                     CMPBool *out_has_event) {
  if (queue == NULL || out_event == NULL || out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (queue->count == 0u) {
    memset(out_event, 0, sizeof(*out_event));
    *out_has_event = CMP_FALSE;
    return CMP_OK;
  }
  *out_event = queue->events[queue->head];
  queue->head = (queue->head + 1u) % CMP_WIN32_EVENT_CAPACITY;
  queue->count -= 1u;
  *out_has_event = CMP_TRUE;
  return CMP_OK;
}

static int cmp_win32_backend_push_event(struct CMPWin32Backend *backend,
                                        const CMPInputEvent *event) {
  int rc;

  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_win32_event_queue_push(&backend->event_queue, event);
  if (rc == CMP_ERR_OVERFLOW) {
    return CMP_OK;
  }
  return rc;
}

static cmp_u32 cmp_win32_get_modifiers(void) {
  cmp_u32 mods = 0u;

  if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
    mods |= CMP_MOD_SHIFT;
  }
  if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
    mods |= CMP_MOD_CTRL;
  }
  if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
    mods |= CMP_MOD_ALT;
  }
  if ((GetKeyState(VK_LWIN) & 0x8000) != 0 ||
      (GetKeyState(VK_RWIN) & 0x8000) != 0) {
    mods |= CMP_MOD_META;
  }
  if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
    mods |= CMP_MOD_CAPS;
  }
  if ((GetKeyState(VK_NUMLOCK) & 0x0001) != 0) {
    mods |= CMP_MOD_NUM;
  }
  return mods;
}

static cmp_u32 cmp_win32_get_time_ms(void) { return (cmp_u32)GetTickCount(); }

static int cmp_win32_utf8_to_wide_alloc(struct CMPWin32Backend *backend,
                                        const char *utf8, int utf8_len,
                                        wchar_t **out_wide, int *out_wide_len) {
  wchar_t *wide;
  int wide_len;
  int rc;

  if (backend == NULL || utf8 == NULL || out_wide == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_wide = NULL;
  if (out_wide_len != NULL) {
    *out_wide_len = 0;
  }

  wide_len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8_len,
                                 NULL, 0);
  if (wide_len <= 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (utf8_len >= 0) {
    rc = backend->allocator.alloc(backend->allocator.ctx,
                                  (cmp_usize)(wide_len + 1) * sizeof(wchar_t),
                                  (void **)&wide);
    CMP_WIN32_RETURN_IF_ERROR(rc);
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8_len, wide,
                            wide_len) != wide_len) {
      backend->allocator.free(backend->allocator.ctx, wide);
      return CMP_ERR_INVALID_ARGUMENT;
    }
    wide[wide_len] = L'\0';
  } else {
    rc = backend->allocator.alloc(backend->allocator.ctx,
                                  (cmp_usize)wide_len * sizeof(wchar_t),
                                  (void **)&wide);
    CMP_WIN32_RETURN_IF_ERROR(rc);
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, utf8_len, wide,
                            wide_len) != wide_len) {
      backend->allocator.free(backend->allocator.ctx, wide);
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }

  *out_wide = wide;
  if (out_wide_len != NULL) {
    if (utf8_len >= 0) {
      *out_wide_len = wide_len;
    } else if (wide_len > 0) {
      *out_wide_len = wide_len - 1;
    } else {
      *out_wide_len = 0;
    }
  }
  return CMP_OK;
}

static int cmp_win32_wide_to_utf8(const wchar_t *wide, int wide_len,
                                  char *buffer, cmp_usize buffer_size,
                                  cmp_usize *out_length) {
  int required;

  if (wide == NULL || buffer == NULL || out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  required =
      WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, NULL, 0, NULL, NULL);
  if (required <= 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if ((cmp_usize)required > buffer_size) {
    if (wide_len < 0) {
      *out_length = (cmp_usize)(required - 1);
    } else {
      *out_length = (cmp_usize)required;
    }
    return CMP_ERR_RANGE;
  }

  if (WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, buffer, required, NULL,
                          NULL) != required) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (wide_len < 0 && required > 0) {
    *out_length = (cmp_usize)(required - 1);
  } else {
    *out_length = (cmp_usize)required;
  }
  return CMP_OK;
}

static int cmp_win32_backend_resolve(struct CMPWin32Backend *backend,
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
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_object_get_type_id((const CMPObjectHeader *)resolved, &actual_type);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return CMP_OK;
}

static int cmp_win32_object_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}

static int cmp_win32_object_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}

static int cmp_win32_object_get_type_id(void *obj, cmp_u32 *out_type_id) {
  return cmp_object_get_type_id((const CMPObjectHeader *)obj, out_type_id);
}

static int cmp_win32_window_destroy(void *obj) {
  CMPWin32Window *window;
  struct CMPWin32Backend *backend;
  int rc;

  window = (CMPWin32Window *)obj;
  backend = window->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (window->mem_dc != NULL) {
    DeleteDC(window->mem_dc);
    window->mem_dc = NULL;
  }
  if (window->dib != NULL) {
    DeleteObject(window->dib);
    window->dib = NULL;
    window->dib_pixels = NULL;
  }

  if (window->hwnd != NULL) {
    DestroyWindow(window->hwnd);
    window->hwnd = NULL;
  }

  rc = backend->allocator.free(backend->allocator.ctx, window);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static void
cmp_win32_window_cleanup_unregistered(struct CMPWin32Backend *backend,
                                      CMPWin32Window *window) {
  if (window == NULL || backend == NULL) {
    return;
  }
  if (window->mem_dc != NULL) {
    DeleteDC(window->mem_dc);
    window->mem_dc = NULL;
  }
  if (window->dib != NULL) {
    DeleteObject(window->dib);
    window->dib = NULL;
    window->dib_pixels = NULL;
  }
  if (window->hwnd != NULL) {
    DestroyWindow(window->hwnd);
    window->hwnd = NULL;
  }
  backend->allocator.free(backend->allocator.ctx, window);
}

static int cmp_win32_texture_destroy(void *obj) {
  CMPWin32Texture *texture;
  struct CMPWin32Backend *backend;
  int rc;

  texture = (CMPWin32Texture *)obj;
  backend = texture->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  texture->header.handle);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (texture->mem_dc != NULL) {
    DeleteDC(texture->mem_dc);
    texture->mem_dc = NULL;
  }
  if (texture->dib != NULL) {
    DeleteObject(texture->dib);
    texture->dib = NULL;
    texture->pixels = NULL;
  }

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static void
cmp_win32_texture_cleanup_unregistered(struct CMPWin32Backend *backend,
                                       CMPWin32Texture *texture) {
  if (texture == NULL || backend == NULL) {
    return;
  }
  if (texture->mem_dc != NULL) {
    DeleteDC(texture->mem_dc);
    texture->mem_dc = NULL;
  }
  if (texture->dib != NULL) {
    DeleteObject(texture->dib);
    texture->dib = NULL;
    texture->pixels = NULL;
  }
  backend->allocator.free(backend->allocator.ctx, texture);
}

static int cmp_win32_font_destroy(void *obj) {
  CMPWin32Font *font;
  struct CMPWin32Backend *backend;
  int rc;

  font = (CMPWin32Font *)obj;
  backend = font->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (font->font != NULL) {
    DeleteObject(font->font);
    font->font = NULL;
  }

  rc = backend->allocator.free(backend->allocator.ctx, font);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static void cmp_win32_font_cleanup_unregistered(struct CMPWin32Backend *backend,
                                                CMPWin32Font *font) {
  if (font == NULL || backend == NULL) {
    return;
  }
  if (font->font != NULL) {
    DeleteObject(font->font);
    font->font = NULL;
  }
  backend->allocator.free(backend->allocator.ctx, font);
}

static const CMPObjectVTable g_cmp_win32_window_vtable = {
    cmp_win32_object_retain, cmp_win32_object_release, cmp_win32_window_destroy,
    cmp_win32_object_get_type_id};

static const CMPObjectVTable g_cmp_win32_texture_vtable = {
    cmp_win32_object_retain, cmp_win32_object_release,
    cmp_win32_texture_destroy, cmp_win32_object_get_type_id};

static const CMPObjectVTable g_cmp_win32_font_vtable = {
    cmp_win32_object_retain, cmp_win32_object_release, cmp_win32_font_destroy,
    cmp_win32_object_get_type_id};

static int cmp_win32_window_flags_to_style(cmp_u32 flags, DWORD *out_style,
                                           DWORD *out_ex_style) {
  DWORD style;
  DWORD ex_style;

  if (out_style == NULL || out_ex_style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  style = 0u;
  ex_style = 0u;

  if ((flags & CMP_WS_WINDOW_FULLSCREEN) != 0u) {
    style = WS_POPUP;
    ex_style = WS_EX_TOPMOST;
  } else if ((flags & CMP_WS_WINDOW_BORDERLESS) != 0u) {
    style = WS_POPUP;
  } else {
    style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    if ((flags & CMP_WS_WINDOW_RESIZABLE) != 0u) {
      style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
    }
  }

  *out_style = style;
  *out_ex_style = ex_style;
  return CMP_OK;
}

static CMPScalar cmp_win32_query_dpi_scale(HWND hwnd) {
  HDC hdc;
  int dpi;
  CMPScalar scale;

  hdc = GetDC(hwnd);
  if (hdc == NULL) {
    return 1.0f;
  }
  dpi = GetDeviceCaps(hdc, LOGPIXELSX);
  ReleaseDC(hwnd, hdc);
  if (dpi <= 0) {
    return 1.0f;
  }
  scale = (CMPScalar)dpi / 96.0f;
  if (scale <= 0.0f) {
    scale = 1.0f;
  }
  return scale;
}

static int cmp_win32_window_ensure_backbuffer(CMPWin32Window *window,
                                              cmp_i32 width, cmp_i32 height) {
  BITMAPINFO bmi;
  void *pixels;
  HBITMAP dib;
  HDC mem_dc;

  if (window == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }

  if (window->mem_dc == NULL) {
    mem_dc = CreateCompatibleDC(NULL);
    if (mem_dc == NULL) {
      return CMP_ERR_UNKNOWN;
    }
    window->mem_dc = mem_dc;
  }

  if (window->dib != NULL && window->width == width &&
      window->height == height) {
    return CMP_OK;
  }

  memset(&bmi, 0, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  dib =
      CreateDIBSection(window->mem_dc, &bmi, DIB_RGB_COLORS, &pixels, NULL, 0);
  if (dib == NULL || pixels == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  SelectObject(window->mem_dc, dib);
  if (window->dib != NULL) {
    DeleteObject(window->dib);
  }
  window->dib = dib;
  window->dib_pixels = pixels;
  window->width = width;
  window->height = height;
  return CMP_OK;
}

static int cmp_win32_apply_transform(CMPWin32Window *window,
                                     const CMPMat3 *transform,
                                     CMPBool has_transform) {
  XFORM xform;

  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (!SetGraphicsMode(window->mem_dc, GM_ADVANCED)) {
    return CMP_ERR_UNKNOWN;
  }

  if (!has_transform) {
    xform.eM11 = 1.0f;
    xform.eM12 = 0.0f;
    xform.eM21 = 0.0f;
    xform.eM22 = 1.0f;
    xform.eDx = 0.0f;
    xform.eDy = 0.0f;
  } else {
    xform.eM11 = transform->m[0];
    xform.eM12 = transform->m[1];
    xform.eM21 = transform->m[3];
    xform.eM22 = transform->m[4];
    xform.eDx = transform->m[6];
    xform.eDy = transform->m[7];
  }

  if (!SetWorldTransform(window->mem_dc, &xform)) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static cmp_u8 cmp_win32_channel_from_scalar(CMPScalar value) {
  if (value <= 0.0f) {
    return 0u;
  }
  if (value >= 1.0f) {
    return 255u;
  }
  return (cmp_u8)(value * 255.0f + 0.5f);
}

static COLORREF cmp_win32_color_to_colorref(CMPColor color) {
  cmp_u8 r = cmp_win32_channel_from_scalar(color.r);
  cmp_u8 g = cmp_win32_channel_from_scalar(color.g);
  cmp_u8 b = cmp_win32_channel_from_scalar(color.b);
  return RGB(r, g, b);
}

static cmp_u8 cmp_win32_premultiply_channel(cmp_u8 c, cmp_u8 a) {
  return (cmp_u8)((((cmp_u32)c) * ((cmp_u32)a) + 127u) / 255u);
}

static void cmp_win32_texture_copy_pixels(cmp_u8 *dst, cmp_i32 dst_stride,
                                          const cmp_u8 *src, cmp_i32 src_stride,
                                          cmp_i32 width, cmp_i32 height,
                                          cmp_u32 format) {
  cmp_i32 y;
  cmp_i32 x;
  cmp_u8 r;
  cmp_u8 g;
  cmp_u8 b;
  cmp_u8 a;
  const cmp_u8 *srow;
  cmp_u8 *drow;

  for (y = 0; y < height; y++) {
    srow = src + (cmp_isize)y * (cmp_isize)src_stride;
    drow = dst + (cmp_isize)y * (cmp_isize)dst_stride;
    for (x = 0; x < width; x++) {
      if (format == CMP_TEX_FORMAT_RGBA8) {
        r = srow[0];
        g = srow[1];
        b = srow[2];
        a = srow[3];
        srow += 4;
      } else if (format == CMP_TEX_FORMAT_BGRA8) {
        b = srow[0];
        g = srow[1];
        r = srow[2];
        a = srow[3];
        srow += 4;
      } else {
        a = srow[0];
        r = 255u;
        g = 255u;
        b = 255u;
        srow += 1;
      }

      r = cmp_win32_premultiply_channel(r, a);
      g = cmp_win32_premultiply_channel(g, a);
      b = cmp_win32_premultiply_channel(b, a);

      drow[0] = b;
      drow[1] = g;
      drow[2] = r;
      drow[3] = a;
      drow += 4;
    }
  }
}

static LRESULT CALLBACK cmp_win32_wndproc(HWND hwnd, UINT msg, WPARAM wparam,
                                          LPARAM lparam) {
  CMPWin32Window *window;
  struct CMPWin32Backend *backend;
  CMPInputEvent event;
  int rc;

  window = (CMPWin32Window *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  if (msg == WM_NCCREATE) {
    CREATESTRUCTW *create = (CREATESTRUCTW *)lparam;
    window = (CMPWin32Window *)create->lpCreateParams;
    if (window != NULL) {
      window->hwnd = hwnd;
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    }
    return TRUE;
  }

  if (window == NULL) {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  backend = window->backend;
  if (backend == NULL) {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  memset(&event, 0, sizeof(event));
  event.window = window->header.handle;
  event.time_ms = (cmp_u32)GetMessageTime();
  event.modifiers = cmp_win32_get_modifiers();

  switch (msg) {
  case WM_CLOSE:
    event.type = CMP_INPUT_WINDOW_CLOSE;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_SETFOCUS:
    event.type = CMP_INPUT_WINDOW_FOCUS;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_KILLFOCUS:
    event.type = CMP_INPUT_WINDOW_BLUR;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_SIZE:
    event.type = CMP_INPUT_WINDOW_RESIZE;
    event.data.window.width = (cmp_i32)LOWORD(lparam);
    event.data.window.height = (cmp_i32)HIWORD(lparam);
    window->width = event.data.window.width;
    window->height = event.data.window.height;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
    event.type = CMP_INPUT_POINTER_DOWN;
    event.data.pointer.pointer_id = 0;
    event.data.pointer.x = (cmp_i32)(short)LOWORD(lparam);
    event.data.pointer.y = (cmp_i32)(short)HIWORD(lparam);
    event.data.pointer.buttons = (cmp_i32)wparam;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_LBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MBUTTONUP:
    event.type = CMP_INPUT_POINTER_UP;
    event.data.pointer.pointer_id = 0;
    event.data.pointer.x = (cmp_i32)(short)LOWORD(lparam);
    event.data.pointer.y = (cmp_i32)(short)HIWORD(lparam);
    event.data.pointer.buttons = (cmp_i32)wparam;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_MOUSEMOVE:
    event.type = CMP_INPUT_POINTER_MOVE;
    event.data.pointer.pointer_id = 0;
    event.data.pointer.x = (cmp_i32)(short)LOWORD(lparam);
    event.data.pointer.y = (cmp_i32)(short)HIWORD(lparam);
    event.data.pointer.buttons = (cmp_i32)wparam;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_MOUSEWHEEL:
    event.type = CMP_INPUT_POINTER_SCROLL;
    event.data.pointer.pointer_id = 0;
    event.data.pointer.x = (cmp_i32)(short)LOWORD(lparam);
    event.data.pointer.y = (cmp_i32)(short)HIWORD(lparam);
    event.data.pointer.scroll_x = 0;
    event.data.pointer.scroll_y = (cmp_i32)(short)HIWORD(wparam);
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    event.type = CMP_INPUT_KEY_DOWN;
    event.data.key.key_code = (cmp_u32)wparam;
    event.data.key.native_code = (cmp_u32)wparam;
    event.data.key.is_repeat = (lparam & (1 << 30)) ? CMP_TRUE : CMP_FALSE;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_KEYUP:
  case WM_SYSKEYUP:
    event.type = CMP_INPUT_KEY_UP;
    event.data.key.key_code = (cmp_u32)wparam;
    event.data.key.native_code = (cmp_u32)wparam;
    event.data.key.is_repeat = CMP_FALSE;
    rc = cmp_win32_backend_push_event(backend, &event);
    CMP_UNUSED(rc);
    return 0;
  case WM_CHAR: {
    wchar_t wide_char;
    char utf8[8];
    int written;

    wide_char = (wchar_t)wparam;
    written = WideCharToMultiByte(CP_UTF8, 0, &wide_char, 1, utf8,
                                  (int)sizeof(utf8) - 1, NULL, NULL);
    if (written > 0) {
      utf8[written] = '\0';
      event.type = CMP_INPUT_TEXT;
      memset(event.data.text.utf8, 0, sizeof(event.data.text.utf8));
      if (written > (int)sizeof(event.data.text.utf8) - 1) {
        written = (int)sizeof(event.data.text.utf8) - 1;
      }
      memcpy(event.data.text.utf8, utf8, (cmp_usize)written);
      event.data.text.length = (cmp_u32)written;
      rc = cmp_win32_backend_push_event(backend, &event);
      CMP_UNUSED(rc);
    }
    return 0;
  }
  case WM_PAINT: {
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    EndPaint(hwnd, &ps);
    return 0;
  }
  case WM_NCDESTROY:
    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    window->hwnd = NULL;
    break;
  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

static int cmp_win32_pump_messages(struct CMPWin32Backend *backend) {
  MSG msg;

  CMP_UNUSED(backend);

  while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return CMP_OK;
}

static int cmp_win32_ws_init(void *ws, const CMPWSConfig *config) {
  struct CMPWin32Backend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.init");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_win32_ws_shutdown(void *ws) {
  struct CMPWin32Backend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.shutdown");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_win32_ws_create_window(void *ws, const CMPWSWindowConfig *config,
                                      CMPHandle *out_window) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  DWORD style;
  DWORD ex_style;
  RECT rect;
  wchar_t *title_wide;
  int rc;
  cmp_i32 width;
  cmp_i32 height;
  int has_title;

  if (ws == NULL || config == NULL || out_window == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->width <= 0 || config->height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.create_window");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWin32Window),
                                (void **)&window);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->dpi_scale = 1.0f;
  window->flags = config->flags;
  window->visible = CMP_FALSE;
  window->has_scale_override = CMP_FALSE;

  rc = cmp_win32_window_flags_to_style(config->flags, &style, &ex_style);
  if (rc != CMP_OK) {
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  width = config->width;
  height = config->height;
  if ((config->flags & CMP_WS_WINDOW_FULLSCREEN) != 0u) {
    width = (cmp_i32)GetSystemMetrics(SM_CXSCREEN);
    height = (cmp_i32)GetSystemMetrics(SM_CYSCREEN);
  } else {
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    if (!AdjustWindowRectEx(&rect, style, FALSE, ex_style)) {
      backend->allocator.free(backend->allocator.ctx, window);
      return CMP_ERR_UNKNOWN;
    }
    width = (cmp_i32)(rect.right - rect.left);
    height = (cmp_i32)(rect.bottom - rect.top);
  }

  title_wide = NULL;
  has_title = (config->utf8_title != NULL && config->utf8_title[0] != '\0');
  if (has_title) {
    rc = cmp_win32_utf8_to_wide_alloc(backend, config->utf8_title, -1,
                                      &title_wide, NULL);
    if (rc != CMP_OK) {
      backend->allocator.free(backend->allocator.ctx, window);
      return rc;
    }
  }

  window->hwnd =
      CreateWindowExW(ex_style, g_cmp_win32_window_class_name,
                      (has_title && title_wide != NULL) ? title_wide : L"",
                      style, CW_USEDEFAULT, CW_USEDEFAULT, (int)width,
                      (int)height, NULL, NULL, backend->instance, window);

  if (title_wide != NULL) {
    backend->allocator.free(backend->allocator.ctx, title_wide);
  }

  if (window->hwnd == NULL) {
    cmp_win32_backend_log_last_error(backend, "ws.create_window");
    backend->allocator.free(backend->allocator.ctx, window);
    return CMP_ERR_UNKNOWN;
  }

  window->dpi_scale = cmp_win32_query_dpi_scale(window->hwnd);

  rc = cmp_object_header_init(&window->header, CMP_WIN32_TYPE_WINDOW, 0,
                              &g_cmp_win32_window_vtable);
  if (rc != CMP_OK) {
    cmp_win32_window_cleanup_unregistered(backend, window);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  if (rc != CMP_OK) {
    cmp_win32_window_cleanup_unregistered(backend, window);
    return rc;
  }

  *out_window = window->header.handle;
  return CMP_OK;
}

static int cmp_win32_ws_destroy_window(void *ws, CMPHandle window) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.destroy_window");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_win32_ws_show_window(void *ws, CMPHandle window) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.show_window");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  ShowWindow(resolved->hwnd, SW_SHOW);
  resolved->visible = CMP_TRUE;
  return CMP_OK;
}

static int cmp_win32_ws_hide_window(void *ws, CMPHandle window) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.hide_window");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  ShowWindow(resolved->hwnd, SW_HIDE);
  resolved->visible = CMP_FALSE;
  return CMP_OK;
}

static int cmp_win32_ws_set_window_title(void *ws, CMPHandle window,
                                         const char *utf8_title) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  wchar_t *wide;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_title");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_utf8_to_wide_alloc(backend, utf8_title, -1, &wide, NULL);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (!SetWindowTextW(resolved->hwnd, wide)) {
    backend->allocator.free(backend->allocator.ctx, wide);
    return CMP_ERR_UNKNOWN;
  }

  backend->allocator.free(backend->allocator.ctx, wide);
  return CMP_OK;
}

static int cmp_win32_ws_set_window_size(void *ws, CMPHandle window,
                                        cmp_i32 width, cmp_i32 height) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  RECT rect;
  DWORD style;
  DWORD ex_style;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_size");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  style = (DWORD)GetWindowLongPtr(resolved->hwnd, GWL_STYLE);
  ex_style = (DWORD)GetWindowLongPtr(resolved->hwnd, GWL_EXSTYLE);

  rect.left = 0;
  rect.top = 0;
  rect.right = width;
  rect.bottom = height;
  if (!AdjustWindowRectEx(&rect, style, FALSE, ex_style)) {
    return CMP_ERR_UNKNOWN;
  }

  if (!SetWindowPos(resolved->hwnd, NULL, 0, 0, rect.right - rect.left,
                    rect.bottom - rect.top,
                    SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE)) {
    return CMP_ERR_UNKNOWN;
  }

  resolved->width = width;
  resolved->height = height;
  return CMP_OK;
}

static int cmp_win32_ws_get_window_size(void *ws, CMPHandle window,
                                        cmp_i32 *out_width,
                                        cmp_i32 *out_height) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  RECT rect;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_size");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (!GetClientRect(resolved->hwnd, &rect)) {
    return CMP_ERR_UNKNOWN;
  }

  *out_width = (cmp_i32)(rect.right - rect.left);
  *out_height = (cmp_i32)(rect.bottom - rect.top);
  resolved->width = *out_width;
  resolved->height = *out_height;
  return CMP_OK;
}

static int cmp_win32_ws_set_window_dpi_scale(void *ws, CMPHandle window,
                                             CMPScalar scale) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO,
                             "ws.set_window_dpi_scale");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  resolved->dpi_scale_override = scale;
  resolved->has_scale_override = CMP_TRUE;
  return CMP_OK;
}

static int cmp_win32_ws_get_window_dpi_scale(void *ws, CMPHandle window,
                                             CMPScalar *out_scale) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO,
                             "ws.get_window_dpi_scale");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (resolved->has_scale_override) {
    *out_scale = resolved->dpi_scale_override;
  } else {
    *out_scale = cmp_win32_query_dpi_scale(resolved->hwnd);
    resolved->dpi_scale = *out_scale;
  }
  return CMP_OK;
}

static int cmp_win32_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct CMPWin32Backend *backend;
  cmp_usize length;
  wchar_t *wide;
  int rc;
  HGLOBAL mem;
  void *dest;
  cmp_usize bytes;

  if (ws == NULL || utf8_text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO,
                             "ws.set_clipboard_text");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  length = 0;
  while (utf8_text[length] != '\0') {
    length += 1u;
  }
  if (length > backend->clipboard_limit) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_win32_utf8_to_wide_alloc(backend, utf8_text, -1, &wide, NULL);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (!OpenClipboard(NULL)) {
    backend->allocator.free(backend->allocator.ctx, wide);
    return CMP_ERR_STATE;
  }

  EmptyClipboard();

  bytes = (cmp_usize)(wcslen(wide) + 1u) * sizeof(wchar_t);
  mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
  if (mem == NULL) {
    CloseClipboard();
    backend->allocator.free(backend->allocator.ctx, wide);
    return CMP_ERR_OUT_OF_MEMORY;
  }

  dest = GlobalLock(mem);
  if (dest == NULL) {
    GlobalFree(mem);
    CloseClipboard();
    backend->allocator.free(backend->allocator.ctx, wide);
    return CMP_ERR_UNKNOWN;
  }
  memcpy(dest, wide, bytes);
  GlobalUnlock(mem);

  if (SetClipboardData(CF_UNICODETEXT, mem) == NULL) {
    GlobalFree(mem);
    CloseClipboard();
    backend->allocator.free(backend->allocator.ctx, wide);
    return CMP_ERR_UNKNOWN;
  }

  CloseClipboard();
  backend->allocator.free(backend->allocator.ctx, wide);
  return CMP_OK;
}

static int cmp_win32_ws_get_clipboard_text(void *ws, char *buffer,
                                           cmp_usize buffer_size,
                                           cmp_usize *out_length) {
  struct CMPWin32Backend *backend;
  HANDLE mem;
  wchar_t *wide;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO,
                             "ws.get_clipboard_text");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (!OpenClipboard(NULL)) {
    return CMP_ERR_STATE;
  }

  mem = GetClipboardData(CF_UNICODETEXT);
  if (mem == NULL) {
    CloseClipboard();
    buffer[0] = '\0';
    *out_length = 0;
    return CMP_OK;
  }

  wide = (wchar_t *)GlobalLock(mem);
  if (wide == NULL) {
    CloseClipboard();
    return CMP_ERR_UNKNOWN;
  }

  rc = cmp_win32_wide_to_utf8(wide, -1, buffer, buffer_size, out_length);
  GlobalUnlock(mem);
  CloseClipboard();
  return rc;
}

static int cmp_win32_ws_poll_event(void *ws, CMPInputEvent *out_event,
                                   CMPBool *out_has_event) {
  struct CMPWin32Backend *backend;
  int rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.poll_event");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (backend->event_queue.count == 0u) {
    rc = cmp_win32_pump_messages(backend);
    CMP_WIN32_RETURN_IF_ERROR(rc);
  }

  return cmp_win32_event_queue_pop(&backend->event_queue, out_event,
                                   out_has_event);
}

static int cmp_win32_ws_pump_events(void *ws) {
  struct CMPWin32Backend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.pump_events");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  return cmp_win32_pump_messages(backend);
}

static int cmp_win32_ws_get_time_ms(void *ws, cmp_u32 *out_time_ms) {
  struct CMPWin32Backend *backend;
  int rc;

  if (ws == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)ws;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  backend->time_ms = cmp_win32_get_time_ms();
  *out_time_ms = backend->time_ms;
  return CMP_OK;
}

static const CMPWSVTable g_cmp_win32_ws_vtable = {
    cmp_win32_ws_init,
    cmp_win32_ws_shutdown,
    cmp_win32_ws_create_window,
    cmp_win32_ws_destroy_window,
    cmp_win32_ws_show_window,
    cmp_win32_ws_hide_window,
    cmp_win32_ws_set_window_title,
    cmp_win32_ws_set_window_size,
    cmp_win32_ws_get_window_size,
    cmp_win32_ws_set_window_dpi_scale,
    cmp_win32_ws_get_window_dpi_scale,
    cmp_win32_ws_set_clipboard_text,
    cmp_win32_ws_get_clipboard_text,
    cmp_win32_ws_poll_event,
    cmp_win32_ws_pump_events,
    cmp_win32_ws_get_time_ms};

static int cmp_win32_gfx_begin_frame(void *gfx, CMPHandle window, cmp_i32 width,
                                     cmp_i32 height, CMPScalar dpi_scale) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
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

  backend = (struct CMPWin32Backend *)gfx;
  if (backend->active_window != NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_window_ensure_backbuffer(resolved, width, height);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  while (backend->clip_depth > 0u) {
    RestoreDC(resolved->mem_dc, -1);
    backend->clip_depth -= 1u;
  }
  SelectClipRgn(resolved->mem_dc, NULL);

  resolved->width = width;
  resolved->height = height;
  resolved->dpi_scale = dpi_scale;
  backend->active_window = resolved;
  backend->has_transform = CMP_FALSE;
  backend->clip_depth = 0u;

  rc = cmp_win32_apply_transform(resolved, &backend->transform,
                                 backend->has_transform);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_win32_gfx_end_frame(void *gfx, CMPHandle window) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *resolved;
  HDC hdc;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)gfx;
  rc = cmp_win32_backend_resolve(backend, window, CMP_WIN32_TYPE_WINDOW,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (backend->active_window != resolved) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.end_frame");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (resolved->mem_dc != NULL && resolved->hwnd != NULL) {
    hdc = GetDC(resolved->hwnd);
    if (hdc != NULL) {
      BitBlt(hdc, 0, 0, resolved->width, resolved->height, resolved->mem_dc, 0,
             0, SRCCOPY);
      ReleaseDC(resolved->hwnd, hdc);
    }
  }

  while (backend->clip_depth > 0u) {
    RestoreDC(resolved->mem_dc, -1);
    backend->clip_depth -= 1u;
  }

  backend->active_window = NULL;
  return CMP_OK;
}

static int cmp_win32_gfx_clear(void *gfx, CMPColor color) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  HBRUSH brush;
  RECT rect;
  COLORREF ref;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.clear");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  ref = cmp_win32_color_to_colorref(color);
  brush = CreateSolidBrush(ref);
  if (brush == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  rect.left = 0;
  rect.top = 0;
  rect.right = window->width;
  rect.bottom = window->height;
  FillRect(window->mem_dc, &rect, brush);
  DeleteObject(brush);
  return CMP_OK;
}

static int cmp_win32_gfx_draw_rect(void *gfx, const CMPRect *rect,
                                   CMPColor color, CMPScalar corner_radius) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  RECT r;
  COLORREF ref;
  HBRUSH brush;
  HPEN pen;
  HPEN old_pen;
  HBRUSH old_brush;
  int radius;
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

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (rect->width == 0.0f || rect->height == 0.0f) {
    return CMP_OK;
  }

  r.left = (int)rect->x;
  r.top = (int)rect->y;
  r.right = (int)(rect->x + rect->width);
  r.bottom = (int)(rect->y + rect->height);

  ref = cmp_win32_color_to_colorref(color);
  brush = CreateSolidBrush(ref);
  if (brush == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  radius = (int)(corner_radius + 0.5f);
  if (radius <= 0) {
    FillRect(window->mem_dc, &r, brush);
    DeleteObject(brush);
    return CMP_OK;
  }

  pen = CreatePen(PS_NULL, 0, ref);
  if (pen == NULL) {
    DeleteObject(brush);
    return CMP_ERR_UNKNOWN;
  }

  old_pen = (HPEN)SelectObject(window->mem_dc, pen);
  old_brush = (HBRUSH)SelectObject(window->mem_dc, brush);
  RoundRect(window->mem_dc, r.left, r.top, r.right, r.bottom, radius * 2,
            radius * 2);
  SelectObject(window->mem_dc, old_pen);
  SelectObject(window->mem_dc, old_brush);
  DeleteObject(pen);
  DeleteObject(brush);
  return CMP_OK;
}

static int cmp_win32_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                   CMPScalar x1, CMPScalar y1, CMPColor color,
                                   CMPScalar thickness) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  COLORREF ref;
  HPEN pen;
  HPEN old_pen;
  int width;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_line");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  width = (int)(thickness + 0.5f);
  if (width <= 0) {
    width = 1;
  }

  ref = cmp_win32_color_to_colorref(color);
  pen = CreatePen(PS_SOLID, width, ref);
  if (pen == NULL) {
    return CMP_ERR_UNKNOWN;
  }
  old_pen = (HPEN)SelectObject(window->mem_dc, pen);
  MoveToEx(window->mem_dc, (int)x0, (int)y0, NULL);
  LineTo(window->mem_dc, (int)x1, (int)y1);
  SelectObject(window->mem_dc, old_pen);
  DeleteObject(pen);
  return CMP_OK;
}

static int cmp_win32_path_build(HDC dc, const CMPPath *path) {
  CMPScalar current_x;
  CMPScalar current_y;
  CMPScalar start_x;
  CMPScalar start_y;
  CMPBool has_current;
  cmp_usize i;

  if (dc == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (path->count == 0) {
    return CMP_OK;
  }

  has_current = CMP_FALSE;
  current_x = 0.0f;
  current_y = 0.0f;
  start_x = 0.0f;
  start_y = 0.0f;

  for (i = 0; i < path->count; ++i) {
    const CMPPathCmd *cmd = &path->commands[i];
    switch (cmd->type) {
    case CMP_PATH_CMD_MOVE_TO:
      current_x = cmd->data.move_to.x;
      current_y = cmd->data.move_to.y;
      start_x = current_x;
      start_y = current_y;
      has_current = CMP_TRUE;
      if (!MoveToEx(dc, (int)current_x, (int)current_y, NULL)) {
        return CMP_ERR_UNKNOWN;
      }
      break;
    case CMP_PATH_CMD_LINE_TO:
      if (!has_current) {
        return CMP_ERR_STATE;
      }
      current_x = cmd->data.line_to.x;
      current_y = cmd->data.line_to.y;
      if (!LineTo(dc, (int)current_x, (int)current_y)) {
        return CMP_ERR_UNKNOWN;
      }
      break;
    case CMP_PATH_CMD_QUAD_TO: {
      CMPScalar cx1;
      CMPScalar cy1;
      CMPScalar cx2;
      CMPScalar cy2;
      CMPScalar x;
      CMPScalar y;
      POINT pts[3];

      if (!has_current) {
        return CMP_ERR_STATE;
      }

      x = cmd->data.quad_to.x;
      y = cmd->data.quad_to.y;
      cx1 = current_x + (cmd->data.quad_to.cx - current_x) * (2.0f / 3.0f);
      cy1 = current_y + (cmd->data.quad_to.cy - current_y) * (2.0f / 3.0f);
      cx2 = x + (cmd->data.quad_to.cx - x) * (2.0f / 3.0f);
      cy2 = y + (cmd->data.quad_to.cy - y) * (2.0f / 3.0f);

      pts[0].x = (LONG)cx1;
      pts[0].y = (LONG)cy1;
      pts[1].x = (LONG)cx2;
      pts[1].y = (LONG)cy2;
      pts[2].x = (LONG)x;
      pts[2].y = (LONG)y;

      if (!PolyBezierTo(dc, pts, 3)) {
        return CMP_ERR_UNKNOWN;
      }
      current_x = x;
      current_y = y;
      break;
    }
    case CMP_PATH_CMD_CUBIC_TO: {
      CMPScalar x;
      CMPScalar y;
      POINT pts[3];

      if (!has_current) {
        return CMP_ERR_STATE;
      }

      x = cmd->data.cubic_to.x;
      y = cmd->data.cubic_to.y;

      pts[0].x = (LONG)cmd->data.cubic_to.cx1;
      pts[0].y = (LONG)cmd->data.cubic_to.cy1;
      pts[1].x = (LONG)cmd->data.cubic_to.cx2;
      pts[1].y = (LONG)cmd->data.cubic_to.cy2;
      pts[2].x = (LONG)x;
      pts[2].y = (LONG)y;

      if (!PolyBezierTo(dc, pts, 3)) {
        return CMP_ERR_UNKNOWN;
      }
      current_x = x;
      current_y = y;
      break;
    }
    case CMP_PATH_CMD_CLOSE:
      if (!has_current) {
        return CMP_ERR_STATE;
      }
      if (!CloseFigure(dc)) {
        return CMP_ERR_UNKNOWN;
      }
      current_x = start_x;
      current_y = start_y;
      has_current = CMP_FALSE;
      break;
    default:
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  return CMP_OK;
}

static int cmp_win32_gfx_draw_path(void *gfx, const CMPPath *path,
                                   CMPColor color) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  COLORREF ref;
  HBRUSH brush;
  HPEN pen;
  HPEN old_pen;
  HBRUSH old_brush;
  int old_fill_mode;
  int rc;
  int result;

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

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_path");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  ref = cmp_win32_color_to_colorref(color);
  brush = CreateSolidBrush(ref);
  if (brush == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  pen = CreatePen(PS_NULL, 0, ref);
  if (pen == NULL) {
    DeleteObject(brush);
    return CMP_ERR_UNKNOWN;
  }

  old_pen = (HPEN)SelectObject(window->mem_dc, pen);
  old_brush = (HBRUSH)SelectObject(window->mem_dc, brush);
  old_fill_mode = SetPolyFillMode(window->mem_dc, WINDING);
  if (old_fill_mode == 0) {
    SelectObject(window->mem_dc, old_pen);
    SelectObject(window->mem_dc, old_brush);
    DeleteObject(pen);
    DeleteObject(brush);
    return CMP_ERR_UNKNOWN;
  }

  result = CMP_OK;
  if (!BeginPath(window->mem_dc)) {
    result = CMP_ERR_UNKNOWN;
  } else {
    result = cmp_win32_path_build(window->mem_dc, path);
    if (result != CMP_OK) {
      AbortPath(window->mem_dc);
    } else if (!EndPath(window->mem_dc)) {
      AbortPath(window->mem_dc);
      result = CMP_ERR_UNKNOWN;
    } else if (!FillPath(window->mem_dc)) {
      result = CMP_ERR_UNKNOWN;
    }
  }

  SetPolyFillMode(window->mem_dc, old_fill_mode);
  SelectObject(window->mem_dc, old_pen);
  SelectObject(window->mem_dc, old_brush);
  DeleteObject(pen);
  DeleteObject(brush);
  return result;
}

static int cmp_win32_gfx_push_clip(void *gfx, const CMPRect *rect) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  int rc;
  int save_id;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.push_clip");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (backend->clip_depth >= CMP_WIN32_CLIP_STACK_CAPACITY) {
    return CMP_ERR_OVERFLOW;
  }

  save_id = SaveDC(window->mem_dc);
  if (save_id == 0) {
    return CMP_ERR_UNKNOWN;
  }
  IntersectClipRect(window->mem_dc, (int)rect->x, (int)rect->y,
                    (int)(rect->x + rect->width),
                    (int)(rect->y + rect->height));
  backend->clip_depth += 1u;
  return CMP_OK;
}

static int cmp_win32_gfx_pop_clip(void *gfx) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (backend->clip_depth == 0u) {
    return CMP_ERR_STATE;
  }

  if (!RestoreDC(window->mem_dc, -1)) {
    return CMP_ERR_UNKNOWN;
  }
  backend->clip_depth -= 1u;
  rc = cmp_win32_apply_transform(window, &backend->transform,
                                 backend->has_transform);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_win32_gfx_set_transform(void *gfx, const CMPMat3 *transform) {
  struct CMPWin32Backend *backend;
  CMPWin32Window *window;
  int rc;

  if (gfx == NULL || transform == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.set_transform");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  backend->transform = *transform;
  backend->has_transform = CMP_TRUE;
  return cmp_win32_apply_transform(window, transform, CMP_TRUE);
}

static int cmp_win32_gfx_create_texture(void *gfx, cmp_i32 width,
                                        cmp_i32 height, cmp_u32 format,
                                        const void *pixels, cmp_usize size,
                                        CMPHandle *out_texture) {
  struct CMPWin32Backend *backend;
  CMPWin32Texture *texture;
  BITMAPINFO bmi;
  void *dib_pixels;
  HBITMAP dib;
  HDC mem_dc;
  cmp_usize required;
  cmp_usize row_bytes;
  cmp_usize data_bytes;
  cmp_u32 bpp;
  int rc;

  if (gfx == NULL || out_texture == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }
  if (format != CMP_TEX_FORMAT_RGBA8 && format != CMP_TEX_FORMAT_BGRA8 &&
      format != CMP_TEX_FORMAT_A8) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.create_texture");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  bpp = (format == CMP_TEX_FORMAT_A8) ? 1u : 4u;
  rc = cmp_win32_mul_usize((cmp_usize)width, (cmp_usize)height, &required);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  rc = cmp_win32_mul_usize(required, (cmp_usize)bpp, &data_bytes);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  if (pixels != NULL && size < data_bytes) {
    return CMP_ERR_RANGE;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWin32Texture),
                                (void **)&texture);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->bytes_per_pixel = bpp;
  texture->stride = width * 4;

  mem_dc = CreateCompatibleDC(NULL);
  if (mem_dc == NULL) {
    backend->allocator.free(backend->allocator.ctx, texture);
    return CMP_ERR_UNKNOWN;
  }

  memset(&bmi, 0, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  dib = CreateDIBSection(mem_dc, &bmi, DIB_RGB_COLORS, &dib_pixels, NULL, 0);
  if (dib == NULL || dib_pixels == NULL) {
    DeleteDC(mem_dc);
    backend->allocator.free(backend->allocator.ctx, texture);
    return CMP_ERR_UNKNOWN;
  }

  SelectObject(mem_dc, dib);
  texture->mem_dc = mem_dc;
  texture->dib = dib;
  texture->pixels = dib_pixels;

  if (pixels != NULL && size != 0) {
    rc = cmp_win32_mul_usize((cmp_usize)width, (cmp_usize)bpp, &row_bytes);
    if (rc != CMP_OK) {
      cmp_win32_texture_cleanup_unregistered(backend, texture);
      return rc;
    }
    cmp_win32_texture_copy_pixels((cmp_u8 *)texture->pixels, texture->stride,
                                  (const cmp_u8 *)pixels, (cmp_i32)row_bytes,
                                  width, height, format);
  }

  rc = cmp_object_header_init(&texture->header, CMP_WIN32_TYPE_TEXTURE, 0,
                              &g_cmp_win32_texture_vtable);
  if (rc != CMP_OK) {
    cmp_win32_texture_cleanup_unregistered(backend, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != CMP_OK) {
    cmp_win32_texture_cleanup_unregistered(backend, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return CMP_OK;
}

static int cmp_win32_gfx_update_texture(void *gfx, CMPHandle texture, cmp_i32 x,
                                        cmp_i32 y, cmp_i32 width,
                                        cmp_i32 height, const void *pixels,
                                        cmp_usize size) {
  struct CMPWin32Backend *backend;
  CMPWin32Texture *resolved;
  cmp_usize required;
  cmp_usize row_bytes;
  cmp_u8 *dst;
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

  backend = (struct CMPWin32Backend *)gfx;
  rc = cmp_win32_backend_resolve(backend, texture, CMP_WIN32_TYPE_TEXTURE,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.update_texture");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (x + width > resolved->width || y + height > resolved->height) {
    return CMP_ERR_RANGE;
  }

  rc = cmp_win32_mul_usize((cmp_usize)width, (cmp_usize)height, &required);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  rc = cmp_win32_mul_usize(required, (cmp_usize)resolved->bytes_per_pixel,
                           &required);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (pixels != NULL && size < required) {
    return CMP_ERR_RANGE;
  }
  if (pixels == NULL || size == 0) {
    return CMP_OK;
  }

  rc = cmp_win32_mul_usize((cmp_usize)width,
                           (cmp_usize)resolved->bytes_per_pixel, &row_bytes);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  dst = (cmp_u8 *)resolved->pixels +
        (cmp_isize)y * (cmp_isize)resolved->stride + (cmp_isize)x * 4;
  cmp_win32_texture_copy_pixels(dst, resolved->stride, (const cmp_u8 *)pixels,
                                (cmp_i32)row_bytes, width, height,
                                resolved->format);
  return CMP_OK;
}

static int cmp_win32_gfx_destroy_texture(void *gfx, CMPHandle texture) {
  struct CMPWin32Backend *backend;
  CMPWin32Texture *resolved;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)gfx;
  rc = cmp_win32_backend_resolve(backend, texture, CMP_WIN32_TYPE_TEXTURE,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                             "gfx.destroy_texture");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_win32_gfx_draw_texture(void *gfx, CMPHandle texture,
                                      const CMPRect *src, const CMPRect *dst,
                                      CMPScalar opacity) {
  struct CMPWin32Backend *backend;
  CMPWin32Texture *resolved;
  CMPWin32Window *window;
  BLENDFUNCTION blend;
  int rc;
  int src_x;
  int src_y;
  int src_w;
  int src_h;
  int dst_x;
  int dst_y;
  int dst_w;
  int dst_h;
  cmp_u8 alpha;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)gfx;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_resolve(backend, texture, CMP_WIN32_TYPE_TEXTURE,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  src_x = (int)src->x;
  src_y = (int)src->y;
  src_w = (int)src->width;
  src_h = (int)src->height;
  dst_x = (int)dst->x;
  dst_y = (int)dst->y;
  dst_w = (int)dst->width;
  dst_h = (int)dst->height;

  if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
    return CMP_OK;
  }
  if (src_x < 0 || src_y < 0 || src_x + src_w > resolved->width ||
      src_y + src_h > resolved->height) {
    return CMP_ERR_RANGE;
  }
  if (opacity <= 0.0f) {
    return CMP_OK;
  }

  if (backend->alpha_blend != NULL) {
    alpha = (cmp_u8)(opacity * 255.0f + 0.5f);
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = (BYTE)alpha;
    blend.AlphaFormat = AC_SRC_ALPHA;

    if (!backend->alpha_blend(window->mem_dc, dst_x, dst_y, dst_w, dst_h,
                              resolved->mem_dc, src_x, src_y, src_w, src_h,
                              blend)) {
      return CMP_ERR_UNKNOWN;
    }
  } else {
    int old_mode;
    if (opacity < 1.0f) {
      return CMP_ERR_UNSUPPORTED;
    }
    old_mode = SetStretchBltMode(window->mem_dc, HALFTONE);
    if (!StretchBlt(window->mem_dc, dst_x, dst_y, dst_w, dst_h,
                    resolved->mem_dc, src_x, src_y, src_w, src_h, SRCCOPY)) {
      if (old_mode != 0) {
        SetStretchBltMode(window->mem_dc, old_mode);
      }
      return CMP_ERR_UNKNOWN;
    }
    if (old_mode != 0) {
      SetStretchBltMode(window->mem_dc, old_mode);
    }
  }
  return CMP_OK;
}

static const CMPGfxVTable g_cmp_win32_gfx_vtable = {
    cmp_win32_gfx_begin_frame,    cmp_win32_gfx_end_frame,
    cmp_win32_gfx_clear,          cmp_win32_gfx_draw_rect,
    cmp_win32_gfx_draw_line,      cmp_win32_gfx_draw_path,
    cmp_win32_gfx_push_clip,      cmp_win32_gfx_pop_clip,
    cmp_win32_gfx_set_transform,  cmp_win32_gfx_create_texture,
    cmp_win32_gfx_update_texture, cmp_win32_gfx_destroy_texture,
    cmp_win32_gfx_draw_texture};

static int cmp_win32_text_create_font(void *text, const char *utf8_family,
                                      cmp_i32 size_px, cmp_i32 weight,
                                      CMPBool italic, CMPHandle *out_font) {
  struct CMPWin32Backend *backend;
  CMPWin32Font *font;
  wchar_t *family;
  int rc;
  int height;
  int font_weight;
  HFONT hfont;

  if (text == NULL || out_font == NULL || utf8_family == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size_px <= 0) {
    return CMP_ERR_RANGE;
  }
  if (weight < 100 || weight > 900) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.create_font");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_utf8_to_wide_alloc(backend, utf8_family, -1, &family, NULL);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  height = -size_px;
  font_weight = weight;
  hfont = CreateFontW(height, 0, 0, 0, font_weight, italic ? TRUE : FALSE,
                      FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                      DEFAULT_PITCH | FF_DONTCARE, family);

  backend->allocator.free(backend->allocator.ctx, family);

  if (hfont == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPWin32Font),
                                (void **)&font);
  CMP_WIN32_RETURN_IF_ERROR_CLEANUP(rc, DeleteObject(hfont));

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->font = hfont;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? CMP_TRUE : CMP_FALSE;

  rc = cmp_object_header_init(&font->header, CMP_WIN32_TYPE_FONT, 0,
                              &g_cmp_win32_font_vtable);
  if (rc != CMP_OK) {
    cmp_win32_font_cleanup_unregistered(backend, font);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  if (rc != CMP_OK) {
    cmp_win32_font_cleanup_unregistered(backend, font);
    return rc;
  }

  *out_font = font->header.handle;
  return CMP_OK;
}

static int cmp_win32_text_destroy_font(void *text, CMPHandle font) {
  struct CMPWin32Backend *backend;
  CMPWin32Font *resolved;
  int rc;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)text;
  rc = cmp_win32_backend_resolve(backend, font, CMP_WIN32_TYPE_FONT,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.destroy_font");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_win32_text_measure_text(void *text, CMPHandle font,
                                       const char *utf8, cmp_usize utf8_len,
                                       CMPScalar *out_width,
                                       CMPScalar *out_height,
                                       CMPScalar *out_baseline) {
  struct CMPWin32Backend *backend;
  CMPWin32Font *resolved;
  wchar_t *wide;
  int wide_len;
  int rc;
  HDC hdc;
  HFONT old_font;
  SIZE size;
  TEXTMETRIC tm;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_len > (cmp_usize)INT_MAX) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)text;
  rc = cmp_win32_backend_resolve(backend, font, CMP_WIN32_TYPE_FONT,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.measure_text");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  hdc = GetDC(NULL);
  if (hdc == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  old_font = (HFONT)SelectObject(hdc, resolved->font);
  if (!GetTextMetrics(hdc, &tm)) {
    SelectObject(hdc, old_font);
    ReleaseDC(NULL, hdc);
    return CMP_ERR_UNKNOWN;
  }

  if (utf8_len == 0) {
    size.cx = 0;
    size.cy = tm.tmHeight;
  } else {
    wide = NULL;
    wide_len = 0;
    rc = cmp_win32_utf8_to_wide_alloc(backend, utf8, (int)utf8_len, &wide,
                                      &wide_len);
    if (rc != CMP_OK) {
      SelectObject(hdc, old_font);
      ReleaseDC(NULL, hdc);
      return rc;
    }
    if (!GetTextExtentPoint32W(hdc, wide, wide_len, &size)) {
      backend->allocator.free(backend->allocator.ctx, wide);
      SelectObject(hdc, old_font);
      ReleaseDC(NULL, hdc);
      return CMP_ERR_UNKNOWN;
    }
    backend->allocator.free(backend->allocator.ctx, wide);
  }

  SelectObject(hdc, old_font);
  ReleaseDC(NULL, hdc);

  *out_width = (CMPScalar)size.cx;
  *out_height = (CMPScalar)size.cy;
  *out_baseline = (CMPScalar)tm.tmAscent;
  return CMP_OK;
}

static int cmp_win32_text_draw_text(void *text, CMPHandle font,
                                    const char *utf8, cmp_usize utf8_len,
                                    CMPScalar x, CMPScalar y, CMPColor color) {
  struct CMPWin32Backend *backend;
  CMPWin32Font *resolved;
  CMPWin32Window *window;
  wchar_t *wide;
  int wide_len;
  int rc;
  HFONT old_font;
  COLORREF ref;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8_len > (cmp_usize)INT_MAX) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPWin32Backend *)text;
  window = backend->active_window;
  if (window == NULL || window->mem_dc == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_win32_backend_resolve(backend, font, CMP_WIN32_TYPE_FONT,
                                 (void **)&resolved);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.draw_text");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (utf8_len == 0) {
    return CMP_OK;
  }

  wide = NULL;
  wide_len = 0;
  rc = cmp_win32_utf8_to_wide_alloc(backend, utf8, (int)utf8_len, &wide,
                                    &wide_len);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  old_font = (HFONT)SelectObject(window->mem_dc, resolved->font);
  SetBkMode(window->mem_dc, TRANSPARENT);
  ref = cmp_win32_color_to_colorref(color);
  SetTextColor(window->mem_dc, ref);

  TextOutW(window->mem_dc, (int)x, (int)y, wide, wide_len);

  SelectObject(window->mem_dc, old_font);
  backend->allocator.free(backend->allocator.ctx, wide);
  return CMP_OK;
}

static const CMPTextVTable g_cmp_win32_text_vtable = {
    cmp_win32_text_create_font, cmp_win32_text_destroy_font,
    cmp_win32_text_measure_text, cmp_win32_text_draw_text};

static int cmp_win32_io_read_file(void *io, const char *utf8_path, void *buffer,
                                  cmp_usize buffer_size, cmp_usize *out_read) {
  struct CMPWin32Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)io;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_io_read_file_alloc(void *io, const char *utf8_path,
                                        const CMPAllocator *allocator,
                                        void **out_data, cmp_usize *out_size) {
  struct CMPWin32Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)io;
  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_io_write_file(void *io, const char *utf8_path,
                                   const void *data, cmp_usize size,
                                   CMPBool overwrite) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)io;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.write_file");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_io_file_exists(void *io, const char *utf8_path,
                                    CMPBool *out_exists) {
  struct CMPWin32Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)io;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.file_exists");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_exists = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_io_delete_file(void *io, const char *utf8_path) {
  struct CMPWin32Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)io;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.delete_file");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_io_stat_file(void *io, const char *utf8_path,
                                  CMPFileInfo *out_info) {
  struct CMPWin32Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)io;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.stat_file");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return CMP_ERR_UNSUPPORTED;
}

static const CMPIOVTable g_cmp_win32_io_vtable = {
    cmp_win32_io_read_file,   cmp_win32_io_read_file_alloc,
    cmp_win32_io_write_file,  cmp_win32_io_file_exists,
    cmp_win32_io_delete_file, cmp_win32_io_stat_file};

static int cmp_win32_sensors_is_available(void *sensors, cmp_u32 type,
                                          CMPBool *out_available) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)sensors;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                             "sensors.is_available");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_available = CMP_FALSE;
  return CMP_OK;
}

static int cmp_win32_sensors_start(void *sensors, cmp_u32 type) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)sensors;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.start");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_sensors_stop(void *sensors, cmp_u32 type) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)sensors;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.stop");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_sensors_read(void *sensors, cmp_u32 type,
                                  CMPSensorReading *out_reading,
                                  CMPBool *out_has_reading) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)sensors;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.read");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = CMP_FALSE;
  return CMP_OK;
}

static const CMPSensorsVTable g_cmp_win32_sensors_vtable = {
    cmp_win32_sensors_is_available, cmp_win32_sensors_start,
    cmp_win32_sensors_stop, cmp_win32_sensors_read};

static int cmp_win32_camera_open(void *camera, cmp_u32 camera_id) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(camera_id);

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)camera;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.open");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_camera_open_with_config(void *camera,
                                             const CMPCameraConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_win32_camera_open(camera, config->camera_id);
}

static int cmp_win32_camera_close(void *camera) {
  struct CMPWin32Backend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)camera;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.close");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_camera_start(void *camera) {
  struct CMPWin32Backend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)camera;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.start");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_camera_stop(void *camera) {
  struct CMPWin32Backend *backend;
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)camera;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.stop");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_camera_read_frame(void *camera, CMPCameraFrame *out_frame,
                                       CMPBool *out_has_frame) {
  struct CMPWin32Backend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)camera;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.read_frame");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  return CMP_OK;
}

static const CMPCameraVTable g_cmp_win32_camera_vtable = {
    cmp_win32_camera_open,  cmp_win32_camera_open_with_config,
    cmp_win32_camera_close, cmp_win32_camera_start,
    cmp_win32_camera_stop,  cmp_win32_camera_read_frame};

static int cmp_win32_network_request(void *net,
                                     const CMPNetworkRequest *request,
                                     const CMPAllocator *allocator,
                                     CMPNetworkResponse *out_response) {
  struct CMPWin32Backend *backend;
  URL_COMPONENTS components;
  HINTERNET session;
  HINTERNET connection;
  HINTERNET request_handle;
  wchar_t *url_wide;
  wchar_t *method_wide;
  wchar_t *headers_wide;
  wchar_t *host_wide;
  wchar_t *object_wide;
  void *body;
  void *new_body;
  cmp_usize total_size;
  cmp_usize capacity;
  cmp_usize path_len;
  cmp_usize extra_len;
  cmp_usize object_len;
  cmp_usize alloc_size;
  DWORD flags;
  DWORD status_code;
  DWORD status_code_size;
  DWORD bytes_available;
  DWORD bytes_read;
  DWORD body_len;
  DWORD last_error;
  INTERNET_PORT port;
  int rc;
  int log_rc;

  if (net == NULL || request == NULL || allocator == NULL ||
      out_response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)net;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.request");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (request->method == NULL || request->url == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->method[0] == '\0' || request->url[0] == '\0') {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->body_size > 0 && request->body == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (request->timeout_ms > 0u && request->timeout_ms > (cmp_u32)INT_MAX) {
    return CMP_ERR_RANGE;
  }
  if (request->body_size > (cmp_usize) ~(DWORD)0) {
    return CMP_ERR_RANGE;
  }

  session = NULL;
  connection = NULL;
  request_handle = NULL;
  url_wide = NULL;
  method_wide = NULL;
  headers_wide = NULL;
  host_wide = NULL;
  object_wide = NULL;
  body = NULL;
  total_size = 0;
  capacity = 0;

  memset(out_response, 0, sizeof(*out_response));

  rc = cmp_win32_utf8_to_wide_alloc(backend, request->url, -1, &url_wide, NULL);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  memset(&components, 0, sizeof(components));
  components.dwStructSize = sizeof(components);
  components.dwSchemeLength = (DWORD)-1;
  components.dwHostNameLength = (DWORD)-1;
  components.dwUrlPathLength = (DWORD)-1;
  components.dwExtraInfoLength = (DWORD)-1;

  if (!WinHttpCrackUrl(url_wide, 0, 0, &components)) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc = cmp_win32_backend_log_last_error(backend, "network.crack_url");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  if (components.nScheme != INTERNET_SCHEME_HTTP &&
      components.nScheme != INTERNET_SCHEME_HTTPS) {
    rc = CMP_ERR_UNSUPPORTED;
    goto cleanup;
  }

  if (components.dwHostNameLength == 0 || components.lpszHostName == NULL) {
    rc = CMP_ERR_INVALID_ARGUMENT;
    goto cleanup;
  }

  rc = cmp_win32_network_copy_wide_range(backend, components.lpszHostName,
                                         components.dwHostNameLength,
                                         &host_wide);
  if (rc != CMP_OK) {
    goto cleanup;
  }

  path_len = (cmp_usize)components.dwUrlPathLength;
  extra_len = (cmp_usize)components.dwExtraInfoLength;
  if (path_len > 0 && components.lpszUrlPath == NULL) {
    rc = CMP_ERR_INVALID_ARGUMENT;
    goto cleanup;
  }
  if (extra_len > 0 && components.lpszExtraInfo == NULL) {
    rc = CMP_ERR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (path_len == 0 && extra_len == 0) {
    object_len = 1u;
  } else if (path_len == 0) {
    rc = cmp_win32_add_usize(1u, extra_len, &object_len);
    if (rc != CMP_OK) {
      goto cleanup;
    }
  } else {
    rc = cmp_win32_add_usize(path_len, extra_len, &object_len);
    if (rc != CMP_OK) {
      goto cleanup;
    }
  }

  rc = cmp_win32_add_usize(object_len, 1u, &alloc_size);
  if (rc != CMP_OK) {
    goto cleanup;
  }
  rc = cmp_win32_mul_usize(alloc_size, (cmp_usize)sizeof(wchar_t), &alloc_size);
  if (rc != CMP_OK) {
    goto cleanup;
  }

  rc = backend->allocator.alloc(backend->allocator.ctx, alloc_size,
                                (void **)&object_wide);
  if (rc != CMP_OK) {
    goto cleanup;
  }

  if (path_len == 0 && extra_len == 0) {
    object_wide[0] = L'/';
    object_wide[1] = L'\0';
  } else if (path_len == 0) {
    object_wide[0] = L'/';
    if (extra_len > 0) {
      memcpy(object_wide + 1, components.lpszExtraInfo,
             (size_t)extra_len * sizeof(wchar_t));
    }
    object_wide[object_len] = L'\0';
  } else {
    if (path_len > 0) {
      memcpy(object_wide, components.lpszUrlPath,
             (size_t)path_len * sizeof(wchar_t));
    }
    if (extra_len > 0) {
      memcpy(object_wide + path_len, components.lpszExtraInfo,
             (size_t)extra_len * sizeof(wchar_t));
    }
    object_wide[object_len] = L'\0';
  }

  rc = cmp_win32_utf8_to_wide_alloc(backend, request->method, -1, &method_wide,
                                    NULL);
  if (rc != CMP_OK) {
    goto cleanup;
  }

  if (request->headers != NULL && request->headers[0] != '\0') {
    rc = cmp_win32_utf8_to_wide_alloc(backend, request->headers, -1,
                                      &headers_wide, NULL);
    if (rc != CMP_OK) {
      goto cleanup;
    }
  }

  session = WinHttpOpen(L"LibCMPC/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (session == NULL) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc = cmp_win32_backend_log_last_error(backend, "network.open_session");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  if (request->timeout_ms > 0u) {
    if (!WinHttpSetTimeouts(session, (int)request->timeout_ms,
                            (int)request->timeout_ms, (int)request->timeout_ms,
                            (int)request->timeout_ms)) {
      last_error = GetLastError();
      rc = cmp_win32_network_error_from_winhttp(last_error);
      log_rc =
          cmp_win32_backend_log_last_error(backend, "network.set_timeouts");
      if (log_rc != CMP_OK && rc == CMP_OK) {
        rc = log_rc;
      }
      goto cleanup;
    }
  }

  port = components.nPort;
  if (port == 0) {
    if (components.nScheme == INTERNET_SCHEME_HTTPS) {
      port = INTERNET_DEFAULT_HTTPS_PORT;
    } else {
      port = INTERNET_DEFAULT_HTTP_PORT;
    }
  }

  connection = WinHttpConnect(session, host_wide, port, 0);
  if (connection == NULL) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc = cmp_win32_backend_log_last_error(backend, "network.connect");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  flags = 0u;
  if (components.nScheme == INTERNET_SCHEME_HTTPS) {
    flags |= WINHTTP_FLAG_SECURE;
  }

  request_handle = WinHttpOpenRequest(connection, method_wide, object_wide,
                                      NULL, WINHTTP_NO_REFERER,
                                      WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
  if (request_handle == NULL) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc = cmp_win32_backend_log_last_error(backend, "network.open_request");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  body_len = (DWORD)request->body_size;
  if (!WinHttpSendRequest(request_handle,
                          headers_wide != NULL ? headers_wide
                                               : WINHTTP_NO_ADDITIONAL_HEADERS,
                          headers_wide != NULL ? (DWORD)-1 : 0,
                          request->body_size > 0 ? (LPVOID)request->body : NULL,
                          body_len, body_len, 0)) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc = cmp_win32_backend_log_last_error(backend, "network.send_request");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  if (!WinHttpReceiveResponse(request_handle, NULL)) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc =
        cmp_win32_backend_log_last_error(backend, "network.receive_response");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  status_code = 0u;
  status_code_size = sizeof(status_code);
  if (!WinHttpQueryHeaders(
          request_handle, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
          WINHTTP_HEADER_NAME_BY_INDEX, &status_code, &status_code_size,
          WINHTTP_NO_HEADER_INDEX)) {
    last_error = GetLastError();
    rc = cmp_win32_network_error_from_winhttp(last_error);
    log_rc = cmp_win32_backend_log_last_error(backend, "network.query_status");
    if (log_rc != CMP_OK && rc == CMP_OK) {
      rc = log_rc;
    }
    goto cleanup;
  }

  for (;;) {
    bytes_available = 0;
    if (!WinHttpQueryDataAvailable(request_handle, &bytes_available)) {
      last_error = GetLastError();
      rc = cmp_win32_network_error_from_winhttp(last_error);
      log_rc = cmp_win32_backend_log_last_error(backend,
                                                "network.query_data_available");
      if (log_rc != CMP_OK && rc == CMP_OK) {
        rc = log_rc;
      }
      goto cleanup;
    }
    if (bytes_available == 0) {
      break;
    }

    rc = cmp_win32_add_usize(total_size, (cmp_usize)bytes_available,
                             &alloc_size);
    if (rc != CMP_OK) {
      goto cleanup;
    }

    if (alloc_size > capacity) {
      new_body = NULL;
      if (body == NULL) {
        rc = allocator->alloc(allocator->ctx, alloc_size, &new_body);
      } else {
        rc = allocator->realloc(allocator->ctx, body, alloc_size, &new_body);
      }
      if (rc != CMP_OK) {
        goto cleanup;
      }
      body = new_body;
      capacity = alloc_size;
    }

    bytes_read = 0;
    if (!WinHttpReadData(request_handle, (cmp_u8 *)body + total_size,
                         bytes_available, &bytes_read)) {
      last_error = GetLastError();
      rc = cmp_win32_network_error_from_winhttp(last_error);
      log_rc = cmp_win32_backend_log_last_error(backend, "network.read_data");
      if (log_rc != CMP_OK && rc == CMP_OK) {
        rc = log_rc;
      }
      goto cleanup;
    }
    if (bytes_read == 0) {
      rc = CMP_ERR_IO;
      goto cleanup;
    }
    if (bytes_read > bytes_available) {
      rc = CMP_ERR_IO;
      goto cleanup;
    }

    total_size += (cmp_usize)bytes_read;
  }

  out_response->status_code = (cmp_u32)status_code;
  out_response->body = body;
  out_response->body_size = total_size;
  body = NULL;
  rc = CMP_OK;

cleanup:
  if (request_handle != NULL) {
    WinHttpCloseHandle(request_handle);
  }
  if (connection != NULL) {
    WinHttpCloseHandle(connection);
  }
  if (session != NULL) {
    WinHttpCloseHandle(session);
  }
  if (url_wide != NULL) {
    backend->allocator.free(backend->allocator.ctx, url_wide);
  }
  if (method_wide != NULL) {
    backend->allocator.free(backend->allocator.ctx, method_wide);
  }
  if (headers_wide != NULL) {
    backend->allocator.free(backend->allocator.ctx, headers_wide);
  }
  if (host_wide != NULL) {
    backend->allocator.free(backend->allocator.ctx, host_wide);
  }
  if (object_wide != NULL) {
    backend->allocator.free(backend->allocator.ctx, object_wide);
  }
  if (body != NULL) {
    allocator->free(allocator->ctx, body);
  }

  if (rc != CMP_OK) {
    memset(out_response, 0, sizeof(*out_response));
  }

  return rc;
}

static int cmp_win32_network_free_response(void *net,
                                           const CMPAllocator *allocator,
                                           CMPNetworkResponse *response) {
  struct CMPWin32Backend *backend;
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

  backend = (struct CMPWin32Backend *)net;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                             "network.free_response");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (response->body != NULL) {
    rc = allocator->free(allocator->ctx, (void *)response->body);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  response->body = NULL;
  response->body_size = 0;
  response->status_code = 0;
  return CMP_OK;
}

static const CMPNetworkVTable g_cmp_win32_network_vtable = {
    cmp_win32_network_request, cmp_win32_network_free_response};

static int cmp_win32_tasks_thread_create(void *tasks, CMPThreadFn entry,
                                         void *user, CMPHandle *out_thread) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(entry);
  CMP_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                             "tasks.thread_create");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_tasks_thread_join(void *tasks, CMPHandle thread) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(thread);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_join");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_tasks_mutex_create(void *tasks, CMPHandle *out_mutex) {
  struct CMPWin32Backend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_tasks_mutex_destroy(void *tasks, CMPHandle mutex) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                             "tasks.mutex_destroy");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_tasks_mutex_lock(void *tasks, CMPHandle mutex) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_tasks_mutex_unlock(void *tasks, CMPHandle mutex) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_tasks_sleep_ms(void *tasks, cmp_u32 ms) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(ms);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return CMP_OK;
}

static int cmp_win32_tasks_post(void *tasks, CMPTaskFn fn, void *user) {
  struct CMPWin32Backend *backend;
  int rc;

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static int cmp_win32_tasks_post_delayed(void *tasks, CMPTaskFn fn, void *user,
                                        cmp_u32 delay_ms) {
  struct CMPWin32Backend *backend;
  int rc;

  CMP_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)tasks;
  rc =
      cmp_win32_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  CMP_WIN32_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static const CMPTasksVTable g_cmp_win32_tasks_vtable = {
    cmp_win32_tasks_thread_create, cmp_win32_tasks_thread_join,
    cmp_win32_tasks_mutex_create,  cmp_win32_tasks_mutex_destroy,
    cmp_win32_tasks_mutex_lock,    cmp_win32_tasks_mutex_unlock,
    cmp_win32_tasks_sleep_ms,      cmp_win32_tasks_post,
    cmp_win32_tasks_post_delayed};

static int cmp_win32_env_get_io(void *env, CMPIO *out_io) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_io");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return CMP_OK;
}

static int cmp_win32_env_get_sensors(void *env, CMPSensors *out_sensors) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_sensors");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return CMP_OK;
}

static int cmp_win32_env_get_camera(void *env, CMPCamera *out_camera) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_camera");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return CMP_OK;
}

static int cmp_win32_env_get_image(void *env, CMPImage *out_image) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_image");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  out_image->ctx = NULL;
  out_image->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_env_get_video(void *env, CMPVideo *out_video) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_video");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  out_video->ctx = NULL;
  out_video->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_env_get_audio(void *env, CMPAudio *out_audio) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_audio");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  out_audio->ctx = NULL;
  out_audio->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_win32_env_get_network(void *env, CMPNetwork *out_network) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_network");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return CMP_OK;
}

static int cmp_win32_env_get_tasks(void *env, CMPTasks *out_tasks) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_tasks");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return CMP_OK;
}

static int cmp_win32_env_get_time_ms(void *env, cmp_u32 *out_time_ms) {
  struct CMPWin32Backend *backend;
  int rc;

  if (env == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPWin32Backend *)env;
  rc = cmp_win32_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_time_ms");
  CMP_WIN32_RETURN_IF_ERROR(rc);

  backend->time_ms = cmp_win32_get_time_ms();
  *out_time_ms = backend->time_ms;
  return CMP_OK;
}

static const CMPEnvVTable g_cmp_win32_env_vtable = {
    cmp_win32_env_get_io,      cmp_win32_env_get_sensors,
    cmp_win32_env_get_camera,  cmp_win32_env_get_image,
    cmp_win32_env_get_video,   cmp_win32_env_get_audio,
    cmp_win32_env_get_network, cmp_win32_env_get_tasks,
    cmp_win32_env_get_time_ms};

int CMP_CALL cmp_win32_backend_create(const CMPWin32BackendConfig *config,
                                      CMPWin32Backend **out_backend) {
  CMPWin32BackendConfig local_config;
  CMPAllocator allocator;
  struct CMPWin32Backend *backend;
  WNDCLASSEXW wc;
  ATOM atom;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_win32_backend_config_init(&local_config);
    CMP_WIN32_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_win32_backend_validate_config(config);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    CMP_WIN32_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(struct CMPWin32Backend),
                       (void **)&backend);
  CMP_WIN32_RETURN_IF_ERROR(rc);
  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->log_enabled = config->enable_logging ? CMP_TRUE : CMP_FALSE;
  backend->inline_tasks = config->inline_tasks ? CMP_TRUE : CMP_FALSE;
  backend->clipboard_limit = config->clipboard_limit;
  backend->instance = GetModuleHandle(NULL);

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

  memset(&wc, 0, sizeof(wc));
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = cmp_win32_wndproc;
  wc.hInstance = backend->instance;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.lpszClassName = g_cmp_win32_window_class_name;

  atom = RegisterClassExW(&wc);
  if (atom == 0) {
    DWORD err = GetLastError();
    if (err != ERROR_CLASS_ALREADY_EXISTS) {
      cmp_handle_system_default_destroy(&backend->handles);
      if (backend->log_owner) {
        cmp_log_shutdown();
      }
      allocator.free(allocator.ctx, backend);
      return CMP_ERR_UNKNOWN;
    }
  } else {
    backend->class_registered = CMP_TRUE;
  }
  backend->window_class = atom;

  cmp_win32_backend_init_alpha_blend(backend);
  cmp_win32_event_queue_init(&backend->event_queue);

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_cmp_win32_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_cmp_win32_gfx_vtable;
  backend->gfx.text_vtable = &g_cmp_win32_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_cmp_win32_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_cmp_win32_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_cmp_win32_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_cmp_win32_camera_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_cmp_win32_network_vtable;
  backend->tasks.ctx = backend;
  backend->tasks.vtable = &g_cmp_win32_tasks_vtable;
  backend->initialized = CMP_TRUE;

  *out_backend = backend;
  return CMP_OK;
}

int CMP_CALL cmp_win32_backend_destroy(CMPWin32Backend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_handle_system_default_destroy(&backend->handles);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  first_error = CMP_OK;

  if (backend->class_registered) {
    if (!UnregisterClassW(g_cmp_win32_window_class_name, backend->instance) &&
        first_error == CMP_OK) {
      first_error = CMP_ERR_UNKNOWN;
    }
    backend->class_registered = CMP_FALSE;
  }

  if (backend->msimg32 != NULL) {
    FreeLibrary(backend->msimg32);
    backend->msimg32 = NULL;
    backend->alpha_blend = NULL;
  }

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

int CMP_CALL cmp_win32_backend_get_ws(CMPWin32Backend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_ws = backend->ws;
  return CMP_OK;
}

int CMP_CALL cmp_win32_backend_get_gfx(CMPWin32Backend *backend,
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

int CMP_CALL cmp_win32_backend_get_env(CMPWin32Backend *backend,
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

int CMP_CALL cmp_win32_backend_create(const CMPWin32BackendConfig *config,
                                      CMPWin32Backend **out_backend) {
  CMPWin32BackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_win32_backend_config_init(&local_config);
    CMP_WIN32_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_win32_backend_validate_config(config);
  CMP_WIN32_RETURN_IF_ERROR(rc);

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_win32_backend_destroy(CMPWin32Backend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_win32_backend_get_ws(CMPWin32Backend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_win32_backend_get_gfx(CMPWin32Backend *backend,
                                       CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_win32_backend_get_env(CMPWin32Backend *backend,
                                       CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return CMP_ERR_UNSUPPORTED;
}

#endif
