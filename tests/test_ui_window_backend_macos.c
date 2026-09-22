/* clang-format off */
#include "../include/ui_window_backend.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#endif
/* clang-format on */

struct ui_window {
  void *window;
  void *view;
  void *context;
  int width;
  int height;
  int is_shown;
  int is_closing;
};

#if defined(__APPLE__) && defined(__MACH__)
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <CoreGraphics/CoreGraphics.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

static void post_test_mouse_event(unsigned long type) {
  id appCls = (id)objc_getClass("NSApplication");
  id app = ((id(*)(id, SEL))objc_msgSend)(
      appCls, sel_registerName("sharedApplication"));
  id evtCls = (id)objc_getClass("NSEvent");
  CGPoint loc;
  id evt;
  loc.x = 10.0;
  loc.y = 10.0;
  evt = ((id(*)(id, SEL, unsigned long, CGPoint, unsigned long, double, long,
                id, long, long, float))objc_msgSend)(
      evtCls,
      sel_registerName("mouseEventWithType:location:modifierFlags:timestamp:"
                       "windowNumber:context:eventNumber:clickCount:pressure:"),
      type, loc, 0, 0.0, 0, (id)0, 0, 1, 1.0f);
  if (evt) {
    ((void (*)(id, SEL, id, BOOL))objc_msgSend)(
        app, sel_registerName("postEvent:atStart:"), evt, (BOOL)1);
  }
}

static void post_test_key_event(void) {
  id appCls = (id)objc_getClass("NSApplication");
  id app = ((id(*)(id, SEL))objc_msgSend)(
      appCls, sel_registerName("sharedApplication"));
  id evtCls = (id)objc_getClass("NSEvent");
  id strCls = (id)objc_getClass("NSString");
  id str = ((id(*)(id, SEL, const char *))objc_msgSend)(
      strCls, sel_registerName("stringWithUTF8String:"), "a");
  CGPoint loc;
  id evt;
  loc.x = 10.0;
  loc.y = 10.0;
  evt = ((id(*)(id, SEL, unsigned long, CGPoint, unsigned long, double, long,
                id, id, id, BOOL, unsigned short))objc_msgSend)(
      evtCls,
      sel_registerName(
          "keyEventWithType:location:modifierFlags:timestamp:"
          "windowNumber:context:characters:charactersIgnoringModifiers:"
          "isARepeat:keyCode:"),
      10, loc, 0, 0.0, 0, (id)0, str, str, (BOOL)0, 0);
  if (evt) {
    ((void (*)(id, SEL, id, BOOL))objc_msgSend)(
        app, sel_registerName("postEvent:atStart:"), evt, (BOOL)1);
  }
}

static void post_test_other_event(void) {
  id appCls = (id)objc_getClass("NSApplication");
  id app = ((id(*)(id, SEL))objc_msgSend)(
      appCls, sel_registerName("sharedApplication"));
  id evtCls = (id)objc_getClass("NSEvent");
  CGPoint loc;
  id evt;
  loc.x = 10.0;
  loc.y = 10.0;
  evt = ((id(*)(id, SEL, unsigned long, CGPoint, unsigned long, double, long,
                id, short, long, long))objc_msgSend)(
      evtCls,
      sel_registerName("otherEventWithType:location:modifierFlags:timestamp:"
                       "windowNumber:context:subtype:data1:data2:"),
      14, loc, 0, 0.0, 0, (id)0, 0, 0, 0);
  if (evt) {
    ((void (*)(id, SEL, id, BOOL))objc_msgSend)(
        app, sel_registerName("postEvent:atStart:"), evt, (BOOL)1);
  }
}
#endif
#endif

extern ui_error_t
ui_window_backend_macos_create(struct ui_window_backend **out_backend);
extern ui_error_t
ui_window_backend_macos_destroy(struct ui_window_backend *backend);

