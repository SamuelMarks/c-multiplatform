#include "cmpc/cmp_backend_null.h"

#include "cmpc/cmp_log.h"
#include "cmpc/cmp_object.h"

#include <string.h>

#ifdef CMP_TESTING
static cmp_usize g_cmp_null_test_fail_after = 0u;
static cmp_usize g_cmp_null_test_call_count = 0u;

static void cmp_null_backend_test_maybe_fail(int *rc) {
  if (rc == NULL) {
    return;
  }
  if (g_cmp_null_test_fail_after == 0u) {
    return;
  }
  g_cmp_null_test_call_count += 1u;
  if (g_cmp_null_test_call_count == g_cmp_null_test_fail_after) {
    *rc = CMP_ERR_IO;
  }
}

#define CMP_NULL_TEST_MAYBE_FAIL(rc_ptr)                                       \
  cmp_null_backend_test_maybe_fail(rc_ptr)
#else
#define CMP_NULL_TEST_MAYBE_FAIL(rc_ptr)                                       \
  do {                                                                         \
  } while (0)
#endif

#define CMP_NULL_RETURN_IF_ERROR(rc)                                           \
  do {                                                                         \
    CMP_NULL_TEST_MAYBE_FAIL(&(rc));                                           \
    if ((rc) != CMP_OK) {                                                      \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_NULL_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                          \
  do {                                                                         \
    CMP_NULL_TEST_MAYBE_FAIL(&(rc));                                           \
    if ((rc) != CMP_OK) {                                                      \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_NULL_DEFAULT_HANDLE_CAPACITY 64

#define CMP_NULL_TYPE_WINDOW 1
#define CMP_NULL_TYPE_TEXTURE 2
#define CMP_NULL_TYPE_FONT 3

typedef struct CMPNullWindow {
  CMPObjectHeader header;
  struct CMPNullBackend *backend;
  cmp_i32 width;
  cmp_i32 height;
  CMPScalar dpi_scale;
  cmp_u32 flags;
  CMPBool visible;
} CMPNullWindow;

typedef struct CMPNullTexture {
  CMPObjectHeader header;
  struct CMPNullBackend *backend;
  cmp_i32 width;
  cmp_i32 height;
  cmp_u32 format;
} CMPNullTexture;

typedef struct CMPNullFont {
  CMPObjectHeader header;
  struct CMPNullBackend *backend;
  cmp_i32 size_px;
  cmp_i32 weight;
  CMPBool italic;
} CMPNullFont;

struct CMPNullBackend {
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
  CMPBool initialized;
  CMPBool log_enabled;
  CMPBool log_owner;
  CMPBool inline_tasks;
  cmp_u32 time_ms;
  char *clipboard;
  cmp_usize clipboard_capacity;
  cmp_usize clipboard_length;
  cmp_usize clipboard_limit;
};

static int cmp_null_backend_log(struct CMPNullBackend *backend,
                                CMPLogLevel level, const char *message) {
  if (!backend->log_enabled) {
    return CMP_OK;
  }
  return cmp_log_write(level, "m3.null", message);
}

static int cmp_null_backend_resolve(struct CMPNullBackend *backend,
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
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_object_get_type_id((const CMPObjectHeader *)resolved, &actual_type);
  CMP_NULL_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return CMP_OK;
}

static int cmp_null_object_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}

static int cmp_null_object_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}

static int cmp_null_object_get_type_id(void *obj, cmp_u32 *out_type_id) {
  return cmp_object_get_type_id((const CMPObjectHeader *)obj, out_type_id);
}

static int cmp_null_window_destroy(void *obj) {
  CMPNullWindow *window;
  struct CMPNullBackend *backend;
  int rc;

  window = (CMPNullWindow *)obj;
  backend = window->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, window);
  CMP_NULL_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_null_texture_destroy(void *obj) {
  CMPNullTexture *texture;
  struct CMPNullBackend *backend;
  int rc;

  texture = (CMPNullTexture *)obj;
  backend = texture->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  texture->header.handle);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  CMP_NULL_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_null_font_destroy(void *obj) {
  CMPNullFont *font;
  struct CMPNullBackend *backend;
  int rc;

  font = (CMPNullFont *)obj;
  backend = font->backend;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, font);
  CMP_NULL_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static const CMPObjectVTable g_cmp_null_window_vtable = {
    cmp_null_object_retain, cmp_null_object_release, cmp_null_window_destroy,
    cmp_null_object_get_type_id};

static const CMPObjectVTable g_cmp_null_texture_vtable = {
    cmp_null_object_retain, cmp_null_object_release, cmp_null_texture_destroy,
    cmp_null_object_get_type_id};

static const CMPObjectVTable g_cmp_null_font_vtable = {
    cmp_null_object_retain, cmp_null_object_release, cmp_null_font_destroy,
    cmp_null_object_get_type_id};

static int cmp_null_ws_init(void *ws, const CMPWSConfig *config) {
  struct CMPNullBackend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.init");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_ws_shutdown(void *ws) {
  struct CMPNullBackend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.shutdown");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_ws_create_window(void *ws, const CMPWSWindowConfig *config,
                                     CMPHandle *out_window) {
  struct CMPNullBackend *backend;
  CMPNullWindow *window;
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

  backend = (struct CMPNullBackend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.create_window");
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPNullWindow),
                                (void **)&window);
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->dpi_scale = 1.0f;
  window->flags = config->flags;
  window->visible = CMP_FALSE;

  rc = cmp_object_header_init(&window->header, CMP_NULL_TYPE_WINDOW, 0,
                              &g_cmp_null_window_vtable);
  CMP_NULL_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, window));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  CMP_NULL_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, window));

  *out_window = window->header.handle;
  return CMP_OK;
}

