/**
 * @file ui_window_backend_linux_x11.c
 * @brief ui_window_backend_linux_x11.c implementation.
 */

/* clang-format off */
#include "../include/ui_window_backend_linux.h"
#include <stddef.h>
#if (defined(__linux__) || defined(__FreeBSD__)) && defined(HAVE_X11)
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
#endif
/* clang-format on */

#if (defined(__linux__) || defined(__FreeBSD__)) && defined(HAVE_X11)

/**
 * @struct ui_window
 * \brief ui_window
 */
struct ui_window {
  Display *display;                                   /**< display */
  Window window;                                      /**< window */
  GLXContext glc;                                     /**< glc */
  void *context;                                      /**< context */
  int is_closing;                                     /**< is_closing */
  int width;                                          /**< width */
  int height;                                         /**< height */
  float scale_factor;                                 /**< scale_factor */
  Atom wm_delete_window;                              /**< wm_delete_window */
  ui_error_t (*on_resize_callback)(void *, int, int); /**< on_resize_callback */
  void *on_resize_user_data; /**< on_resize_user_data */
  ui_error_t (*on_dpi_change_callback)(void *, float,
                                       float); /**< on_dpi_change_callback */
  void *on_dpi_change_user_data;               /**< on_dpi_change_user_data */
};

/**
 * @brief linux_create_window.
 * @param backend Parameter backend.
 * @param title Parameter title.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_window Parameter out_window.
 * @return Return value.
 */
