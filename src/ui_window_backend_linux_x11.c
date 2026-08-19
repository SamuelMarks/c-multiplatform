#if defined(__linux__) || defined(__FreeBSD__)

/* clang-format off */
#include "../include/ui_window_backend_linux.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>
#include <GL/gl.h>
/* clang-format on */

struct ui_window {
  Display *display;
  Window window;
  GLXContext glc;
  void *context;
  int is_closing;
  Atom wm_delete_window;
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

  win_obj->display = dpy;
  win_obj->window = win;
  win_obj->glc = glc;
  win_obj->context = NULL;
  win_obj->is_closing = 0;
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
    } else if (xev.type == ClientMessage) {
      if ((Atom)xev.xclient.data.l[0] == window->wm_delete_window) {
        out_event->type = UI_EVENT_WINDOW_CLOSE;
      }
    }
  }

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
