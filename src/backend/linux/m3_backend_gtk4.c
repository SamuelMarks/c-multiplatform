#include "m3/m3_backend_gtk4.h"

#include "m3/m3_log.h"
#include "m3/m3_object.h"

#include <limits.h>
#include <string.h>
#if defined(M3_LIBCURL_AVAILABLE)
#include <curl/curl.h>
#endif
#if defined(M3_GTK4_AVAILABLE)
#include <cairo.h>
#include <gtk/gtk.h>
#include <pango/pangocairo.h>
#endif
#if defined(M3_GTK4_AVAILABLE) && defined(__linux__)
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define M3_GTK4_V4L2_AVAILABLE 1
#else
#define M3_GTK4_V4L2_AVAILABLE 0
#endif

#define M3_GTK4_RETURN_IF_ERROR(rc)                                            \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_GTK4_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                           \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      cleanup;                                                                 \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_GTK4_DEFAULT_HANDLE_CAPACITY 64
#define M3_GTK4_EVENT_CAPACITY 256u
#define M3_GTK4_CLIP_STACK_CAPACITY 32u
#define M3_GTK4_PI 3.14159265358979323846
#define M3_GTK4_PI_2 (M3_GTK4_PI * 0.5)
#define M3_GTK4_CMD_CAPACITY_INITIAL 64u

#define M3_GTK4_CMD_CLEAR 1u
#define M3_GTK4_CMD_RECT 2u
#define M3_GTK4_CMD_LINE 3u
#define M3_GTK4_CMD_PUSH_CLIP 4u
#define M3_GTK4_CMD_POP_CLIP 5u
#define M3_GTK4_CMD_TEXTURE 6u
#define M3_GTK4_CMD_TEXT 7u
#define M3_GTK4_CMD_PATH 8u

#define M3_GTK4_CAMERA_DEFAULT_WIDTH 640u
#define M3_GTK4_CAMERA_DEFAULT_HEIGHT 480u

static int m3_gtk4_backend_validate_config(const M3GTK4BackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
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
m3_gtk4_backend_test_validate_config(const M3GTK4BackendConfig *config) {
  return m3_gtk4_backend_validate_config(config);
}
#endif

int M3_CALL m3_gtk4_backend_is_available(M3Bool *out_available) {
  if (out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#if defined(M3_GTK4_AVAILABLE)
  *out_available = M3_TRUE;
#else
  *out_available = M3_FALSE;
#endif
  return M3_OK;
}

int M3_CALL m3_gtk4_backend_config_init(M3GTK4BackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = M3_GTK4_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (m3_usize) ~(m3_usize)0;
  config->enable_logging = M3_TRUE;
  config->inline_tasks = M3_TRUE;
  config->enable_gdk_texture = M3_TRUE;
  return M3_OK;
}

#if defined(M3_GTK4_AVAILABLE)

#define M3_GTK4_TYPE_WINDOW 1
#define M3_GTK4_TYPE_TEXTURE 2
#define M3_GTK4_TYPE_FONT 3

typedef struct M3GTK4Cmd M3GTK4Cmd;

typedef struct M3GTK4Window {
  M3ObjectHeader header;
  struct M3GTK4Backend *backend;
  GtkWindow *window;
  GtkWidget *area;
  cairo_surface_t *surface;
  m3_i32 surface_width;
  m3_i32 surface_height;
  m3_i32 width;
  m3_i32 height;
  m3_i32 frame_width;
  m3_i32 frame_height;
  M3Scalar dpi_scale;
  M3Scalar dpi_scale_override;
  M3Bool has_scale_override;
  m3_u32 flags;
  M3Bool visible;
  M3Bool use_gsk;
  M3Bool has_gsk_frame;
  M3GTK4Cmd *cmds;
  m3_usize cmd_count;
  m3_usize cmd_capacity;
} M3GTK4Window;

typedef struct M3GTK4Texture {
  M3ObjectHeader header;
  struct M3GTK4Backend *backend;
  m3_i32 width;
  m3_i32 height;
  m3_u32 format;
  m3_u32 bytes_per_pixel;
  m3_i32 stride;
  m3_usize size;
  unsigned char *pixels;
  cairo_surface_t *surface;
  GdkTexture *gdk_texture;
} M3GTK4Texture;

typedef struct M3GTK4Font {
  M3ObjectHeader header;
  struct M3GTK4Backend *backend;
  m3_i32 size_px;
  m3_i32 weight;
  M3Bool italic;
  PangoFontDescription *desc;
} M3GTK4Font;

typedef struct M3GTK4CameraState {
  int fd;
  m3_u32 camera_id;
  m3_u32 width;
  m3_u32 height;
  m3_u32 format;
  m3_usize frame_capacity;
  m3_usize frame_size;
  unsigned char *frame;
  M3Bool opened;
  M3Bool streaming;
} M3GTK4CameraState;

typedef struct M3GTK4CmdRect {
  M3Rect rect;
  M3Color color;
  M3Scalar radius;
} M3GTK4CmdRect;

typedef struct M3GTK4CmdLine {
  M3Scalar x0;
  M3Scalar y0;
  M3Scalar x1;
  M3Scalar y1;
  M3Scalar thickness;
  M3Color color;
} M3GTK4CmdLine;

typedef struct M3GTK4CmdClip {
  M3Rect rect;
} M3GTK4CmdClip;

typedef struct M3GTK4CmdTexture {
  M3Handle texture;
  M3Rect src;
  M3Rect dst;
  M3Scalar opacity;
} M3GTK4CmdTexture;

typedef struct M3GTK4CmdText {
  M3Handle font;
  char *utf8;
  m3_usize len;
  M3Scalar x;
  M3Scalar y;
  M3Color color;
} M3GTK4CmdText;

typedef struct M3GTK4CmdPath {
  M3PathCmd *commands;
  m3_usize count;
  M3Rect bounds;
  M3Color color;
} M3GTK4CmdPath;

typedef struct M3GTK4CmdClear {
  M3Rect rect;
  M3Color color;
} M3GTK4CmdClear;

typedef struct M3GTK4Cmd {
  m3_u32 type;
  union {
    M3GTK4CmdRect rect;
    M3GTK4CmdLine line;
    M3GTK4CmdClip clip;
    M3GTK4CmdTexture texture;
    M3GTK4CmdText text;
    M3GTK4CmdPath path;
    M3GTK4CmdClear clear;
  } data;
};

struct M3GTK4Backend {
  M3Allocator allocator;
  M3HandleSystem handles;
  M3WS ws;
  M3Gfx gfx;
  M3Env env;
  M3IO io;
  M3Sensors sensors;
  M3Camera camera;
  M3GTK4CameraState camera_state;
  M3Network network;
  M3Tasks tasks;
  M3Bool initialized;
  M3Bool log_enabled;
  M3Bool log_owner;
  M3Bool inline_tasks;
  M3Bool use_gsk_default;
  M3Bool use_gdk_texture;
  M3Bool in_frame;
  M3Bool frame_use_gsk;
  m3_u32 time_ms;
  char *clipboard;
  m3_usize clipboard_capacity;
  m3_usize clipboard_length;
  m3_usize clipboard_limit;
  M3GTK4Window *active_window;
  cairo_t *frame_cr;
  M3Mat3 transform;
  M3Bool has_transform;
  M3Mat3 gsk_transform;
  M3Bool gsk_transform_simple;
  M3Rect clip_stack[M3_GTK4_CLIP_STACK_CAPACITY];
  m3_usize clip_depth;
  M3InputEvent event_queue[M3_GTK4_EVENT_CAPACITY];
  m3_usize event_head;
  m3_usize event_count;
};

static m3_u32 g_m3_gtk4_refcount = 0u;

static int m3_gtk4_backend_log(struct M3GTK4Backend *backend, M3LogLevel level,
                               const char *message) {
  if (!backend->log_enabled) {
    return M3_OK;
  }
  return m3_log_write(level, "m3.gtk4", message);
}

static int m3_gtk4_global_init(struct M3GTK4Backend *backend) {
  GdkDisplay *display;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (g_m3_gtk4_refcount == 0u) {
    gtk_init();
    display = gdk_display_get_default();
    if (display == NULL) {
      return M3_ERR_UNSUPPORTED;
    }
  }

  g_m3_gtk4_refcount += 1u;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "gtk4.init");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_gtk4_global_shutdown(struct M3GTK4Backend *backend) {
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (g_m3_gtk4_refcount == 0u) {
    return M3_ERR_STATE;
  }

  g_m3_gtk4_refcount -= 1u;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "gtk4.shutdown");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static m3_u32 m3_gtk4_get_time_ms(void) {
  gint64 us;

  us = g_get_monotonic_time();
  if (us < 0) {
    return 0u;
  }
  return (m3_u32)(us / 1000);
}

static m3_u32 m3_gtk4_modifiers_from_state(GdkModifierType state) {
  m3_u32 mods;

  mods = 0u;
  if ((state & GDK_SHIFT_MASK) != 0) {
    mods |= M3_MOD_SHIFT;
  }
  if ((state & GDK_CONTROL_MASK) != 0) {
    mods |= M3_MOD_CTRL;
  }
  if ((state & GDK_ALT_MASK) != 0) {
    mods |= M3_MOD_ALT;
  }
  if ((state & GDK_META_MASK) != 0) {
    mods |= M3_MOD_META;
  }
  if ((state & GDK_LOCK_MASK) != 0) {
    mods |= M3_MOD_CAPS;
  }
  if ((state & GDK_MOD2_MASK) != 0) {
    mods |= M3_MOD_NUM;
  }
  return mods;
}

static m3_i32 m3_gtk4_buttons_from_state(GdkModifierType state) {
  m3_i32 buttons;

  buttons = 0;
  if ((state & GDK_BUTTON1_MASK) != 0) {
    buttons |= 1;
  }
  if ((state & GDK_BUTTON2_MASK) != 0) {
    buttons |= 1 << 1;
  }
  if ((state & GDK_BUTTON3_MASK) != 0) {
    buttons |= 1 << 2;
  }
  if ((state & GDK_BUTTON4_MASK) != 0) {
    buttons |= 1 << 3;
  }
  if ((state & GDK_BUTTON5_MASK) != 0) {
    buttons |= 1 << 4;
  }
  return buttons;
}

static int m3_gtk4_queue_event(struct M3GTK4Backend *backend,
                               const M3InputEvent *event) {
  m3_usize tail;
  int rc;

  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->event_count >= M3_GTK4_EVENT_CAPACITY) {
    rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_WARN, "event.queue_full");
    M3_GTK4_RETURN_IF_ERROR(rc);
    return M3_ERR_BUSY;
  }

  tail = (backend->event_head + backend->event_count) % M3_GTK4_EVENT_CAPACITY;
  backend->event_queue[tail] = *event;
  backend->event_count += 1;
  return M3_OK;
}

static M3Scalar m3_gtk4_abs_scalar(M3Scalar value) {
  return (value < 0.0f) ? -value : value;
}

static M3Bool m3_gtk4_transform_is_simple(const M3Mat3 *transform) {
  if (transform == NULL) {
    return M3_FALSE;
  }
  if (transform->m[1] != 0.0f || transform->m[3] != 0.0f) {
    return M3_FALSE;
  }
  if (transform->m[2] != 0.0f || transform->m[5] != 0.0f) {
    return M3_FALSE;
  }
  if (transform->m[8] != 1.0f) {
    return M3_FALSE;
  }
  return M3_TRUE;
}

static int m3_gtk4_transform_point_simple(const M3Mat3 *transform, M3Scalar x,
                                          M3Scalar y, M3Scalar *out_x,
                                          M3Scalar *out_y) {
  if (transform == NULL || out_x == NULL || out_y == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_x = transform->m[0] * x + transform->m[6];
  *out_y = transform->m[4] * y + transform->m[7];
  return M3_OK;
}

static int m3_gtk4_transform_rect_simple(const M3Mat3 *transform,
                                         const M3Rect *rect, M3Rect *out_rect) {
  M3Scalar x0;
  M3Scalar y0;
  M3Scalar x1;
  M3Scalar y1;
  M3Scalar min_x;
  M3Scalar min_y;
  M3Scalar max_x;
  M3Scalar max_y;
  int rc;

  if (transform == NULL || rect == NULL || out_rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = m3_gtk4_transform_point_simple(transform, rect->x, rect->y, &x0, &y0);
  M3_GTK4_RETURN_IF_ERROR(rc);
  rc = m3_gtk4_transform_point_simple(transform, rect->x + rect->width,
                                      rect->y + rect->height, &x1, &y1);
  M3_GTK4_RETURN_IF_ERROR(rc);

  min_x = (x0 < x1) ? x0 : x1;
  min_y = (y0 < y1) ? y0 : y1;
  max_x = (x0 > x1) ? x0 : x1;
  max_y = (y0 > y1) ? y0 : y1;

  out_rect->x = min_x;
  out_rect->y = min_y;
  out_rect->width = max_x - min_x;
  out_rect->height = max_y - min_y;
  return M3_OK;
}

static int m3_gtk4_cmd_list_reset(M3GTK4Window *window) {
  struct M3GTK4Backend *backend;
  m3_usize i;
  int rc;
  int first_error;

  if (window == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = window->backend;
  if (backend == NULL) {
    return M3_ERR_STATE;
  }

  first_error = M3_OK;
  for (i = 0; i < window->cmd_count; ++i) {
    if (window->cmds[i].type == M3_GTK4_CMD_TEXT) {
      if (window->cmds[i].data.text.utf8 != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx,
                                     window->cmds[i].data.text.utf8);
        if (rc != M3_OK && first_error == M3_OK) {
          first_error = rc;
        }
        window->cmds[i].data.text.utf8 = NULL;
        window->cmds[i].data.text.len = 0;
      }
    } else if (window->cmds[i].type == M3_GTK4_CMD_PATH) {
      if (window->cmds[i].data.path.commands != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx,
                                     window->cmds[i].data.path.commands);
        if (rc != M3_OK && first_error == M3_OK) {
          first_error = rc;
        }
        window->cmds[i].data.path.commands = NULL;
        window->cmds[i].data.path.count = 0;
      }
    }
  }

  window->cmd_count = 0;
  return first_error;
}

static int m3_gtk4_cmd_list_push(M3GTK4Window *window, const M3GTK4Cmd *cmd) {
  struct M3GTK4Backend *backend;
  m3_usize capacity;
  m3_usize new_capacity;
  m3_usize max_value;
  void *mem;
  int rc;

  if (window == NULL || cmd == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = window->backend;
  if (backend == NULL) {
    return M3_ERR_STATE;
  }

  capacity = window->cmd_capacity;
  if (window->cmd_count >= capacity) {
    new_capacity =
        (capacity == 0) ? M3_GTK4_CMD_CAPACITY_INITIAL : capacity * 2u;
    max_value = (m3_usize) ~(m3_usize)0;
    if (new_capacity > max_value / sizeof(M3GTK4Cmd)) {
      return M3_ERR_OVERFLOW;
    }

    if (window->cmds == NULL) {
      rc = backend->allocator.alloc(backend->allocator.ctx,
                                    new_capacity * sizeof(M3GTK4Cmd), &mem);
    } else {
      rc = backend->allocator.realloc(backend->allocator.ctx, window->cmds,
                                      new_capacity * sizeof(M3GTK4Cmd), &mem);
    }
    M3_GTK4_RETURN_IF_ERROR(rc);
    window->cmds = (M3GTK4Cmd *)mem;
    window->cmd_capacity = new_capacity;
  }

  window->cmds[window->cmd_count] = *cmd;
  window->cmd_count += 1u;
  return M3_OK;
}

static int m3_gtk4_ensure_surface(M3GTK4Window *window, m3_i32 width,
                                  m3_i32 height) {
  cairo_surface_t *surface;
  cairo_status_t status;

  if (window == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }

  if (window->surface != NULL && window->surface_width == width &&
      window->surface_height == height) {
    return M3_OK;
  }

  if (window->surface != NULL) {
    cairo_surface_destroy(window->surface);
    window->surface = NULL;
  }

  surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)width, (int)height);
  status = cairo_surface_status(surface);
  if (status != CAIRO_STATUS_SUCCESS) {
    if (surface != NULL) {
      cairo_surface_destroy(surface);
    }
    return M3_ERR_OUT_OF_MEMORY;
  }

  window->surface = surface;
  window->surface_width = width;
  window->surface_height = height;
  return M3_OK;
}

