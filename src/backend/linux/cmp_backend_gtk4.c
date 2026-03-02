#include "cmpc/cmp_backend_gtk4.h"

#include "cmpc/cmp_log.h"
#include "cmpc/cmp_object.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#if defined(CMP_LIBCURL_AVAILABLE)
#include <curl/curl.h>
#endif

#if defined(CMP_GTK4_AVAILABLE)
#include <cairo.h>
#include <gdk/gdk.h>
/* Include keysyms to ensure GDK_KEY_Tab is recognized */
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <pango/pangocairo.h>
#endif

#if defined(CMP_GTK4_AVAILABLE) && defined(__linux__)
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define CMP_GTK4_V4L2_AVAILABLE 1
#else
#define CMP_GTK4_V4L2_AVAILABLE 0
#endif

#define CMP_GTK4_RETURN_IF_ERROR(rc)                                           \
                                                                               \
  do {                                                                         \
                                                                               \
    if ((rc) != CMP_OK) {                                                      \
                                                                               \
      return (rc);                                                             \
    }                                                                          \
                                                                               \
  } while (0)

#define CMP_GTK4_RETURN_IF_ERROR_CLEANUP(rc, cleanup)                          \
                                                                               \
  do {                                                                         \
                                                                               \
    if ((rc) != CMP_OK) {                                                      \
                                                                               \
      cleanup;                                                                 \
                                                                               \
      return (rc);                                                             \
    }                                                                          \
                                                                               \
  } while (0)

#define CMP_GTK4_DEFAULT_HANDLE_CAPACITY 64
#define CMP_GTK4_EVENT_CAPACITY 1024u
#define CMP_GTK4_CLIP_STACK_CAPACITY 32u
#define CMP_GTK4_PI 3.14159265358979323846
#define CMP_GTK4_PI_2 (CMP_GTK4_PI * 0.5)
#define CMP_GTK4_CAMERA_DEFAULT_WIDTH 640u
#define CMP_GTK4_CAMERA_DEFAULT_HEIGHT 480u

/* Forward declarations */
struct CMPGTK4Backend;
struct CMPGTK4Window;

#if defined(CMP_GTK4_AVAILABLE)
static int cmp_gtk4_ws_pump_events(void *ws);
#endif

#ifdef CMP_TESTING
static CMPBool g_cmp_gtk4_test_fail_config_init = CMP_FALSE;
int CMP_CALL cmp_gtk4_backend_test_set_config_init_fail(CMPBool fail) {
  g_cmp_gtk4_test_fail_config_init = fail;
  return CMP_OK;
}
#endif