static int cmp_null_ws_destroy_window(void *ws, CMPHandle window) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.destroy_window");
  CMP_NULL_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_null_ws_show_window(void *ws, CMPHandle window) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.show_window");
  CMP_NULL_RETURN_IF_ERROR(rc);

  resolved->visible = CMP_TRUE;
  return CMP_OK;
}

static int cmp_null_ws_hide_window(void *ws, CMPHandle window) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.hide_window");
  CMP_NULL_RETURN_IF_ERROR(rc);

  resolved->visible = CMP_FALSE;
  return CMP_OK;
}

static int cmp_null_ws_set_window_title(void *ws, CMPHandle window,
                                        const char *utf8_title) {
  struct CMPNullBackend *backend;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW, NULL);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_title");
  CMP_NULL_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static int cmp_null_ws_set_window_size(void *ws, CMPHandle window,
                                       cmp_i32 width, cmp_i32 height) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_size");
  CMP_NULL_RETURN_IF_ERROR(rc);

  resolved->width = width;
  resolved->height = height;
  return CMP_OK;
}

static int cmp_null_ws_get_window_size(void *ws, CMPHandle window,
                                       cmp_i32 *out_width,
                                       cmp_i32 *out_height) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_size");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_width = resolved->width;
  *out_height = resolved->height;
  return CMP_OK;
}

static int cmp_null_ws_set_window_dpi_scale(void *ws, CMPHandle window,
                                            CMPScalar scale) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.set_window_dpi_scale");
  CMP_NULL_RETURN_IF_ERROR(rc);

  resolved->dpi_scale = scale;
  return CMP_OK;
}

static int cmp_null_ws_get_window_dpi_scale(void *ws, CMPHandle window,
                                            CMPScalar *out_scale) {
  struct CMPNullBackend *backend;
  CMPNullWindow *resolved;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.get_window_dpi_scale");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_scale = resolved->dpi_scale;
  return CMP_OK;
}

static int cmp_null_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct CMPNullBackend *backend;
  cmp_usize length;
  cmp_usize required;
  int rc;

  if (ws == NULL || utf8_text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.set_clipboard_text");
  CMP_NULL_RETURN_IF_ERROR(rc);

  length = 0;
  while (utf8_text[length] != '\0') {
    length += 1;
  }
  if (length > backend->clipboard_limit) {
    return CMP_ERR_RANGE;
  }

  required = length + 1;
  if (required > backend->clipboard_capacity) {
    void *mem = NULL;
    if (backend->clipboard == NULL) {
      rc = backend->allocator.alloc(backend->allocator.ctx, required, &mem);
    } else {
      rc = backend->allocator.realloc(backend->allocator.ctx,
                                      backend->clipboard, required, &mem);
    }
    CMP_NULL_RETURN_IF_ERROR(rc);
    backend->clipboard = (char *)mem;
    backend->clipboard_capacity = required;
  }

  memcpy(backend->clipboard, utf8_text, required);
  backend->clipboard_length = length;
  return CMP_OK;
}

