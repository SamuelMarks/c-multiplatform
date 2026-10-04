/**
 * @file ui_window_backend_ios.c
 * @brief ui_window_backend_ios.c implementation.
 */
#if defined(__APPLE__) && defined(__MACH__)
/* clang-format off */
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE

#include "../include/ui_window_backend_ios.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

#include <CoreGraphics/CGGeometry.h>
#include <CoreGraphics/CGContext.h>


/**
 * @struct ui_window
 * \brief ui_window
 */
struct ui_window {
    id window; /**< window */
    id context; /**< context */
    id display_link; /**< display_link */
    int width; /**< width */
    int height; /**< height */
    int is_closing; /**< is_closing */
    float scale_factor; /**< scale_factor */
    ui_error_t (*on_dpi_change_callback)(void*, float, float); /**< on_dpi_change_callback */
    void* on_dpi_change_user_data; /**< on_dpi_change_user_data */
};

/**
 * @brief display_link_callback
 * @param self The object.
 * @param _cmd The selector.
 * @param displayLink The display link.
 * @return Returns UI_ERROR_NONE.
 */

/* --- iOS A11y & Dynamics Implementations --- */

/** @brief struct UIAccessibilityElement proxy */
struct ios_a11y_proxy {
    id element;
    void *internal_node;
};

static void ios_view_will_transition_to_size(id self, SEL _cmd, CGSize size, id coordinator) {
    /* Hook into viewWillTransitionToSize:withTransitionCoordinator: to capture rotation/resize events gracefully. */
    /* Synchronize the C engine's framebuffer resize logic, and invalidate layout caches immediately to prevent tearing. */
}

static void ios_view_safe_area_insets_did_change(id self, SEL _cmd) {
    /* Bridge viewSafeAreaInsetsDidChange directly into ui_safe_area_manager.h */
}

static void ios_content_size_category_did_change(id self, SEL _cmd, id notification) {
    /* Listen for UIContentSizeCategoryDidChangeNotification to capture OS-level font scale adjustments */
}

static long ios_accessibility_element_count(id self, SEL _cmd) {
    return 0;
}

static id ios_accessibility_element_at_index(id self, SEL _cmd, long index) {
    return NULL;
}

static long ios_index_of_accessibility_element(id self, SEL _cmd, id element) {
    return 0;
}

static void register_ios_classes(void) {
    Class clsUIView = objc_getClass("UIEngineView");
    Class clsUIViewController = objc_getClass("UIEngineViewController");

    if (clsUIView) {
        Class customView = objc_allocateClassPair(clsUIView, "UIEngineView", 0);
        if (customView) {
            class_addMethod(customView, sel_registerName("viewSafeAreaInsetsDidChange"), (IMP)ios_view_safe_area_insets_did_change, "v@:");
            class_addMethod(customView, sel_registerName("accessibilityElementCount"), (IMP)ios_accessibility_element_count, "q@:");
            class_addMethod(customView, sel_registerName("accessibilityElementAtIndex:"), (IMP)ios_accessibility_element_at_index, "@@:q");
            class_addMethod(customView, sel_registerName("indexOfAccessibilityElement:"), (IMP)ios_index_of_accessibility_element, "q@:@");
            objc_registerClassPair(customView);
        }
    }

    if (clsUIViewController) {
        Class customVC = objc_allocateClassPair(clsUIViewController, "UIEngineViewController", 0);
        if (customVC) {
            class_addMethod(customVC, sel_registerName("viewWillTransitionToSize:withTransitionCoordinator:"), (IMP)ios_view_will_transition_to_size, "v@:{CGSize=dd}@");
            objc_registerClassPair(customVC);
        }
    }
}

static ui_error_t display_link_callback(id self, SEL _cmd, id displayLink) {
    if (!self || !_cmd || !displayLink) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    /* In a real implementation, this would push an event to the queue or trigger rendering */
    return UI_ERROR_NONE;
}

/**
 * @brief ios_create_window.
 * @param backend Parameter backend.
 * @param title Parameter title.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_window Parameter out_window.
 * @return Return value.
 */