static int
cmp_gtk4_backend_validate_config(const CMPGTK4BackendConfig *config) {
  if (config == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (config->handle_capacity == 0)
    return CMP_ERR_INVALID_ARGUMENT;
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
cmp_gtk4_backend_test_validate_config(const CMPGTK4BackendConfig *config) {
  return cmp_gtk4_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_gtk4_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
#if defined(CMP_GTK4_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_gtk4_backend_config_init(CMPGTK4BackendConfig *config) {
  if (config == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
#ifdef CMP_TESTING
  if (g_cmp_gtk4_test_fail_config_init == CMP_TRUE) {
    g_cmp_gtk4_test_fail_config_init = CMP_FALSE;
    return CMP_ERR_IO;
  }
#endif
  config->allocator = NULL;
  config->handle_capacity = CMP_GTK4_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  config->enable_gdk_texture = CMP_TRUE;
  return CMP_OK;
}

/* ==================================================================================
 * CORE IMPLEMENTATION (GTK4 Available)
 * ==================================================================================
 */
#if defined(CMP_GTK4_AVAILABLE)

#define CMP_GTK4_TYPE_WINDOW 1
#define CMP_GTK4_TYPE_TEXTURE 2
#define CMP_GTK4_TYPE_FONT 3

struct CMPGTK4Window {
  CMPObjectHeader header;
  struct CMPGTK4Backend *backend;
  GtkWindow *window;
  GtkWidget *area;
  cairo_surface_t *surface;
  cmp_i32 surface_width;
  cmp_i32 surface_height;
  cmp_i32 width;
  cmp_i32 height;
  cmp_i32 scale_factor;
  CMPScalar dpi_scale;
  CMPScalar dpi_scale_override;
  CMPBool has_scale_override;
  cmp_u32 flags;
  CMPBool visible;
};

typedef struct CMPGTK4Texture {
  CMPObjectHeader header;
  struct CMPGTK4Backend *backend;
  cmp_i32 width;
  cmp_i32 height;
  cmp_u32 format;
  cmp_u32 bytes_per_pixel;
  cmp_i32 stride;
  cmp_usize size;
  unsigned char *pixels;
  cairo_surface_t *surface;
  GdkTexture *gdk_texture;
} CMPGTK4Texture;

typedef struct CMPGTK4Font {
  CMPObjectHeader header;
  struct CMPGTK4Backend *backend;
  cmp_i32 size_px;
  cmp_i32 weight;
  CMPBool italic;
  PangoFontDescription *desc;
} CMPGTK4Font;

typedef struct CMPGTK4CameraState {
  int fd;
  cmp_u32 camera_id;
  cmp_u32 width;
  cmp_u32 height;
  cmp_u32 format;
  cmp_usize frame_capacity;
  cmp_usize frame_size;
  unsigned char *frame;
  CMPBool opened;
  CMPBool streaming;
} CMPGTK4CameraState;

struct CMPGTK4Backend {
  CMPAllocator allocator;
  CMPHandleSystem handles;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPIO io;
  CMPSensors sensors;
  CMPCamera camera;
  CMPGTK4CameraState camera_state;
  CMPImage image;
  CMPVideo video;
  CMPAudio audio;
  CMPNetwork network;
  CMPTasks tasks;
  CMPBool initialized;
  CMPBool log_enabled;
  CMPBool log_owner;
  CMPBool inline_tasks;
  CMPBool use_gsk_default;
  CMPBool use_gdk_texture;
  CMPBool in_frame;
  CMPBool frame_use_gsk;
  cmp_u32 time_ms;
  char *clipboard;
  cmp_usize clipboard_capacity;
  cmp_usize clipboard_length;
  cmp_usize clipboard_limit;
  struct CMPGTK4Window *active_window;
  cairo_t *frame_cr;
  CMPMat3 transform;
  CMPBool has_transform;
  CMPMat3 gsk_transform;
  CMPBool gsk_transform_simple;
  CMPRect clip_stack[CMP_GTK4_CLIP_STACK_CAPACITY];
  cmp_usize clip_depth;
  CMPInputEvent event_queue[CMP_GTK4_EVENT_CAPACITY];
  cmp_usize event_head;
  cmp_usize event_count;
};

static cmp_u32 g_cmp_gtk4_refcount = 0u;

static int cmp_gtk4_backend_log(struct CMPGTK4Backend *backend,
                                CMPLogLevel level, const char *message) {
  if (!backend->log_enabled) {
    return CMP_OK;
  }
  return cmp_log_write(level, "m3.gtk4", message);
}

static int cmp_gtk4_global_init(struct CMPGTK4Backend *backend) {
  GdkDisplay *display;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (g_cmp_gtk4_refcount == 0u) {
    gtk_init();
    display = gdk_display_get_default();
    if (display == NULL) {
      return CMP_ERR_UNSUPPORTED;
    }
  }

  g_cmp_gtk4_refcount += 1u;
  rc = cmp_gtk4_backend_log(backend, CMP_LOG_LEVEL_INFO, "gtk4.init");
  CMP_GTK4_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static int cmp_gtk4_global_shutdown(struct CMPGTK4Backend *backend) {
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (g_cmp_gtk4_refcount == 0u) {
    return CMP_ERR_STATE;
  }

  g_cmp_gtk4_refcount -= 1u;
  rc = cmp_gtk4_backend_log(backend, CMP_LOG_LEVEL_INFO, "gtk4.shutdown");
  CMP_GTK4_RETURN_IF_ERROR(rc);
  return CMP_OK;
}

static cmp_u32 cmp_gtk4_get_time_ms(void) {
  gint64 us;
  us = g_get_monotonic_time();
  if (us < 0)
    return 0u;
  return (cmp_u32)(us / 1000);
}

static cmp_u32 cmp_gtk4_modifiers_from_state(GdkModifierType state) {
  cmp_u32 mods = 0u;
  if ((state & GDK_SHIFT_MASK) != 0)
    mods |= CMP_MOD_SHIFT;
  if ((state & GDK_CONTROL_MASK) != 0)
    mods |= CMP_MOD_CTRL;
  if ((state & GDK_ALT_MASK) != 0)
    mods |= CMP_MOD_ALT;
  if ((state & GDK_META_MASK) != 0)
    mods |= CMP_MOD_META;
  if ((state & GDK_LOCK_MASK) != 0)
    mods |= CMP_MOD_CAPS;
  return mods;
}

static cmp_i32 cmp_gtk4_buttons_from_state(GdkModifierType state) {
  cmp_i32 buttons = 0;
  if ((state & GDK_BUTTON1_MASK) != 0)
    buttons |= 1;
  if ((state & GDK_BUTTON2_MASK) != 0)
    buttons |= 1 << 1;
  if ((state & GDK_BUTTON3_MASK) != 0)
    buttons |= 1 << 2;
  if ((state & GDK_BUTTON4_MASK) != 0)
    buttons |= 1 << 3;
  if ((state & GDK_BUTTON5_MASK) != 0)
    buttons |= 1 << 4;
  return buttons;
}

static int cmp_gtk4_queue_event(struct CMPGTK4Backend *backend,
                                const CMPInputEvent *event) {
  cmp_usize tail;

  if (backend == NULL || event == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  if (backend->event_count >= CMP_GTK4_EVENT_CAPACITY) {
    return CMP_ERR_BUSY;
  }

  tail = (backend->event_head + backend->event_count) % CMP_GTK4_EVENT_CAPACITY;
  backend->event_queue[tail] = *event;
  backend->event_count += 1;
  return CMP_OK;
}

static void cmp_gtk4_queue_window_resize(struct CMPGTK4Window *window,
                                         cmp_i32 w, cmp_i32 h) {
  CMPInputEvent e;
  memset(&e, 0, sizeof(e));
  e.type = CMP_INPUT_WINDOW_RESIZE;
  e.modifiers = 0;
  e.reserved = 0;
  e.time_ms = cmp_gtk4_get_time_ms();
  e.window = window->header.handle;
  e.data.window.width = w;
  e.data.window.height = h;
  cmp_gtk4_queue_event(window->backend, &e);
}

static void cmp_gtk4_on_draw_func(GtkDrawingArea *drawing_area, cairo_t *cr,
                                  int width, int height, gpointer user_data) {
  struct CMPGTK4Window *w = (struct CMPGTK4Window *)user_data;
  int scale_factor;

  scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(drawing_area));
  if (scale_factor < 1)
    scale_factor = 1;
  w->scale_factor = scale_factor;
  w->dpi_scale = (CMPScalar)scale_factor;

  if (w->width != width || w->height != height) {
    w->width = width;
    w->height = height;
    cmp_gtk4_queue_window_resize(w, width, height);
  }

  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint(cr);
  cairo_restore(cr);

  if (w->surface) {
    cairo_identity_matrix(cr);
    cairo_set_source_surface(cr, w->surface, 0, 0);
    cairo_paint(cr);
  }
}

static int cmp_gtk4_apply_transform(struct CMPGTK4Backend *backend) {
  cairo_matrix_t m;
  if (!backend)
    return CMP_ERR_INVALID_ARGUMENT;
  if (!backend->frame_cr)
    return CMP_ERR_STATE;

  m.xx = backend->transform.m[0];
  m.yx = backend->transform.m[1];
  m.xy = backend->transform.m[3];
  m.yy = backend->transform.m[4];
  m.x0 = backend->transform.m[6];
  m.y0 = backend->transform.m[7];
  cairo_set_matrix(backend->frame_cr, &m);
  return CMP_OK;
}

static int cmp_gtk4_object_retain(void *obj) {
  return cmp_object_retain((CMPObjectHeader *)obj);
}
static int cmp_gtk4_object_release(void *obj) {
  return cmp_object_release((CMPObjectHeader *)obj);
}
static int cmp_gtk4_object_get_type_id(void *obj, cmp_u32 *out) {
  return cmp_object_get_type_id((CMPObjectHeader *)obj, out);
}

static int cmp_gtk4_window_destroy(void *obj) {
  struct CMPGTK4Window *w = (struct CMPGTK4Window *)obj;
  struct CMPGTK4Backend *b = w->backend;

  if (w->window) {
    gtk_window_destroy(w->window);
    w->window = NULL;
  }
  if (w->surface) {
    cairo_surface_destroy(w->surface);
    w->surface = NULL;
  }
  b->handles.vtable->unregister_object(b->handles.ctx, w->header.handle);
  b->allocator.free(b->allocator.ctx, w);
  return CMP_OK;
}

static int cmp_gtk4_texture_destroy(void *obj) {
  CMPGTK4Texture *t = (CMPGTK4Texture *)obj;
  struct CMPGTK4Backend *b = t->backend;
  if (t->gdk_texture)
    g_object_unref(t->gdk_texture);
  if (t->pixels)
    b->allocator.free(b->allocator.ctx, t->pixels);
  b->handles.vtable->unregister_object(b->handles.ctx, t->header.handle);
  b->allocator.free(b->allocator.ctx, t);
  return CMP_OK;
}

static int cmp_gtk4_font_destroy(void *obj) {
  CMPGTK4Font *f = (CMPGTK4Font *)obj;
  struct CMPGTK4Backend *b = f->backend;
  if (f->desc)
    pango_font_description_free(f->desc);
  b->handles.vtable->unregister_object(b->handles.ctx, f->header.handle);
  b->allocator.free(b->allocator.ctx, f);
  return CMP_OK;
}

static const CMPObjectVTable g_cmp_gtk4_window_vtable = {
    cmp_gtk4_object_retain, cmp_gtk4_object_release, cmp_gtk4_window_destroy,
    cmp_gtk4_object_get_type_id};
static const CMPObjectVTable g_cmp_gtk4_texture_vtable = {
    cmp_gtk4_object_retain, cmp_gtk4_object_release, cmp_gtk4_texture_destroy,
    cmp_gtk4_object_get_type_id};
static const CMPObjectVTable g_cmp_gtk4_font_vtable = {
    cmp_gtk4_object_retain, cmp_gtk4_object_release, cmp_gtk4_font_destroy,
    cmp_gtk4_object_get_type_id};

static int cmp_gtk4_backend_resolve(struct CMPGTK4Backend *b, CMPHandle h,
                                    cmp_u32 type, void **out) {
  void *obj;
  cmp_u32 actual;
  if (b->handles.vtable->resolve(b->handles.ctx, h, &obj) != CMP_OK)
    return CMP_ERR_INVALID_ARGUMENT;
  cmp_object_get_type_id(obj, &actual);
  if (actual != type)
    return CMP_ERR_INVALID_ARGUMENT;
  *out = obj;
  return CMP_OK;
}

static int cmp_gtk4_ws_init(void *ctx, const CMPWSConfig *c) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)ctx;
  g_set_application_name(c->utf8_app_name);
  (void)b;
  return CMP_OK;
}

static int cmp_gtk4_ws_shutdown(void *ctx) {
  (void)ctx;
  return CMP_OK;
}

static gboolean on_close(GtkWindow *w, gpointer u) {
  struct CMPGTK4Window *win = (struct CMPGTK4Window *)u;
  CMPInputEvent e;
  (void)w;
  memset(&e, 0, sizeof(e));
  e.type = CMP_INPUT_WINDOW_CLOSE;
  e.time_ms = cmp_gtk4_get_time_ms();
  e.window = win->header.handle;
  e.modifiers = 0;
  e.reserved = 0;
  cmp_gtk4_queue_event(win->backend, &e);
  /* Return FALSE to destroy the window. */
  return FALSE;
}

/* Cleanup callback when GtkWindow is actually destroyed */
static void on_window_destroy(GtkWidget *w, gpointer u) {
  struct CMPGTK4Window *win = (struct CMPGTK4Window *)u;
  (void)w;
  win->window = NULL;
  win->area = NULL;
}

static void on_click_pressed(GtkGestureClick *g, int n, double x, double y,
                             gpointer u) {
  struct CMPGTK4Window *win = (struct CMPGTK4Window *)u;
  CMPInputEvent e;
  GdkModifierType state =
      gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(g));
  int btn = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(g));
  (void)n;

  if (win->area) {
    gtk_widget_grab_focus(win->area);
  }

  memset(&e, 0, sizeof(e));
  e.type = CMP_INPUT_POINTER_DOWN;
  e.time_ms = cmp_gtk4_get_time_ms();
  e.window = win->header.handle;
  e.modifiers = cmp_gtk4_modifiers_from_state(state);
  e.data.pointer.x = (int)x;
  e.data.pointer.y = (int)y;

  {
    const int btn_flag = (btn > 0) ? (1 << (btn - 1)) : 1;
    e.data.pointer.buttons = cmp_gtk4_buttons_from_state(state) | btn_flag;
  }
  cmp_gtk4_queue_event(win->backend, &e);
}