static ui_error_t linux_create_window(struct ui_window_backend *backend,
                                      const char *title, int width, int height,
                                      struct ui_window **out_window) {
  Display *dpy;
  Window win;
  int screen;
  struct ui_window *win_obj;
  Atom wm_delete_window;
  GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
  XVisualInfo *vi;
  Colormap cmap;
  XSetWindowAttributes swa;
  GLXContext glc;

  if (!backend || !title || !out_window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dpy = XOpenDisplay(NULL);
  if (!dpy) {
    return UI_ERROR_UNKNOWN;
  }

  screen = DefaultScreen(dpy);
  vi = glXChooseVisual(dpy, screen, att);
  if (!vi) {
    XCloseDisplay(dpy);
    return UI_ERROR_UNKNOWN;
  }

  cmap =
      XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

  win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 10, 10, width, height,
                      0, vi->depth, InputOutput, vi->visual,
                      CWColormap | CWEventMask, &swa);

  XStoreName(dpy, win, title);

  wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dpy, win, &wm_delete_window, 1);

  glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);

  win_obj =
      (struct ui_window *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window));
  if (!win_obj) {
    glXMakeCurrent(dpy, None, NULL);
    glXDestroyContext(dpy, glc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    XFree(vi);
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(win_obj, 0, sizeof(struct ui_window));

  win_obj->display = dpy;
  win_obj->window = win;
  win_obj->glc = glc;
  win_obj->context = NULL;
  win_obj->is_closing = 0;
  win_obj->width = width;
  win_obj->height = height;
  win_obj->scale_factor = 1.0f;
  win_obj->wm_delete_window = wm_delete_window;

  XFree(vi);

  *out_window = win_obj;
  return UI_ERROR_NONE;
}

/**
 * @brief linux_destroy_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t linux_destroy_window(struct ui_window_backend *backend,
                                       struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window->display) {
    if (window->glc) {
      glXMakeCurrent((Display *)window->display, None, NULL);
      glXDestroyContext((Display *)window->display, window->glc);
    }
    if (window->window) {
      XDestroyWindow((Display *)window->display, window->window);
    }
    XCloseDisplay((Display *)window->display);
  }
  C_MULTIPLATFORM_FREE(window);
  return UI_ERROR_NONE;
}

/**
 * @brief linux_show_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t linux_show_window(struct ui_window_backend *backend,
                                    struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  XMapWindow((Display *)window->display, window->window);
  XFlush((Display *)window->display);
  return UI_ERROR_NONE;
}

/**
 * @brief linux_hide_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t linux_hide_window(struct ui_window_backend *backend,
                                    struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  XUnmapWindow((Display *)window->display, window->window);
  XFlush((Display *)window->display);
  return UI_ERROR_NONE;
}

/**
 * @brief linux_poll_events.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_event Parameter out_event.
 * @param out_has_event Parameter out_has_event.
 * @return Return value.
 */
static ui_error_t linux_poll_events(struct ui_window_backend *backend,
                                    struct ui_window *window,
                                    struct ui_event *out_event,
                                    int *out_has_event) {
  Display *dpy;
  XEvent xev;

  if (!backend || !window || !out_event || !out_has_event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  dpy = (Display *)window->display;
  *out_has_event = 0;
  out_event->type = UI_EVENT_NONE;

  if (XPending(dpy) > 0) {
    XNextEvent(dpy, &xev);
    *out_has_event = 1;

    if (xev.type == DestroyNotify) {
      out_event->type = UI_EVENT_WINDOW_CLOSE;
    } else if (xev.type == ConfigureNotify) {
      out_event->type = UI_EVENT_WINDOW_RESIZE;
      out_event->event_data.window.width = xev.xconfigure.width;
      out_event->event_data.window.height = xev.xconfigure.height;
      if (window->on_resize_callback) {
        ui_error_t cb_rc = window->on_resize_callback(
            window->on_resize_user_data, xev.xconfigure.width,
            xev.xconfigure.height);
        if (cb_rc != UI_ERROR_NONE) {
          return cb_rc;
        }
      }
    } else if (xev.type == ClientMessage) {
      if ((Atom)xev.xclient.data.l[0] == window->wm_delete_window) {
        out_event->type = UI_EVENT_WINDOW_CLOSE;
      }
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief linux_set_on_resize_callback.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t linux_set_on_resize_callback(
    struct ui_window_backend *backend, struct ui_window *window,
    ui_error_t (*callback)(void *, int, int), void *user_data) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  window->on_resize_callback = callback;
  window->on_resize_user_data = user_data;
  return UI_ERROR_NONE;
}

static ui_error_t linux_x11_get_scale_factor(struct ui_window_backend *backend,
                                             struct ui_window *window,
                                             float *out_scale_factor) {
  if (!backend || !window || !out_scale_factor) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window->scale_factor > 0.0f) {
    *out_scale_factor = window->scale_factor;
  } else {
    *out_scale_factor = 1.0f;
  }
  return UI_ERROR_NONE;
}

static ui_error_t
linux_x11_get_framebuffer_size(struct ui_window_backend *backend,
                               struct ui_window *window, int *out_fb_width,
                               int *out_fb_height) {
  float scale;
  ui_error_t rc;

  if (!backend || !window || !out_fb_width || !out_fb_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  scale = 1.0f;
  rc = linux_x11_get_scale_factor(backend, window, &scale);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  *out_fb_width = (int)((float)window->width * scale);
  *out_fb_height = (int)((float)window->height * scale);
  return UI_ERROR_NONE;
}

static ui_error_t linux_x11_set_on_dpi_change_callback(
    struct ui_window_backend *backend, struct ui_window *window,
    ui_error_t (*callback)(void *, float, float), void *user_data) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  window->on_dpi_change_callback = callback;
  window->on_dpi_change_user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief linux_get_os_handle.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_handle Parameter out_handle.
 * @return Return value.
 */
static ui_error_t linux_get_os_handle(struct ui_window_backend *backend,
                                      struct ui_window *window,
                                      void **out_handle) {
  if (!backend || !window || !out_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_handle = (void *)(size_t)window->window;
  return UI_ERROR_NONE;
}

/**
 * @brief linux_swap_buffers.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t linux_swap_buffers(struct ui_window_backend *backend,
                                     struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  glXSwapBuffers((Display *)window->display, window->window);
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
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
  backend->get_os_handle = linux_get_os_handle;
  backend->set_on_resize_callback = linux_set_on_resize_callback;
  backend->get_scale_factor = linux_x11_get_scale_factor;
  backend->get_framebuffer_size = linux_x11_get_framebuffer_size;
  backend->set_on_dpi_change_callback = linux_x11_set_on_dpi_change_callback;
  backend->user_data = NULL;

  *out_backend = backend;
  return UI_ERROR_NONE;
}

/* \brief ui_error
 */
ui_error_t ui_window_backend_linux_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(backend);
  return UI_ERROR_NONE;
}

#else
/* Non-Linux Platform Stub (or Linux without X11 when Wayland is not active) */

#if !defined(UI_ENABLE_WAYLAND)
ui_error_t
ui_window_backend_linux_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/* \brief ui_error
 */
ui_error_t ui_window_backend_linux_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
#endif