static ui_error_t ios_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    struct ui_window* win;
    Class clsUIScreen;
    Class clsUIWindow;
    Class clsUIViewController;
    Class clsUIView;
    Class clsCAEAGLLayer;
    Class clsCADisplayLink;
    Class clsNSRunLoop;
    id window_obj;
    id view_controller_obj;
    id view_obj;
    id layer_obj;
    id display_link_obj;
    id run_loop_obj;
    id run_loop_mode;

    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    win = (struct ui_window*)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window));
    if (!win) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    register_ios_classes();
    clsUIScreen = objc_getClass("UIScreen");
    clsUIWindow = objc_getClass("UIWindow");
    clsUIViewController = objc_getClass("UIEngineViewController");
    clsUIView = objc_getClass("UIEngineView");
    clsCAEAGLLayer = objc_getClass("CAEAGLLayer");
    clsCADisplayLink = objc_getClass("CADisplayLink");
    clsNSRunLoop = objc_getClass("NSRunLoop");

    if (!clsUIScreen || !clsUIWindow || !clsUIViewController || !clsUIView || !clsCAEAGLLayer || !clsCADisplayLink || !clsNSRunLoop) {
        C_MULTIPLATFORM_FREE(win);
        return UI_ERROR_UNKNOWN;
    }

    /* Instantiate window */
    window_obj = ((id (*)(id, SEL))objc_msgSend)((id)clsUIWindow, sel_registerName("alloc"));
    if (!window_obj) {
        C_MULTIPLATFORM_FREE(win);
        return UI_ERROR_OUT_OF_MEMORY;
    }
    window_obj = ((id (*)(id, SEL))objc_msgSend)(window_obj, sel_registerName("init"));

    /* Instantiate view controller */
    view_controller_obj = ((id (*)(id, SEL))objc_msgSend)((id)clsUIViewController, sel_registerName("alloc"));
    if (!view_controller_obj) {
        C_MULTIPLATFORM_FREE(win);
        return UI_ERROR_OUT_OF_MEMORY;
    }
    view_controller_obj = ((id (*)(id, SEL))objc_msgSend)(view_controller_obj, sel_registerName("init"));

    /* Instantiate root view */
    view_obj = ((id (*)(id, SEL))objc_msgSend)((id)clsUIView, sel_registerName("alloc"));
    if (!view_obj) {
        C_MULTIPLATFORM_FREE(win);
        return UI_ERROR_OUT_OF_MEMORY;
    }
    view_obj = ((id (*)(id, SEL))objc_msgSend)(view_obj, sel_registerName("init"));

    /* Back view with CAEAGLLayer for OpenGL ES 2 rendering */
    /* Note: In a pure C implementation without subclassing UIView to override +layerClass,
       we must manually instantiate and add the sublayer, or use object_setClass tricks.
       For this backend stub, we will instantiate the layer and add it to the view. */
    layer_obj = ((id (*)(id, SEL))objc_msgSend)((id)clsCAEAGLLayer, sel_registerName("layer"));
    if (layer_obj) {
        id view_layer = ((id (*)(id, SEL))objc_msgSend)(view_obj, sel_registerName("layer"));
        ((void (*)(id, SEL, id))objc_msgSend)(view_layer, sel_registerName("addSublayer:"), layer_obj);
    }

    /* Set up hierarchy */
    ((void (*)(id, SEL, id))objc_msgSend)(view_controller_obj, sel_registerName("setView:"), view_obj);
    ((void (*)(id, SEL, id))objc_msgSend)(window_obj, sel_registerName("setRootViewController:"), view_controller_obj);
    ((void (*)(id, SEL))objc_msgSend)(window_obj, sel_registerName("makeKeyAndVisible"));

    /* Setup CADisplayLink */
    display_link_obj = NULL;

    /* We need a target for the display link. In a real app we'd allocate a custom ObjC class at runtime.
       For this stub, we skip actual registration if we can't create the class easily in pure C,
       or we use a dummy target. We will try to get the current run loop. */
    run_loop_obj = ((id (*)(id, SEL))objc_msgSend)((id)clsNSRunLoop, sel_registerName("currentRunLoop"));
    if (run_loop_obj) {
        id ns_string_cls = objc_getClass("NSString");
        if (ns_string_cls) {
            run_loop_mode = ((id (*)(id, SEL, const char*))objc_msgSend)(ns_string_cls, sel_registerName("stringWithUTF8String:"), "NSRunLoopCommonModes");
            if (run_loop_mode) {
                /* Registration omitted for brevity and safety in pure C stub */
            }
        }
    }

    win->window = window_obj;
    win->context = view_obj;
    win->display_link = display_link_obj;
    win->width = width;
    win->height = height;
    win->is_closing = 0;
    win->scale_factor = 2.0f;
    win->on_dpi_change_callback = NULL;
    win->on_dpi_change_user_data = NULL;

    *out_window = win;
    return UI_ERROR_NONE;
}

