#if defined(__linux__) || defined(__FreeBSD__)

/* clang-format off */
#include "../include/ui_window_backend_linux.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <string.h>


#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#endif
#include "xdg-shell-client-protocol.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

/* clang-format on */

struct ui_window {
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct xdg_wm_base *xdg_wm_base;

  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;

  struct wl_egl_window *egl_window;
  EGLDisplay egl_display;
  EGLContext egl_context;
  EGLSurface egl_surface;

  void *context;
  int is_closing;
  int width;
  int height;
  int needs_swap;
  struct ui_event pending_event;
  int has_pending_event;
};

static ui_error_t log_xdg_toplevel_configure(void *data,
                                             struct xdg_toplevel *xdg_toplevel,
                                             int32_t width, int32_t height,
                                             struct wl_array *states) {
  struct ui_window *win = (struct ui_window *)data;
  (void)xdg_toplevel;
  (void)states;

  if (width > 0 && height > 0 &&
      (win->width != width || win->height != height)) {
    win->width = width;
    win->height = height;
    if (win->egl_window) {
      wl_egl_window_resize(win->egl_window, width, height, 0, 0);
    }
    win->pending_event.type = UI_EVENT_WINDOW_RESIZE;
    win->pending_event.event_data.window.width = width;
    win->pending_event.event_data.window.height = height;
    win->has_pending_event = 1;
  }
  return UI_ERROR_NONE;
}

static ui_error_t log_xdg_toplevel_close(void *data,
                                         struct xdg_toplevel *xdg_toplevel) {
  struct ui_window *win = (struct ui_window *)data;
  (void)xdg_toplevel;
  win->is_closing = 1;
  win->pending_event.type = UI_EVENT_WINDOW_CLOSE;
  win->has_pending_event = 1;
  return UI_ERROR_NONE;
}

static ui_error_t
log_xdg_toplevel_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
                                  int32_t width, int32_t height) {
  (void)data;
  (void)xdg_toplevel;
  (void)width;
  (void)height;
  return UI_ERROR_NONE;
}

static ui_error_t
log_xdg_toplevel_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel,
                                 struct wl_array *capabilities) {
  (void)data;
  (void)xdg_toplevel;
  (void)capabilities;
  return UI_ERROR_NONE;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    log_xdg_toplevel_configure, log_xdg_toplevel_close,
    log_xdg_toplevel_configure_bounds, log_xdg_toplevel_wm_capabilities};

static ui_error_t log_xdg_surface_configure(void *data,
                                            struct xdg_surface *xdg_surface,
                                            uint32_t serial) {
  struct ui_window *win = (struct ui_window *)data;
  (void)win;
  xdg_surface_ack_configure(xdg_surface, serial);
  return UI_ERROR_NONE;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    log_xdg_surface_configure};

static ui_error_t log_xdg_wm_base_ping(void *data,
                                       struct xdg_wm_base *xdg_wm_base,
                                       uint32_t serial) {
  (void)data;
  xdg_wm_base_pong(xdg_wm_base, serial);
  return UI_ERROR_NONE;
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    log_xdg_wm_base_ping};

static ui_error_t log_registry_handler(void *data, struct wl_registry *registry,
                                       uint32_t id, const char *interface,
                                       uint32_t version) {
  struct ui_window *win = (struct ui_window *)data;
  (void)version;
  if (strcmp(interface, "wl_compositor") == 0) {
    win->compositor = (struct wl_compositor *)wl_registry_bind(
        registry, id, &wl_compositor_interface, 1);
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    win->xdg_wm_base = (struct xdg_wm_base *)wl_registry_bind(
        registry, id, &xdg_wm_base_interface, 1);
    xdg_wm_base_add_listener(win->xdg_wm_base, &xdg_wm_base_listener, win);
  }
  return UI_ERROR_NONE;
}

static ui_error_t log_registry_remover(void *data, struct wl_registry *registry,
                                       uint32_t id) {
  (void)data;
  (void)registry;
  (void)id;
  return UI_ERROR_NONE;
}