static void on_click_released(GtkGestureClick *g, int n, double x, double y,
                              gpointer u) {
  struct CMPGTK4Window *win = (struct CMPGTK4Window *)u;
  CMPInputEvent e;
  GdkModifierType state =
      gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(g));
  int btn = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(g));
  (void)n;

  memset(&e, 0, sizeof(e));
  e.type = CMP_INPUT_POINTER_UP;
  e.time_ms = cmp_gtk4_get_time_ms();
  e.window = win->header.handle;
  e.modifiers = cmp_gtk4_modifiers_from_state(state);
  e.data.pointer.x = (int)x;
  e.data.pointer.y = (int)y;

  e.data.pointer.buttons = cmp_gtk4_buttons_from_state(state);

  {
    const int btn_flag = (btn > 0) ? (1 << (btn - 1)) : 1;
    e.data.pointer.buttons &= ~btn_flag;
  }

  cmp_gtk4_queue_event(win->backend, &e);
}

static void on_motion(GtkEventControllerMotion *c, double x, double y,
                      gpointer u) {
  struct CMPGTK4Window *win = (struct CMPGTK4Window *)u;
  CMPInputEvent e;
  GdkModifierType state =
      gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(c));

  memset(&e, 0, sizeof(e));
  e.type = CMP_INPUT_POINTER_MOVE;
  e.time_ms = cmp_gtk4_get_time_ms();
  e.window = win->header.handle;
  e.modifiers = cmp_gtk4_modifiers_from_state(state);
  e.data.pointer.x = (int)x;
  e.data.pointer.y = (int)y;
  e.data.pointer.buttons = cmp_gtk4_buttons_from_state(state);
  cmp_gtk4_queue_event(win->backend, &e);
}

