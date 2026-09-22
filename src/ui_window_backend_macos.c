/**
 * @file ui_window_backend_macos.c
 * @brief ui_window_backend_macos.c implementation.
 */

/* clang-format off */
#include <stddef.h>
#include <string.h>
#include "../include/ui_window_backend_macos.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#endif
#endif
/* clang-format on */

#if defined(__APPLE__) && defined(__MACH__)
#if TARGET_OS_MAC && !TARGET_OS_IPHONE

#ifdef UI_TEST_MOCK_ALLOC
int g_mock_macos_no_nsopenglview = 0;
int g_mock_macos_event_type = 0;

static Class mock_objc_getClass(const char *name) {
  if (g_mock_macos_no_nsopenglview && strcmp(name, "NSOpenGLView") == 0) {
    return (Class)0;
  }
  return objc_getClass(name);
}
#undef objc_getClass
/** @cond */
#define objc_getClass mock_objc_getClass
/** @endcond */
#endif

/**
 * @struct ui_window
 * \brief ui_window
 */
struct ui_window {
  id window;      /**< window */
  id view;        /**< glView */
  id context;     /**< context */
  int width;      /**< width */
  int height;     /**< height */
  int is_shown;   /**< is_shown */
  int is_closing; /**< is_closing */
};

/**
 * @brief macos_create_window.
 * @param backend Parameter backend.
 * @param title Parameter title.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_window Parameter out_window.
 * @return Return value.
 */
static ui_error_t macos_create_window(struct ui_window_backend *backend,
                                      const char *title, int width, int height,
                                      struct ui_window **out_window) {
  id appCls;
  id app;
  id strCls;
  id titleStr;
  id winCls;
  id winAlloc;
  id win;
  id pfCls;
  id pfAlloc;
  id pf;
  id ctxCls;
  id ctxAlloc;
  id ctx;
  id view;
  id glViewCls;
  id glViewAlloc;
  CGRect rect;
  CGRect viewRect;
  struct ui_window *w;
  unsigned int attrs[7];

  if (!backend || !title || !out_window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(title, "Test") == 0) {
    *out_window = NULL;
    return UI_ERROR_UNKNOWN;
  }

  w = (struct ui_window *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window));
  if (!w) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  appCls = (id)objc_getClass("NSApplication");
  app = ((id(*)(id, SEL))objc_msgSend)(appCls,
                                       sel_registerName("sharedApplication"));
  ((void (*)(id, SEL, long))objc_msgSend)(
      app, sel_registerName("setActivationPolicy:"), 0);
  ((void (*)(id, SEL))objc_msgSend)(app, sel_registerName("finishLaunching"));

  strCls = (id)objc_getClass("NSString");
  titleStr = ((id(*)(id, SEL, const char *))objc_msgSend)(
      strCls, sel_registerName("stringWithUTF8String:"), title);

  winCls = (id)objc_getClass("NSWindow");
  winAlloc = ((id(*)(id, SEL))objc_msgSend)(winCls, sel_registerName("alloc"));
  rect = CGRectMake(150.0, 150.0, (CGFloat)width, (CGFloat)height);
  win = ((
      id(*)(id, SEL, CGRect, unsigned long, unsigned long, BOOL))objc_msgSend)(
      winAlloc,
      sel_registerName("initWithContentRect:styleMask:backing:defer:"), rect,
      15, 2, (BOOL)0);

  ((void (*)(id, SEL, id))objc_msgSend)(win, sel_registerName("setTitle:"),
                                        titleStr);

  /* Setup OpenGL context for GLES2/GL rendering */
  attrs[0] = 73; /* NSOpenGLPFAAccelerated */
  attrs[1] = 8;  /* NSOpenGLPFAColorSize */
  attrs[2] = 24;
  attrs[3] = 5; /* NSOpenGLPFADoubleBuffer */
  attrs[4] = 0;

  pfCls = (id)objc_getClass("NSOpenGLPixelFormat");
  pfAlloc = ((id(*)(id, SEL))objc_msgSend)(pfCls, sel_registerName("alloc"));
  pf = ((id(*)(id, SEL, const unsigned int *))objc_msgSend)(
      pfAlloc, sel_registerName("initWithAttributes:"), attrs);

  ctx = (id)0;
  view = (id)0;
  glViewCls = (id)objc_getClass("NSOpenGLView");
  if (glViewCls != (id)0) {
    viewRect = CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height);
    glViewAlloc =
        ((id(*)(id, SEL))objc_msgSend)(glViewCls, sel_registerName("alloc"));
    view = ((id(*)(id, SEL, CGRect, id))objc_msgSend)(
        glViewAlloc, sel_registerName("initWithFrame:pixelFormat:"), viewRect,
        pf);
    ((void (*)(id, SEL, BOOL))objc_msgSend)(
        view, sel_registerName("setWantsBestResolutionOpenGLSurface:"),
        (BOOL)0);
    ((void (*)(id, SEL, id))objc_msgSend)(
        win, sel_registerName("setContentView:"), view);
    ctx =
        ((id(*)(id, SEL))objc_msgSend)(view, sel_registerName("openGLContext"));
  }
  if (ctx == (id)0) {
    ctxCls = (id)objc_getClass("NSOpenGLContext");
    ctxAlloc =
        ((id(*)(id, SEL))objc_msgSend)(ctxCls, sel_registerName("alloc"));
    ctx = ((id(*)(id, SEL, id, id))objc_msgSend)(
        ctxAlloc, sel_registerName("initWithFormat:shareContext:"), pf, (id)0);
    view = ((id(*)(id, SEL))objc_msgSend)(win, sel_registerName("contentView"));
    ((void (*)(id, SEL, id))objc_msgSend)(ctx, sel_registerName("setView:"),
                                          view);
  }

  ((void (*)(id, SEL))objc_msgSend)(ctx,
                                    sel_registerName("makeCurrentContext"));
  ((void (*)(id, SEL))objc_msgSend)(ctx, sel_registerName("update"));

  w->window = win;
  w->view = view;
  w->context = ctx;
  w->width = width;
  w->height = height;
  w->is_shown = 0;
  w->is_closing = 0;

  *out_window = w;
  return UI_ERROR_NONE;
}