static int m3_gtk4_apply_transform(struct M3GTK4Backend *backend) {
  cairo_matrix_t matrix;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }

  matrix.xx = backend->transform.m[0];
  matrix.yx = backend->transform.m[1];
  matrix.xy = backend->transform.m[3];
  matrix.yy = backend->transform.m[4];
  matrix.x0 = backend->transform.m[6];
  matrix.y0 = backend->transform.m[7];
  cairo_set_matrix(backend->frame_cr, &matrix);
  return M3_OK;
}

static void m3_gtk4_queue_window_resize(M3GTK4Window *window, m3_i32 width,
                                        m3_i32 height) {
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  int rc;

  if (window == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  event.type = M3_INPUT_WINDOW_RESIZE;
  event.modifiers = 0u;
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.window.width = width;
  event.data.window.height = height;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
}

typedef struct M3GTK4ClipboardRead {
  gboolean done;
  char *text;
} M3GTK4ClipboardRead;

static void m3_gtk4_clipboard_read_cb(GObject *source, GAsyncResult *result,
                                      gpointer user_data) {
  M3GTK4ClipboardRead *state;

  state = (M3GTK4ClipboardRead *)user_data;
  if (state == NULL) {
    return;
  }

  state->text =
      gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), result, NULL);
  state->done = TRUE;
}

static void m3_gtk4_on_snapshot(GtkWidget *widget, GtkSnapshot *snapshot,
                                gpointer user_data) {
  M3GTK4Window *window;
  cairo_t *cr;
  graphene_rect_t rect;

  M3_UNUSED(widget);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  if (window->use_gsk && window->has_gsk_frame && window->cmd_count > 0) {
    m3_gtk4_snapshot_cmds(window, snapshot);
    return;
  }

  if (window->surface == NULL) {
    return;
  }

  rect.origin.x = 0.0f;
  rect.origin.y = 0.0f;
  rect.size.width = (float)window->surface_width;
  rect.size.height = (float)window->surface_height;

  cr = gtk_snapshot_append_cairo(snapshot, &rect);
  if (cr == NULL) {
    return;
  }

  cairo_set_source_surface(cr, window->surface, 0.0, 0.0);
  cairo_paint(cr);
  cairo_destroy(cr);
}

static void m3_gtk4_on_size_allocate(GtkWidget *widget, int width, int height,
                                     int baseline, gpointer user_data) {
  M3GTK4Window *window;

  M3_UNUSED(widget);
  M3_UNUSED(baseline);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  if (window->width != (m3_i32)width || window->height != (m3_i32)height) {
    window->width = (m3_i32)width;
    window->height = (m3_i32)height;
    m3_gtk4_queue_window_resize(window, window->width, window->height);
  }
}

static gboolean m3_gtk4_on_close_request(GtkWindow *gtk_window,
                                         gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  int rc;

  M3_UNUSED(gtk_window);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return FALSE;
  }

  backend = window->backend;
  if (backend == NULL) {
    return FALSE;
  }

  event.type = M3_INPUT_WINDOW_CLOSE;
  event.modifiers = 0u;
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  memset(&event.data, 0, sizeof(event.data));

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);

  return TRUE;
}

static void m3_gtk4_on_focus_enter(GtkEventControllerFocus *controller,
                                   gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  int rc;

  M3_UNUSED(controller);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  event.type = M3_INPUT_WINDOW_FOCUS;
  event.modifiers = 0u;
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  memset(&event.data, 0, sizeof(event.data));

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
}

static void m3_gtk4_on_focus_leave(GtkEventControllerFocus *controller,
                                   gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  int rc;

  M3_UNUSED(controller);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  event.type = M3_INPUT_WINDOW_BLUR;
  event.modifiers = 0u;
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  memset(&event.data, 0, sizeof(event.data));

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
}

static void m3_gtk4_on_motion(GtkEventControllerMotion *controller, double x,
                              double y, gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  GdkModifierType state;
  int rc;

  M3_UNUSED(controller);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  state = gtk_event_controller_get_current_event_state(
      GTK_EVENT_CONTROLLER(controller));

  event.type = M3_INPUT_POINTER_MOVE;
  event.modifiers = m3_gtk4_modifiers_from_state(state);
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.pointer.pointer_id = 0;
  event.data.pointer.x = (m3_i32)x;
  event.data.pointer.y = (m3_i32)y;
  event.data.pointer.buttons = m3_gtk4_buttons_from_state(state);
  event.data.pointer.scroll_x = 0;
  event.data.pointer.scroll_y = 0;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
}

static void m3_gtk4_on_click_pressed(GtkGestureClick *gesture, int n_press,
                                     double x, double y, gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  GdkModifierType state;
  int button;
  m3_i32 button_mask;
  int rc;

  M3_UNUSED(n_press);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  state = gtk_event_controller_get_current_event_state(
      GTK_EVENT_CONTROLLER(gesture));
  button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
  button_mask = 0;
  if (button > 0 && button <= 31) {
    button_mask = (m3_i32)(1 << (button - 1));
  }

  event.type = M3_INPUT_POINTER_DOWN;
  event.modifiers = m3_gtk4_modifiers_from_state(state);
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.pointer.pointer_id = 0;
  event.data.pointer.x = (m3_i32)x;
  event.data.pointer.y = (m3_i32)y;
  event.data.pointer.buttons = m3_gtk4_buttons_from_state(state) | button_mask;
  event.data.pointer.scroll_x = 0;
  event.data.pointer.scroll_y = 0;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
}

static void m3_gtk4_on_click_released(GtkGestureClick *gesture, int n_press,
                                      double x, double y, gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  GdkModifierType state;
  int button;
  m3_i32 button_mask;
  int rc;

  M3_UNUSED(n_press);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  state = gtk_event_controller_get_current_event_state(
      GTK_EVENT_CONTROLLER(gesture));
  button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
  button_mask = 0;
  if (button > 0 && button <= 31) {
    button_mask = (m3_i32)(1 << (button - 1));
  }

  event.type = M3_INPUT_POINTER_UP;
  event.modifiers = m3_gtk4_modifiers_from_state(state);
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.pointer.pointer_id = 0;
  event.data.pointer.x = (m3_i32)x;
  event.data.pointer.y = (m3_i32)y;
  event.data.pointer.buttons = m3_gtk4_buttons_from_state(state) & ~button_mask;
  event.data.pointer.scroll_x = 0;
  event.data.pointer.scroll_y = 0;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
}

static gboolean m3_gtk4_on_scroll(GtkEventControllerScroll *controller,
                                  double dx, double dy, gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  GdkModifierType state;
  int rc;

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return FALSE;
  }

  backend = window->backend;
  if (backend == NULL) {
    return FALSE;
  }

  state = gtk_event_controller_get_current_event_state(
      GTK_EVENT_CONTROLLER(controller));

  event.type = M3_INPUT_POINTER_SCROLL;
  event.modifiers = m3_gtk4_modifiers_from_state(state);
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.pointer.pointer_id = 0;
  event.data.pointer.x = 0;
  event.data.pointer.y = 0;
  event.data.pointer.buttons = m3_gtk4_buttons_from_state(state);
  event.data.pointer.scroll_x = (m3_i32)dx;
  event.data.pointer.scroll_y = (m3_i32)dy;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
  return TRUE;
}

static gboolean m3_gtk4_on_key_pressed(GtkEventControllerKey *controller,
                                       guint keyval, guint keycode,
                                       GdkModifierType state,
                                       gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  gunichar ch;
  char utf8[8];
  int utf8_len;
  int rc;

  M3_UNUSED(controller);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return FALSE;
  }

  backend = window->backend;
  if (backend == NULL) {
    return FALSE;
  }

  event.type = M3_INPUT_KEY_DOWN;
  event.modifiers = m3_gtk4_modifiers_from_state(state);
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.key.key_code = (m3_u32)keyval;
  event.data.key.native_code = (m3_u32)keycode;
  event.data.key.is_repeat = M3_FALSE;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);

  ch = gdk_keyval_to_unicode(keyval);
  if (ch != 0) {
    utf8_len = g_unichar_to_utf8(ch, utf8);
    if (utf8_len > 0 && utf8_len < (int)sizeof(utf8)) {
      utf8[utf8_len] = '\0';
      event.type = M3_INPUT_TEXT;
      event.modifiers = m3_gtk4_modifiers_from_state(state);
      event.reserved = 0u;
      event.time_ms = m3_gtk4_get_time_ms();
      event.window = window->header.handle;
      memset(&event.data, 0, sizeof(event.data));
      memcpy(event.data.text.utf8, utf8, (size_t)(utf8_len + 1));
      event.data.text.length = (m3_u32)utf8_len;
      rc = m3_gtk4_queue_event(backend, &event);
      M3_UNUSED(rc);
    }
  }

  return TRUE;
}

static gboolean m3_gtk4_on_key_released(GtkEventControllerKey *controller,
                                        guint keyval, guint keycode,
                                        GdkModifierType state,
                                        gpointer user_data) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  M3InputEvent event;
  int rc;

  M3_UNUSED(controller);

  window = (M3GTK4Window *)user_data;
  if (window == NULL) {
    return FALSE;
  }

  backend = window->backend;
  if (backend == NULL) {
    return FALSE;
  }

  event.type = M3_INPUT_KEY_UP;
  event.modifiers = m3_gtk4_modifiers_from_state(state);
  event.reserved = 0u;
  event.time_ms = m3_gtk4_get_time_ms();
  event.window = window->header.handle;
  event.data.key.key_code = (m3_u32)keyval;
  event.data.key.native_code = (m3_u32)keycode;
  event.data.key.is_repeat = M3_FALSE;

  rc = m3_gtk4_queue_event(backend, &event);
  M3_UNUSED(rc);
  return TRUE;
}

static int m3_gtk4_backend_resolve(struct M3GTK4Backend *backend,
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
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_object_get_type_id((const M3ObjectHeader *)resolved, &actual_type);
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (actual_type != type_id) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (out_obj != NULL) {
    *out_obj = resolved;
  }
  return M3_OK;
}

static int m3_gtk4_object_retain(void *obj) {
  return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_gtk4_object_release(void *obj) {
  return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_gtk4_object_get_type_id(void *obj, m3_u32 *out_type_id) {
  return m3_object_get_type_id((const M3ObjectHeader *)obj, out_type_id);
}

static int m3_gtk4_window_destroy(void *obj) {
  M3GTK4Window *window;
  struct M3GTK4Backend *backend;
  int cleanup_rc;
  int rc;

  window = (M3GTK4Window *)obj;
  backend = window->backend;

  if (window->cmds != NULL) {
    cleanup_rc = m3_gtk4_cmd_list_reset(window);
    if (cleanup_rc != M3_OK) {
      return cleanup_rc;
    }
    cleanup_rc = backend->allocator.free(backend->allocator.ctx, window->cmds);
    if (cleanup_rc != M3_OK) {
      return cleanup_rc;
    }
    window->cmds = NULL;
    window->cmd_capacity = 0;
    window->cmd_count = 0;
  }

  if (window->window != NULL) {
    gtk_window_destroy(window->window);
    window->window = NULL;
  }
  if (window->surface != NULL) {
    cairo_surface_destroy(window->surface);
    window->surface = NULL;
  }
  window->area = NULL;

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  window->header.handle);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, window);
  M3_GTK4_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_gtk4_texture_destroy(void *obj) {
  M3GTK4Texture *texture;
  struct M3GTK4Backend *backend;
  int rc;

  texture = (M3GTK4Texture *)obj;
  backend = texture->backend;

  if (texture->gdk_texture != NULL) {
    g_object_unref(texture->gdk_texture);
    texture->gdk_texture = NULL;
  }
  if (texture->surface != NULL) {
    cairo_surface_destroy(texture->surface);
    texture->surface = NULL;
  }
  if (texture->pixels != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, texture->pixels);
    if (rc != M3_OK) {
      return rc;
    }
    texture->pixels = NULL;
    texture->size = 0u;
  }

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  texture->header.handle);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, texture);
  M3_GTK4_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static int m3_gtk4_font_destroy(void *obj) {
  M3GTK4Font *font;
  struct M3GTK4Backend *backend;
  int rc;

  font = (M3GTK4Font *)obj;
  backend = font->backend;

  if (font->desc != NULL) {
    pango_font_description_free(font->desc);
    font->desc = NULL;
  }

  rc = backend->handles.vtable->unregister_object(backend->handles.ctx,
                                                  font->header.handle);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = backend->allocator.free(backend->allocator.ctx, font);
  M3_GTK4_RETURN_IF_ERROR(rc);

  return M3_OK;
}

static const M3ObjectVTable g_m3_gtk4_window_vtable = {
    m3_gtk4_object_retain, m3_gtk4_object_release, m3_gtk4_window_destroy,
    m3_gtk4_object_get_type_id};

static const M3ObjectVTable g_m3_gtk4_texture_vtable = {
    m3_gtk4_object_retain, m3_gtk4_object_release, m3_gtk4_texture_destroy,
    m3_gtk4_object_get_type_id};

static const M3ObjectVTable g_m3_gtk4_font_vtable = {
    m3_gtk4_object_retain, m3_gtk4_object_release, m3_gtk4_font_destroy,
    m3_gtk4_object_get_type_id};