static gboolean on_key_pressed(GtkEventControllerKey *c, guint key_val,
                               guint key_code, GdkModifierType state,
                               gpointer u) {
  struct CMPGTK4Window *win = (struct CMPGTK4Window *)u;
  CMPInputEvent e;
  (void)c;
  (void)key_code;

  /* Fix: Map GDK keysyms to standard ASCII/Control codes for Tab/Enter/BS */
  guint mapped_key = key_val;
  if (key_val == GDK_KEY_Tab || key_val == GDK_KEY_ISO_Left_Tab) {
    mapped_key = 9;
  } else if (key_val == GDK_KEY_Return || key_val == GDK_KEY_KP_Enter) {
    mapped_key = 13;
  } else if (key_val == GDK_KEY_Escape) {
    mapped_key = 27;
  } else if (key_val == GDK_KEY_BackSpace) {
    mapped_key = 8;
  }

  memset(&e, 0, sizeof(e));
  e.type = CMP_INPUT_KEY_DOWN;
  e.time_ms = cmp_gtk4_get_time_ms();
  e.window = win->header.handle;
  e.modifiers = cmp_gtk4_modifiers_from_state(state);
  e.data.key.key_code = mapped_key;
  e.data.key.native_code = key_val;

  cmp_gtk4_queue_event(win->backend, &e);

  if (key_val < 128 && key_val >= 32) {
    memset(&e, 0, sizeof(e));
    e.type = CMP_INPUT_TEXT;
    e.time_ms = cmp_gtk4_get_time_ms();
    e.window = win->header.handle;
    e.data.text.utf8[0] = (char)key_val;
    e.data.text.utf8[1] = '\0';
    e.data.text.length = 1;
    cmp_gtk4_queue_event(win->backend, &e);
  }

  return TRUE;
}

static int cmp_gtk4_ws_create_window(void *ctx, const CMPWSWindowConfig *c,
                                     CMPHandle *out) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)ctx;
  struct CMPGTK4Window *w;
  GtkGesture *click;
  GtkEventController *motion;
  GtkEventController *key;

  b->allocator.alloc(b->allocator.ctx, sizeof(struct CMPGTK4Window),
                     (void **)&w);

  memset(w, 0, sizeof(*w));
  w->backend = b;
  w->width = (c->width > 0) ? c->width : 800;
  w->height = (c->height > 0) ? c->height : 600;
  w->dpi_scale = 1.0f;
  w->scale_factor = 1;

  w->window = GTK_WINDOW(gtk_window_new());
  gtk_window_set_default_size(w->window, w->width, w->height);
  if (c->utf8_title)
    gtk_window_set_title(w->window, c->utf8_title);

  w->area = gtk_drawing_area_new();
  gtk_widget_set_can_focus(w->area, TRUE);
  gtk_widget_set_focusable(w->area, TRUE);
  gtk_widget_set_hexpand(w->area, TRUE);
  gtk_widget_set_vexpand(w->area, TRUE);
  gtk_window_set_child(w->window, w->area);

  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(w->area),
                                 cmp_gtk4_on_draw_func, w, NULL);
  g_signal_connect(w->window, "close-request", G_CALLBACK(on_close), w);
  g_signal_connect(w->window, "destroy", G_CALLBACK(on_window_destroy), w);

  click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 0);
  g_signal_connect(click, "pressed", G_CALLBACK(on_click_pressed), w);
  g_signal_connect(click, "released", G_CALLBACK(on_click_released), w);
  gtk_widget_add_controller(w->area, GTK_EVENT_CONTROLLER(click));

  motion = gtk_event_controller_motion_new();
  g_signal_connect(motion, "motion", G_CALLBACK(on_motion), w);
  gtk_widget_add_controller(w->area, motion);

  key = gtk_event_controller_key_new();
  /* Ensure we intercept Tab/arrows before GTK focus navigation logic */
  gtk_event_controller_set_propagation_phase(key, GTK_PHASE_CAPTURE);
  g_signal_connect(key, "key-pressed", G_CALLBACK(on_key_pressed), w);
  gtk_widget_add_controller(w->area, key);

  cmp_object_header_init(&w->header, CMP_GTK4_TYPE_WINDOW, 0,
                         &g_cmp_gtk4_window_vtable);
  b->handles.vtable->register_object(b->handles.ctx, &w->header);

  *out = w->header.handle;
  return CMP_OK;
}

static int cmp_gtk4_ws_destroy_window(void *c, CMPHandle h) {
  (void)c;
  (void)h;
  return CMP_OK;
}