static const struct wl_registry_listener registry_listener = {
    log_registry_handler, log_registry_remover};

static ui_error_t linux_create_window(struct ui_window_backend *backend,
                                      const char *title, int width, int height,
                                      struct ui_window **out_window) {
  struct ui_window *win_obj;
  EGLint num_config;
  EGLConfig egl_config;
  EGLint egl_major, egl_minor;
  EGLint attribs[] = {EGL_SURFACE_TYPE,
                      EGL_WINDOW_BIT,
                      EGL_RED_SIZE,
                      8,
                      EGL_GREEN_SIZE,
                      8,
                      EGL_BLUE_SIZE,
                      8,
                      EGL_ALPHA_SIZE,
                      8,
                      EGL_RENDERABLE_TYPE,
                      EGL_OPENGL_BIT,
                      EGL_NONE};

  if (!backend || !title || !out_window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  win_obj =
      (struct ui_window *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window));
  if (!win_obj) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(win_obj, 0, sizeof(struct ui_window));
  win_obj->width = width;
  win_obj->height = height;

  win_obj->display = wl_display_connect(NULL);
  if (!win_obj->display) {
    C_MULTIPLATFORM_FREE(win_obj);
    return UI_ERROR_UNKNOWN;
  }

  win_obj->registry = wl_display_get_registry(win_obj->display);
  wl_registry_add_listener(win_obj->registry, &registry_listener, win_obj);
  wl_display_roundtrip(win_obj->display);

  if (!win_obj->compositor || !win_obj->xdg_wm_base) {
    wl_display_disconnect(win_obj->display);
    C_MULTIPLATFORM_FREE(win_obj);
    return UI_ERROR_UNKNOWN;
  }

  win_obj->surface = wl_compositor_create_surface(win_obj->compositor);
  win_obj->xdg_surface =
      xdg_wm_base_get_xdg_surface(win_obj->xdg_wm_base, win_obj->surface);
  xdg_surface_add_listener(win_obj->xdg_surface, &xdg_surface_listener,
                           win_obj);

  win_obj->xdg_toplevel = xdg_surface_get_toplevel(win_obj->xdg_surface);
  xdg_toplevel_add_listener(win_obj->xdg_toplevel, &xdg_toplevel_listener,
                            win_obj);
  xdg_toplevel_set_title(win_obj->xdg_toplevel, title);

  wl_surface_commit(win_obj->surface);
  wl_display_roundtrip(win_obj->display);

  win_obj->egl_window = wl_egl_window_create(win_obj->surface, width, height);

  win_obj->egl_display = eglGetDisplay((EGLNativeDisplayType)win_obj->display);
  if (win_obj->egl_display == EGL_NO_DISPLAY) {
    goto cleanup;
  }

  if (!eglInitialize(win_obj->egl_display, &egl_major, &egl_minor)) {
    goto cleanup;
  }

  eglBindAPI(EGL_OPENGL_API);

  if (!eglChooseConfig(win_obj->egl_display, attribs, &egl_config, 1,
                       &num_config) ||
      num_config == 0) {
    goto cleanup;
  }

  win_obj->egl_context =
      eglCreateContext(win_obj->egl_display, egl_config, EGL_NO_CONTEXT, NULL);
  if (win_obj->egl_context == EGL_NO_CONTEXT) {
    goto cleanup;
  }

  win_obj->egl_surface =
      eglCreateWindowSurface(win_obj->egl_display, egl_config,
                             (EGLNativeWindowType)win_obj->egl_window, NULL);
  if (win_obj->egl_surface == EGL_NO_SURFACE) {
    goto cleanup;
  }

  if (!eglMakeCurrent(win_obj->egl_display, win_obj->egl_surface,
                      win_obj->egl_surface, win_obj->egl_context)) {
    goto cleanup;
  }

  *out_window = win_obj;
  return UI_ERROR_NONE;

cleanup:
  if (win_obj->egl_display != EGL_NO_DISPLAY) {
    eglTerminate(win_obj->egl_display);
  }
  if (win_obj->egl_window) {
    wl_egl_window_destroy(win_obj->egl_window);
  }
  if (win_obj->xdg_toplevel) {
    xdg_toplevel_destroy(win_obj->xdg_toplevel);
  }
  if (win_obj->xdg_surface) {
    xdg_surface_destroy(win_obj->xdg_surface);
  }
  if (win_obj->surface) {
    wl_surface_destroy(win_obj->surface);
  }
  wl_display_disconnect(win_obj->display);
  C_MULTIPLATFORM_FREE(win_obj);
  return UI_ERROR_UNKNOWN;
}