static int cmp_null_ws_get_clipboard_text(void *ws, char *buffer,
                                          cmp_usize buffer_size,
                                          cmp_usize *out_length) {
  struct CMPNullBackend *backend;
  cmp_usize required;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO,
                            "ws.get_clipboard_text");
  CMP_NULL_RETURN_IF_ERROR(rc);

  required = backend->clipboard_length + 1;
  if (buffer_size < required) {
    *out_length = backend->clipboard_length;
    return CMP_ERR_RANGE;
  }

  if (backend->clipboard_length == 0) {
    buffer[0] = '\0';
  } else {
    memcpy(buffer, backend->clipboard, required);
  }
  *out_length = backend->clipboard_length;
  return CMP_OK;
}

static int cmp_null_ws_poll_event(void *ws, CMPInputEvent *out_event,
                                  CMPBool *out_has_event) {
  struct CMPNullBackend *backend;
  int rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.poll_event");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_event, 0, sizeof(*out_event));
  *out_has_event = CMP_FALSE;
  return CMP_OK;
}

static int cmp_null_ws_pump_events(void *ws) {
  struct CMPNullBackend *backend;
  int rc;

  if (ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.pump_events");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_ws_get_time_ms(void *ws, cmp_u32 *out_time_ms) {
  struct CMPNullBackend *backend;
  int rc;

  if (ws == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)ws;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  CMP_NULL_RETURN_IF_ERROR(rc);

  backend->time_ms += 16u;
  *out_time_ms = backend->time_ms;
  return CMP_OK;
}

static const CMPWSVTable g_cmp_null_ws_vtable = {
    cmp_null_ws_init,
    cmp_null_ws_shutdown,
    cmp_null_ws_create_window,
    cmp_null_ws_destroy_window,
    cmp_null_ws_show_window,
    cmp_null_ws_hide_window,
    cmp_null_ws_set_window_title,
    cmp_null_ws_set_window_size,
    cmp_null_ws_get_window_size,
    cmp_null_ws_set_window_dpi_scale,
    cmp_null_ws_get_window_dpi_scale,
    cmp_null_ws_set_clipboard_text,
    cmp_null_ws_get_clipboard_text,
    cmp_null_ws_poll_event,
    cmp_null_ws_pump_events,
    cmp_null_ws_get_time_ms};

static int cmp_null_gfx_begin_frame(void *gfx, CMPHandle window, cmp_i32 width,
                                    cmp_i32 height, CMPScalar dpi_scale) {
  struct CMPNullBackend *backend;
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

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW, NULL);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_end_frame(void *gfx, CMPHandle window) {
  struct CMPNullBackend *backend;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_resolve(backend, window, CMP_NULL_TYPE_WINDOW, NULL);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.end_frame");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_clear(void *gfx, CMPColor color) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(color);

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.clear");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_draw_rect(void *gfx, const CMPRect *rect,
                                  CMPColor color, CMPScalar corner_radius) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(color);

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }
  if (corner_radius < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0,
                                  CMPScalar x1, CMPScalar y1, CMPColor color,
                                  CMPScalar thickness) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(x0);
  CMP_UNUSED(y0);
  CMP_UNUSED(x1);
  CMP_UNUSED(y1);
  CMP_UNUSED(color);

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_line");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_draw_path(void *gfx, const CMPPath *path,
                                  CMPColor color) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(color); /* GCOVR_EXCL_LINE */

  if (gfx == NULL || path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return CMP_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return CMP_ERR_STATE;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_path");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_push_clip(void *gfx, const CMPRect *rect) {
  struct CMPNullBackend *backend;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.push_clip");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_pop_clip(void *gfx) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;                         /* GCOVR_EXCL_LINE */

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_set_transform(void *gfx, const CMPMat3 *transform) {
  struct CMPNullBackend *backend;
  int rc; /* GCOVR_EXCL_LINE */

  if (gfx == NULL || transform == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.set_transform");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_create_texture(void *gfx, cmp_i32 width, cmp_i32 height,
                                       cmp_u32 format,
                                       const void *pixels, /* GCOVR_EXCL_LINE */
                                       cmp_usize size, CMPHandle *out_texture) {
  struct CMPNullBackend *backend;
  CMPNullTexture *texture; /* GCOVR_EXCL_LINE */
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

  backend = (struct CMPNullBackend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.create_texture");
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPNullTexture),
                                (void **)&texture);
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;

  rc = cmp_object_header_init(&texture->header, CMP_NULL_TYPE_TEXTURE, 0,
                              &g_cmp_null_texture_vtable);
  CMP_NULL_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, texture));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  CMP_NULL_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx,
                                  texture)); /* GCOVR_EXCL_LINE */

  *out_texture = texture->header.handle;
  return CMP_OK;
}