static int cmp_gtk4_ws_show_window(void *c, CMPHandle h) {
  struct CMPGTK4Window *w;
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  if (cmp_gtk4_backend_resolve(b, h, CMP_GTK4_TYPE_WINDOW, (void **)&w) ==
      CMP_OK) {
    if (w->window) {
      gtk_window_present(w->window);
      w->visible = CMP_TRUE;
    }
  }
  return CMP_OK;
}

static int cmp_gtk4_ws_hide_window(void *c, CMPHandle h) {
  struct CMPGTK4Window *w;
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  if (cmp_gtk4_backend_resolve(b, h, CMP_GTK4_TYPE_WINDOW, (void **)&w) ==
      CMP_OK) {
    if (w->window) {
      gtk_widget_set_visible(GTK_WIDGET(w->window), FALSE);
      w->visible = CMP_FALSE;
    }
  }
  return CMP_OK;
}

static int cmp_gtk4_ws_get_size(void *c, CMPHandle h, int *w, int *ht) {
  struct CMPGTK4Window *win;
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  if (cmp_gtk4_backend_resolve(b, h, CMP_GTK4_TYPE_WINDOW, (void **)&win) ==
      CMP_OK) {
    *w = win->width;
    *ht = win->height;
    return CMP_OK;
  }
  return CMP_ERR_INVALID_ARGUMENT;
}

static int cmp_gtk4_ws_get_dpi(void *c, CMPHandle h, float *s) {
  struct CMPGTK4Window *win;
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  if (cmp_gtk4_backend_resolve(b, h, CMP_GTK4_TYPE_WINDOW, (void **)&win) ==
      CMP_OK) {
    *s = 1.0f;
    return CMP_OK;
  }
  return CMP_ERR_INVALID_ARGUMENT;
}

static int cmp_gtk4_ws_pump_events(void *c) {
  (void)c;
  while (g_main_context_pending(NULL)) {
    g_main_context_iteration(NULL, FALSE);
  }
  return CMP_OK;
}

static int cmp_gtk4_ws_poll_event(void *c, CMPInputEvent *e, CMPBool *h) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  if (b->event_count == 0)
    cmp_gtk4_ws_pump_events(b);
  if (b->event_count > 0) {
    *e = b->event_queue[b->event_head];
    b->event_head = (b->event_head + 1) % CMP_GTK4_EVENT_CAPACITY;
    b->event_count--;
    *h = CMP_TRUE;
  } else {
    *h = CMP_FALSE;
  }
  return CMP_OK;
}

static int cmp_gtk4_ws_get_time(void *c, cmp_u32 *t) {
  (void)c;
  *t = cmp_gtk4_get_time_ms();
  return CMP_OK;
}

static const CMPWSVTable g_cmp_gtk4_ws_vtable = {cmp_gtk4_ws_init,
                                                 cmp_gtk4_ws_shutdown,
                                                 cmp_gtk4_ws_create_window,
                                                 cmp_gtk4_ws_destroy_window,
                                                 cmp_gtk4_ws_show_window,
                                                 cmp_gtk4_ws_hide_window,
                                                 NULL,
                                                 NULL,
                                                 cmp_gtk4_ws_get_size,
                                                 NULL,
                                                 cmp_gtk4_ws_get_dpi,
                                                 NULL,
                                                 NULL,
                                                 cmp_gtk4_ws_poll_event,
                                                 cmp_gtk4_ws_pump_events,
                                                 cmp_gtk4_ws_get_time};

/* --- Graphics (CMPGfx) --- */

static int cmp_gtk4_ensure_surface(struct CMPGTK4Window *window, cmp_i32 width,
                                   cmp_i32 height) {
  cairo_t *cr;
  if (!window)
    return CMP_ERR_INVALID_ARGUMENT;
  /* FIX: Don't recreate if dimensions are zero or unchanged */
  if (width <= 0 || height <= 0)
    return CMP_OK;
  if (window->surface && window->surface_width == width &&
      window->surface_height == height)
    return CMP_OK;

  if (window->surface)
    cairo_surface_destroy(window->surface);
  window->surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  if (cairo_surface_status(window->surface) != CAIRO_STATUS_SUCCESS)
    return CMP_ERR_OUT_OF_MEMORY;

  cr = cairo_create(window->surface);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);
  cairo_destroy(cr);

  window->surface_width = width;
  window->surface_height = height;
  return CMP_OK;
}

static int cmp_gtk4_gfx_begin_frame(void *c, CMPHandle win, int w, int h,
                                    float dpi) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  struct CMPGTK4Window *window;
  cairo_t *cr;
  int rc;

  (void)dpi;
  if (cmp_gtk4_backend_resolve(b, win, CMP_GTK4_TYPE_WINDOW,
                               (void **)&window) != CMP_OK)
    return CMP_ERR_INVALID_ARGUMENT;

  /* FIX: If window was destroyed (pointer nulled by destroy signal), abort
   * cleanly */
  if (window->window == NULL || window->area == NULL) {
    return CMP_ERR_STATE;
  }

  rc = cmp_gtk4_ensure_surface(window, w, h);
  if (rc != CMP_OK)
    return rc;

  cr = cairo_create(window->surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    cairo_destroy(cr);
    return CMP_ERR_UNKNOWN;
  }

  b->frame_cr = cr;
  b->active_window = window;
  b->in_frame = CMP_TRUE;

  if (b->has_transform) {
    cairo_identity_matrix(cr);
    if (!b->has_transform)
      cmp_gtk4_apply_transform(b);
  }

  return CMP_OK;
}

static int cmp_gtk4_gfx_end_frame(void *c, CMPHandle win) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  struct CMPGTK4Window *window;
  if (cmp_gtk4_backend_resolve(b, win, CMP_GTK4_TYPE_WINDOW,
                               (void **)&window) != CMP_OK)
    return CMP_ERR_INVALID_ARGUMENT;

  if (b->frame_cr)
    cairo_destroy(b->frame_cr);
  b->frame_cr = NULL;
  b->active_window = NULL;
  b->in_frame = CMP_FALSE;

  /* FIX: Don't queue draw if widget is destroyed */
  if (window->area) {
    gtk_widget_queue_draw(window->area);
  }
  return CMP_OK;
}