static int m3_gtk4_ws_init(void *ws, const M3WSConfig *config) {
  struct M3GTK4Backend *backend;
  int rc;

  if (ws == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.init");
  M3_GTK4_RETURN_IF_ERROR(rc);

  g_set_application_name(config->utf8_app_name);
  g_set_prgname(config->utf8_app_id);
  return M3_OK;
}

static int m3_gtk4_ws_shutdown(void *ws) {
  struct M3GTK4Backend *backend;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.shutdown");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_OK;
}

static int m3_gtk4_ws_create_window(void *ws, const M3WSWindowConfig *config,
                                    M3Handle *out_window) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *window;
  GtkWindow *gtk_window;
  GtkWidget *area;
  GtkGestureClick *click;
  GtkEventControllerScroll *scroll;
  GtkEventControllerKey *key;
  GtkEventControllerFocus *focus;
  GtkEventControllerMotion *motion;
  int scale;
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

  backend = (struct M3GTK4Backend *)ws;
  out_window->id = 0u;
  out_window->generation = 0u;

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.create_window");
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3GTK4Window),
                                (void **)&window);
  M3_GTK4_RETURN_IF_ERROR(rc);

  memset(window, 0, sizeof(*window));
  window->backend = backend;
  window->width = config->width;
  window->height = config->height;
  window->frame_width = 0;
  window->frame_height = 0;
  window->dpi_scale = 1.0f;
  window->flags = config->flags;
  window->visible = M3_FALSE;
  window->has_scale_override = M3_FALSE;
  window->use_gsk = backend->use_gsk_default ? M3_TRUE : M3_FALSE;
  window->has_gsk_frame = M3_FALSE;
  window->cmds = NULL;
  window->cmd_count = 0;
  window->cmd_capacity = 0;

  gtk_window = GTK_WINDOW(gtk_window_new());
  if (gtk_window == NULL) {
    backend->allocator.free(backend->allocator.ctx, window);
    return M3_ERR_UNKNOWN;
  }

  area = gtk_drawing_area_new();
  if (area == NULL) {
    gtk_window_destroy(gtk_window);
    backend->allocator.free(backend->allocator.ctx, window);
    return M3_ERR_UNKNOWN;
  }

  gtk_widget_set_hexpand(area, TRUE);
  gtk_widget_set_vexpand(area, TRUE);
  gtk_widget_set_can_focus(area, TRUE);

  gtk_window_set_child(gtk_window, area);
  gtk_window_set_title(gtk_window, config->utf8_title);
  gtk_window_set_default_size(gtk_window, config->width, config->height);
  gtk_window_set_resizable(
      gtk_window, (config->flags & M3_WS_WINDOW_RESIZABLE) != 0 ? TRUE : FALSE);
  if ((config->flags & M3_WS_WINDOW_BORDERLESS) != 0) {
    gtk_window_set_decorated(gtk_window, FALSE);
  }
  if ((config->flags & M3_WS_WINDOW_FULLSCREEN) != 0) {
    gtk_window_fullscreen(gtk_window);
  }

  window->window = gtk_window;
  window->area = area;
  scale = gtk_widget_get_scale_factor(GTK_WIDGET(gtk_window));
  if (scale > 0) {
    window->dpi_scale = (M3Scalar)scale;
  }

  if (!window->use_gsk) {
    rc = m3_gtk4_ensure_surface(window, window->width, window->height);
    if (rc != M3_OK) {
      gtk_window_destroy(gtk_window);
      backend->allocator.free(backend->allocator.ctx, window);
      return rc;
    }
  }

  rc = m3_object_header_init(&window->header, M3_GTK4_TYPE_WINDOW, 0,
                             &g_m3_gtk4_window_vtable);
  if (rc != M3_OK) {
    if (window->surface != NULL) {
      cairo_surface_destroy(window->surface);
      window->surface = NULL;
    }
    gtk_window_destroy(gtk_window);
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &window->header);
  if (rc != M3_OK) {
    if (window->surface != NULL) {
      cairo_surface_destroy(window->surface);
      window->surface = NULL;
    }
    gtk_window_destroy(gtk_window);
    backend->allocator.free(backend->allocator.ctx, window);
    return rc;
  }

  g_signal_connect(area, "snapshot", G_CALLBACK(m3_gtk4_on_snapshot), window);
  g_signal_connect(area, "size-allocate", G_CALLBACK(m3_gtk4_on_size_allocate),
                   window);
  g_signal_connect(gtk_window, "close-request",
                   G_CALLBACK(m3_gtk4_on_close_request), window);

  motion = gtk_event_controller_motion_new();
  if (motion == NULL) {
    m3_object_release(&window->header);
    return M3_ERR_OUT_OF_MEMORY;
  }
  g_signal_connect(motion, "motion", G_CALLBACK(m3_gtk4_on_motion), window);
  gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(motion));

  click = gtk_gesture_click_new();
  if (click == NULL) {
    m3_object_release(&window->header);
    return M3_ERR_OUT_OF_MEMORY;
  }
  g_signal_connect(click, "pressed", G_CALLBACK(m3_gtk4_on_click_pressed),
                   window);
  g_signal_connect(click, "released", G_CALLBACK(m3_gtk4_on_click_released),
                   window);
  gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(click));

  scroll =
      gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);
  if (scroll == NULL) {
    m3_object_release(&window->header);
    return M3_ERR_OUT_OF_MEMORY;
  }
  g_signal_connect(scroll, "scroll", G_CALLBACK(m3_gtk4_on_scroll), window);
  gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(scroll));

  key = gtk_event_controller_key_new();
  if (key == NULL) {
    m3_object_release(&window->header);
    return M3_ERR_OUT_OF_MEMORY;
  }
  g_signal_connect(key, "key-pressed", G_CALLBACK(m3_gtk4_on_key_pressed),
                   window);
  g_signal_connect(key, "key-released", G_CALLBACK(m3_gtk4_on_key_released),
                   window);
  gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(key));

  focus = gtk_event_controller_focus_new();
  if (focus == NULL) {
    m3_object_release(&window->header);
    return M3_ERR_OUT_OF_MEMORY;
  }
  g_signal_connect(focus, "enter", G_CALLBACK(m3_gtk4_on_focus_enter), window);
  g_signal_connect(focus, "leave", G_CALLBACK(m3_gtk4_on_focus_leave), window);
  gtk_widget_add_controller(area, GTK_EVENT_CONTROLLER(focus));

  *out_window = window->header.handle;
  return M3_OK;
}

static int m3_gtk4_ws_destroy_window(void *ws, M3Handle window) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.destroy_window");
  M3_GTK4_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_gtk4_ws_show_window(void *ws, M3Handle window) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.show_window");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    gtk_window_present(resolved->window);
  }
  if (resolved->area != NULL) {
    gtk_widget_grab_focus(resolved->area);
  }
  resolved->visible = M3_TRUE;
  return M3_OK;
}

static int m3_gtk4_ws_hide_window(void *ws, M3Handle window) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.hide_window");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    gtk_widget_hide(GTK_WIDGET(resolved->window));
  }
  resolved->visible = M3_FALSE;
  return M3_OK;
}

static int m3_gtk4_ws_set_window_title(void *ws, M3Handle window,
                                       const char *utf8_title) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (ws == NULL || utf8_title == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_title");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    gtk_window_set_title(resolved->window, utf8_title);
  }
  return M3_OK;
}

static int m3_gtk4_ws_set_window_size(void *ws, M3Handle window, m3_i32 width,
                                      m3_i32 height) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_size");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (resolved->window != NULL) {
    gtk_window_set_default_size(resolved->window, width, height);
  }
  resolved->width = width;
  resolved->height = height;
  m3_gtk4_queue_window_resize(resolved, width, height);
  return M3_OK;
}

static int m3_gtk4_ws_get_window_size(void *ws, M3Handle window,
                                      m3_i32 *out_width, m3_i32 *out_height) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int width;
  int height;
  int rc;

  if (ws == NULL || out_width == NULL || out_height == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_size");
  M3_GTK4_RETURN_IF_ERROR(rc);

  width = resolved->width;
  height = resolved->height;
  if (resolved->area != NULL) {
    width = gtk_widget_get_width(resolved->area);
    height = gtk_widget_get_height(resolved->area);
  }
  if (width <= 0 || height <= 0) {
    width = resolved->width;
    height = resolved->height;
  }

  *out_width = (m3_i32)width;
  *out_height = (m3_i32)height;
  return M3_OK;
}

static int m3_gtk4_ws_set_window_dpi_scale(void *ws, M3Handle window,
                                           M3Scalar scale) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (scale <= 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO,
                           "ws.set_window_dpi_scale");
  M3_GTK4_RETURN_IF_ERROR(rc);

  resolved->dpi_scale = scale;
  resolved->dpi_scale_override = scale;
  resolved->has_scale_override = M3_TRUE;
  return M3_OK;
}

static int m3_gtk4_ws_get_window_dpi_scale(void *ws, M3Handle window,
                                           M3Scalar *out_scale) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int scale;
  int rc;

  if (ws == NULL || out_scale == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO,
                           "ws.get_window_dpi_scale");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (resolved->has_scale_override) {
    *out_scale = resolved->dpi_scale_override;
    return M3_OK;
  }

  scale = 0;
  if (resolved->window != NULL) {
    scale = gtk_widget_get_scale_factor(GTK_WIDGET(resolved->window));
  }
  if (scale > 0) {
    resolved->dpi_scale = (M3Scalar)scale;
  }
  *out_scale = resolved->dpi_scale;
  return M3_OK;
}