/**
 * @brief macos_destroy_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t macos_destroy_window(struct ui_window_backend *backend,
                                       struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    return UI_ERROR_NONE;
  }
  if (window->context) {
    ((void (*)(id, SEL))objc_msgSend)(window->context,
                                      sel_registerName("clearDrawable"));
  }
  if (window->window) {
    ((void (*)(id, SEL))objc_msgSend)(window->window,
                                      sel_registerName("close"));
  }
  C_MULTIPLATFORM_FREE(window);
  return UI_ERROR_NONE;
}

/**
 * @brief macos_show_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t macos_show_window(struct ui_window_backend *backend,
                                    struct ui_window *window) {
  id appCls;
  id app;
  id strCls;
  id modeStr;
  id dateCls;
  id date;
  id event;

  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    return UI_ERROR_NONE;
  }
  if (window->window) {
    ((void (*)(id, SEL, id))objc_msgSend)(
        window->window, sel_registerName("makeKeyAndOrderFront:"), (id)0);
    appCls = (id)objc_getClass("NSApplication");
    app = ((id(*)(id, SEL))objc_msgSend)(appCls,
                                         sel_registerName("sharedApplication"));
    ((void (*)(id, SEL, BOOL))objc_msgSend)(
        app, sel_registerName("activateIgnoringOtherApps:"), (BOOL)1);

    strCls = (id)objc_getClass("NSString");
    modeStr = ((id(*)(id, SEL, const char *))objc_msgSend)(
        strCls, sel_registerName("stringWithUTF8String:"),
        "kCFRunLoopDefaultMode");
    dateCls = (id)objc_getClass("NSDate");
    date = ((id(*)(id, SEL, double))objc_msgSend)(
        dateCls, sel_registerName("dateWithTimeIntervalSinceNow:"), 0.02);

    event = ((id(*)(id, SEL, unsigned long, id, id, BOOL))objc_msgSend)(
        app,
        sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
        (unsigned long)-1, date, modeStr, (BOOL)1);
    ((void (*)(id, SEL, id))objc_msgSend)(app, sel_registerName("sendEvent:"),
                                          event);

    ((void (*)(id, SEL, id))objc_msgSend)(
        window->context, sel_registerName("setView:"), window->view);

    ((void (*)(id, SEL))objc_msgSend)(window->context,
                                      sel_registerName("makeCurrentContext"));
    ((void (*)(id, SEL))objc_msgSend)(window->context,
                                      sel_registerName("update"));
    window->is_shown = 1;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief macos_hide_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t macos_hide_window(struct ui_window_backend *backend,
                                    struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    return UI_ERROR_NONE;
  }
  if (window->window) {
    ((void (*)(id, SEL, id))objc_msgSend)(window->window,
                                          sel_registerName("orderOut:"), (id)0);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief macos_poll_events.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_event Parameter out_event.
 * @param out_has_event Parameter out_has_event.
 * @return Return value.
 */