int main(void) {
  struct ui_window_backend *backend = NULL;
  ui_error_t err;
  int failed = 0;

  printf("Running ui_window_backend_macos tests...\n");

  err = ui_window_backend_macos_create(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_macos_destroy(NULL);
  failed |= (err != UI_ERROR_INVALID_ARGUMENT);

  err = ui_window_backend_macos_create(&backend);
#if defined(__APPLE__) && defined(__MACH__)
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
  failed |= (err != UI_ERROR_NONE);
  if (backend) {
    struct ui_window *window = NULL;
    struct ui_event evt;
    int has_evt;

    failed |= (backend->create_window(NULL, "Test", 100, 100, &window) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->create_window(backend, NULL, 100, 100, &window) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->create_window(backend, "Test", 100, 100, NULL) !=
               UI_ERROR_INVALID_ARGUMENT);

    /* Implementation returns UNKNOWN for now until FFI lands */
    failed |= (backend->create_window(backend, "Test", 100, 100, &window) !=
               UI_ERROR_UNKNOWN);

    {
      struct ui_window *real_win = NULL;
      ui_error_t rc_create;

#ifdef UI_TEST_MOCK_ALLOC
      extern int g_malloc_fail_countdown;
      extern int g_mock_macos_no_nsopenglview;
      extern int g_mock_macos_event_type;

      /* Malloc failure on create_window */
      g_malloc_fail_countdown = 0;
      failed |= (backend->create_window(backend, "OOM", 200, 200, &real_win) !=
                 UI_ERROR_OUT_OF_MEMORY);
      g_malloc_fail_countdown = -1;

      /* Fallback path when NSOpenGLView is not used */
      g_mock_macos_no_nsopenglview = 1;
      rc_create =
          backend->create_window(backend, "Fallback Win", 200, 200, &real_win);
      g_mock_macos_no_nsopenglview = 0;
      failed |= (rc_create != UI_ERROR_NONE || !real_win);
      if (rc_create == UI_ERROR_NONE && real_win) {
        backend->destroy_window(backend, real_win);
        real_win = NULL;
      }
#endif

      rc_create =
          backend->create_window(backend, "My Window", 200, 200, &real_win);
      failed |= (rc_create != UI_ERROR_NONE || !real_win);
      if (rc_create == UI_ERROR_NONE && real_win) {
        backend->show_window(backend, real_win);
        backend->poll_events(backend, real_win, &evt, &has_evt);

#ifdef UI_TEST_MOCK_ALLOC
        /* Test event types using real Cocoa posted events */
        post_test_mouse_event(1); /* MOUSE_DOWN */
        backend->poll_events(backend, real_win, &evt, &has_evt);
        failed |= (evt.type != UI_EVENT_MOUSE_DOWN || !has_evt);

        post_test_mouse_event(2); /* MOUSE_UP */
        backend->poll_events(backend, real_win, &evt, &has_evt);
        failed |= (evt.type != UI_EVENT_MOUSE_UP || !has_evt);

        post_test_key_event(); /* KEY_DOWN */
        backend->poll_events(backend, real_win, &evt, &has_evt);
        failed |= (evt.type != UI_EVENT_KEY_DOWN || !has_evt);

        post_test_other_event(); /* OTHER */
        backend->poll_events(backend, real_win, &evt, &has_evt);
        failed |= (evt.type != UI_EVENT_NONE || has_evt);

        /* Empty event queue check (event == NULL) */
        backend->poll_events(backend, real_win, &evt, &has_evt);
        failed |= (evt.type != UI_EVENT_NONE || has_evt);
#endif

        backend->swap_buffers(backend, real_win);
        backend->hide_window(backend, real_win);
        backend->destroy_window(backend, real_win);
      }

      /* Test dummy windows with NULL members */
      {
        struct ui_window *dummy =
            (struct ui_window *)malloc(sizeof(struct ui_window));
        if (dummy) {
          memset(dummy, 0, sizeof(*dummy));
          backend->show_window(backend, dummy);
          dummy->width = 100;
          dummy->height = 100;
          dummy->is_closing = 1;
          backend->poll_events(backend, dummy, &evt, &has_evt);
          failed |= (evt.type != UI_EVENT_WINDOW_CLOSE || !has_evt);
          backend->hide_window(backend, dummy);
          backend->swap_buffers(backend, dummy);
          backend->destroy_window(backend, dummy);
        }

        /* Test dummy window with view non-NULL but context NULL */
        dummy = (struct ui_window *)malloc(sizeof(struct ui_window));
        if (dummy) {
          memset(dummy, 0, sizeof(*dummy));
          dummy->view = (void *)1;
          backend->show_window(backend, dummy);
          backend->destroy_window(backend, dummy);
        }
      }
    }

    failed |=
        (backend->destroy_window(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->destroy_window(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->destroy_window(backend, (struct ui_window *)1) !=
               UI_ERROR_NONE);

    failed |= (backend->show_window(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->show_window(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->show_window(backend, (struct ui_window *)1) != UI_ERROR_NONE);

    failed |= (backend->hide_window(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->hide_window(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->hide_window(backend, (struct ui_window *)1) != UI_ERROR_NONE);

    failed |= (backend->poll_events(NULL, window, &evt, &has_evt) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, NULL, &evt, &has_evt) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, NULL,
                                    &has_evt) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, &evt,
                                    NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, &evt,
                                    &has_evt) != UI_ERROR_NONE);

    failed |=
        (backend->swap_buffers(NULL, window) != UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->swap_buffers(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->swap_buffers(backend, (struct ui_window *)1) !=
               UI_ERROR_NONE);
  }

#ifdef UI_TEST_MOCK_ALLOC
  extern int g_malloc_fail_countdown;
  g_malloc_fail_countdown = 0;
  failed |=
      (ui_window_backend_macos_create(&backend) != UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
#endif

  err = ui_window_backend_macos_destroy(backend);
  failed |= (err != UI_ERROR_NONE);
#else
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_macos_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif
#else
  failed |= (err != UI_ERROR_UNKNOWN);
  err = ui_window_backend_macos_destroy((struct ui_window_backend *)0x1234);
  failed |= (err != UI_ERROR_UNKNOWN);
#endif

  printf("ui_window_backend_macos tests passed.\n");
  return failed;
}
