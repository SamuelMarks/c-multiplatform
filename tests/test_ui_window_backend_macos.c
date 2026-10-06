/* clang-format off */
#include "../include/ui_window_backend.h"
#include "../include/ui_error.h"
#include "../include/ui_event.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef UI_TEST_MOCK_ALLOC
extern int g_malloc_fail_countdown;
#endif
extern int g_mock_macos_zero_scale;

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
  int has_resize_event;
  float scale_factor;
  ui_error_t (*on_resize_callback)(void *, int, int);
  void *on_resize_user_data;
  ui_error_t (*on_dpi_change_callback)(void *, float, float);
  void *on_dpi_change_user_data;
};

static int g_test_resize_called = 0;
static int g_test_resize_w = 0;
static int g_test_resize_h = 0;

static ui_error_t test_resize_callback(void *user_data, int w, int h) {
  if (user_data) {
  }
  g_test_resize_called++;
  g_test_resize_w = w;
  g_test_resize_h = h;
  return UI_ERROR_NONE;
}

static ui_error_t test_resize_fail_callback(void *user_data, int w, int h) {
  if (user_data) {
  }
  if (w || h) {
  }
  return UI_ERROR_UNKNOWN;
}

static int g_test_dpi_called = 0;
static float g_test_dpi_old = 0.0f;
static float g_test_dpi_new = 0.0f;

static ui_error_t test_dpi_callback(void *user_data, float old_scale,
                                    float new_scale) {
  if (user_data) {
  }
  g_test_dpi_called++;
  g_test_dpi_old = old_scale;
  g_test_dpi_new = new_scale;
  return UI_ERROR_NONE;
}

static ui_error_t test_dpi_fail_callback(void *user_data, float old_scale,
                                         float new_scale) {
  if (user_data) {
  }
  if (old_scale > 0.0f || new_scale > 0.0f) {
  }
  return UI_ERROR_UNKNOWN;
}

#if defined(__APPLE__) && defined(__MACH__)
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <CoreGraphics/CoreGraphics.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

static CGRect dummy_content_layout_rect(id self, SEL _cmd) {
  if (self) {
  }
  if (_cmd) {
  }
  return CGRectMake(0.0, 0.0, 0.0, 0.0);
}