static int m3_gtk4_ws_set_clipboard_text(void *ws, const char *utf8_text) {
  struct M3GTK4Backend *backend;
  m3_usize length;
  GdkDisplay *display;
  GdkClipboard *clipboard;
  int rc;

  if (ws == NULL || utf8_text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_clipboard_text");
  M3_GTK4_RETURN_IF_ERROR(rc);

  length = 0;
  while (utf8_text[length] != '\0') {
    length += 1;
  }
  if (length > backend->clipboard_limit) {
    return M3_ERR_RANGE;
  }

  display = gdk_display_get_default();
  if (display == NULL) {
    return M3_ERR_STATE;
  }

  clipboard = gdk_display_get_clipboard(display);
  if (clipboard == NULL) {
    return M3_ERR_STATE;
  }

  gdk_clipboard_set_text(clipboard, utf8_text);
  backend->clipboard_length = length;
  return M3_OK;
}

static int m3_gtk4_ws_get_clipboard_text(void *ws, char *buffer,
                                         m3_usize buffer_size,
                                         m3_usize *out_length) {
  struct M3GTK4Backend *backend;
  GdkDisplay *display;
  GdkClipboard *clipboard;
  M3GTK4ClipboardRead state;
  m3_usize length;
  int rc;

  if (ws == NULL || buffer == NULL || out_length == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_clipboard_text");
  M3_GTK4_RETURN_IF_ERROR(rc);

  display = gdk_display_get_default();
  if (display == NULL) {
    return M3_ERR_STATE;
  }

  clipboard = gdk_display_get_clipboard(display);
  if (clipboard == NULL) {
    return M3_ERR_STATE;
  }

  state.done = FALSE;
  state.text = NULL;
  gdk_clipboard_read_text_async(clipboard, NULL, m3_gtk4_clipboard_read_cb,
                                &state);

  while (!state.done) {
    g_main_context_iteration(NULL, TRUE);
  }

  if (state.text == NULL) {
    if (buffer_size == 0) {
      return M3_ERR_RANGE;
    }
    buffer[0] = '\0';
    *out_length = 0;
    return M3_OK;
  }

  length = 0;
  while (state.text[length] != '\0') {
    length += 1;
  }

  if (buffer_size < length + 1) {
    *out_length = length;
    g_free(state.text);
    return M3_ERR_RANGE;
  }

  memcpy(buffer, state.text, length + 1);
  *out_length = length;
  g_free(state.text);
  return M3_OK;
}

static int m3_gtk4_ws_poll_event(void *ws, M3InputEvent *out_event,
                                 M3Bool *out_has_event) {
  struct M3GTK4Backend *backend;
  int rc;

  if (ws == NULL || out_event == NULL || out_has_event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.poll_event");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (backend->event_count == 0) {
    rc = m3_gtk4_ws_pump_events(ws);
    M3_GTK4_RETURN_IF_ERROR(rc);
  }

  if (backend->event_count == 0) {
    memset(out_event, 0, sizeof(*out_event));
    *out_has_event = M3_FALSE;
    return M3_OK;
  }

  *out_event = backend->event_queue[backend->event_head];
  backend->event_head = (backend->event_head + 1u) % M3_GTK4_EVENT_CAPACITY;
  backend->event_count -= 1u;
  *out_has_event = M3_TRUE;
  return M3_OK;
}

static int m3_gtk4_ws_pump_events(void *ws) {
  struct M3GTK4Backend *backend;
  int rc;

  if (ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.pump_events");
  M3_GTK4_RETURN_IF_ERROR(rc);

  while (g_main_context_iteration(NULL, FALSE)) {
  }
  return M3_OK;
}

static int m3_gtk4_ws_get_time_ms(void *ws, m3_u32 *out_time_ms) {
  struct M3GTK4Backend *backend;
  int rc;
  m3_u32 time_ms;

  if (ws == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)ws;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.get_time_ms");
  M3_GTK4_RETURN_IF_ERROR(rc);

  time_ms = m3_gtk4_get_time_ms();
  backend->time_ms = time_ms;
  *out_time_ms = time_ms;
  return M3_OK;
}

static const M3WSVTable g_m3_gtk4_ws_vtable = {m3_gtk4_ws_init,
                                               m3_gtk4_ws_shutdown,
                                               m3_gtk4_ws_create_window,
                                               m3_gtk4_ws_destroy_window,
                                               m3_gtk4_ws_show_window,
                                               m3_gtk4_ws_hide_window,
                                               m3_gtk4_ws_set_window_title,
                                               m3_gtk4_ws_set_window_size,
                                               m3_gtk4_ws_get_window_size,
                                               m3_gtk4_ws_set_window_dpi_scale,
                                               m3_gtk4_ws_get_window_dpi_scale,
                                               m3_gtk4_ws_set_clipboard_text,
                                               m3_gtk4_ws_get_clipboard_text,
                                               m3_gtk4_ws_poll_event,
                                               m3_gtk4_ws_pump_events,
                                               m3_gtk4_ws_get_time_ms};

static void m3_gtk4_set_source_color(cairo_t *cr, M3Color color) {
  cairo_set_source_rgba(cr, (double)color.r, (double)color.g, (double)color.b,
                        (double)color.a);
}

static void m3_gtk4_path_rounded_rect(cairo_t *cr, const M3Rect *rect,
                                      M3Scalar radius) {
  double x;
  double y;
  double w;
  double h;
  double r;

  if (cr == NULL || rect == NULL) {
    return;
  }

  x = rect->x;
  y = rect->y;
  w = rect->width;
  h = rect->height;
  r = radius;

  if (r <= 0.0) {
    cairo_rectangle(cr, x, y, w, h);
    return;
  }

  if (r > w * 0.5) {
    r = w * 0.5;
  }
  if (r > h * 0.5) {
    r = h * 0.5;
  }

  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - r, y + r, r, -M3_GTK4_PI_2, 0.0);
  cairo_arc(cr, x + w - r, y + h - r, r, 0.0, M3_GTK4_PI_2);
  cairo_arc(cr, x + r, y + h - r, r, M3_GTK4_PI_2, M3_GTK4_PI);
  cairo_arc(cr, x + r, y + r, r, M3_GTK4_PI, 3.0 * M3_GTK4_PI_2);
  cairo_close_path(cr);
}

static int m3_gtk4_cairo_build_path(cairo_t *cr, const M3PathCmd *commands,
                                    m3_usize count) {
  M3Scalar current_x;
  M3Scalar current_y;
  M3Scalar start_x;
  M3Scalar start_y;
  M3Bool has_current;
  m3_usize i;

  if (cr == NULL || commands == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (count == 0) {
    return M3_OK;
  }

  has_current = M3_FALSE;
  current_x = 0.0f;
  current_y = 0.0f;
  start_x = 0.0f;
  start_y = 0.0f;

  cairo_new_path(cr);
  for (i = 0; i < count; ++i) {
    const M3PathCmd *cmd = &commands[i];
    switch (cmd->type) {
    case M3_PATH_CMD_MOVE_TO:
      current_x = cmd->data.move_to.x;
      current_y = cmd->data.move_to.y;
      start_x = current_x;
      start_y = current_y;
      has_current = M3_TRUE;
      cairo_move_to(cr, (double)current_x, (double)current_y);
      break;
    case M3_PATH_CMD_LINE_TO:
      if (!has_current) {
        return M3_ERR_STATE;
      }
      current_x = cmd->data.line_to.x;
      current_y = cmd->data.line_to.y;
      cairo_line_to(cr, (double)current_x, (double)current_y);
      break;
    case M3_PATH_CMD_QUAD_TO: {
      M3Scalar cx1;
      M3Scalar cy1;
      M3Scalar cx2;
      M3Scalar cy2;
      M3Scalar x;
      M3Scalar y;

      if (!has_current) {
        return M3_ERR_STATE;
      }
      x = cmd->data.quad_to.x;
      y = cmd->data.quad_to.y;
      cx1 = current_x + (cmd->data.quad_to.cx - current_x) * (2.0f / 3.0f);
      cy1 = current_y + (cmd->data.quad_to.cy - current_y) * (2.0f / 3.0f);
      cx2 = x + (cmd->data.quad_to.cx - x) * (2.0f / 3.0f);
      cy2 = y + (cmd->data.quad_to.cy - y) * (2.0f / 3.0f);
      cairo_curve_to(cr, (double)cx1, (double)cy1, (double)cx2, (double)cy2,
                     (double)x, (double)y);
      current_x = x;
      current_y = y;
      break;
    }
    case M3_PATH_CMD_CUBIC_TO:
      if (!has_current) {
        return M3_ERR_STATE;
      }
      cairo_curve_to(
          cr, (double)cmd->data.cubic_to.cx1, (double)cmd->data.cubic_to.cy1,
          (double)cmd->data.cubic_to.cx2, (double)cmd->data.cubic_to.cy2,
          (double)cmd->data.cubic_to.x, (double)cmd->data.cubic_to.y);
      current_x = cmd->data.cubic_to.x;
      current_y = cmd->data.cubic_to.y;
      break;
    case M3_PATH_CMD_CLOSE:
      if (!has_current) {
        return M3_ERR_STATE;
      }
      cairo_close_path(cr);
      current_x = start_x;
      current_y = start_y;
      has_current = M3_FALSE;
      break;
    default:
      return M3_ERR_INVALID_ARGUMENT;
    }
  }
  return M3_OK;
}

static int m3_gtk4_path_bounds_update(M3Scalar x, M3Scalar y, M3Scalar *min_x,
                                      M3Scalar *min_y, M3Scalar *max_x,
                                      M3Scalar *max_y, M3Bool *has_bounds) {
  if (min_x == NULL || min_y == NULL || max_x == NULL || max_y == NULL ||
      has_bounds == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (!*has_bounds) {
    *min_x = x;
    *max_x = x;
    *min_y = y;
    *max_y = y;
    *has_bounds = M3_TRUE;
    return M3_OK;
  }

  if (x < *min_x) {
    *min_x = x;
  }
  if (x > *max_x) {
    *max_x = x;
  }
  if (y < *min_y) {
    *min_y = y;
  }
  if (y > *max_y) {
    *max_y = y;
  }
  return M3_OK;
}

static void m3_gtk4_color_to_rgba(M3Color color, GdkRGBA *out_rgba) {
  if (out_rgba == NULL) {
    return;
  }
  out_rgba->red = color.r;
  out_rgba->green = color.g;
  out_rgba->blue = color.b;
  out_rgba->alpha = color.a;
}

static void m3_gtk4_snapshot_draw_rect(GtkSnapshot *snapshot,
                                       const M3Rect *rect, M3Color color,
                                       M3Scalar radius) {
  graphene_rect_t bounds;
  cairo_t *cr;
  M3Rect local;

  if (snapshot == NULL || rect == NULL) {
    return;
  }

  bounds.origin.x = rect->x;
  bounds.origin.y = rect->y;
  bounds.size.width = rect->width;
  bounds.size.height = rect->height;

  if (radius <= 0.0f) {
    GdkRGBA rgba;
    m3_gtk4_color_to_rgba(color, &rgba);
    gtk_snapshot_append_color(snapshot, &rgba, &bounds);
    return;
  }

  cr = gtk_snapshot_append_cairo(snapshot, &bounds);
  if (cr == NULL) {
    return;
  }
  m3_gtk4_set_source_color(cr, color);
  local.x = 0.0f;
  local.y = 0.0f;
  local.width = rect->width;
  local.height = rect->height;
  m3_gtk4_path_rounded_rect(cr, &local, radius);
  cairo_fill(cr);
  cairo_destroy(cr);
}

static void m3_gtk4_snapshot_draw_line(GtkSnapshot *snapshot,
                                       const M3GTK4CmdLine *line) {
  graphene_rect_t bounds;
  cairo_t *cr;
  M3Scalar min_x;
  M3Scalar min_y;
  M3Scalar max_x;
  M3Scalar max_y;
  M3Scalar half;

  if (snapshot == NULL || line == NULL) {
    return;
  }

  if (line->thickness <= 0.0f) {
    return;
  }

  if (line->x0 == line->x1 || line->y0 == line->y1) {
    M3Rect rect;
    GdkRGBA rgba;
    min_x = (line->x0 < line->x1) ? line->x0 : line->x1;
    min_y = (line->y0 < line->y1) ? line->y0 : line->y1;
    max_x = (line->x0 > line->x1) ? line->x0 : line->x1;
    max_y = (line->y0 > line->y1) ? line->y0 : line->y1;
    half = line->thickness * 0.5f;
    rect.x = min_x - half;
    rect.y = min_y - half;
    rect.width = (max_x - min_x) + line->thickness;
    rect.height = (max_y - min_y) + line->thickness;
    bounds.origin.x = rect.x;
    bounds.origin.y = rect.y;
    bounds.size.width = rect.width;
    bounds.size.height = rect.height;
    m3_gtk4_color_to_rgba(line->color, &rgba);
    gtk_snapshot_append_color(snapshot, &rgba, &bounds);
    return;
  }

  min_x = (line->x0 < line->x1) ? line->x0 : line->x1;
  min_y = (line->y0 < line->y1) ? line->y0 : line->y1;
  max_x = (line->x0 > line->x1) ? line->x0 : line->x1;
  max_y = (line->y0 > line->y1) ? line->y0 : line->y1;
  half = line->thickness * 0.5f;
  bounds.origin.x = min_x - half;
  bounds.origin.y = min_y - half;
  bounds.size.width = (max_x - min_x) + line->thickness;
  bounds.size.height = (max_y - min_y) + line->thickness;

  cr = gtk_snapshot_append_cairo(snapshot, &bounds);
  if (cr == NULL) {
    return;
  }
  m3_gtk4_set_source_color(cr, line->color);
  cairo_set_line_width(cr, (double)line->thickness);
  cairo_move_to(cr, (double)(line->x0 - bounds.origin.x),
                (double)(line->y0 - bounds.origin.y));
  cairo_line_to(cr, (double)(line->x1 - bounds.origin.x),
                (double)(line->y1 - bounds.origin.y));
  cairo_stroke(cr);
  cairo_destroy(cr);
}

static void m3_gtk4_snapshot_draw_texture(GtkSnapshot *snapshot,
                                          M3GTK4Window *window,
                                          const M3GTK4CmdTexture *cmd) {
  struct M3GTK4Backend *backend;
  M3GTK4Texture *resolved;
  cairo_t *cr;
  graphene_rect_t bounds;
  graphene_point_t origin;
  graphene_rect_t clip_bounds;
  graphene_rect_t tex_bounds;
  M3Scalar src_width;
  M3Scalar src_height;
  M3Scalar dst_width;
  M3Scalar dst_height;
  float scale_x;
  float scale_y;
  int use_opacity;
  int rc;

  if (snapshot == NULL || window == NULL || cmd == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  rc = m3_gtk4_backend_resolve(backend, cmd->texture, M3_GTK4_TYPE_TEXTURE,
                               (void **)&resolved);
  if (rc != M3_OK || resolved == NULL || resolved->surface == NULL) {
    return;
  }

  src_width = cmd->src.width;
  src_height = cmd->src.height;
  dst_width = cmd->dst.width;
  dst_height = cmd->dst.height;
  if (src_width <= 0.0f || src_height <= 0.0f || dst_width <= 0.0f ||
      dst_height <= 0.0f) {
    return;
  }
  if (cmd->src.x < 0.0f || cmd->src.y < 0.0f ||
      cmd->src.x + src_width > (M3Scalar)resolved->width ||
      cmd->src.y + src_height > (M3Scalar)resolved->height) {
    return;
  }

  if (backend->use_gdk_texture && resolved->format != M3_TEX_FORMAT_A8) {
    cairo_surface_flush(resolved->surface);
    if (resolved->gdk_texture == NULL) {
      resolved->gdk_texture = gdk_texture_new_for_surface(resolved->surface);
    }
    if (resolved->gdk_texture != NULL) {
      scale_x = (float)(dst_width / src_width);
      scale_y = (float)(dst_height / src_height);
      use_opacity = (cmd->opacity < 1.0f) ? 1 : 0;

      gtk_snapshot_save(snapshot);
      if (use_opacity) {
        gtk_snapshot_push_opacity(snapshot, (double)cmd->opacity);
      }

      origin.x = cmd->dst.x;
      origin.y = cmd->dst.y;
      gtk_snapshot_translate(snapshot, &origin);
      gtk_snapshot_scale(snapshot, scale_x, scale_y);

      clip_bounds.origin.x = 0.0f;
      clip_bounds.origin.y = 0.0f;
      clip_bounds.size.width = src_width;
      clip_bounds.size.height = src_height;
      gtk_snapshot_push_clip(snapshot, &clip_bounds);

      tex_bounds.origin.x = -cmd->src.x;
      tex_bounds.origin.y = -cmd->src.y;
      tex_bounds.size.width = (float)resolved->width;
      tex_bounds.size.height = (float)resolved->height;
      gtk_snapshot_append_texture(snapshot, resolved->gdk_texture, &tex_bounds);

      gtk_snapshot_pop(snapshot);
      if (use_opacity) {
        gtk_snapshot_pop(snapshot);
      }
      gtk_snapshot_restore(snapshot);
      return;
    }
  }

  bounds.origin.x = cmd->dst.x;
  bounds.origin.y = cmd->dst.y;
  bounds.size.width = dst_width;
  bounds.size.height = dst_height;

  cr = gtk_snapshot_append_cairo(snapshot, &bounds);
  if (cr == NULL) {
    return;
  }
  cairo_save(cr);
  cairo_rectangle(cr, 0.0, 0.0, (double)dst_width, (double)dst_height);
  cairo_clip(cr);
  cairo_translate(cr, 0.0, 0.0);
  cairo_scale(cr, (double)dst_width / (double)src_width,
              (double)dst_height / (double)src_height);
  cairo_set_source_surface(cr, resolved->surface, (double)-cmd->src.x,
                           (double)-cmd->src.y);
  cairo_paint_with_alpha(cr, (double)cmd->opacity);
  cairo_restore(cr);
  cairo_destroy(cr);
}

static void m3_gtk4_snapshot_draw_text(GtkSnapshot *snapshot,
                                       M3GTK4Window *window,
                                       const M3GTK4CmdText *cmd) {
  struct M3GTK4Backend *backend;
  M3GTK4Font *resolved;
  graphene_rect_t bounds;
  cairo_t *cr;
  PangoLayout *layout;
  PangoContext *context;
  int width_px;
  int height_px;
  int rc;

  if (snapshot == NULL || window == NULL || cmd == NULL) {
    return;
  }

  backend = window->backend;
  if (backend == NULL) {
    return;
  }

  rc = m3_gtk4_backend_resolve(backend, cmd->font, M3_GTK4_TYPE_FONT,
                               (void **)&resolved);
  if (rc != M3_OK || resolved == NULL || resolved->desc == NULL) {
    return;
  }
  if (cmd->len > (m3_usize)G_MAXINT) {
    return;
  }

  context = NULL;
  if (window->area != NULL) {
    context = gtk_widget_get_pango_context(window->area);
  } else if (window->window != NULL) {
    context = gtk_widget_get_pango_context(GTK_WIDGET(window->window));
  }
  if (context == NULL) {
    return;
  }

  layout = pango_layout_new(context);
  if (layout == NULL) {
    return;
  }

  pango_layout_set_font_description(layout, resolved->desc);
  pango_layout_set_text(layout, (cmd->utf8 != NULL) ? cmd->utf8 : "",
                        (int)cmd->len);
  pango_layout_get_pixel_size(layout, &width_px, &height_px);

  bounds.origin.x = cmd->x;
  bounds.origin.y = cmd->y;
  bounds.size.width = (float)width_px;
  bounds.size.height = (float)height_px;

  cr = gtk_snapshot_append_cairo(snapshot, &bounds);
  if (cr == NULL) {
    g_object_unref(layout);
    return;
  }

  m3_gtk4_set_source_color(cr, cmd->color);
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout);
  cairo_destroy(cr);
}

static void m3_gtk4_snapshot_draw_path(GtkSnapshot *snapshot,
                                       const M3GTK4CmdPath *cmd) {
  graphene_rect_t bounds;
  cairo_t *cr;
  int rc;

  if (snapshot == NULL || cmd == NULL) {
    return;
  }
  if (cmd->commands == NULL || cmd->count == 0) {
    return;
  }

  bounds.origin.x = cmd->bounds.x;
  bounds.origin.y = cmd->bounds.y;
  bounds.size.width = cmd->bounds.width;
  bounds.size.height = cmd->bounds.height;

  cr = gtk_snapshot_append_cairo(snapshot, &bounds);
  if (cr == NULL) {
    return;
  }

  cairo_save(cr);
  cairo_translate(cr, (double)-cmd->bounds.x, (double)-cmd->bounds.y);
  m3_gtk4_set_source_color(cr, cmd->color);
  rc = m3_gtk4_cairo_build_path(cr, cmd->commands, cmd->count);
  if (rc == M3_OK) {
    cairo_fill(cr);
  }
  cairo_restore(cr);
  cairo_destroy(cr);
}

static void m3_gtk4_snapshot_cmds(M3GTK4Window *window, GtkSnapshot *snapshot) {
  m3_usize i;
  const M3GTK4Cmd *cmd;

  if (window == NULL || snapshot == NULL) {
    return;
  }

  for (i = 0; i < window->cmd_count; ++i) {
    cmd = &window->cmds[i];
    switch (cmd->type) {
    case M3_GTK4_CMD_CLEAR:
      m3_gtk4_snapshot_draw_rect(snapshot, &cmd->data.clear.rect,
                                 cmd->data.clear.color, 0.0f);
      break;
    case M3_GTK4_CMD_RECT:
      m3_gtk4_snapshot_draw_rect(snapshot, &cmd->data.rect.rect,
                                 cmd->data.rect.color, cmd->data.rect.radius);
      break;
    case M3_GTK4_CMD_LINE:
      m3_gtk4_snapshot_draw_line(snapshot, &cmd->data.line);
      break;
    case M3_GTK4_CMD_PUSH_CLIP: {
      graphene_rect_t clip_rect;
      clip_rect.origin.x = cmd->data.clip.rect.x;
      clip_rect.origin.y = cmd->data.clip.rect.y;
      clip_rect.size.width = cmd->data.clip.rect.width;
      clip_rect.size.height = cmd->data.clip.rect.height;
      gtk_snapshot_push_clip(snapshot, &clip_rect);
      break;
    }
    case M3_GTK4_CMD_POP_CLIP:
      gtk_snapshot_pop(snapshot);
      break;
    case M3_GTK4_CMD_TEXTURE:
      m3_gtk4_snapshot_draw_texture(snapshot, window, &cmd->data.texture);
      break;
    case M3_GTK4_CMD_TEXT:
      m3_gtk4_snapshot_draw_text(snapshot, window, &cmd->data.text);
      break;
    case M3_GTK4_CMD_PATH:
      m3_gtk4_snapshot_draw_path(snapshot, &cmd->data.path);
      break;
    default:
      break;
    }
  }
}

static void m3_gtk4_write_argb32_pixel(unsigned char *dst, unsigned char r,
                                       unsigned char g, unsigned char b,
                                       unsigned char a) {
  unsigned int alpha;
  unsigned char pr;
  unsigned char pg;
  unsigned char pb;

  alpha = (unsigned int)a;
  if (alpha == 0) {
    pr = 0;
    pg = 0;
    pb = 0;
  } else {
    pr = (unsigned char)((r * alpha + 127u) / 255u);
    pg = (unsigned char)((g * alpha + 127u) / 255u);
    pb = (unsigned char)((b * alpha + 127u) / 255u);
  }

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  dst[0] = pb;
  dst[1] = pg;
  dst[2] = pr;
  dst[3] = (unsigned char)alpha;
#else
  dst[0] = (unsigned char)alpha;
  dst[1] = pr;
  dst[2] = pg;
  dst[3] = pb;
#endif
}

static int m3_gtk4_copy_texture_region(M3GTK4Texture *texture,
                                       const void *pixels, m3_usize size,
                                       m3_i32 x, m3_i32 y, m3_i32 width,
                                       m3_i32 height) {
  const unsigned char *src;
  unsigned char *dst;
  m3_usize expected;
  m3_i32 row;
  m3_i32 col;
  m3_usize src_stride;
  const unsigned char *pixel;
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;

  if (texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (texture->pixels == NULL || texture->surface == NULL) {
    return M3_ERR_STATE;
  }
  if (pixels == NULL) {
    if (size == 0) {
      return M3_OK;
    }
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (x < 0 || y < 0 || x + width > texture->width ||
      y + height > texture->height) {
    return M3_ERR_RANGE;
  }

  src = (const unsigned char *)pixels;

  if (texture->format == M3_TEX_FORMAT_A8) {
    expected = (m3_usize)width * (m3_usize)height;
    if (size < expected) {
      return M3_ERR_RANGE;
    }
    src_stride = (m3_usize)width;
    for (row = 0; row < height; ++row) {
      dst = texture->pixels + (m3_usize)(y + row) * (m3_usize)texture->stride +
            (m3_usize)x;
      memcpy(dst, src + (m3_usize)row * src_stride, (m3_usize)width);
    }
    cairo_surface_mark_dirty(texture->surface);
    return M3_OK;
  }

  expected = (m3_usize)width * (m3_usize)height * 4u;
  if (size < expected) {
    return M3_ERR_RANGE;
  }

  src_stride = (m3_usize)width * 4u;
  for (row = 0; row < height; ++row) {
    dst = texture->pixels + (m3_usize)(y + row) * (m3_usize)texture->stride +
          (m3_usize)x * 4u;
    for (col = 0; col < width; ++col) {
      pixel = src + (m3_usize)row * src_stride + (m3_usize)col * 4u;
      if (texture->format == M3_TEX_FORMAT_RGBA8) {
        r = pixel[0];
        g = pixel[1];
        b = pixel[2];
        a = pixel[3];
      } else {
        b = pixel[0];
        g = pixel[1];
        r = pixel[2];
        a = pixel[3];
      }

      m3_gtk4_write_argb32_pixel(dst + (m3_usize)col * 4u, r, g, b, a);
    }
  }
  cairo_surface_mark_dirty(texture->surface);
  return M3_OK;
}

static int m3_gtk4_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width,
                                   m3_i32 height, M3Scalar dpi_scale) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  cairo_t *cr;
  cairo_status_t status;
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

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->in_frame) {
    return M3_ERR_STATE;
  }

  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.begin_frame");
  M3_GTK4_RETURN_IF_ERROR(rc);

  backend->clip_depth = 0u;
  resolved->dpi_scale = dpi_scale;
  resolved->width = width;
  resolved->height = height;
  resolved->frame_width = width;
  resolved->frame_height = height;

  if (resolved->use_gsk) {
    rc = m3_gtk4_cmd_list_reset(resolved);
    M3_GTK4_RETURN_IF_ERROR(rc);
    resolved->has_gsk_frame = M3_FALSE;
    if (backend->has_transform) {
      backend->gsk_transform = backend->transform;
      backend->gsk_transform_simple =
          m3_gtk4_transform_is_simple(&backend->gsk_transform);
      if (!backend->gsk_transform_simple) {
        return M3_ERR_UNSUPPORTED;
      }
    } else {
      backend->gsk_transform.m[0] = 1.0f;
      backend->gsk_transform.m[1] = 0.0f;
      backend->gsk_transform.m[2] = 0.0f;
      backend->gsk_transform.m[3] = 0.0f;
      backend->gsk_transform.m[4] = 1.0f;
      backend->gsk_transform.m[5] = 0.0f;
      backend->gsk_transform.m[6] = 0.0f;
      backend->gsk_transform.m[7] = 0.0f;
      backend->gsk_transform.m[8] = 1.0f;
      backend->gsk_transform_simple = M3_TRUE;
    }
    backend->active_window = resolved;
    backend->in_frame = M3_TRUE;
    backend->frame_use_gsk = M3_TRUE;
    backend->frame_cr = NULL;
    return M3_OK;
  }
  backend->frame_use_gsk = M3_FALSE;

  rc = m3_gtk4_ensure_surface(resolved, width, height);
  M3_GTK4_RETURN_IF_ERROR(rc);

  cr = cairo_create(resolved->surface);
  status = cairo_status(cr);
  if (status != CAIRO_STATUS_SUCCESS) {
    cairo_destroy(cr);
    return M3_ERR_UNKNOWN;
  }

  backend->frame_cr = cr;
  backend->active_window = resolved;
  backend->in_frame = M3_TRUE;

  if (backend->has_transform) {
    rc = m3_gtk4_apply_transform(backend);
    if (rc != M3_OK) {
      cairo_destroy(cr);
      backend->frame_cr = NULL;
      backend->active_window = NULL;
      backend->in_frame = M3_FALSE;
      return rc;
    }
  } else {
    cairo_identity_matrix(cr);
  }
  return M3_OK;
}

static int m3_gtk4_gfx_end_frame(void *gfx, M3Handle window) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *resolved;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)gfx;
  rc = m3_gtk4_backend_resolve(backend, window, M3_GTK4_TYPE_WINDOW,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (!backend->in_frame || backend->active_window != resolved) {
    return M3_ERR_STATE;
  }

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.end_frame");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (backend->frame_use_gsk) {
    backend->in_frame = M3_FALSE;
    backend->frame_use_gsk = M3_FALSE;
    backend->active_window = NULL;
    resolved->has_gsk_frame = M3_TRUE;
    if (resolved->area != NULL) {
      gtk_widget_queue_draw(resolved->area);
    }
    return M3_OK;
  }

  if (resolved->surface != NULL) {
    cairo_surface_flush(resolved->surface);
  }
  cairo_destroy(backend->frame_cr);
  backend->frame_cr = NULL;
  backend->active_window = NULL;
  backend->in_frame = M3_FALSE;
  backend->frame_use_gsk = M3_FALSE;

  if (resolved->area != NULL) {
    gtk_widget_queue_draw(resolved->area);
  }
  return M3_OK;
}

static int m3_gtk4_gfx_clear(void *gfx, M3Color color) {
  struct M3GTK4Backend *backend;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->frame_use_gsk) {
    M3GTK4Cmd cmd;
    M3GTK4Window *window;

    window = backend->active_window;
    if (window == NULL) {
      return M3_ERR_STATE;
    }
    if (window->frame_width <= 0 || window->frame_height <= 0) {
      return M3_ERR_RANGE;
    }

    cmd.type = M3_GTK4_CMD_CLEAR;
    cmd.data.clear.rect.x = 0.0f;
    cmd.data.clear.rect.y = 0.0f;
    cmd.data.clear.rect.width = (M3Scalar)window->frame_width;
    cmd.data.clear.rect.height = (M3Scalar)window->frame_height;
    cmd.data.clear.color = color;
    rc = m3_gtk4_cmd_list_push(window, &cmd);
    M3_GTK4_RETURN_IF_ERROR(rc);
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.clear");
  M3_GTK4_RETURN_IF_ERROR(rc);

  cairo_save(backend->frame_cr);
  cairo_set_operator(backend->frame_cr, CAIRO_OPERATOR_SOURCE);
  m3_gtk4_set_source_color(backend->frame_cr, color);
  cairo_paint(backend->frame_cr);
  cairo_restore(backend->frame_cr);
  return M3_OK;
}

static int m3_gtk4_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color,
                                 M3Scalar corner_radius) {
  struct M3GTK4Backend *backend;
  M3GTK4Cmd cmd;
  M3Rect transformed;
  M3Scalar radius;
  M3Scalar sx;
  M3Scalar sy;
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

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }

    rc = m3_gtk4_transform_rect_simple(&backend->gsk_transform, rect,
                                       &transformed);
    M3_GTK4_RETURN_IF_ERROR(rc);

    sx = m3_gtk4_abs_scalar(backend->gsk_transform.m[0]);
    sy = m3_gtk4_abs_scalar(backend->gsk_transform.m[4]);
    radius = corner_radius;
    if (sx < sy) {
      radius = corner_radius * sx;
    } else {
      radius = corner_radius * sy;
    }

    cmd.type = M3_GTK4_CMD_RECT;
    cmd.data.rect.rect = transformed;
    cmd.data.rect.color = color;
    cmd.data.rect.radius = radius;
    rc = m3_gtk4_cmd_list_push(backend->active_window, &cmd);
    M3_GTK4_RETURN_IF_ERROR(rc);
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_rect");
  M3_GTK4_RETURN_IF_ERROR(rc);

  cairo_save(backend->frame_cr);
  m3_gtk4_set_source_color(backend->frame_cr, color);
  m3_gtk4_path_rounded_rect(backend->frame_cr, rect, corner_radius);
  cairo_fill(backend->frame_cr);
  cairo_restore(backend->frame_cr);
  return M3_OK;
}

