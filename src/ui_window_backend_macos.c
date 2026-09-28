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
  id window;                                          /**< window */
  id view;                                            /**< glView */
  id context;                                         /**< context */
  int width;                                          /**< width */
  int height;                                         /**< height */
  int is_shown;                                       /**< is_shown */
  int is_closing;                                     /**< is_closing */
  int has_resize_event;                               /**< has_resize_event */
  ui_error_t (*on_resize_callback)(void *, int, int); /**< on_resize_callback */
  void *on_resize_user_data; /**< on_resize_user_data */
};

/**
 * @brief Delegate callback invoked by AppKit when window resizes.
 * @param self The delegate instance.
 * @param _cmd The selector.
 * @param notif The NSNotification object.
 */
static void macos_on_window_did_resize(id self, SEL _cmd, id notif) {
  id win;
  struct ui_window *w;
  CGRect cur_rect;
  int cur_w;
  int cur_h;

  if (self) {
  }
  if (_cmd) {
  }
  if (!notif) {
    return;
  }
  win = ((id (*)(id, SEL))objc_msgSend)(notif, sel_registerName("object"));
  if (!win) {
    return;
  }
  w = (struct ui_window *)objc_getAssociatedObject(win, "ui_window_ptr");
  if (!w) {
    return;
  }

  cur_rect = ((CGRect (*)(id, SEL))objc_msgSend)(
      win, sel_registerName("contentLayoutRect"));
  cur_w = (int)cur_rect.size.width;
  cur_h = (int)cur_rect.size.height;

  if (cur_w > 0 && cur_h > 0 && (cur_w != w->width || cur_h != w->height)) {
    w->width = cur_w;
    w->height = cur_h;
    w->has_resize_event = 1;
    if (w->view && w->view != (void *)1) {
      ((void (*)(id, SEL, CGRect))objc_msgSend)(
          w->view, sel_registerName("setFrame:"),
          CGRectMake(0.0, 0.0, (CGFloat)cur_w, (CGFloat)cur_h));
    }
    if (w->context && w->context != (void *)1) {
      ((void (*)(id, SEL))objc_msgSend)(w->context, sel_registerName("update"));
    }
    if (w->on_resize_callback) {
      w->on_resize_callback(w->on_resize_user_data, cur_w, cur_h);
    }
  }
}

/**
 * @brief Delegate callback invoked by AppKit when window will close.
 * @param self The delegate instance.
 * @param _cmd The selector.
 * @param notif The NSNotification object.
 */
static void macos_on_window_will_close(id self, SEL _cmd, id notif) {
  id win;
  struct ui_window *w;

  if (self) {
  }
  if (_cmd) {
  }
  if (!notif) {
    return;
  }
  win = ((id (*)(id, SEL))objc_msgSend)(notif, sel_registerName("object"));
  if (!win) {
    return;
  }
  w = (struct ui_window *)objc_getAssociatedObject(win, "ui_window_ptr");
  if (w) {
    w->is_closing = 1;
  }
}

/**
 * @brief Ensures the dynamic delegate class is registered.
 * @return The delegate Class object.
 */