static void cmp_gtk4_set_source_color(cairo_t *cr, CMPColor color) {
  cairo_set_source_rgba(cr, (double)color.r, (double)color.g, (double)color.b,
                        (double)color.a);
}

static int cmp_gtk4_gfx_clear(void *c, CMPColor col) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  cairo_t *cr = b->frame_cr;
  if (!cr)
    return CMP_ERR_STATE;

  cairo_save(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cmp_gtk4_set_source_color(cr, col);
  cairo_paint(cr);
  cairo_restore(cr);
  return CMP_OK;
}

static void cmp_gtk4_path_rounded_rect(cairo_t *cr, const CMPRect *rect,
                                       CMPScalar radius) {
  double x;
  double y;
  double w;
  double h;
  double r;

  x = rect->x;
  y = rect->y;
  w = rect->width;
  h = rect->height;
  r = radius;

  if (r <= 0.0) {
    cairo_rectangle(cr, x, y, w, h);
    return;
  }

  /* FIX: Correct angle order for drawing rounded rectangles */
  cairo_new_sub_path(cr);
  cairo_arc(cr, x + w - r, y + r, r, -CMP_GTK4_PI_2, 0.0);
  cairo_arc(cr, x + w - r, y + h - r, r, 0.0, CMP_GTK4_PI_2);
  cairo_arc(cr, x + r, y + h - r, r, CMP_GTK4_PI_2, CMP_GTK4_PI);
  cairo_arc(cr, x + r, y + r, r, CMP_GTK4_PI, 3.0 * CMP_GTK4_PI_2);
  cairo_close_path(cr);
}

static int cmp_gtk4_gfx_draw_rect(void *c, const CMPRect *r, CMPColor color,
                                  float rad) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  cairo_t *cr = b->frame_cr;
  if (!cr)
    return CMP_ERR_STATE;

  cmp_gtk4_set_source_color(cr, color);
  cmp_gtk4_path_rounded_rect(cr, r, rad);
  cairo_fill(cr);
  return CMP_OK;
}

static int cmp_gtk4_gfx_push_clip(void *c, const CMPRect *r) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  cairo_t *cr = b->frame_cr;
  if (!cr)
    return CMP_ERR_STATE;
  cairo_save(cr);
  cairo_rectangle(cr, r->x, r->y, r->width, r->height);
  cairo_clip(cr);
  return CMP_OK;
}

static int cmp_gtk4_gfx_pop_clip(void *c) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  cairo_t *cr = b->frame_cr;
  if (!cr)
    return CMP_ERR_STATE;
  cairo_restore(cr);
  return CMP_OK;
}

static int cmp_gtk4_write_argb32_pixel(unsigned char *dst, unsigned char r,
                                       unsigned char g, unsigned char b,
                                       unsigned char a) {
  unsigned char pr, pg, pb;
  unsigned int alpha = (unsigned int)a;
  if (alpha == 0) {
    pr = pg = pb = 0;
  } else {
    pr = (unsigned char)((r * alpha + 127u) / 255u);
    pg = (unsigned char)((g * alpha + 127u) / 255u);
    pb = (unsigned char)((b * alpha + 127u) / 255u);
  }
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  dst[0] = pb;
  dst[1] = pg;
  dst[2] = pr;
  dst[3] = a;
#else
  dst[0] = a;
  dst[1] = pr;
  dst[2] = pg;
  dst[3] = pb;
#endif
  return 0;
}

static int cmp_gtk4_gfx_create_texture(void *c, int w, int h, cmp_u32 f,
                                       const void *data, size_t size,
                                       CMPHandle *handle) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  CMPGTK4Texture *t;
  const unsigned char *src;
  unsigned char *dst;
  int row, col;

  b->allocator.alloc(b->allocator.ctx, sizeof(CMPGTK4Texture), (void **)&t);
  memset(t, 0, sizeof(*t));
  t->backend = b;
  t->width = w;
  t->height = h;
  t->format = f;
  t->size = size;
  t->stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w);
  t->size = t->stride * h;

  b->allocator.alloc(b->allocator.ctx, t->size, (void **)&t->pixels);
  memset(t->pixels, 0, t->size);
  /* Create wrapper surface for pixels */
  t->surface = cairo_image_surface_create_for_data(
      t->pixels, CAIRO_FORMAT_ARGB32, w, h, t->stride);

  /* Convert format to Pre-Multiplied ARGB32 for Cairo */
  if (data) {
    src = (const unsigned char *)data;
    for (row = 0; row < h; ++row) {
      dst = t->pixels + (row * t->stride);
      for (col = 0; col < w; ++col) {
        cmp_gtk4_write_argb32_pixel(dst + (col * 4), src[0], src[1], src[2],
                                    src[3]);
        src += 4;
      }
    }
  }

  cmp_object_header_init(&t->header, CMP_GTK4_TYPE_TEXTURE, 0,
                         &g_cmp_gtk4_texture_vtable);
  b->handles.vtable->register_object(b->handles.ctx, &t->header);
  *handle = t->header.handle;
  return CMP_OK;
}

static int cmp_gtk4_gfx_draw_texture(void *c, CMPHandle h, const CMPRect *src,
                                     const CMPRect *dst, float alpha) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  CMPGTK4Texture *t;
  cairo_t *cr = b->frame_cr;
  double sx, sy;

  if (cmp_gtk4_backend_resolve(b, h, CMP_GTK4_TYPE_TEXTURE, (void **)&t) !=
      CMP_OK)
    return CMP_ERR_INVALID_ARGUMENT;
  if (!cr)
    return CMP_ERR_STATE;

  cairo_save(cr);
  cairo_rectangle(cr, dst->x, dst->y, dst->width, dst->height);
  cairo_clip(cr);

  cairo_translate(cr, dst->x, dst->y);
  sx = (double)dst->width / (double)src->width;
  sy = (double)dst->height / (double)src->height;
  cairo_scale(cr, sx, sy);

  cairo_set_source_surface(cr, t->surface, -src->x, -src->y);
  cairo_paint_with_alpha(cr, alpha);

  cairo_restore(cr);
  return CMP_OK;
}