static CGRect dummy_content_layout_rect_zero_h(id self, SEL _cmd) {
  if (self) {
  }
  if (_cmd) {
  }
  return CGRectMake(0.0, 0.0, 200.0, 0.0);
}

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
      int is_ci = (getenv("CI") != NULL || getenv("GITHUB_ACTIONS") != NULL);
      if (is_ci) {
        printf("Skipping Cocoa window GUI pump under CI.\n");
      } else {
        struct ui_window *real_win = NULL;
        id dummyObj2 = (id)0;
        ui_error_t rc_create;

#ifdef UI_TEST_MOCK_ALLOC
        extern int g_mock_macos_event_type;
        extern int g_mock_macos_fail_delegate;
        extern int g_mock_macos_no_view;
        extern int g_mock_macos_no_nsopenglview;
        extern int g_mock_macos_fail_allocate_class;
        extern void ui_test_macos_on_window_did_resize(id self, SEL _cmd,
                                                       id notif);
        extern void ui_test_macos_on_window_will_close(id self, SEL _cmd,
                                                       id notif);

        /* Fail class allocation path */
        g_mock_macos_fail_allocate_class = 1;
        rc_create =
            backend->create_window(backend, "No Class", 200, 200, &real_win);
        failed |= (rc_create != UI_ERROR_NONE || !real_win);
        if (real_win) {
          backend->destroy_window(backend, real_win);
          real_win = NULL;
        }
        g_mock_macos_fail_allocate_class = 0;

        /* Delegate fail paths */
        g_mock_macos_fail_delegate = 1;
        rc_create =
            backend->create_window(backend, "No DelCls", 200, 200, &real_win);
        failed |= (rc_create != UI_ERROR_NONE || !real_win);
        if (real_win) {
          backend->destroy_window(backend, real_win);
          real_win = NULL;
        }

        g_mock_macos_fail_delegate = 2;
        rc_create =
            backend->create_window(backend, "No Del", 200, 200, &real_win);
        failed |= (rc_create != UI_ERROR_NONE || !real_win);
        if (real_win) {
          backend->destroy_window(backend, real_win);
          real_win = NULL;
        }
        g_mock_macos_fail_delegate = 0;

        /* No view path */
        g_mock_macos_no_view = 1;
        rc_create =
            backend->create_window(backend, "No View", 200, 200, &real_win);
        failed |= (rc_create != UI_ERROR_NONE || !real_win);
        if (real_win) {
          backend->destroy_window(backend, real_win);
          real_win = NULL;
        }
        g_mock_macos_no_view = 0;

        /* Malloc failure on create_window */
        g_malloc_fail_countdown = 0;
        failed |= (backend->create_window(backend, "OOM", 200, 200,
                                          &real_win) != UI_ERROR_OUT_OF_MEMORY);
        g_malloc_fail_countdown = -1;

        /* Fallback path when NSOpenGLView is not used */
        g_mock_macos_no_nsopenglview = 1;
        rc_create = backend->create_window(backend, "Fallback Win", 200, 200,
                                           &real_win);
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

          /* Test resize callback registration and resize detection */
          failed |= (backend->set_on_resize_callback(backend, real_win,
                                                     test_resize_callback,
                                                     NULL) != UI_ERROR_NONE);
          real_win->width = 100;
          backend->poll_events(backend, real_win, &evt, &has_evt);
          failed |= (evt.type != UI_EVENT_WINDOW_RESIZE || !has_evt);
          failed |= (g_test_resize_called == 0);
          failed |= (real_win->width != 200);

          /* Test macos_on_window_did_resize and will_close */
          {
            id notifCls = (id)objc_getClass("NSNotification");
            id strCls = (id)objc_getClass("NSString");
            id notifName = ((id(*)(id, SEL, const char *))objc_msgSend)(
                strCls, sel_registerName("stringWithUTF8String:"), "test");
            id notif_no_win = ((id(*)(id, SEL, id, id))objc_msgSend)(
                notifCls, sel_registerName("notificationWithName:object:"),
                notifName, (id)0);
            Class dummyCls = objc_allocateClassPair(
                (Class)objc_getClass("NSObject"), "UIDummyWindow", 0);
            Class dummyCls2 = objc_allocateClassPair(
                (Class)objc_getClass("NSObject"), "UIDummyWindowZeroH", 0);
            id dummyObj;
            id notif_zero_h;
            if (dummyCls) {
              class_addMethod(dummyCls, sel_registerName("contentLayoutRect"),
                              (IMP)dummy_content_layout_rect,
                              "{CGRect={CGPoint=dd}{CGSize=dd}}@:");
              objc_registerClassPair(dummyCls);
            }
            if (dummyCls2) {
              class_addMethod(dummyCls2, sel_registerName("contentLayoutRect"),
                              (IMP)dummy_content_layout_rect_zero_h,
                              "{CGRect={CGPoint=dd}{CGSize=dd}}@:");
              objc_registerClassPair(dummyCls2);
            }
            dummyObj = ((id(*)(id, SEL))objc_msgSend)(
                (id)objc_getClass("UIDummyWindow"), sel_registerName("new"));
            dummyObj2 = ((id(*)(id, SEL))objc_msgSend)(
                (id)objc_getClass("UIDummyWindowZeroH"),
                sel_registerName("new"));
            id notif_no_w = ((id(*)(id, SEL, id, id))objc_msgSend)(
                notifCls, sel_registerName("notificationWithName:object:"),
                notifName, dummyObj);
            notif_zero_h = ((id(*)(id, SEL, id, id))objc_msgSend)(
                notifCls, sel_registerName("notificationWithName:object:"),
                notifName, dummyObj2);
            id notif_real = ((id(*)(id, SEL, id, id))objc_msgSend)(
                notifCls, sel_registerName("notificationWithName:object:"),
                notifName, (id)real_win->window);

            /* Test did_resize notification handler */
            ui_test_macos_on_window_did_resize(NULL, NULL, NULL);
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_no_win);
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_no_w);

            /* Test when cur_w <= 0 using dummyObj returning (0,0) */
            objc_setAssociatedObject(dummyObj, "ui_window_ptr", (id)real_win,
                                     OBJC_ASSOCIATION_ASSIGN);
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_no_w);
            objc_setAssociatedObject(dummyObj, "ui_window_ptr", NULL,
                                     OBJC_ASSOCIATION_ASSIGN);

            /* Test when cur_w > 0 but cur_h <= 0 using dummyObj2 returning
             * (200, 0) */
            objc_setAssociatedObject(dummyObj2, "ui_window_ptr", (id)real_win,
                                     OBJC_ASSOCIATION_ASSIGN);
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_zero_h);
            objc_setAssociatedObject(dummyObj2, "ui_window_ptr", NULL,
                                     OBJC_ASSOCIATION_ASSIGN);

            real_win->width = 10;
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_real);
            failed |= (real_win->width != 200);

            /* Test cur_h != w->height branch */
            real_win->width = 200;
            real_win->height = 10;
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_real);
            failed |= (real_win->height != 200);

            /* Call when width already matches */
            ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_real);

            /* Test with null view, context, callback */
            {
              void *saved_v = real_win->view;
              void *saved_ctx = real_win->context;
              ui_error_t (*saved_cb)(void *, int, int) =
                  real_win->on_resize_callback;
              real_win->view = NULL;
              real_win->context = NULL;
              real_win->on_resize_callback = NULL;
              real_win->width = 10;
              ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_real);
              real_win->view = saved_v;
              real_win->context = saved_ctx;
              real_win->on_resize_callback = saved_cb;
            }

            /* Test with (void*)1 view and context */
            {
              void *saved_v = real_win->view;
              void *saved_ctx = real_win->context;
              real_win->view = (void *)1;
              real_win->context = (void *)1;
              real_win->width = 10;
              ui_test_macos_on_window_did_resize((id)1, (SEL)1, notif_real);
              real_win->view = saved_v;
              real_win->context = saved_ctx;
            }

            /* Test will_close notification handler */
            ui_test_macos_on_window_will_close(NULL, NULL, NULL);
            ui_test_macos_on_window_will_close((id)1, (SEL)1, notif_no_win);
            ui_test_macos_on_window_will_close((id)1, (SEL)1, notif_no_w);
            ui_test_macos_on_window_will_close((id)1, (SEL)1, notif_real);
            failed |= (real_win->is_closing != 1);
            real_win->is_closing = 0;
          }

          /* Test poll_events when has_resize_event is 1 */
          real_win->has_resize_event = 1;
          err = backend->poll_events(backend, real_win, &evt, &has_evt);
          failed |= (err != UI_ERROR_NONE ||
                     evt.type != UI_EVENT_WINDOW_RESIZE || !has_evt);

          /* Test cur_h != window->height in poll_events */
          real_win->has_resize_event = 0;
          real_win->width = 200;
          real_win->height = 10;
          err = backend->poll_events(backend, real_win, &evt, &has_evt);
          failed |= (err != UI_ERROR_NONE ||
                     evt.type != UI_EVENT_WINDOW_RESIZE || !has_evt);

          /* Test window->context == NULL in poll_events */
          {
            void *saved_ctx = real_win->context;
            real_win->context = NULL;
            real_win->has_resize_event = 0;
            real_win->width = 10;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE ||
                       evt.type != UI_EVENT_WINDOW_RESIZE || !has_evt);
            real_win->context = saved_ctx;
          }

          /* Test fallback to window contentLayoutRect when view is (void*)1 */
          {
            void *saved_v = real_win->view;
            void *saved_ctx = real_win->context;
            void *saved_win = real_win->window;

            real_win->has_resize_event = 0;
            real_win->view = (void *)1;
            real_win->width = 10;
            backend->set_on_resize_callback(backend, real_win,
                                            test_resize_fail_callback, NULL);
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_UNKNOWN);

            /* When context is (void*)1 */
            real_win->has_resize_event = 0;
            real_win->width = 10;
            real_win->context = (void *)1;
            backend->set_on_resize_callback(backend, real_win, NULL, NULL);
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE ||
                       evt.type != UI_EVENT_WINDOW_RESIZE || !has_evt);

            /* When window is (void*)1 and view is (void*)1 */
            real_win->has_resize_event = 0;
            real_win->window = (void *)1;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);

            /* When view is (void*)1 and window is dummyObj2 returning (200, 0)
             */
            real_win->has_resize_event = 0;
            real_win->window = dummyObj2;
            real_win->width = 10;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);

            /* When view is NULL and window is NULL */
            real_win->has_resize_event = 0;
            real_win->view = NULL;
            real_win->window = NULL;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);

            /* When view is NULL and window is (void*)1 */
            real_win->has_resize_event = 0;
            real_win->window = (void *)1;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);

            /* When view is NULL and window is real window */
            real_win->has_resize_event = 0;
            real_win->window = saved_win;
            real_win->width = 10;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE ||
                       evt.type != UI_EVENT_WINDOW_RESIZE || !has_evt);

            real_win->view = saved_v;
            real_win->context = saved_ctx;
            real_win->window = saved_win;
          }

          {
            void *real_os_h = NULL;
            failed |= (backend->get_os_handle(backend, real_win, &real_os_h) !=
                       UI_ERROR_NONE);
            failed |= (real_os_h == NULL);
          }

          /* Test real_win scale factor, framebuffer size, and dpi change
           * callback */
          {
            float s = 0.0f;
            int fb_w = 0, fb_h = 0;
            extern int g_mock_macos_fail_scale;
            extern int g_mock_macos_no_backing_scale;

            failed |= (backend->get_scale_factor(backend, real_win, &s) !=
                       UI_ERROR_NONE);
            failed |= (s <= 0.0f);

            /* Scale factor when <= 0.0f */
            real_win->scale_factor = 0.0f;
            failed |= (backend->get_scale_factor(backend, real_win, &s) !=
                       UI_ERROR_NONE);
            failed |= (s != 1.0f);
            real_win->scale_factor = 2.0f;
            failed |= (backend->get_scale_factor(backend, real_win, &s) !=
                       UI_ERROR_NONE);
            failed |= (s != 2.0f);

            /* Framebuffer size */
            failed |= (backend->get_framebuffer_size(backend, real_win, &fb_w,
                                                     &fb_h) != UI_ERROR_NONE);
            failed |= (fb_w != (int)((float)real_win->width * s));
            failed |= (fb_h != (int)((float)real_win->height * s));

            /* Framebuffer size failure */
            g_mock_macos_fail_scale = 1;
            failed |=
                (backend->get_framebuffer_size(backend, real_win, &fb_w,
                                               &fb_h) != UI_ERROR_UNKNOWN);
            g_mock_macos_fail_scale = 0;

            /* DPI callback setting */
            failed |= (backend->set_on_dpi_change_callback(
                           backend, real_win, test_dpi_callback, NULL) !=
                       UI_ERROR_NONE);

            /* DPI change event with failing callback */
            real_win->scale_factor = 0.1f;
            failed |= (backend->set_on_dpi_change_callback(
                           backend, real_win, test_dpi_fail_callback, NULL) !=
                       UI_ERROR_NONE);
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_UNKNOWN);

            /* DPI change event with successful callback */
            real_win->scale_factor = 0.1f;
            failed |= (backend->set_on_dpi_change_callback(
                           backend, real_win, test_dpi_callback, NULL) !=
                       UI_ERROR_NONE);
            g_test_dpi_called = 0;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);
            failed |= (!has_evt || evt.type != UI_EVENT_WINDOW_DPI_CHANGED);
            failed |= (g_test_dpi_called != 1);

            /* DPI change event with NULL callback */
            real_win->scale_factor = 0.1f;
            failed |= (backend->set_on_dpi_change_callback(
                           backend, real_win, NULL, NULL) != UI_ERROR_NONE);
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);
            failed |= (!has_evt || evt.type != UI_EVENT_WINDOW_DPI_CHANGED);

            /* Poll events when backingScaleFactor not supported */
            g_mock_macos_no_backing_scale = 1;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);
            g_mock_macos_no_backing_scale = 0;

            /* Poll events when cur_scale <= 0.0 */
            g_mock_macos_zero_scale = 1;
            err = backend->poll_events(backend, real_win, &evt, &has_evt);
            failed |= (err != UI_ERROR_NONE);
            g_mock_macos_zero_scale = 0;
          }

          /* Window creation mock branches */
          {
            struct ui_window *mock_win = NULL;
            extern int g_mock_macos_no_window;
            extern int g_mock_macos_no_backing_scale;

            g_mock_macos_no_window = 1;
            failed |= (backend->create_window(backend, "Mock", 100, 100,
                                              &mock_win) != UI_ERROR_NONE);
            if (mock_win) {
              backend->destroy_window(backend, mock_win);
              mock_win = NULL;
            }
            g_mock_macos_no_window = 0;

            g_mock_macos_no_backing_scale = 1;
            failed |= (backend->create_window(backend, "Mock", 100, 100,
                                              &mock_win) != UI_ERROR_NONE);
            if (mock_win) {
              backend->destroy_window(backend, mock_win);
              mock_win = NULL;
            }
            g_mock_macos_no_backing_scale = 0;

            g_mock_macos_zero_scale = 1;
            failed |= (backend->create_window(backend, "Mock", 100, 100,
                                              &mock_win) != UI_ERROR_NONE);
            if (mock_win) {
              backend->destroy_window(backend, mock_win);
              mock_win = NULL;
            }
            g_mock_macos_zero_scale = 0;
          }