static int cmp_null_gfx_update_texture(void *gfx, CMPHandle texture, cmp_i32 x,
                                       cmp_i32 y, cmp_i32 width, cmp_i32 height,
                                       const void *pixels, cmp_usize size) {
  struct CMPNullBackend *backend;
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

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_resolve(backend, texture, CMP_NULL_TYPE_TEXTURE, NULL);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.update_texture");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_null_gfx_destroy_texture(void *gfx, CMPHandle texture) {
  struct CMPNullBackend *backend;
  CMPNullTexture *resolved;
  int rc;

  if (gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_resolve(backend, texture, CMP_NULL_TYPE_TEXTURE,
                                (void **)&resolved); /* GCOVR_EXCL_LINE */
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc =
      cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
  CMP_NULL_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_null_gfx_draw_texture(void *gfx, CMPHandle texture,
                                     const CMPRect *src, const CMPRect *dst,
                                     CMPScalar opacity) {
  struct CMPNullBackend *backend;
  int rc;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return CMP_ERR_RANGE;
  }

  backend = (struct CMPNullBackend *)gfx;
  rc = cmp_null_backend_resolve(backend, texture, CMP_NULL_TYPE_TEXTURE, NULL);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static const CMPGfxVTable g_cmp_null_gfx_vtable = {
    cmp_null_gfx_begin_frame,
    cmp_null_gfx_end_frame,
    cmp_null_gfx_clear,
    cmp_null_gfx_draw_rect,
    cmp_null_gfx_draw_line,
    cmp_null_gfx_draw_path,
    cmp_null_gfx_push_clip,
    cmp_null_gfx_pop_clip,
    cmp_null_gfx_set_transform,
    cmp_null_gfx_create_texture, /* GCOVR_EXCL_LINE */
    cmp_null_gfx_update_texture,
    cmp_null_gfx_destroy_texture,
    cmp_null_gfx_draw_texture};

static int cmp_null_text_create_font(void *text, const char *utf8_family,
                                     cmp_i32 size_px, cmp_i32 weight,
                                     CMPBool italic, CMPHandle *out_font) {
  struct CMPNullBackend *backend;
  CMPNullFont *font;
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

  backend = (struct CMPNullBackend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.create_font");
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPNullFont),
                                (void **)&font);
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? CMP_TRUE : CMP_FALSE;

  rc = cmp_object_header_init(&font->header, CMP_NULL_TYPE_FONT, 0,
                              &g_cmp_null_font_vtable); /* GCOVR_EXCL_LINE */
  CMP_NULL_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, font));

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  CMP_NULL_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, font));

  *out_font = font->header.handle;
  return CMP_OK;
}

static int cmp_null_text_destroy_font(void *text, CMPHandle font) {
  struct CMPNullBackend *backend;
  CMPNullFont *resolved;
  int rc;

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)text;
  rc = cmp_null_backend_resolve(backend, font, CMP_NULL_TYPE_FONT,
                                (void **)&resolved);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.destroy_font");
  CMP_NULL_RETURN_IF_ERROR(rc);

  return cmp_object_release(&resolved->header);
}