static int m3_gtk4_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0,
                                 M3Scalar x1, M3Scalar y1, M3Color color,
                                 M3Scalar thickness) {
  struct M3GTK4Backend *backend;
  M3GTK4Cmd cmd;
  M3Scalar tx0;
  M3Scalar ty0;
  M3Scalar tx1;
  M3Scalar ty1;
  M3Scalar sx;
  M3Scalar sy;
  M3Scalar scale;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (thickness < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }

    rc = m3_gtk4_transform_point_simple(&backend->gsk_transform, x0, y0, &tx0,
                                        &ty0);
    M3_GTK4_RETURN_IF_ERROR(rc);
    rc = m3_gtk4_transform_point_simple(&backend->gsk_transform, x1, y1, &tx1,
                                        &ty1);
    M3_GTK4_RETURN_IF_ERROR(rc);

    sx = m3_gtk4_abs_scalar(backend->gsk_transform.m[0]);
    sy = m3_gtk4_abs_scalar(backend->gsk_transform.m[4]);
    scale = (sx + sy) * 0.5f;

    cmd.type = M3_GTK4_CMD_LINE;
    cmd.data.line.x0 = tx0;
    cmd.data.line.y0 = ty0;
    cmd.data.line.x1 = tx1;
    cmd.data.line.y1 = ty1;
    cmd.data.line.thickness = thickness * scale;
    cmd.data.line.color = color;
    rc = m3_gtk4_cmd_list_push(backend->active_window, &cmd);
    M3_GTK4_RETURN_IF_ERROR(rc);
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_line");
  M3_GTK4_RETURN_IF_ERROR(rc);

  cairo_save(backend->frame_cr);
  m3_gtk4_set_source_color(backend->frame_cr, color);
  cairo_set_line_width(backend->frame_cr, (double)thickness);
  cairo_move_to(backend->frame_cr, (double)x0, (double)y0);
  cairo_line_to(backend->frame_cr, (double)x1, (double)y1);
  cairo_stroke(backend->frame_cr);
  cairo_restore(backend->frame_cr);
  return M3_OK;
}

static int m3_gtk4_gfx_draw_path(void *gfx, const M3Path *path, M3Color color) {
  struct M3GTK4Backend *backend;
  M3GTK4Window *window;
  M3GTK4Cmd cmd;
  M3PathCmd *commands;
  M3Scalar min_x;
  M3Scalar min_y;
  M3Scalar max_x;
  M3Scalar max_y;
  M3Bool has_bounds;
  m3_usize max_value;
  m3_usize bytes;
  m3_usize i;
  void *mem;
  int rc;

  if (gfx == NULL || path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (path->commands == NULL) {
    return M3_ERR_STATE;
  }
  if (path->count > path->capacity) {
    return M3_ERR_STATE;
  }
  if (path->count == 0) {
    return M3_OK;
  }

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }

    window = backend->active_window;
    max_value = (m3_usize) ~(m3_usize)0;
    if (path->count > max_value / (m3_usize)sizeof(M3PathCmd)) {
      return M3_ERR_OVERFLOW;
    }
    bytes = path->count * (m3_usize)sizeof(M3PathCmd);
    rc = backend->allocator.alloc(backend->allocator.ctx, bytes, &mem);
    M3_GTK4_RETURN_IF_ERROR(rc);

    commands = (M3PathCmd *)mem;
    has_bounds = M3_FALSE;
    min_x = 0.0f;
    min_y = 0.0f;
    max_x = 0.0f;
    max_y = 0.0f;

    for (i = 0; i < path->count; ++i) {
      const M3PathCmd *src = &path->commands[i];
      M3PathCmd *dst = &commands[i];

      dst->type = src->type;
      switch (src->type) {
      case M3_PATH_CMD_MOVE_TO:
        rc = m3_gtk4_transform_point_simple(
            &backend->gsk_transform, src->data.move_to.x, src->data.move_to.y,
            &dst->data.move_to.x, &dst->data.move_to.y);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_path_bounds_update(dst->data.move_to.x,
                                        dst->data.move_to.y, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        break;
      case M3_PATH_CMD_LINE_TO:
        rc = m3_gtk4_transform_point_simple(
            &backend->gsk_transform, src->data.line_to.x, src->data.line_to.y,
            &dst->data.line_to.x, &dst->data.line_to.y);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_path_bounds_update(dst->data.line_to.x,
                                        dst->data.line_to.y, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        break;
      case M3_PATH_CMD_QUAD_TO: {
        M3Scalar tx;
        M3Scalar ty;

        rc = m3_gtk4_transform_point_simple(
            &backend->gsk_transform, src->data.quad_to.cx, src->data.quad_to.cy,
            &dst->data.quad_to.cx, &dst->data.quad_to.cy);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_transform_point_simple(&backend->gsk_transform,
                                            src->data.quad_to.x,
                                            src->data.quad_to.y, &tx, &ty);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        dst->data.quad_to.x = tx;
        dst->data.quad_to.y = ty;

        rc = m3_gtk4_path_bounds_update(dst->data.quad_to.cx,
                                        dst->data.quad_to.cy, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_path_bounds_update(dst->data.quad_to.x,
                                        dst->data.quad_to.y, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        break;
      }
      case M3_PATH_CMD_CUBIC_TO: {
        M3Scalar tx;
        M3Scalar ty;

        rc = m3_gtk4_transform_point_simple(
            &backend->gsk_transform, src->data.cubic_to.cx1,
            src->data.cubic_to.cy1, &dst->data.cubic_to.cx1,
            &dst->data.cubic_to.cy1);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_transform_point_simple(
            &backend->gsk_transform, src->data.cubic_to.cx2,
            src->data.cubic_to.cy2, &dst->data.cubic_to.cx2,
            &dst->data.cubic_to.cy2);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_transform_point_simple(&backend->gsk_transform,
                                            src->data.cubic_to.x,
                                            src->data.cubic_to.y, &tx, &ty);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        dst->data.cubic_to.x = tx;
        dst->data.cubic_to.y = ty;

        rc = m3_gtk4_path_bounds_update(dst->data.cubic_to.cx1,
                                        dst->data.cubic_to.cy1, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_path_bounds_update(dst->data.cubic_to.cx2,
                                        dst->data.cubic_to.cy2, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        rc = m3_gtk4_path_bounds_update(dst->data.cubic_to.x,
                                        dst->data.cubic_to.y, &min_x, &min_y,
                                        &max_x, &max_y, &has_bounds);
        M3_GTK4_RETURN_IF_ERROR_CLEANUP(
            rc, backend->allocator.free(backend->allocator.ctx, mem));
        break;
      }
      case M3_PATH_CMD_CLOSE:
        break;
      default:
        backend->allocator.free(backend->allocator.ctx, mem);
        return M3_ERR_INVALID_ARGUMENT;
      }
    }

    if (!has_bounds) {
      backend->allocator.free(backend->allocator.ctx, mem);
      return M3_OK;
    }

    cmd.type = M3_GTK4_CMD_PATH;
    cmd.data.path.commands = commands;
    cmd.data.path.count = path->count;
    cmd.data.path.color = color;
    cmd.data.path.bounds.x = min_x;
    cmd.data.path.bounds.y = min_y;
    cmd.data.path.bounds.width = max_x - min_x;
    cmd.data.path.bounds.height = max_y - min_y;
    rc = m3_gtk4_cmd_list_push(window, &cmd);
    if (rc != M3_OK) {
      backend->allocator.free(backend->allocator.ctx, commands);
      return rc;
    }
    return M3_OK;
  }

  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_path");
  M3_GTK4_RETURN_IF_ERROR(rc);

  cairo_save(backend->frame_cr);
  m3_gtk4_set_source_color(backend->frame_cr, color);
  rc = m3_gtk4_cairo_build_path(backend->frame_cr, path->commands, path->count);
  if (rc == M3_OK) {
    cairo_fill(backend->frame_cr);
  }
  cairo_restore(backend->frame_cr);
  return rc;
}

static int m3_gtk4_gfx_push_clip(void *gfx, const M3Rect *rect) {
  struct M3GTK4Backend *backend;
  M3GTK4Cmd cmd;
  M3Rect transformed;
  int rc;

  if (gfx == NULL || rect == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (rect->width < 0.0f || rect->height < 0.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (backend->clip_depth >= M3_GTK4_CLIP_STACK_CAPACITY) {
      return M3_ERR_RANGE;
    }
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }
    rc = m3_gtk4_transform_rect_simple(&backend->gsk_transform, rect,
                                       &transformed);
    M3_GTK4_RETURN_IF_ERROR(rc);
    cmd.type = M3_GTK4_CMD_PUSH_CLIP;
    cmd.data.clip.rect = transformed;
    rc = m3_gtk4_cmd_list_push(backend->active_window, &cmd);
    M3_GTK4_RETURN_IF_ERROR(rc);
    backend->clip_depth += 1u;
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }
  if (backend->clip_depth >= M3_GTK4_CLIP_STACK_CAPACITY) {
    return M3_ERR_RANGE;
  }
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.push_clip");
  M3_GTK4_RETURN_IF_ERROR(rc);

  cairo_save(backend->frame_cr);
  cairo_rectangle(backend->frame_cr, rect->x, rect->y, rect->width,
                  rect->height);
  cairo_clip(backend->frame_cr);
  backend->clip_stack[backend->clip_depth] = *rect;
  backend->clip_depth += 1u;
  return M3_OK;
}

static int m3_gtk4_gfx_pop_clip(void *gfx) {
  struct M3GTK4Backend *backend;
  M3GTK4Cmd cmd;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)gfx;
  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (backend->clip_depth == 0u) {
      return M3_ERR_STATE;
    }
    cmd.type = M3_GTK4_CMD_POP_CLIP;
    rc = m3_gtk4_cmd_list_push(backend->active_window, &cmd);
    M3_GTK4_RETURN_IF_ERROR(rc);
    backend->clip_depth -= 1u;
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }
  if (backend->clip_depth == 0u) {
    return M3_ERR_STATE;
  }
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.pop_clip");
  M3_GTK4_RETURN_IF_ERROR(rc);

  cairo_restore(backend->frame_cr);
  backend->clip_depth -= 1u;
  return M3_OK;
}

static int m3_gtk4_gfx_set_transform(void *gfx, const M3Mat3 *transform) {
  struct M3GTK4Backend *backend;
  int rc;

  if (gfx == NULL || transform == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)gfx;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.set_transform");
  M3_GTK4_RETURN_IF_ERROR(rc);

  backend->transform = *transform;
  backend->has_transform = M3_TRUE;
  if (backend->frame_use_gsk) {
    backend->gsk_transform = *transform;
    backend->gsk_transform_simple = m3_gtk4_transform_is_simple(transform);
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }
    return M3_OK;
  }
  if (backend->frame_cr != NULL) {
    rc = m3_gtk4_apply_transform(backend);
    M3_GTK4_RETURN_IF_ERROR(rc);
  }
  return M3_OK;
}

static int m3_gtk4_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height,
                                      m3_u32 format, const void *pixels,
                                      m3_usize size, M3Handle *out_texture) {
  struct M3GTK4Backend *backend;
  M3GTK4Texture *texture;
  cairo_format_t cairo_format;
  cairo_surface_t *surface;
  int stride;
  int rc;

  if (gfx == NULL || out_texture == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (width <= 0 || height <= 0) {
    return M3_ERR_RANGE;
  }
  if (format != M3_TEX_FORMAT_RGBA8 && format != M3_TEX_FORMAT_BGRA8 &&
      format != M3_TEX_FORMAT_A8) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (pixels == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)gfx;
  out_texture->id = 0u;
  out_texture->generation = 0u;

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.create_texture");
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3GTK4Texture),
                                (void **)&texture);
  M3_GTK4_RETURN_IF_ERROR(rc);

  memset(texture, 0, sizeof(*texture));
  texture->backend = backend;
  texture->width = width;
  texture->height = height;
  texture->format = format;
  texture->bytes_per_pixel = (format == M3_TEX_FORMAT_A8) ? 1u : 4u;

  cairo_format =
      (format == M3_TEX_FORMAT_A8) ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32;
  stride = cairo_format_stride_for_width(cairo_format, width);
  if (stride <= 0) {
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_RANGE;
  }
  texture->stride = stride;
  texture->size = (m3_usize)stride * (m3_usize)height;

  rc = backend->allocator.alloc(backend->allocator.ctx, texture->size,
                                (void **)&texture->pixels);
  M3_GTK4_RETURN_IF_ERROR_CLEANUP(
      rc, backend->allocator.free(backend->allocator.ctx, texture));
  memset(texture->pixels, 0, texture->size);

  surface = cairo_image_surface_create_for_data(texture->pixels, cairo_format,
                                                width, height, stride);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    backend->allocator.free(backend->allocator.ctx, texture->pixels);
    backend->allocator.free(backend->allocator.ctx, texture);
    return M3_ERR_OUT_OF_MEMORY;
  }
  texture->surface = surface;

  if (pixels != NULL && size != 0) {
    rc =
        m3_gtk4_copy_texture_region(texture, pixels, size, 0, 0, width, height);
    if (rc != M3_OK) {
      cairo_surface_destroy(texture->surface);
      backend->allocator.free(backend->allocator.ctx, texture->pixels);
      backend->allocator.free(backend->allocator.ctx, texture);
      return rc;
    }
  }

  rc = m3_object_header_init(&texture->header, M3_GTK4_TYPE_TEXTURE, 0,
                             &g_m3_gtk4_texture_vtable);
  if (rc != M3_OK) {
    cairo_surface_destroy(texture->surface);
    backend->allocator.free(backend->allocator.ctx, texture->pixels);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &texture->header);
  if (rc != M3_OK) {
    cairo_surface_destroy(texture->surface);
    backend->allocator.free(backend->allocator.ctx, texture->pixels);
    backend->allocator.free(backend->allocator.ctx, texture);
    return rc;
  }

  *out_texture = texture->header.handle;
  return M3_OK;
}