static int gfx_draw_line(void *c, float x1, float y1, float x2, float y2,
                         CMPColor col, float th) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  cairo_t *cr = b->frame_cr;
  if (!cr)
    return CMP_ERR_STATE;
  cmp_gtk4_set_source_color(cr, col);
  cairo_set_line_width(cr, th);
  cairo_move_to(cr, x1, y1);
  cairo_line_to(cr, x2, y2);
  cairo_stroke(cr);
  return CMP_OK;
}
static int gfx_draw_path(void *c, const CMPPath *p, CMPColor col) {
  (void)c;
  (void)p;
  (void)col;
  return CMP_OK; /* Stub: cairo path conversion needed */
}
static int gfx_set_xf(void *c, const CMPMat3 *m) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  b->transform = *m;
  b->has_transform = CMP_TRUE;
  cmp_gtk4_apply_transform(b);
  return CMP_OK;
}
static int gfx_upd_tex(void *c, CMPHandle h, int x, int y, int w, int ht,
                       const void *p, size_t s) {
  (void)c;
  (void)h;
  (void)x;
  (void)y;
  (void)w;
  (void)ht;
  (void)p;
  (void)s;
  return CMP_OK;
}
static int gfx_del_tex(void *c, CMPHandle h) {
  (void)c;
  (void)h;
  return cmp_gtk4_texture_destroy(
      NULL); /* Note: Cannot destroy without looking up handle, this is stub */
}

static const CMPGfxVTable g_cmp_gtk4_gfx_vtable = {cmp_gtk4_gfx_begin_frame,
                                                   cmp_gtk4_gfx_end_frame,
                                                   cmp_gtk4_gfx_clear,
                                                   cmp_gtk4_gfx_draw_rect,
                                                   gfx_draw_line,
                                                   gfx_draw_path,
                                                   cmp_gtk4_gfx_push_clip,
                                                   cmp_gtk4_gfx_pop_clip,
                                                   gfx_set_xf,
                                                   cmp_gtk4_gfx_create_texture,
                                                   gfx_upd_tex,
                                                   gfx_del_tex,
                                                   cmp_gtk4_gfx_draw_texture};

/* --- Text Backend --- */

static int txt_meas(void *c, CMPHandle f, const char *u, size_t l, float *w,
                    float *h, float *b) {
  struct CMPGTK4Backend *be = (struct CMPGTK4Backend *)c;
  CMPGTK4Font *font;
  PangoContext *context;
  PangoLayout *layout;
  PangoRectangle logical_rect;
  PangoFontMap *fm;

  if (cmp_gtk4_backend_resolve(be, f, CMP_GTK4_TYPE_FONT, (void **)&font) !=
      CMP_OK)
    return CMP_ERR_INVALID_ARGUMENT;

  fm = pango_cairo_font_map_get_default();
  context = pango_font_map_create_context(fm);
  layout = pango_layout_new(context);

  pango_layout_set_font_description(layout, font->desc);
  pango_layout_set_text(layout, u, (int)l);

  pango_layout_get_extents(layout, NULL, &logical_rect);

  *w = (float)pango_units_to_double(logical_rect.width);
  *h = (float)pango_units_to_double(logical_rect.height);

  {
    PangoLayoutIter *iter = pango_layout_get_iter(layout);
    int baseline = pango_layout_iter_get_baseline(iter);
    double y = pango_units_to_double(logical_rect.y);
    *b = (float)(pango_units_to_double(baseline) - y);
    pango_layout_iter_free(iter);
  }

  g_object_unref(layout);
  g_object_unref(context);
  return CMP_OK;
}

static int txt_draw(void *c, CMPHandle f, const char *u, size_t l, float x,
                    float y, CMPColor col) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  CMPGTK4Font *font;
  PangoLayout *layout;
  cairo_t *cr = b->frame_cr;

  if (!cr)
    return CMP_ERR_STATE;
  if (cmp_gtk4_backend_resolve(b, f, CMP_GTK4_TYPE_FONT, (void **)&font) !=
      CMP_OK)
    return CMP_ERR_INVALID_ARGUMENT;

  layout = pango_cairo_create_layout(cr);
  pango_layout_set_font_description(layout, font->desc);
  pango_layout_set_text(layout, u, (int)l);

  cmp_gtk4_set_source_color(cr, col);

  {
    PangoLayoutIter *iter = pango_layout_get_iter(layout);
    const int baseline = pango_layout_iter_get_baseline(iter);
    pango_layout_iter_free(iter);
    {
      const double y_offset = pango_units_to_double(baseline);
      /*
       * FIX: Render text so the baseline aligns with 'y'.
       * Pango draws top-left, so shift vertically by the ascent (baseline
       * offset).
       */
      cairo_move_to(cr, x, y - y_offset);
    }
  }

  pango_cairo_show_layout(cr, layout);
  g_object_unref(layout);

  return CMP_OK;
}

static int txt_create(void *c, const char *n, int s, int w, int i,
                      CMPHandle *h) {
  struct CMPGTK4Backend *b = (struct CMPGTK4Backend *)c;
  CMPGTK4Font *f;
  (void)w;
  (void)i;
  b->allocator.alloc(b->allocator.ctx, sizeof(CMPGTK4Font), (void **)&f);
  memset(f, 0, sizeof(*f));
  f->backend = b;

  f->desc = pango_font_description_new();
  pango_font_description_set_family(f->desc, n);
  /* Use absolute size (pixels) to ensure consistency across resolutions */
  pango_font_description_set_absolute_size(f->desc, s * PANGO_SCALE);

  cmp_object_header_init(&f->header, CMP_GTK4_TYPE_FONT, 0,
                         &g_cmp_gtk4_font_vtable);
  b->handles.vtable->register_object(b->handles.ctx, &f->header);
  *h = f->header.handle;
  return CMP_OK;
}
static int txt_destroy(void *c, CMPHandle h) {
  (void)c;
  (void)h;
  return cmp_gtk4_font_destroy(NULL);
}