static int cmp_null_text_measure_text(void *text, CMPHandle font,
                                      const char *utf8, cmp_usize utf8_len,
                                      CMPScalar *out_width,
                                      CMPScalar *out_height,
                                      CMPScalar *out_baseline) {
  struct CMPNullBackend *backend;
  CMPNullFont *resolved;
  CMPScalar size;
  int rc; /* GCOVR_EXCL_LINE */

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)text;
  rc = cmp_null_backend_resolve(backend, font, CMP_NULL_TYPE_FONT,
                                (void **)&resolved); /* GCOVR_EXCL_LINE */
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.measure_text");
  CMP_NULL_RETURN_IF_ERROR(rc);

  size = (CMPScalar)resolved->size_px;
  *out_width = size * (CMPScalar)utf8_len * 0.5f;
  *out_height = size;
  *out_baseline = size * 0.8f;
  return CMP_OK;
}

static int cmp_null_text_draw_text(void *text, CMPHandle font, const char *utf8,
                                   cmp_usize utf8_len, CMPScalar x,
                                   CMPScalar y,      /* GCOVR_EXCL_LINE */
                                   CMPColor color) { /* GCOVR_EXCL_LINE */
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(x);
  CMP_UNUSED(y);
  CMP_UNUSED(color); /* GCOVR_EXCL_LINE */

  if (text == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)text;
  rc = cmp_null_backend_resolve(backend, font, CMP_NULL_TYPE_FONT, NULL);
  CMP_NULL_RETURN_IF_ERROR(rc);

  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.draw_text");
  CMP_NULL_RETURN_IF_ERROR(rc);

  return CMP_OK;
}

static const CMPTextVTable g_cmp_null_text_vtable = {
    cmp_null_text_create_font, cmp_null_text_destroy_font, /* GCOVR_EXCL_LINE */
    cmp_null_text_measure_text, cmp_null_text_draw_text};

static int cmp_null_io_read_file(void *io, const char *utf8_path, void *buffer,
                                 cmp_usize buffer_size, cmp_usize *out_read) {
  struct CMPNullBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)io;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int
cmp_null_io_read_file_alloc(void *io, const char *utf8_path,
                            const CMPAllocator *allocator, /* GCOVR_EXCL_LINE */
                            void **out_data, cmp_usize *out_size) {
  struct CMPNullBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)io;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_io_write_file(void *io, const char *utf8_path,
                                  const void *data, cmp_usize size,
                                  CMPBool overwrite) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;

  CMP_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)io;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.write_file");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_io_file_exists(void *io, const char *utf8_path,
                                   CMPBool *out_exists) {
  struct CMPNullBackend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)io;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.file_exists");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_exists = CMP_FALSE;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_io_delete_file(void *io, const char *utf8_path) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)io;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.delete_file");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_io_stat_file(void *io, const char *utf8_path,
                                 CMPFileInfo *out_info) {
  struct CMPNullBackend *backend;
  int rc; /* GCOVR_EXCL_LINE */

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)io;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.stat_file");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return CMP_ERR_UNSUPPORTED;
}

static const CMPIOVTable g_cmp_null_io_vtable = {
    cmp_null_io_read_file,   cmp_null_io_read_file_alloc,
    cmp_null_io_write_file,  cmp_null_io_file_exists,
    cmp_null_io_delete_file, cmp_null_io_stat_file};

static int cmp_null_sensors_is_available(void *sensors, cmp_u32 type,
                                         CMPBool *out_available) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;                         /* GCOVR_EXCL_LINE */

  CMP_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)sensors;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                            "sensors.is_available");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_available = CMP_FALSE;
  return CMP_OK;
}

static int cmp_null_sensors_start(void *sensors, cmp_u32 type) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)sensors;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.start");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_sensors_stop(void *sensors, cmp_u32 type) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;                         /* GCOVR_EXCL_LINE */

  CMP_UNUSED(type);

  if (sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)sensors;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.stop");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int
cmp_null_sensors_read(void *sensors, cmp_u32 type,
                      CMPSensorReading *out_reading,
                      CMPBool *out_has_reading) { /* GCOVR_EXCL_LINE */
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)sensors;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.read");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = CMP_FALSE;
  return CMP_OK;
}

static const CMPSensorsVTable g_cmp_null_sensors_vtable = {
    cmp_null_sensors_is_available, cmp_null_sensors_start,
    cmp_null_sensors_stop, cmp_null_sensors_read};