static int m3_gtk4_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x,
                                      m3_i32 y, m3_i32 width, m3_i32 height,
                                      const void *pixels, m3_usize size) {
  struct M3GTK4Backend *backend;
  M3GTK4Texture *resolved;
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

  backend = (struct M3GTK4Backend *)gfx;
  rc = m3_gtk4_backend_resolve(backend, texture, M3_GTK4_TYPE_TEXTURE,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.update_texture");
  M3_GTK4_RETURN_IF_ERROR(rc);
  rc = m3_gtk4_copy_texture_region(resolved, pixels, size, x, y, width, height);
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (resolved->gdk_texture != NULL) {
    g_object_unref(resolved->gdk_texture);
    resolved->gdk_texture = NULL;
  }
  return M3_OK;
}

static int m3_gtk4_gfx_destroy_texture(void *gfx, M3Handle texture) {
  struct M3GTK4Backend *backend;
  M3GTK4Texture *resolved;
  int rc;

  if (gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)gfx;
  rc = m3_gtk4_backend_resolve(backend, texture, M3_GTK4_TYPE_TEXTURE,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
  M3_GTK4_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_gtk4_gfx_draw_texture(void *gfx, M3Handle texture,
                                    const M3Rect *src, const M3Rect *dst,
                                    M3Scalar opacity) {
  struct M3GTK4Backend *backend;
  M3GTK4Texture *resolved;
  double scale_x;
  double scale_y;
  M3GTK4Cmd cmd;
  M3Rect transformed;
  int rc;

  if (gfx == NULL || src == NULL || dst == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (opacity < 0.0f || opacity > 1.0f) {
    return M3_ERR_RANGE;
  }

  backend = (struct M3GTK4Backend *)gfx;
  rc = m3_gtk4_backend_resolve(backend, texture, M3_GTK4_TYPE_TEXTURE,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (resolved->surface == NULL) {
    return M3_ERR_STATE;
  }
  if (src->width <= 0.0f || src->height <= 0.0f || dst->width <= 0.0f ||
      dst->height <= 0.0f) {
    return M3_ERR_RANGE;
  }
  if (src->x < 0.0f || src->y < 0.0f ||
      src->x + src->width > (M3Scalar)resolved->width ||
      src->y + src->height > (M3Scalar)resolved->height) {
    return M3_ERR_RANGE;
  }

  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }
    rc = m3_gtk4_transform_rect_simple(&backend->gsk_transform, dst,
                                       &transformed);
    M3_GTK4_RETURN_IF_ERROR(rc);
    cmd.type = M3_GTK4_CMD_TEXTURE;
    cmd.data.texture.texture = texture;
    cmd.data.texture.src = *src;
    cmd.data.texture.dst = transformed;
    cmd.data.texture.opacity = opacity;
    rc = m3_gtk4_cmd_list_push(backend->active_window, &cmd);
    M3_GTK4_RETURN_IF_ERROR(rc);
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_texture");
  M3_GTK4_RETURN_IF_ERROR(rc);

  scale_x = (double)dst->width / (double)src->width;
  scale_y = (double)dst->height / (double)src->height;

  cairo_save(backend->frame_cr);
  cairo_rectangle(backend->frame_cr, dst->x, dst->y, dst->width, dst->height);
  cairo_clip(backend->frame_cr);
  cairo_translate(backend->frame_cr, dst->x, dst->y);
  cairo_scale(backend->frame_cr, scale_x, scale_y);
  cairo_set_source_surface(backend->frame_cr, resolved->surface, -src->x,
                           -src->y);
  cairo_paint_with_alpha(backend->frame_cr, (double)opacity);
  cairo_restore(backend->frame_cr);
  return M3_OK;
}

static const M3GfxVTable g_m3_gtk4_gfx_vtable = {
    m3_gtk4_gfx_begin_frame,    m3_gtk4_gfx_end_frame,
    m3_gtk4_gfx_clear,          m3_gtk4_gfx_draw_rect,
    m3_gtk4_gfx_draw_line,      m3_gtk4_gfx_draw_path,
    m3_gtk4_gfx_push_clip,      m3_gtk4_gfx_pop_clip,
    m3_gtk4_gfx_set_transform,  m3_gtk4_gfx_create_texture,
    m3_gtk4_gfx_update_texture, m3_gtk4_gfx_destroy_texture,
    m3_gtk4_gfx_draw_texture};

static PangoWeight m3_gtk4_pango_weight(m3_i32 weight) {
  if (weight <= 200) {
    return PANGO_WEIGHT_THIN;
  }
  if (weight <= 300) {
    return PANGO_WEIGHT_LIGHT;
  }
  if (weight <= 400) {
    return PANGO_WEIGHT_NORMAL;
  }
  if (weight <= 500) {
    return PANGO_WEIGHT_MEDIUM;
  }
  if (weight <= 600) {
    return PANGO_WEIGHT_SEMIBOLD;
  }
  if (weight <= 700) {
    return PANGO_WEIGHT_BOLD;
  }
  if (weight <= 800) {
    return PANGO_WEIGHT_HEAVY;
  }
  return PANGO_WEIGHT_ULTRABOLD;
}

static int m3_gtk4_text_create_font(void *text, const char *utf8_family,
                                    m3_i32 size_px, m3_i32 weight,
                                    M3Bool italic, M3Handle *out_font) {
  struct M3GTK4Backend *backend;
  M3GTK4Font *font;
  PangoFontDescription *desc;
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

  backend = (struct M3GTK4Backend *)text;
  out_font->id = 0u;
  out_font->generation = 0u;

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.create_font");
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3GTK4Font),
                                (void **)&font);
  M3_GTK4_RETURN_IF_ERROR(rc);

  memset(font, 0, sizeof(*font));
  font->backend = backend;
  font->size_px = size_px;
  font->weight = weight;
  font->italic = italic ? M3_TRUE : M3_FALSE;

  desc = pango_font_description_new();
  if (desc == NULL) {
    backend->allocator.free(backend->allocator.ctx, font);
    return M3_ERR_OUT_OF_MEMORY;
  }
  pango_font_description_set_family(desc, utf8_family);
  pango_font_description_set_size(desc, size_px * PANGO_SCALE);
  pango_font_description_set_weight(desc, m3_gtk4_pango_weight(weight));
  pango_font_description_set_style(desc, italic ? PANGO_STYLE_ITALIC
                                                : PANGO_STYLE_NORMAL);
  font->desc = desc;

  rc = m3_object_header_init(&font->header, M3_GTK4_TYPE_FONT, 0,
                             &g_m3_gtk4_font_vtable);
  if (rc != M3_OK) {
    pango_font_description_free(desc);
    backend->allocator.free(backend->allocator.ctx, font);
    return rc;
  }

  rc = backend->handles.vtable->register_object(backend->handles.ctx,
                                                &font->header);
  if (rc != M3_OK) {
    pango_font_description_free(desc);
    backend->allocator.free(backend->allocator.ctx, font);
    return rc;
  }

  *out_font = font->header.handle;
  return M3_OK;
}

static int m3_gtk4_text_destroy_font(void *text, M3Handle font) {
  struct M3GTK4Backend *backend;
  M3GTK4Font *resolved;
  int rc;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)text;
  rc = m3_gtk4_backend_resolve(backend, font, M3_GTK4_TYPE_FONT,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.destroy_font");
  M3_GTK4_RETURN_IF_ERROR(rc);

  return m3_object_release(&resolved->header);
}

static int m3_gtk4_text_measure_text(void *text, M3Handle font,
                                     const char *utf8, m3_usize utf8_len,
                                     M3Scalar *out_width, M3Scalar *out_height,
                                     M3Scalar *out_baseline) {
  struct M3GTK4Backend *backend;
  M3GTK4Font *resolved;
  cairo_surface_t *surface;
  cairo_t *cr;
  PangoLayout *layout;
  int width_px;
  int height_px;
  int baseline;
  int rc;

  if (text == NULL || out_width == NULL || out_height == NULL ||
      out_baseline == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)text;
  rc = m3_gtk4_backend_resolve(backend, font, M3_GTK4_TYPE_FONT,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.measure_text");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (resolved->desc == NULL) {
    return M3_ERR_STATE;
  }
  if (utf8_len > (m3_usize)G_MAXINT) {
    return M3_ERR_RANGE;
  }

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return M3_ERR_UNKNOWN;
  }

  layout = pango_cairo_create_layout(cr);
  if (layout == NULL) {
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return M3_ERR_OUT_OF_MEMORY;
  }

  pango_layout_set_font_description(layout, resolved->desc);
  pango_layout_set_text(layout, (utf8 != NULL) ? utf8 : "", (int)utf8_len);
  pango_layout_get_pixel_size(layout, &width_px, &height_px);
  baseline = pango_layout_get_baseline(layout);

  *out_width = (M3Scalar)width_px;
  *out_height = (M3Scalar)height_px;
  *out_baseline = (M3Scalar)baseline / (M3Scalar)PANGO_SCALE;

  g_object_unref(layout);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return M3_OK;
}

static int m3_gtk4_text_draw_text(void *text, M3Handle font, const char *utf8,
                                  m3_usize utf8_len, M3Scalar x, M3Scalar y,
                                  M3Color color) {
  struct M3GTK4Backend *backend;
  M3GTK4Font *resolved;
  PangoLayout *layout;
  M3GTK4Cmd cmd;
  M3Scalar tx;
  M3Scalar ty;
  void *mem;
  int rc;

  if (text == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (utf8 == NULL && utf8_len != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)text;
  rc = m3_gtk4_backend_resolve(backend, font, M3_GTK4_TYPE_FONT,
                               (void **)&resolved);
  M3_GTK4_RETURN_IF_ERROR(rc);

  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.draw_text");
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (backend->frame_use_gsk) {
    if (!backend->in_frame || backend->active_window == NULL) {
      return M3_ERR_STATE;
    }
    if (!backend->gsk_transform_simple) {
      return M3_ERR_UNSUPPORTED;
    }
    rc =
        m3_gtk4_transform_point_simple(&backend->gsk_transform, x, y, &tx, &ty);
    M3_GTK4_RETURN_IF_ERROR(rc);

    cmd.type = M3_GTK4_CMD_TEXT;
    cmd.data.text.font = font;
    cmd.data.text.x = tx;
    cmd.data.text.y = ty;
    cmd.data.text.color = color;
    cmd.data.text.len = utf8_len;
    cmd.data.text.utf8 = NULL;

    if (utf8_len > 0) {
      rc =
          backend->allocator.alloc(backend->allocator.ctx, utf8_len + 1u, &mem);
      M3_GTK4_RETURN_IF_ERROR(rc);
      memcpy(mem, utf8, utf8_len);
      ((char *)mem)[utf8_len] = '\0';
      cmd.data.text.utf8 = (char *)mem;
    }

    rc = m3_gtk4_cmd_list_push(backend->active_window, &cmd);
    if (rc != M3_OK) {
      if (cmd.data.text.utf8 != NULL) {
        backend->allocator.free(backend->allocator.ctx, cmd.data.text.utf8);
      }
      return rc;
    }
    return M3_OK;
  }
  if (backend->frame_cr == NULL) {
    return M3_ERR_STATE;
  }

  if (resolved->desc == NULL) {
    return M3_ERR_STATE;
  }
  if (utf8_len > (m3_usize)G_MAXINT) {
    return M3_ERR_RANGE;
  }

  layout = pango_cairo_create_layout(backend->frame_cr);
  if (layout == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  pango_layout_set_font_description(layout, resolved->desc);
  pango_layout_set_text(layout, (utf8 != NULL) ? utf8 : "", (int)utf8_len);

  cairo_save(backend->frame_cr);
  m3_gtk4_set_source_color(backend->frame_cr, color);
  cairo_move_to(backend->frame_cr, (double)x, (double)y);
  pango_cairo_show_layout(backend->frame_cr, layout);
  cairo_restore(backend->frame_cr);

  g_object_unref(layout);
  return M3_OK;
}

static const M3TextVTable g_m3_gtk4_text_vtable = {
    m3_gtk4_text_create_font, m3_gtk4_text_destroy_font,
    m3_gtk4_text_measure_text, m3_gtk4_text_draw_text};

static int m3_gtk4_io_read_file(void *io, const char *utf8_path, void *buffer,
                                m3_usize buffer_size, m3_usize *out_read) {
  struct M3GTK4Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_read == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (buffer == NULL && buffer_size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)io;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_read = 0;
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_io_read_file_alloc(void *io, const char *utf8_path,
                                      const M3Allocator *allocator,
                                      void **out_data, m3_usize *out_size) {
  struct M3GTK4Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || allocator == NULL ||
      out_data == NULL || out_size == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (allocator->alloc == NULL || allocator->realloc == NULL ||
      allocator->free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)io;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file_alloc");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_data = NULL;
  *out_size = 0;
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_io_write_file(void *io, const char *utf8_path,
                                 const void *data, m3_usize size,
                                 M3Bool overwrite) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(overwrite);

  if (io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (data == NULL && size != 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)io;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.write_file");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_io_file_exists(void *io, const char *utf8_path,
                                  M3Bool *out_exists) {
  struct M3GTK4Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_exists == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)io;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.file_exists");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_exists = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_io_delete_file(void *io, const char *utf8_path) {
  struct M3GTK4Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)io;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.delete_file");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_io_stat_file(void *io, const char *utf8_path,
                                M3FileInfo *out_info) {
  struct M3GTK4Backend *backend;
  int rc;

  if (io == NULL || utf8_path == NULL || out_info == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)io;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.stat_file");
  M3_GTK4_RETURN_IF_ERROR(rc);

  memset(out_info, 0, sizeof(*out_info));
  return M3_ERR_UNSUPPORTED;
}

static const M3IOVTable g_m3_gtk4_io_vtable = {
    m3_gtk4_io_read_file,   m3_gtk4_io_read_file_alloc, m3_gtk4_io_write_file,
    m3_gtk4_io_file_exists, m3_gtk4_io_delete_file,     m3_gtk4_io_stat_file};

static int m3_gtk4_sensors_is_available(void *sensors, m3_u32 type,
                                        M3Bool *out_available) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL || out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)sensors;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.is_available");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_available = M3_FALSE;
  return M3_OK;
}

static int m3_gtk4_sensors_start(void *sensors, m3_u32 type) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)sensors;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.start");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_sensors_stop(void *sensors, m3_u32 type) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)sensors;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.stop");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_sensors_read(void *sensors, m3_u32 type,
                                M3SensorReading *out_reading,
                                M3Bool *out_has_reading) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(type);

  if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)sensors;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.read");
  M3_GTK4_RETURN_IF_ERROR(rc);

  memset(out_reading, 0, sizeof(*out_reading));
  *out_has_reading = M3_FALSE;
  return M3_OK;
}