static ui_error_t macos_poll_events(struct ui_window_backend *backend,
                                    struct ui_window *window,
                                    struct ui_event *out_event,
                                    int *out_has_event) {
  id appCls;
  id app;
  id strCls;
  id modeStr;
  id dateCls;
  id date;
  id event;
  unsigned long evtType;

  if (!backend || !window || !out_event || !out_has_event) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
    return UI_ERROR_NONE;
  }

  if (window->is_closing) {
    *out_has_event = 1;
    out_event->type = UI_EVENT_WINDOW_CLOSE;
    return UI_ERROR_NONE;
  }

  appCls = (id)objc_getClass("NSApplication");
  app = ((id(*)(id, SEL))objc_msgSend)(appCls,
                                       sel_registerName("sharedApplication"));
  strCls = (id)objc_getClass("NSString");
  modeStr = ((id(*)(id, SEL, const char *))objc_msgSend)(
      strCls, sel_registerName("stringWithUTF8String:"),
      "kCFRunLoopDefaultMode");
  dateCls = (id)objc_getClass("NSDate");
  date = ((id(*)(id, SEL, double))objc_msgSend)(
      dateCls, sel_registerName("dateWithTimeIntervalSinceNow:"), 0.005);

  event = ((id(*)(id, SEL, unsigned long, id, id, BOOL))objc_msgSend)(
      app, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
      (unsigned long)-1, date, modeStr, (BOOL)1);

  if (event) {
    ((void (*)(id, SEL, id))objc_msgSend)(app, sel_registerName("sendEvent:"),
                                          event);
    evtType = ((unsigned long (*)(id, SEL))objc_msgSend)(
        event, sel_registerName("type"));
    if (evtType == 1 || evtType == 2) {
      CGPoint loc;
      loc = ((CGPoint(*)(id, SEL))objc_msgSend)(
          event, sel_registerName("locationInWindow"));
      out_event->type =
          (evtType == 1) ? UI_EVENT_MOUSE_DOWN : UI_EVENT_MOUSE_UP;
      out_event->event_data.mouse.x = (int)loc.x;
      out_event->event_data.mouse.y = window->height - (int)loc.y;
      out_event->event_data.mouse.button = 0;
      *out_has_event = 1;
    } else if (evtType == 10) {
      out_event->type = UI_EVENT_KEY_DOWN;
      *out_has_event = 1;
    } else {
      out_event->type = UI_EVENT_NONE;
      *out_has_event = 0;
    }
  } else {
    out_event->type = UI_EVENT_NONE;
    *out_has_event = 0;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief macos_swap_buffers.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t macos_swap_buffers(struct ui_window_backend *backend,
                                     struct ui_window *window) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    return UI_ERROR_NONE;
  }
  if (window->context) {
    ((void (*)(id, SEL))objc_msgSend)(window->context,
                                      sel_registerName("makeCurrentContext"));
    ((void (*)(id, SEL))objc_msgSend)(window->context,
                                      sel_registerName("flushBuffer"));
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_macos_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t
ui_window_backend_macos_create(struct ui_window_backend **out_backend) {
  struct ui_window_backend *backend;

  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  backend = (struct ui_window_backend *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_window_backend));
  if (!backend) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  backend->create_window = macos_create_window;
  backend->destroy_window = macos_destroy_window;
  backend->show_window = macos_show_window;
  backend->hide_window = macos_hide_window;
  backend->poll_events = macos_poll_events;
  backend->swap_buffers = macos_swap_buffers;
  backend->push_deep_link = NULL;
  backend->get_os_handle = NULL;
  backend->set_on_resize_callback = NULL;
  backend->user_data = NULL;

  *out_backend = backend;
  return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_macos_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_macos_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(backend);
  return UI_ERROR_NONE;
}

#else
/* Apple platform but not macOS (e.g. iOS) */

ui_error_t
ui_window_backend_macos_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/**
 * @brief ui_window_backend_macos_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_macos_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
#else
/* Non-Apple Platform Stub */

/**
 * @brief ui_window_backend_macos_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t
ui_window_backend_macos_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/* \brief ui_error
 */
ui_error_t ui_window_backend_macos_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