static int cmp_null_camera_open(void *camera, cmp_u32 camera_id) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(camera_id);

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)camera;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.open");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_camera_open_with_config(void *camera,
                                            const CMPCameraConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return cmp_null_camera_open(camera, config->camera_id);
}

static int cmp_null_camera_close(void *camera) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;                         /* GCOVR_EXCL_LINE */

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)camera;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.close");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_camera_start(void *camera) {
  struct CMPNullBackend *backend;
  int rc; /* GCOVR_EXCL_LINE */

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)camera;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.start");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_camera_stop(void *camera) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)camera;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.stop");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_camera_read_frame(void *camera, CMPCameraFrame *out_frame,
                                      CMPBool *out_has_frame) {
  struct CMPNullBackend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)camera;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.read_frame");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  return CMP_OK;
}

static const CMPCameraVTable g_cmp_null_camera_vtable = {
    cmp_null_camera_open,
    cmp_null_camera_open_with_config, /* GCOVR_EXCL_LINE */
    cmp_null_camera_close,
    cmp_null_camera_start,
    cmp_null_camera_stop,
    cmp_null_camera_read_frame};

static int cmp_null_image_decode(void *image,
                                 const CMPImageDecodeRequest *request,
                                 const CMPAllocator *allocator,
                                 CMPImageData *out_image) {
  struct CMPNullBackend *backend;
  int rc;

  if (image == NULL || request == NULL || allocator == NULL ||
      out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)image;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "image.decode");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_image, 0, sizeof(*out_image));
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_image_free(void *image, const CMPAllocator *allocator,
                               CMPImageData *image_data) {
  struct CMPNullBackend *backend;
  int rc;

  if (image == NULL || allocator == NULL || image_data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)image;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "image.free");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPImageVTable g_cmp_null_image_vtable = {cmp_null_image_decode,
                                                       cmp_null_image_free};

static int cmp_null_video_open(void *video,
                               const CMPVideoOpenRequest *request) {
  struct CMPNullBackend *backend;
  int rc;

  if (video == NULL || request == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)video;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "video.open");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_video_close(void *video) {
  struct CMPNullBackend *backend;
  int rc;

  if (video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)video;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "video.close");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_video_read_frame(void *video, CMPVideoFrame *out_frame,
                                     CMPBool *out_has_frame) {
  struct CMPNullBackend *backend;
  int rc;

  if (video == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)video;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "video.read_frame");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  return CMP_OK;
}

static const CMPVideoVTable g_cmp_null_video_vtable = {
    cmp_null_video_open, cmp_null_video_close, cmp_null_video_read_frame};

static int cmp_null_audio_decode(void *audio,
                                 const CMPAudioDecodeRequest *request,
                                 const CMPAllocator *allocator,
                                 CMPAudioData *out_audio) {
  struct CMPNullBackend *backend;
  int rc;

  if (audio == NULL || request == NULL || allocator == NULL ||
      out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)audio;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "audio.decode");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_audio, 0, sizeof(*out_audio));
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_audio_free(void *audio, const CMPAllocator *allocator,
                               CMPAudioData *audio_data) {
  struct CMPNullBackend *backend;
  int rc;

  if (audio == NULL || allocator == NULL || audio_data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)audio;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "audio.free");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPAudioVTable g_cmp_null_audio_vtable = {cmp_null_audio_decode,
                                                       cmp_null_audio_free};