static const M3SensorsVTable g_m3_gtk4_sensors_vtable = {
    m3_gtk4_sensors_is_available, m3_gtk4_sensors_start, m3_gtk4_sensors_stop,
    m3_gtk4_sensors_read};

static int m3_gtk4_camera_path_from_id(m3_u32 camera_id, char *out_path,
                                       m3_usize path_capacity) {
  const char prefix[] = "/dev/video";
  char digits[10];
  m3_u32 value;
  m3_usize prefix_len;
  m3_usize digit_count;
  m3_usize i;

  if (out_path == NULL || path_capacity == 0u) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  prefix_len = (m3_usize)(sizeof(prefix) - 1u);
  value = camera_id;
  digit_count = 0u;
  do {
    if (digit_count >= (m3_usize)M3_COUNTOF(digits)) {
      return M3_ERR_RANGE;
    }
    digits[digit_count] = (char)('0' + (value % 10u));
    digit_count += 1u;
    value /= 10u;
  } while (value > 0u);

  if (path_capacity < prefix_len + digit_count + 1u) {
    return M3_ERR_RANGE;
  }

  memcpy(out_path, prefix, (size_t)prefix_len);
  for (i = 0u; i < digit_count; i += 1u) {
    out_path[prefix_len + i] = digits[digit_count - 1u - i];
  }
  out_path[prefix_len + digit_count] = '\0';
  return M3_OK;
}