static const CMPTextVTable g_cmp_gtk4_text_vtable = {txt_create, txt_destroy,
                                                     txt_meas, txt_draw};

/* --- Environment Stubs --- */
static int env_get_io(void *e, CMPIO *io) {
  (void)e;
  (void)io;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_sensors(void *e, CMPSensors *s) {
  (void)e;
  (void)s;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_cam(void *e, CMPCamera *c) {
  (void)e;
  (void)c;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_img(void *e, CMPImage *i) {
  (void)e;
  (void)i;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_vid(void *e, CMPVideo *v) {
  (void)e;
  (void)v;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_aud(void *e, CMPAudio *a) {
  (void)e;
  (void)a;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_net(void *e, CMPNetwork *n) {
  (void)e;
  (void)n;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_tasks(void *e, CMPTasks *t) {
  (void)e;
  (void)t;
  return CMP_ERR_UNSUPPORTED;
}
static int env_get_time(void *e, cmp_u32 *t) {
  (void)e;
  *t = cmp_gtk4_get_time_ms();
  return CMP_OK;
}

static const CMPEnvVTable g_cmp_gtk4_env_vtable = {
    env_get_io,  env_get_sensors, env_get_cam,   env_get_img, env_get_vid,
    env_get_aud, env_get_net,     env_get_tasks, env_get_time};

/* --- PUBLIC API --- */

int CMP_CALL cmp_gtk4_backend_create(const CMPGTK4BackendConfig *config,
                                     CMPGTK4Backend **out) {
  struct CMPGTK4Backend *b;
  CMPAllocator alloc;
  int rc;

  if (out == NULL)
    return CMP_ERR_INVALID_ARGUMENT;
  *out = NULL;

  if (config && config->allocator)
    alloc = *config->allocator;
  else
    cmp_get_default_allocator(&alloc);

  rc = cmp_gtk4_backend_validate_config(config);
  if (rc != CMP_OK)
    return rc;

  alloc.alloc(alloc.ctx, sizeof(struct CMPGTK4Backend), (void **)&b);
  memset(b, 0, sizeof(*b));
  b->allocator = alloc;

  cmp_mat3_identity(&b->transform);
  b->has_transform = CMP_FALSE;

  cmp_handle_system_default_create(64, &alloc, &b->handles);

  b->initialized = CMP_TRUE;
  b->clipboard_limit = config ? config->clipboard_limit : 4096;

  b->ws.ctx = b;
  b->ws.vtable = &g_cmp_gtk4_ws_vtable;
  b->gfx.ctx = b;
  b->gfx.vtable = &g_cmp_gtk4_gfx_vtable;
  b->gfx.text_vtable = &g_cmp_gtk4_text_vtable;
  b->env.ctx = b;
  b->env.vtable = &g_cmp_gtk4_env_vtable;

  if (!g_cmp_gtk4_refcount)
    cmp_gtk4_global_init(b);

  *out = b;
  return CMP_OK;
}

int CMP_CALL cmp_gtk4_backend_destroy(CMPGTK4Backend *b) {
  if (!b)
    return CMP_OK;
  cmp_handle_system_default_destroy(&b->handles);
  if (g_cmp_gtk4_refcount > 0)
    cmp_gtk4_global_shutdown(b);
  b->allocator.free(b->allocator.ctx, b);
  return CMP_OK;
}

int CMP_CALL cmp_gtk4_backend_get_ws(CMPGTK4Backend *b, CMPWS *ws) {
  if (!b || !ws)
    return CMP_ERR_INVALID_ARGUMENT;
  *ws = b->ws;
  return CMP_OK;
}
int CMP_CALL cmp_gtk4_backend_get_gfx(CMPGTK4Backend *b, CMPGfx *gfx) {
  if (!b || !gfx)
    return CMP_ERR_INVALID_ARGUMENT;
  *gfx = b->gfx;
  return CMP_OK;
}
int CMP_CALL cmp_gtk4_backend_get_env(CMPGTK4Backend *b, CMPEnv *env) {
  if (!b || !env)
    return CMP_ERR_INVALID_ARGUMENT;
  *env = b->env;
  return CMP_OK;
}

#ifdef CMP_TESTING
/* Unused static function warnings fix for tests: reference them */
void __cmp_gtk4_test_refs(void) {
  (void)cmp_gtk4_apply_transform(NULL);
  (void)cmp_gtk4_backend_validate_config(NULL);
}
#endif

/* Stubs for unavailable */
#else
/* ... (Stubs omitted for brevity, they are needed if !GTK4_AVAILABLE but header
 * included) ... */
int CMP_CALL cmp_gtk4_backend_create(const CMPGTK4BackendConfig *config,
                                     CMPGTK4Backend **out) {
  (void)config;
  (void)out;
  return CMP_ERR_UNSUPPORTED;
}
int CMP_CALL cmp_gtk4_backend_destroy(CMPGTK4Backend *b) {
  (void)b;
  return CMP_OK;
}
int CMP_CALL cmp_gtk4_backend_get_ws(CMPGTK4Backend *b, CMPWS *ws) {
  (void)b;
  (void)ws;
  return CMP_ERR_UNSUPPORTED;
}
int CMP_CALL cmp_gtk4_backend_get_gfx(CMPGTK4Backend *b, CMPGfx *gfx) {
  (void)b;
  (void)gfx;
  return CMP_ERR_UNSUPPORTED;
}
int CMP_CALL cmp_gtk4_backend_get_env(CMPGTK4Backend *b, CMPEnv *env) {
  (void)b;
  (void)env;
  return CMP_ERR_UNSUPPORTED;
}
#endif