static int cmp_null_network_request(void *net, const CMPNetworkRequest *request,
                                    const CMPAllocator *allocator,
                                    CMPNetworkResponse *out_response) {
  struct CMPNullBackend *backend;
  int rc;

  if (net == NULL || request == NULL || allocator == NULL ||
      out_response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)net;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.request");
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(out_response, 0, sizeof(*out_response));
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_network_free_response(void *net,
                                          const CMPAllocator *allocator,
                                          CMPNetworkResponse *response) {
  struct CMPNullBackend *backend;
  int rc;

  if (net == NULL || allocator == NULL || response == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)net;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG,
                            "network.free_response");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static const CMPNetworkVTable g_cmp_null_network_vtable = {
    cmp_null_network_request, cmp_null_network_free_response};

static int cmp_null_tasks_thread_create(void *tasks, CMPThreadFn entry,
                                        void *user, CMPHandle *out_thread) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(entry);
  CMP_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc =
      cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_create");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_tasks_thread_join(void *tasks, CMPHandle thread) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(thread); /* GCOVR_EXCL_LINE */

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_join");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_tasks_mutex_create(void *tasks, CMPHandle *out_mutex) {
  struct CMPNullBackend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_tasks_mutex_destroy(void *tasks, CMPHandle mutex) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc =
      cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_tasks_mutex_lock(void *tasks, CMPHandle mutex) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_tasks_mutex_unlock(void *tasks, CMPHandle mutex) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(mutex);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  CMP_NULL_RETURN_IF_ERROR(rc);
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_null_tasks_sleep_ms(void *tasks, cmp_u32 ms) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(ms);

  if (tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  CMP_NULL_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return CMP_OK;
}

static int cmp_null_tasks_post(void *tasks, CMPTaskFn fn, void *user) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;                         /* GCOVR_EXCL_LINE */

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post");
  CMP_NULL_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static int cmp_null_tasks_post_delayed(void *tasks, CMPTaskFn fn, void *user,
                                       cmp_u32 delay_ms) {
  struct CMPNullBackend *backend;
  int rc;

  CMP_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)tasks;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  CMP_NULL_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return CMP_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static const CMPTasksVTable g_cmp_null_tasks_vtable =
    {/* GCOVR_EXCL_LINE */
     cmp_null_tasks_thread_create, cmp_null_tasks_thread_join,
     cmp_null_tasks_mutex_create,  cmp_null_tasks_mutex_destroy,
     cmp_null_tasks_mutex_lock,    cmp_null_tasks_mutex_unlock,
     cmp_null_tasks_sleep_ms,      cmp_null_tasks_post, /* GCOVR_EXCL_LINE */
     cmp_null_tasks_post_delayed};                      /* GCOVR_EXCL_LINE */

static int cmp_null_env_get_io(void *env, CMPIO *out_io) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;                         /* GCOVR_EXCL_LINE */

  if (env == NULL || out_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_io");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return CMP_OK;
}

static int cmp_null_env_get_sensors(void *env, CMPSensors *out_sensors) {
  struct CMPNullBackend *backend;
  int rc; /* GCOVR_EXCL_LINE */

  if (env == NULL || out_sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_sensors");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return CMP_OK;
}

static int cmp_null_env_get_camera(void *env, CMPCamera *out_camera) {
  struct CMPNullBackend *backend; /* GCOVR_EXCL_LINE */
  int rc;

  if (env == NULL || out_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_camera");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return CMP_OK;
}

static int cmp_null_env_get_image(void *env, CMPImage *out_image) {
  struct CMPNullBackend *backend;
  int rc;

  if (env == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_image");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_image = backend->image;
  return CMP_OK;
}

static int cmp_null_env_get_video(void *env, CMPVideo *out_video) {
  struct CMPNullBackend *backend;
  int rc;

  if (env == NULL || out_video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_video");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_video = backend->video;
  return CMP_OK;
}

static int cmp_null_env_get_audio(void *env, CMPAudio *out_audio) {
  struct CMPNullBackend *backend;
  int rc;

  if (env == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_audio");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_audio = backend->audio;
  return CMP_OK;
}

static int cmp_null_env_get_network(void *env, CMPNetwork *out_network) {
  struct CMPNullBackend *backend;
  int rc; /* GCOVR_EXCL_LINE */

  if (env == NULL || out_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_network");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return CMP_OK;
}

static int cmp_null_env_get_tasks(void *env, CMPTasks *out_tasks) {
  struct CMPNullBackend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_tasks");
  CMP_NULL_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return CMP_OK;
}

static int cmp_null_env_get_time_ms(void *env, cmp_u32 *out_time_ms) {
  struct CMPNullBackend *backend;
  int rc;

  if (env == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPNullBackend *)env;
  rc = cmp_null_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_time_ms");
  CMP_NULL_RETURN_IF_ERROR(rc);

  backend->time_ms += 16u;
  *out_time_ms = backend->time_ms;
  return CMP_OK;
}

static const CMPEnvVTable g_cmp_null_env_vtable = {
    cmp_null_env_get_io,      cmp_null_env_get_sensors,
    cmp_null_env_get_camera,  cmp_null_env_get_image,
    cmp_null_env_get_video,   cmp_null_env_get_audio,
    cmp_null_env_get_network, cmp_null_env_get_tasks,
    cmp_null_env_get_time_ms};

int CMP_CALL cmp_null_backend_config_init(CMPNullBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_NULL_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_null_backend_create(const CMPNullBackendConfig *config,
                                     CMPNullBackend **out_backend) {
  CMPNullBackendConfig local_config; /* GCOVR_EXCL_LINE */
  CMPAllocator allocator;            /* GCOVR_EXCL_LINE */
  struct CMPNullBackend *backend;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_null_backend_config_init(&local_config);
    CMP_NULL_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  if (config->handle_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    CMP_NULL_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc =
      allocator.alloc(allocator.ctx, sizeof(CMPNullBackend), (void **)&backend);
  CMP_NULL_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->log_enabled = config->enable_logging ? CMP_TRUE : CMP_FALSE;
  backend->inline_tasks = config->inline_tasks ? CMP_TRUE : CMP_FALSE;
  backend->clipboard_limit = config->clipboard_limit;

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

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_cmp_null_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_cmp_null_gfx_vtable;
  backend->gfx.text_vtable = &g_cmp_null_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_cmp_null_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_cmp_null_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_cmp_null_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_cmp_null_camera_vtable;
  backend->image.ctx = backend;
  backend->image.vtable = &g_cmp_null_image_vtable;
  backend->video.ctx = backend;
  backend->video.vtable = &g_cmp_null_video_vtable;
  backend->audio.ctx = backend;
  backend->audio.vtable = &g_cmp_null_audio_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_cmp_null_network_vtable;
  backend->tasks.ctx = backend;
  backend->tasks.vtable = &g_cmp_null_tasks_vtable;
  backend->initialized = CMP_TRUE;

  *out_backend = backend;
  return CMP_OK;
}

int CMP_CALL cmp_null_backend_destroy(CMPNullBackend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }

  rc = cmp_handle_system_default_destroy(&backend->handles);
  CMP_NULL_RETURN_IF_ERROR(rc);

  first_error = CMP_OK;

  if (backend->clipboard != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, backend->clipboard);
    if (rc != CMP_OK && first_error == CMP_OK) {
      first_error = rc;
    }
    backend->clipboard = NULL;
    backend->clipboard_capacity = 0;
    backend->clipboard_length = 0;
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

int CMP_CALL cmp_null_backend_get_ws(CMPNullBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE; /* GCOVR_EXCL_LINE */
  }
  *out_ws = backend->ws;
  return CMP_OK;
}

int CMP_CALL cmp_null_backend_get_gfx(CMPNullBackend *backend,
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

int CMP_CALL cmp_null_backend_get_env(CMPNullBackend *backend,
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

#ifdef CMP_TESTING
int CMP_CALL cmp_null_backend_test_set_initialized(CMPNullBackend *backend,
                                                   CMPBool initialized) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend->initialized = initialized ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_null_backend_test_object_retain(CMPObjectHeader *obj) {
  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return cmp_null_object_retain(obj);
}

int CMP_CALL cmp_null_backend_test_object_release(CMPObjectHeader *obj) {
  if (obj == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return cmp_null_object_release(obj);
}

int CMP_CALL cmp_null_backend_test_object_get_type_id(CMPObjectHeader *obj,
                                                      cmp_u32 *out_type_id) {
  if (obj == NULL || out_type_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  return cmp_null_object_get_type_id(obj, out_type_id);
}

int CMP_CALL cmp_null_backend_test_set_fail_after(cmp_usize call_index) {
  g_cmp_null_test_fail_after = call_index;
  g_cmp_null_test_call_count = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_null_backend_test_clear_fail_after(void) {
  g_cmp_null_test_fail_after = 0u;
  g_cmp_null_test_call_count = 0u;
  return CMP_OK;
}

int CMP_CALL cmp_null_backend_test_maybe_fail_null(void) {
  cmp_null_backend_test_maybe_fail(NULL);
  return CMP_OK;
}
#endif