static ui_error_t linux_destroy_window(struct ui_window_backend *backend,
                                       struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window->display) {
    if (window->egl_display != EGL_NO_DISPLAY) {
      eglMakeCurrent(window->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                     EGL_NO_CONTEXT);
      if (window->egl_surface != EGL_NO_SURFACE) {
        eglDestroySurface(window->egl_display, window->egl_surface);
      }
      if (window->egl_context != EGL_NO_CONTEXT) {
        eglDestroyContext(window->egl_display, window->egl_context);
      }
      eglTerminate(window->egl_display);
    }
    if (window->egl_window) {
      wl_egl_window_destroy(window->egl_window);
    }
    if (window->xdg_toplevel) {
      xdg_toplevel_destroy(window->xdg_toplevel);
    }
    if (window->xdg_surface) {
      xdg_surface_destroy(window->xdg_surface);
    }
    if (window->surface) {
      wl_surface_destroy(window->surface);
    }
    if (window->xdg_wm_base) {
      xdg_wm_base_destroy(window->xdg_wm_base);
    }
    if (window->compositor) {
      wl_compositor_destroy(window->compositor);
    }
    if (window->registry) {
      wl_registry_destroy(window->registry);
    }
    wl_display_disconnect(window->display);
  }
  C_MULTIPLATFORM_FREE(window);
  return UI_ERROR_NONE;
}

static ui_error_t linux_show_window(struct ui_window_backend *backend,
                                    struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

static ui_error_t linux_hide_window(struct ui_window_backend *backend,
                                    struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_NONE;
}

static ui_error_t linux_poll_events(struct ui_window_backend *backend,
                                    struct ui_window *window,
                                    struct ui_event *out_event,
                                    int *out_has_event) {
  if (!backend || !window || !out_event || !out_has_event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  wl_display_dispatch_pending(window->display);

  if (window->has_pending_event) {
    *out_event = window->pending_event;
    *out_has_event = 1;
    window->has_pending_event = 0;
  } else {
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
  }

  return UI_ERROR_NONE;
}

static ui_error_t linux_swap_buffers(struct ui_window_backend *backend,
                                     struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  eglSwapBuffers(window->egl_display, window->egl_surface);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_window_backend_linux_create(struct ui_window_backend **out_backend) {
  struct ui_window_backend *backend;

  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  backend = (struct ui_window_backend *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_window_backend));
  if (!backend) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  backend->create_window = linux_create_window;
  backend->destroy_window = linux_destroy_window;
  backend->show_window = linux_show_window;
  backend->hide_window = linux_hide_window;
  backend->poll_events = linux_poll_events;
  backend->swap_buffers = linux_swap_buffers;
  backend->push_deep_link = NULL;
  backend->get_os_handle = NULL;
  backend->set_on_resize_callback = NULL;
  backend->user_data = NULL;

  *out_backend = backend;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_window_backend_linux_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(backend);
  return UI_ERROR_NONE;
}

#else
/* Non-Linux Platform Stub */
#include "../include/ui_window_backend_linux.h"
#include <stddef.h>

ui_error_t
ui_window_backend_linux_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/** \brief ui_error */
ui_error_t ui_window_backend_linux_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