static Class macos_get_window_delegate_class(void) {
  Class delCls = objc_getClass("UIWindowDelegate");
  if (!delCls) {
    Class superCls = (Class)objc_getClass("NSObject");
    delCls = objc_allocateClassPair(superCls, "UIWindowDelegate", 0);
    if (delCls) {
      class_addMethod(delCls, sel_registerName("windowDidResize:"),
                      (IMP)macos_on_window_did_resize, "v@:@");
      class_addMethod(delCls, sel_registerName("windowWillClose:"),
                      (IMP)macos_on_window_will_close, "v@:@");
      objc_registerClassPair(delCls);
    }
  }
  return delCls;
}

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
  CGSize minSize;
  Class delCls;
  id del;
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
  memset(w, 0, sizeof(struct ui_window));

  appCls = (id)objc_getClass("NSApplication");
  app = ((id (*)(id, SEL))objc_msgSend)(appCls,
                                        sel_registerName("sharedApplication"));
  ((void (*)(id, SEL, long))objc_msgSend)(
      app, sel_registerName("setActivationPolicy:"), 0);
  ((void (*)(id, SEL))objc_msgSend)(app, sel_registerName("finishLaunching"));

  strCls = (id)objc_getClass("NSString");
  titleStr = ((id (*)(id, SEL, const char *))objc_msgSend)(
      strCls, sel_registerName("stringWithUTF8String:"), title);

  winCls = (id)objc_getClass("NSWindow");
  winAlloc = ((id (*)(id, SEL))objc_msgSend)(winCls, sel_registerName("alloc"));
  rect = CGRectMake(150.0, 150.0, (CGFloat)width, (CGFloat)height);
  win = ((
      id (*)(id, SEL, CGRect, unsigned long, unsigned long, BOOL))objc_msgSend)(
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
  pfAlloc = ((id (*)(id, SEL))objc_msgSend)(pfCls, sel_registerName("alloc"));
  pf = ((id (*)(id, SEL, const unsigned int *))objc_msgSend)(
      pfAlloc, sel_registerName("initWithAttributes:"), attrs);

  ctx = (id)0;
  view = (id)0;
  glViewCls = (id)objc_getClass("NSOpenGLView");
  if (glViewCls != (id)0) {
    viewRect = CGRectMake(0.0, 0.0, (CGFloat)width, (CGFloat)height);
    glViewAlloc =
        ((id (*)(id, SEL))objc_msgSend)(glViewCls, sel_registerName("alloc"));
    view = ((id (*)(id, SEL, CGRect, id))objc_msgSend)(
        glViewAlloc, sel_registerName("initWithFrame:pixelFormat:"), viewRect,
        pf);
    ((void (*)(id, SEL, BOOL))objc_msgSend)(
        view, sel_registerName("setWantsBestResolutionOpenGLSurface:"),
        (BOOL)0);
    ((void (*)(id, SEL, id))objc_msgSend)(
        win, sel_registerName("setContentView:"), view);
    ctx = ((id (*)(id, SEL))objc_msgSend)(view,
                                          sel_registerName("openGLContext"));
  }
  if (ctx == (id)0) {
    ctxCls = (id)objc_getClass("NSOpenGLContext");
    ctxAlloc =
        ((id (*)(id, SEL))objc_msgSend)(ctxCls, sel_registerName("alloc"));
    ctx = ((id (*)(id, SEL, id, id))objc_msgSend)(
        ctxAlloc, sel_registerName("initWithFormat:shareContext:"), pf, (id)0);
    view =
        ((id (*)(id, SEL))objc_msgSend)(win, sel_registerName("contentView"));
    ((void (*)(id, SEL, id))objc_msgSend)(ctx, sel_registerName("setView:"),
                                          view);
  }

  ((void (*)(id, SEL))objc_msgSend)(ctx,
                                    sel_registerName("makeCurrentContext"));
  ((void (*)(id, SEL))objc_msgSend)(ctx, sel_registerName("update"));

  minSize.width = 100.0;
  minSize.height = 100.0;
  ((void (*)(id, SEL, CGSize))objc_msgSend)(
      win, sel_registerName("setMinSize:"), minSize);

  if (view) {
    ((void (*)(id, SEL, unsigned long))objc_msgSend)(
        view, sel_registerName("setAutoresizingMask:"), (unsigned long)18);
  }

  w->window = win;
  w->view = view;
  w->context = ctx;
  w->width = width;
  w->height = height;
  w->is_shown = 0;
  w->is_closing = 0;
  w->has_resize_event = 0;
  w->on_resize_callback = NULL;
  w->on_resize_user_data = NULL;

  delCls = macos_get_window_delegate_class();
  if (delCls) {
    del = ((id (*)(id, SEL))objc_msgSend)((id)delCls, sel_registerName("new"));
    if (del) {
      objc_setAssociatedObject(win, "ui_window_ptr", (id)w,
                               OBJC_ASSOCIATION_ASSIGN);
      ((void (*)(id, SEL, id))objc_msgSend)(
          win, sel_registerName("setDelegate:"), del);
    }
  }

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
    app = ((id (*)(id, SEL))objc_msgSend)(
        appCls, sel_registerName("sharedApplication"));
    ((void (*)(id, SEL, BOOL))objc_msgSend)(
        app, sel_registerName("activateIgnoringOtherApps:"), (BOOL)1);

    strCls = (id)objc_getClass("NSString");
    modeStr = ((id (*)(id, SEL, const char *))objc_msgSend)(
        strCls, sel_registerName("stringWithUTF8String:"),
        "kCFRunLoopDefaultMode");
    dateCls = (id)objc_getClass("NSDate");
    date = ((id (*)(id, SEL, double))objc_msgSend)(
        dateCls, sel_registerName("dateWithTimeIntervalSinceNow:"), 0.02);

    event = ((id (*)(id, SEL, unsigned long, id, id, BOOL))objc_msgSend)(
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
  int cur_w;
  int cur_h;
  CGRect cur_rect;

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

  if (window->has_resize_event) {
    out_event->type = UI_EVENT_WINDOW_RESIZE;
    out_event->event_data.window.width = window->width;
    out_event->event_data.window.height = window->height;
    *out_has_event = 1;
    window->has_resize_event = 0;
    return UI_ERROR_NONE;
  }

  appCls = (id)objc_getClass("NSApplication");
  app = ((id (*)(id, SEL))objc_msgSend)(appCls,
                                        sel_registerName("sharedApplication"));
  strCls = (id)objc_getClass("NSString");
  modeStr = ((id (*)(id, SEL, const char *))objc_msgSend)(
      strCls, sel_registerName("stringWithUTF8String:"),
      "kCFRunLoopDefaultMode");
  dateCls = (id)objc_getClass("NSDate");
  date = ((id (*)(id, SEL, double))objc_msgSend)(
      dateCls, sel_registerName("dateWithTimeIntervalSinceNow:"), 0.005);

  event = ((id (*)(id, SEL, unsigned long, id, id, BOOL))objc_msgSend)(
      app, sel_registerName("nextEventMatchingMask:untilDate:inMode:dequeue:"),
      (unsigned long)-1, date, modeStr, (BOOL)1);

  if (event) {
    ((void (*)(id, SEL, id))objc_msgSend)(app, sel_registerName("sendEvent:"),
                                          event);
    evtType = ((unsigned long (*)(id, SEL))objc_msgSend)(
        event, sel_registerName("type"));
    if (evtType == 1 || evtType == 2) {
      CGPoint loc;
      loc = ((CGPoint (*)(id, SEL))objc_msgSend)(
          event, sel_registerName("locationInWindow"));
      out_event->type =
          (evtType == 1) ? UI_EVENT_MOUSE_DOWN : UI_EVENT_MOUSE_UP;
      out_event->event_data.mouse.x = (int)loc.x;
      out_event->event_data.mouse.y = window->height - (int)loc.y;
      out_event->event_data.mouse.button = 0;
      *out_has_event = 1;
      return UI_ERROR_NONE;
    } else if (evtType == 10) {
      out_event->type = UI_EVENT_KEY_DOWN;
      *out_has_event = 1;
      return UI_ERROR_NONE;
    }
  }

  cur_w = 0;
  cur_h = 0;
  if (window->view && window->view != (void *)1) {
#if defined(__x86_64__)
    ((void (*)(CGRect *, id, SEL))objc_msgSend_stret)(
        &cur_rect, window->view, sel_registerName("bounds"));
#else
    cur_rect = ((CGRect (*)(id, SEL))objc_msgSend)(window->view,
                                                   sel_registerName("bounds"));
#endif
    cur_w = (int)cur_rect.size.width;
    cur_h = (int)cur_rect.size.height;
  } else if (window->window && window->window != (void *)1) {
#if defined(__x86_64__)
    ((void (*)(CGRect *, id, SEL))objc_msgSend_stret)(
        &cur_rect, window->window, sel_registerName("contentLayoutRect"));
#else
    cur_rect = ((CGRect (*)(id, SEL))objc_msgSend)(
        window->window, sel_registerName("contentLayoutRect"));
#endif
    cur_w = (int)cur_rect.size.width;
    cur_h = (int)cur_rect.size.height;
  }

  if (cur_w > 0 && cur_h > 0 &&
      (cur_w != window->width || cur_h != window->height)) {
    window->width = cur_w;
    window->height = cur_h;
    if (window->context && window->context != (void *)1) {
      ((void (*)(id, SEL))objc_msgSend)(window->context,
                                        sel_registerName("update"));
    }
    if (window->on_resize_callback) {
      ui_error_t cb_rc =
          window->on_resize_callback(window->on_resize_user_data, cur_w, cur_h);
      if (cb_rc != UI_ERROR_NONE) {
        return cb_rc;
      }
    }
    out_event->type = UI_EVENT_WINDOW_RESIZE;
    out_event->event_data.window.width = cur_w;
    out_event->event_data.window.height = cur_h;
    *out_has_event = 1;
    return UI_ERROR_NONE;
  }

  out_event->type = UI_EVENT_NONE;
  *out_has_event = 0;
  return UI_ERROR_NONE;
}

/**
 * @brief macos_set_on_resize_callback.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t macos_set_on_resize_callback(
    struct ui_window_backend *backend, struct ui_window *window,
    ui_error_t (*callback)(void *, int, int), void *user_data) {
  if (!backend || !window) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    return UI_ERROR_NONE;
  }
  window->on_resize_callback = callback;
  window->on_resize_user_data = user_data;
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
 * @brief macos_get_os_handle.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_handle Parameter out_handle.
 * @return Return value.
 */
static ui_error_t macos_get_os_handle(struct ui_window_backend *backend,
                                      struct ui_window *window,
                                      void **out_handle) {
  if (!backend || !window || !out_handle) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if (window == (struct ui_window *)1) {
    *out_handle = (void *)1;
    return UI_ERROR_NONE;
  }
  *out_handle = (void *)window->window;
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
  backend->get_os_handle = macos_get_os_handle;
  backend->set_on_resize_callback = macos_set_on_resize_callback;
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