/**
 * @brief ios_destroy_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    if (window->display_link) {
        ((void (*)(id, SEL))objc_msgSend)(window->display_link, sel_registerName("invalidate"));
    }

    C_MULTIPLATFORM_FREE(window);
    return UI_ERROR_NONE;
}

/**
 * @brief ios_show_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_hide_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_poll_events.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_event Parameter out_event.
 * @param out_has_event Parameter out_has_event.
 * @return Return value.
 */
static ui_error_t ios_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    id current_event;
    Class clsUIApplication;
    id shared_app;

    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;

    clsUIApplication = objc_getClass("UIApplication");
    if (clsUIApplication) {
        shared_app = ((id (*)(id, SEL))objc_msgSend)((id)clsUIApplication, sel_registerName("sharedApplication"));
        if (shared_app) {
            /* Drain event queue manually if needed, though usually UIApplicationMain handles this. */
        }
    }

    return UI_ERROR_NONE;
}

/**
 * @brief ios_swap_buffers.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t ios_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/**
 * @brief ios_get_scale_factor.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_scale_factor Parameter out_scale_factor.
 * @return Return value.
 */
static ui_error_t ios_get_scale_factor(struct ui_window_backend* backend, struct ui_window* window, float* out_scale_factor) {
    Class clsUIScreen;
    id mainScreen;
    double scale = 2.0;

    if (!backend || !window || !out_scale_factor) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    register_ios_classes();
    clsUIScreen = objc_getClass("UIScreen");
    if (clsUIScreen) {
        mainScreen = ((id (*)(id, SEL))objc_msgSend)((id)clsUIScreen, sel_registerName("mainScreen"));
        if (mainScreen) {
            scale = ((double (*)(id, SEL))objc_msgSend)(mainScreen, sel_registerName("scale"));
        }
    }

    *out_scale_factor = (float)scale;
    return UI_ERROR_NONE;
}

/**
 * @brief ios_get_framebuffer_size.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_fb_width Parameter out_fb_width.
 * @param out_fb_height Parameter out_fb_height.
 * @return Return value.
 */
static ui_error_t ios_get_framebuffer_size(struct ui_window_backend* backend, struct ui_window* window, int* out_fb_width, int* out_fb_height) {
    float scale = 2.0f;
    ui_error_t rc;

    if (!backend || !window || !out_fb_width || !out_fb_height) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    rc = ios_get_scale_factor(backend, window, &scale);
    if (rc != UI_ERROR_NONE) {
        return rc;
    }

    *out_fb_width = (int)((float)window->width * scale);
    *out_fb_height = (int)((float)window->height * scale);
    return UI_ERROR_NONE;
}

/**
 * @brief ios_set_on_dpi_change_callback.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param callback Parameter callback.
 * @param user_data Parameter user_data.
 * @return Return value.
 */
static ui_error_t ios_set_on_dpi_change_callback(
    struct ui_window_backend* backend, struct ui_window* window,
    ui_error_t (*callback)(void*, float, float), void* user_data) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    window->on_dpi_change_callback = callback;
    window->on_dpi_change_user_data = user_data;
    return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_ios_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    backend = (struct ui_window_backend*)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    backend->create_window = ios_create_window;
    backend->destroy_window = ios_destroy_window;
    backend->show_window = ios_show_window;
    backend->hide_window = ios_hide_window;
    backend->poll_events = ios_poll_events;
    backend->swap_buffers = ios_swap_buffers;
    backend->push_deep_link = NULL;
    backend->get_os_handle = NULL;
    backend->set_on_resize_callback = NULL;
    backend->get_scale_factor = ios_get_scale_factor;
    backend->get_framebuffer_size = ios_get_framebuffer_size;
    backend->set_on_dpi_change_callback = ios_set_on_dpi_change_callback;
    backend->user_data = NULL;

    *out_backend = backend;
    return UI_ERROR_NONE;
}

/**
 * @brief ui_window_backend_ios_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    C_MULTIPLATFORM_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Apple platform but not iOS (e.g. macOS) */
#include "../include/ui_window_backend_ios.h"
#include <stddef.h>

ui_error_t ui_window_backend_ios_create(struct ui_window_backend** out_backend) {
    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;
    return UI_ERROR_UNKNOWN;
}

/**
 * @brief ui_window_backend_ios_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_UNKNOWN;
}
#endif
#else
/* Non-Apple Platform Stub */
#include "../include/ui_window_backend_ios.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief ui_window_backend_ios_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t
ui_window_backend_ios_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/**
 * @brief ui_window_backend_ios_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_ios_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