#if M3_GTK4_V4L2_AVAILABLE
static int m3_gtk4_camera_select_id(const M3CameraConfig *config,
                                    m3_u32 *out_id) {
  if (config == NULL || out_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (config->camera_id != M3_CAMERA_ID_DEFAULT) {
    *out_id = config->camera_id;
    return M3_OK;
  }

  switch (config->facing) {
  case M3_CAMERA_FACING_FRONT:
    *out_id = 1u;
    break;
  case M3_CAMERA_FACING_EXTERNAL:
    *out_id = 2u;
    break;
  case M3_CAMERA_FACING_BACK:
  case M3_CAMERA_FACING_UNSPECIFIED:
  default:
    *out_id = 0u;
    break;
  }

  return M3_OK;
}

static int m3_gtk4_camera_map_errno(int err) {
  if (err == EACCES) {
    return M3_ERR_PERMISSION;
  }
  if (err == ENOENT) {
    return M3_ERR_NOT_FOUND;
  }
  if (err == EBUSY) {
    return M3_ERR_BUSY;
  }
  return M3_ERR_IO;
}

static int m3_gtk4_camera_v4l2_from_format(m3_u32 format, m3_u32 *out_format) {
  if (out_format == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (format) {
  case M3_CAMERA_FORMAT_NV12:
    *out_format = V4L2_PIX_FMT_NV12;
    return M3_OK;
  case M3_CAMERA_FORMAT_BGRA8:
    *out_format = V4L2_PIX_FMT_BGR32;
    return M3_OK;
  case M3_CAMERA_FORMAT_RGBA8:
    *out_format = V4L2_PIX_FMT_RGB32;
    return M3_OK;
  default:
    return M3_ERR_UNSUPPORTED;
  }
}

static int m3_gtk4_camera_format_from_v4l2(m3_u32 v4l2_format,
                                           m3_u32 *out_format) {
  if (out_format == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  switch (v4l2_format) {
  case V4L2_PIX_FMT_NV12:
    *out_format = M3_CAMERA_FORMAT_NV12;
    return M3_OK;
  case V4L2_PIX_FMT_BGR32:
    *out_format = M3_CAMERA_FORMAT_BGRA8;
    return M3_OK;
  case V4L2_PIX_FMT_RGB32:
    *out_format = M3_CAMERA_FORMAT_RGBA8;
    return M3_OK;
  default:
    return M3_ERR_UNSUPPORTED;
  }
}

static int m3_gtk4_camera_try_format(int fd, m3_u32 width, m3_u32 height,
                                     m3_u32 pixfmt,
                                     struct v4l2_format *out_fmt) {
  struct v4l2_format fmt;

  if (out_fmt == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = width;
  fmt.fmt.pix.height = height;
  fmt.fmt.pix.pixelformat = pixfmt;
  fmt.fmt.pix.field = V4L2_FIELD_ANY;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) != 0) {
    return M3_ERR_UNSUPPORTED;
  }

  *out_fmt = fmt;
  return M3_OK;
}

static int m3_gtk4_camera_set_format(int fd, m3_u32 width, m3_u32 height,
                                     m3_u32 format,
                                     struct v4l2_format *out_fmt) {
  int rc;
  m3_u32 v4l2_format;

  if (format == M3_CAMERA_FORMAT_ANY) {
    rc = m3_gtk4_camera_try_format(fd, width, height, V4L2_PIX_FMT_NV12,
                                   out_fmt);
    if (rc == M3_OK) {
      return M3_OK;
    }

    rc = m3_gtk4_camera_try_format(fd, width, height, V4L2_PIX_FMT_BGR32,
                                   out_fmt);
    if (rc == M3_OK) {
      return M3_OK;
    }

    rc = m3_gtk4_camera_try_format(fd, width, height, V4L2_PIX_FMT_RGB32,
                                   out_fmt);
    if (rc == M3_OK) {
      return M3_OK;
    }

    return M3_ERR_UNSUPPORTED;
  }

  rc = m3_gtk4_camera_v4l2_from_format(format, &v4l2_format);
  if (rc != M3_OK) {
    return rc;
  }

  return m3_gtk4_camera_try_format(fd, width, height, v4l2_format, out_fmt);
}

static int m3_gtk4_camera_release(struct M3GTK4Backend *backend) {
  int rc;
  int close_rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = M3_OK;
  if (backend->camera_state.frame != NULL) {
    if (backend->allocator.free == NULL) {
      rc = M3_ERR_INVALID_ARGUMENT;
    } else {
      close_rc = backend->allocator.free(backend->allocator.ctx,
                                         backend->camera_state.frame);
      if (close_rc != M3_OK && rc == M3_OK) {
        rc = close_rc;
      }
    }
    backend->camera_state.frame = NULL;
    backend->camera_state.frame_capacity = 0u;
    backend->camera_state.frame_size = 0u;
  }

  if (backend->camera_state.fd >= 0) {
    if (close(backend->camera_state.fd) != 0 && rc == M3_OK) {
      rc = m3_gtk4_camera_map_errno(errno);
    }
    backend->camera_state.fd = -1;
  }

  backend->camera_state.opened = M3_FALSE;
  backend->camera_state.streaming = M3_FALSE;
  backend->camera_state.width = 0u;
  backend->camera_state.height = 0u;
  backend->camera_state.format = 0u;
  backend->camera_state.camera_id = 0u;
  return rc;
}
#endif

static int m3_gtk4_camera_open_with_config(void *camera,
                                           const M3CameraConfig *config);

static int m3_gtk4_camera_open(void *camera, m3_u32 camera_id) {
  M3CameraConfig config;

  config.camera_id = camera_id;
  config.facing = M3_CAMERA_FACING_UNSPECIFIED;
  config.width = 0u;
  config.height = 0u;
  config.format = M3_CAMERA_FORMAT_ANY;

  return m3_gtk4_camera_open_with_config(camera, &config);
}

static int m3_gtk4_camera_open_with_config(void *camera,
                                           const M3CameraConfig *config) {
  struct M3GTK4Backend *backend;
  int rc;

  if (camera == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)camera;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.open");
  M3_GTK4_RETURN_IF_ERROR(rc);

#if M3_GTK4_V4L2_AVAILABLE
  {
    char device_path[32];
    struct v4l2_capability caps;
    struct v4l2_format fmt;
    unsigned char *frame;
    m3_u32 m3_format;
    m3_u32 camera_id;
    m3_u32 width;
    m3_u32 height;
    int fd;
    int err;

    if (backend->camera_state.opened == M3_TRUE) {
      return M3_ERR_STATE;
    }

    rc = m3_gtk4_camera_select_id(config, &camera_id);
    if (rc != M3_OK) {
      return rc;
    }

    width =
        (config->width == 0u) ? M3_GTK4_CAMERA_DEFAULT_WIDTH : config->width;
    height =
        (config->height == 0u) ? M3_GTK4_CAMERA_DEFAULT_HEIGHT : config->height;

    rc = m3_gtk4_camera_path_from_id(camera_id, device_path,
                                     (m3_usize)sizeof(device_path));
    if (rc != M3_OK) {
      return rc;
    }

    fd = open(device_path, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
      return m3_gtk4_camera_map_errno(errno);
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &caps) != 0) {
      err = errno;
      close(fd);
      return m3_gtk4_camera_map_errno(err);
    }

    if ((caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0u) {
      close(fd);
      return M3_ERR_UNSUPPORTED;
    }
    if ((caps.capabilities & V4L2_CAP_READWRITE) == 0u) {
      close(fd);
      return M3_ERR_UNSUPPORTED;
    }

    rc = m3_gtk4_camera_set_format(fd, width, height, config->format, &fmt);
    if (rc != M3_OK) {
      close(fd);
      return rc;
    }

    rc = m3_gtk4_camera_format_from_v4l2(fmt.fmt.pix.pixelformat, &m3_format);
    if (rc != M3_OK) {
      close(fd);
      return rc;
    }
    if (config->format != M3_CAMERA_FORMAT_ANY && m3_format != config->format) {
      close(fd);
      return M3_ERR_UNSUPPORTED;
    }
    if (fmt.fmt.pix.sizeimage == 0u) {
      close(fd);
      return M3_ERR_UNSUPPORTED;
    }

    if (backend->allocator.alloc == NULL || backend->allocator.free == NULL) {
      close(fd);
      return M3_ERR_INVALID_ARGUMENT;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx,
                                  (m3_usize)fmt.fmt.pix.sizeimage,
                                  (void **)&frame);
    if (rc != M3_OK) {
      close(fd);
      return rc;
    }

    backend->camera_state.fd = fd;
    backend->camera_state.camera_id = camera_id;
    backend->camera_state.width = fmt.fmt.pix.width;
    backend->camera_state.height = fmt.fmt.pix.height;
    backend->camera_state.format = m3_format;
    backend->camera_state.frame = frame;
    backend->camera_state.frame_capacity = (m3_usize)fmt.fmt.pix.sizeimage;
    backend->camera_state.frame_size = 0u;
    backend->camera_state.opened = M3_TRUE;
    backend->camera_state.streaming = M3_FALSE;
    return M3_OK;
  }
#else
  M3_UNUSED(config);
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_gtk4_camera_close(void *camera) {
  struct M3GTK4Backend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)camera;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.close");
  M3_GTK4_RETURN_IF_ERROR(rc);

#if M3_GTK4_V4L2_AVAILABLE
  if (backend->camera_state.opened != M3_TRUE) {
    return M3_ERR_STATE;
  }

  if (backend->camera_state.streaming == M3_TRUE) {
    rc = m3_gtk4_camera_stop(camera);
    if (rc != M3_OK) {
      return rc;
    }
  }

  return m3_gtk4_camera_release(backend);
#else
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_gtk4_camera_start(void *camera) {
  struct M3GTK4Backend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)camera;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.start");
  M3_GTK4_RETURN_IF_ERROR(rc);
#if M3_GTK4_V4L2_AVAILABLE
  if (backend->camera_state.opened != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (backend->camera_state.streaming == M3_TRUE) {
    return M3_ERR_STATE;
  }

  backend->camera_state.streaming = M3_TRUE;
  return M3_OK;
#else
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_gtk4_camera_stop(void *camera) {
  struct M3GTK4Backend *backend;
  int rc;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)camera;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.stop");
  M3_GTK4_RETURN_IF_ERROR(rc);
#if M3_GTK4_V4L2_AVAILABLE
  if (backend->camera_state.opened != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (backend->camera_state.streaming != M3_TRUE) {
    return M3_ERR_STATE;
  }

  backend->camera_state.streaming = M3_FALSE;
  return M3_OK;
#else
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_gtk4_camera_read_frame(void *camera, M3CameraFrame *out_frame,
                                     M3Bool *out_has_frame) {
  struct M3GTK4Backend *backend;
  int rc;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)camera;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.read_frame");
  M3_GTK4_RETURN_IF_ERROR(rc);
#if M3_GTK4_V4L2_AVAILABLE
  {
    ssize_t bytes_read;
    int err;

    if (backend->camera_state.opened != M3_TRUE) {
      return M3_ERR_STATE;
    }
    if (backend->camera_state.streaming != M3_TRUE) {
      return M3_ERR_STATE;
    }
    if (backend->camera_state.fd < 0 || backend->camera_state.frame == NULL) {
      return M3_ERR_STATE;
    }

    bytes_read = read(backend->camera_state.fd, backend->camera_state.frame,
                      (size_t)backend->camera_state.frame_capacity);
    if (bytes_read < 0) {
      err = errno;
      if (err == EAGAIN || err == EWOULDBLOCK) {
        memset(out_frame, 0, sizeof(*out_frame));
        *out_has_frame = M3_FALSE;
        return M3_OK;
      }
      return m3_gtk4_camera_map_errno(err);
    }
    if (bytes_read == 0) {
      memset(out_frame, 0, sizeof(*out_frame));
      *out_has_frame = M3_FALSE;
      return M3_OK;
    }

    backend->camera_state.frame_size = (m3_usize)bytes_read;
    out_frame->format = backend->camera_state.format;
    out_frame->width = backend->camera_state.width;
    out_frame->height = backend->camera_state.height;
    out_frame->data = backend->camera_state.frame;
    out_frame->size = backend->camera_state.frame_size;
    *out_has_frame = M3_TRUE;
    return M3_OK;
  }
#else
  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = M3_FALSE;
  return M3_ERR_UNSUPPORTED;
#endif
}

static const M3CameraVTable g_m3_gtk4_camera_vtable = {
    m3_gtk4_camera_open,  m3_gtk4_camera_open_with_config,
    m3_gtk4_camera_close, m3_gtk4_camera_start,
    m3_gtk4_camera_stop,  m3_gtk4_camera_read_frame};

#if defined(M3_LIBCURL_AVAILABLE)
typedef struct M3GTK4CurlBuffer {
  const M3Allocator *allocator;
  void *data;
  m3_usize size;
  m3_usize capacity;
  int error;
} M3GTK4CurlBuffer;

static m3_u32 g_m3_gtk4_curl_refcount = 0u;

static int m3_gtk4_network_add_usize(m3_usize a, m3_usize b,
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

static int m3_gtk4_network_mul_usize(m3_usize a, m3_usize b,
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

static int m3_gtk4_network_error_from_curl(CURLcode code) {
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

static int m3_gtk4_libcurl_init(void) {
  CURLcode curl_rc;

  if (g_m3_gtk4_curl_refcount == 0u) {
    curl_rc = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_rc != CURLE_OK) {
      return m3_gtk4_network_error_from_curl(curl_rc);
    }
  }

  g_m3_gtk4_curl_refcount += 1u;
  return M3_OK;
}

static int m3_gtk4_libcurl_shutdown(void) {
  if (g_m3_gtk4_curl_refcount == 0u) {
    return M3_ERR_STATE;
  }

  g_m3_gtk4_curl_refcount -= 1u;
  if (g_m3_gtk4_curl_refcount == 0u) {
    curl_global_cleanup();
  }
  return M3_OK;
}

static size_t m3_gtk4_network_write_cb(char *ptr, size_t size, size_t nmemb,
                                       void *userdata) {
  M3GTK4CurlBuffer *buffer;
  m3_usize chunk;
  m3_usize new_size;
  m3_usize alloc_size;
  void *new_data;
  int rc;

  if (userdata == NULL || ptr == NULL) {
    return 0;
  }

  buffer = (M3GTK4CurlBuffer *)userdata;
  if (buffer->allocator == NULL) {
    buffer->error = M3_ERR_INVALID_ARGUMENT;
    return 0;
  }

  if (size == 0 || nmemb == 0) {
    return 0;
  }

  rc = m3_gtk4_network_mul_usize((m3_usize)size, (m3_usize)nmemb, &chunk);
  if (rc != M3_OK) {
    buffer->error = rc;
    return 0;
  }

  rc = m3_gtk4_network_add_usize(buffer->size, chunk, &new_size);
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

static int m3_gtk4_network_append_headers(struct M3GTK4Backend *backend,
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
      rc = m3_gtk4_network_add_usize(key_len, 1u, &line_len);
      if (rc != M3_OK) {
        *out_list = list;
        return rc;
      }
      if (value_len > 0u) {
        rc = m3_gtk4_network_add_usize(line_len, 1u, &line_len);
        if (rc != M3_OK) {
          *out_list = list;
          return rc;
        }
      }
      rc = m3_gtk4_network_add_usize(line_len, value_len, &line_len);
      if (rc != M3_OK) {
        *out_list = list;
        return rc;
      }
      rc = m3_gtk4_network_add_usize(line_len, 1u, &alloc_len);
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

static int m3_gtk4_network_request(void *net, const M3NetworkRequest *request,
                                   const M3Allocator *allocator,
                                   M3NetworkResponse *out_response) {
  struct M3GTK4Backend *backend;
#if defined(M3_LIBCURL_AVAILABLE)
  CURL *curl;
  CURLcode curl_rc;
  struct curl_slist *header_list;
  M3GTK4CurlBuffer buffer;
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

  backend = (struct M3GTK4Backend *)net;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
  M3_GTK4_RETURN_IF_ERROR(rc);

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
    rc = m3_gtk4_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request->method);
  if (curl_rc != CURLE_OK) {
    rc = m3_gtk4_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_USERAGENT, "LibM3C/1.0");
  if (curl_rc != CURLE_OK) {
    rc = m3_gtk4_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc =
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, m3_gtk4_network_write_cb);
  if (curl_rc != CURLE_OK) {
    rc = m3_gtk4_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  if (curl_rc != CURLE_OK) {
    rc = m3_gtk4_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  curl_rc = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  if (curl_rc != CURLE_OK) {
    rc = m3_gtk4_network_error_from_curl(curl_rc);
    goto cleanup;
  }

  if (request->timeout_ms > 0u) {
    timeout_ms = (long)request->timeout_ms;
    curl_rc = curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);
    if (curl_rc != CURLE_OK) {
      rc = m3_gtk4_network_error_from_curl(curl_rc);
      goto cleanup;
    }
    curl_rc = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout_ms);
    if (curl_rc != CURLE_OK) {
      rc = m3_gtk4_network_error_from_curl(curl_rc);
      goto cleanup;
    }
  }

  if (request->headers != NULL && request->headers[0] != '\0') {
    rc =
        m3_gtk4_network_append_headers(backend, request->headers, &header_list);
    if (rc != M3_OK) {
      goto cleanup;
    }
    if (header_list != NULL) {
      curl_rc = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
      if (curl_rc != CURLE_OK) {
        rc = m3_gtk4_network_error_from_curl(curl_rc);
        goto cleanup;
      }
    }
  }

  if (request->body_size > 0u) {
    body_len = (long)request->body_size;
    curl_rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
    if (curl_rc != CURLE_OK) {
      rc = m3_gtk4_network_error_from_curl(curl_rc);
      goto cleanup;
    }
    curl_rc = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_len);
    if (curl_rc != CURLE_OK) {
      rc = m3_gtk4_network_error_from_curl(curl_rc);
      goto cleanup;
    }
  }

  curl_rc = curl_easy_perform(curl);
  if (curl_rc != CURLE_OK) {
    if (buffer.error != M3_OK) {
      rc = buffer.error;
    } else {
      rc = m3_gtk4_network_error_from_curl(curl_rc);
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
    rc = m3_gtk4_network_error_from_curl(curl_rc);
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

static int m3_gtk4_network_free_response(void *net,
                                         const M3Allocator *allocator,
                                         M3NetworkResponse *response) {
  struct M3GTK4Backend *backend;
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

  backend = (struct M3GTK4Backend *)net;
  rc =
      m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.free_response");
  M3_GTK4_RETURN_IF_ERROR(rc);
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

static const M3NetworkVTable g_m3_gtk4_network_vtable = {
    m3_gtk4_network_request, m3_gtk4_network_free_response};

static int m3_gtk4_tasks_thread_create(void *tasks, M3ThreadFn entry,
                                       void *user, M3Handle *out_thread) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(entry);
  M3_UNUSED(user);

  if (tasks == NULL || out_thread == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_create");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_tasks_thread_join(void *tasks, M3Handle thread) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(thread);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_join");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_tasks_mutex_create(void *tasks, M3Handle *out_mutex) {
  struct M3GTK4Backend *backend;
  int rc;

  if (tasks == NULL || out_mutex == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_create");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_tasks_mutex_destroy(void *tasks, M3Handle mutex) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_tasks_mutex_lock(void *tasks, M3Handle mutex) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_tasks_mutex_unlock(void *tasks, M3Handle mutex) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(mutex);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
  M3_GTK4_RETURN_IF_ERROR(rc);
  return M3_ERR_UNSUPPORTED;
}

static int m3_gtk4_tasks_sleep_ms(void *tasks, m3_u32 ms) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(ms);

  if (tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return M3_ERR_UNSUPPORTED;
  }
  return M3_OK;
}

static int m3_gtk4_tasks_post(void *tasks, M3TaskFn fn, void *user) {
  struct M3GTK4Backend *backend;
  int rc;

  if (tasks == NULL || fn == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post");
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return M3_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static int m3_gtk4_tasks_post_delayed(void *tasks, M3TaskFn fn, void *user,
                                      m3_u32 delay_ms) {
  struct M3GTK4Backend *backend;
  int rc;

  M3_UNUSED(delay_ms);

  if (tasks == NULL || fn == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)tasks;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post_delayed");
  M3_GTK4_RETURN_IF_ERROR(rc);
  if (!backend->inline_tasks) {
    return M3_ERR_UNSUPPORTED;
  }
  return fn(user);
}

static const M3TasksVTable g_m3_gtk4_tasks_vtable = {
    m3_gtk4_tasks_thread_create, m3_gtk4_tasks_thread_join,
    m3_gtk4_tasks_mutex_create,  m3_gtk4_tasks_mutex_destroy,
    m3_gtk4_tasks_mutex_lock,    m3_gtk4_tasks_mutex_unlock,
    m3_gtk4_tasks_sleep_ms,      m3_gtk4_tasks_post,
    m3_gtk4_tasks_post_delayed};

static int m3_gtk4_env_get_io(void *env, M3IO *out_io) {
  struct M3GTK4Backend *backend;
  int rc;

  if (env == NULL || out_io == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)env;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_io");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_io = backend->io;
  return M3_OK;
}

static int m3_gtk4_env_get_sensors(void *env, M3Sensors *out_sensors) {
  struct M3GTK4Backend *backend;
  int rc;

  if (env == NULL || out_sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)env;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_sensors");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_sensors = backend->sensors;
  return M3_OK;
}

static int m3_gtk4_env_get_camera(void *env, M3Camera *out_camera) {
  struct M3GTK4Backend *backend;
  int rc;

  if (env == NULL || out_camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)env;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_camera");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_camera = backend->camera;
  return M3_OK;
}

static int m3_gtk4_env_get_network(void *env, M3Network *out_network) {
  struct M3GTK4Backend *backend;
  int rc;

  if (env == NULL || out_network == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)env;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_network");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_network = backend->network;
  return M3_OK;
}

static int m3_gtk4_env_get_tasks(void *env, M3Tasks *out_tasks) {
  struct M3GTK4Backend *backend;
  int rc;

  if (env == NULL || out_tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)env;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_tasks");
  M3_GTK4_RETURN_IF_ERROR(rc);

  *out_tasks = backend->tasks;
  return M3_OK;
}

static int m3_gtk4_env_get_time_ms(void *env, m3_u32 *out_time_ms) {
  struct M3GTK4Backend *backend;
  int rc;
  m3_u32 time_ms;

  if (env == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3GTK4Backend *)env;
  rc = m3_gtk4_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_time_ms");
  M3_GTK4_RETURN_IF_ERROR(rc);

  time_ms = m3_gtk4_get_time_ms();
  backend->time_ms = time_ms;
  *out_time_ms = time_ms;
  return M3_OK;
}

static const M3EnvVTable g_m3_gtk4_env_vtable = {
    m3_gtk4_env_get_io,      m3_gtk4_env_get_sensors, m3_gtk4_env_get_camera,
    m3_gtk4_env_get_network, m3_gtk4_env_get_tasks,   m3_gtk4_env_get_time_ms};

int M3_CALL m3_gtk4_backend_create(const M3GTK4BackendConfig *config,
                                   M3GTK4Backend **out_backend) {
  M3GTK4BackendConfig local_config;
  M3Allocator allocator;
  struct M3GTK4Backend *backend;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_gtk4_backend_config_init(&local_config);
    M3_GTK4_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_gtk4_backend_validate_config(config);
  M3_GTK4_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = m3_get_default_allocator(&allocator);
    M3_GTK4_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(M3GTK4Backend), (void **)&backend);
  M3_GTK4_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->camera_state.fd = -1;
  backend->log_enabled = config->enable_logging ? M3_TRUE : M3_FALSE;
  backend->inline_tasks = config->inline_tasks ? M3_TRUE : M3_FALSE;
  backend->clipboard_limit = config->clipboard_limit;
  backend->use_gsk_default = M3_TRUE;
  backend->use_gdk_texture = config->enable_gdk_texture ? M3_TRUE : M3_FALSE;

  if (backend->log_enabled) {
    rc = m3_log_init(&allocator);
    if (rc != M3_OK && rc != M3_ERR_STATE) {
      allocator.free(allocator.ctx, backend);
      return rc;
    }
    backend->log_owner = (rc == M3_OK) ? M3_TRUE : M3_FALSE;
  }

  rc = m3_gtk4_global_init(backend);
  if (rc != M3_OK) {
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }

#if defined(M3_LIBCURL_AVAILABLE)
  rc = m3_gtk4_libcurl_init();
  if (rc != M3_OK) {
    m3_gtk4_global_shutdown(backend);
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }
#endif

  rc = m3_handle_system_default_create(config->handle_capacity, &allocator,
                                       &backend->handles);
  if (rc != M3_OK) {
#if defined(M3_LIBCURL_AVAILABLE)
    m3_gtk4_libcurl_shutdown();
#endif
    m3_gtk4_global_shutdown(backend);
    if (backend->log_owner) {
      m3_log_shutdown();
    }
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  backend->ws.ctx = backend;
  backend->ws.vtable = &g_m3_gtk4_ws_vtable;
  backend->gfx.ctx = backend;
  backend->gfx.vtable = &g_m3_gtk4_gfx_vtable;
  backend->gfx.text_vtable = &g_m3_gtk4_text_vtable;
  backend->env.ctx = backend;
  backend->env.vtable = &g_m3_gtk4_env_vtable;
  backend->io.ctx = backend;
  backend->io.vtable = &g_m3_gtk4_io_vtable;
  backend->sensors.ctx = backend;
  backend->sensors.vtable = &g_m3_gtk4_sensors_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_m3_gtk4_camera_vtable;
  backend->network.ctx = backend;
  backend->network.vtable = &g_m3_gtk4_network_vtable;
  backend->tasks.ctx = backend;
  backend->tasks.vtable = &g_m3_gtk4_tasks_vtable;
  backend->event_head = 0u;
  backend->event_count = 0u;
  backend->clip_depth = 0u;
  backend->has_transform = M3_FALSE;
  backend->active_window = NULL;
  backend->frame_cr = NULL;
  backend->time_ms = m3_gtk4_get_time_ms();
  backend->initialized = M3_TRUE;

  *out_backend = backend;
  return M3_OK;
}

int M3_CALL m3_gtk4_backend_destroy(M3GTK4Backend *backend) {
  int rc;
  int first_error;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }

  rc = m3_handle_system_default_destroy(&backend->handles);
  M3_GTK4_RETURN_IF_ERROR(rc);

  first_error = M3_OK;

  if (backend->frame_cr != NULL) {
    cairo_destroy(backend->frame_cr);
    backend->frame_cr = NULL;
  }

  if (backend->clipboard != NULL) {
    rc = backend->allocator.free(backend->allocator.ctx, backend->clipboard);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
    backend->clipboard = NULL;
    backend->clipboard_capacity = 0;
    backend->clipboard_length = 0;
  }

#if M3_GTK4_V4L2_AVAILABLE
  if (backend->camera_state.opened == M3_TRUE ||
      backend->camera_state.frame != NULL || backend->camera_state.fd >= 0) {
    rc = m3_gtk4_camera_release(backend);
    if (rc != M3_OK && first_error == M3_OK) {
      first_error = rc;
    }
  }
#endif

#if defined(M3_LIBCURL_AVAILABLE)
  rc = m3_gtk4_libcurl_shutdown();
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }
#endif

  rc = m3_gtk4_global_shutdown(backend);
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }

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

int M3_CALL m3_gtk4_backend_get_ws(M3GTK4Backend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_ws = backend->ws;
  return M3_OK;
}

int M3_CALL m3_gtk4_backend_get_gfx(M3GTK4Backend *backend, M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_gfx = backend->gfx;
  return M3_OK;
}

int M3_CALL m3_gtk4_backend_get_env(M3GTK4Backend *backend, M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_env = backend->env;
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_gtk4_backend_test_set_initialized(M3GTK4Backend *backend,
                                                 M3Bool initialized) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend->initialized = initialized ? M3_TRUE : M3_FALSE;
  return M3_OK;
}
#endif

#else

int M3_CALL m3_gtk4_backend_create(const M3GTK4BackendConfig *config,
                                   M3GTK4Backend **out_backend) {
  M3GTK4BackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_gtk4_backend_config_init(&local_config);
    M3_GTK4_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_gtk4_backend_validate_config(config);
  M3_GTK4_RETURN_IF_ERROR(rc);

  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_gtk4_backend_destroy(M3GTK4Backend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_gtk4_backend_get_ws(M3GTK4Backend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_gtk4_backend_get_gfx(M3GTK4Backend *backend, M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_gtk4_backend_get_env(M3GTK4Backend *backend, M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return M3_ERR_UNSUPPORTED;
}

#endif