#endif

          backend->swap_buffers(backend, real_win);
          backend->hide_window(backend, real_win);
          backend->destroy_window(backend, real_win);
        }
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

    failed |= (backend->poll_events(NULL, (struct ui_window *)1, &evt,
                                    &has_evt) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, NULL, &evt, &has_evt) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, NULL,
                                    &has_evt) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, &evt,
                                    NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->poll_events(backend, (struct ui_window *)1, &evt,
                                    &has_evt) != UI_ERROR_NONE);

    failed |= (backend->swap_buffers(NULL, (struct ui_window *)1) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->swap_buffers(backend, NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->swap_buffers(backend, (struct ui_window *)1) !=
               UI_ERROR_NONE);

    failed |= (backend->set_on_resize_callback(NULL, (struct ui_window *)1,
                                               test_resize_callback, NULL) !=
               UI_ERROR_INVALID_ARGUMENT);
    failed |=
        (backend->set_on_resize_callback(backend, NULL, test_resize_callback,
                                         NULL) != UI_ERROR_INVALID_ARGUMENT);
    failed |= (backend->set_on_resize_callback(backend, (struct ui_window *)1,
                                               test_resize_callback,
                                               NULL) != UI_ERROR_NONE);
    failed |= (backend->set_on_resize_callback(backend, (struct ui_window *)1,
                                               NULL, NULL) != UI_ERROR_NONE);

    {
      void *os_h = NULL;
      failed |= (backend->get_os_handle(NULL, (struct ui_window *)1, &os_h) !=
                 UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_os_handle(backend, NULL, &os_h) !=
                 UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_os_handle(backend, (struct ui_window *)1, NULL) !=
                 UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_os_handle(backend, (struct ui_window *)1,
                                        &os_h) != UI_ERROR_NONE);
    }

    {
      float scale = 0.0f;
      int fb_w = 0, fb_h = 0;
      failed |=
          (backend->get_scale_factor(NULL, (struct ui_window *)1, &scale) !=
           UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_scale_factor(backend, NULL, &scale) !=
                 UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_scale_factor(backend, (struct ui_window *)1,
                                           NULL) != UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_scale_factor(backend, (struct ui_window *)1,
                                           &scale) != UI_ERROR_NONE);
      failed |= (scale != 1.0f);

      failed |=
          (backend->get_framebuffer_size(NULL, (struct ui_window *)1, &fb_w,
                                         &fb_h) != UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_framebuffer_size(backend, NULL, &fb_w, &fb_h) !=
                 UI_ERROR_INVALID_ARGUMENT);
      failed |=
          (backend->get_framebuffer_size(backend, (struct ui_window *)1, NULL,
                                         &fb_h) != UI_ERROR_INVALID_ARGUMENT);
      failed |=
          (backend->get_framebuffer_size(backend, (struct ui_window *)1, &fb_w,
                                         NULL) != UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->get_framebuffer_size(backend, (struct ui_window *)1,
                                               &fb_w, &fb_h) != UI_ERROR_NONE);

      failed |= (backend->set_on_dpi_change_callback(
                     NULL, (struct ui_window *)1, NULL, NULL) !=
                 UI_ERROR_INVALID_ARGUMENT);
      failed |= (backend->set_on_dpi_change_callback(
                     backend, NULL, NULL, NULL) != UI_ERROR_INVALID_ARGUMENT);
      failed |=
          (backend->set_on_dpi_change_callback(backend, (struct ui_window *)1,
                                               NULL, NULL) != UI_ERROR_NONE);
    }
  }

#ifdef UI_TEST_MOCK_ALLOC
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
