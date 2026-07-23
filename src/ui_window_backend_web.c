#if defined(__EMSCRIPTEN__)

/* clang-format off */
#include "../include/ui_window_backend_web.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <emscripten/html5.h>

/** \brief ui_window */
struct ui_window {
    int canvas_width;
    int canvas_height;
    int is_closing;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context;
};

/** \brief web_backend_data */
struct web_backend_data {
    void (*resize_cb)(void*, int, int);
    void* resize_user_data;
    struct ui_window* active_window;
};

static enum ui_error web_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    struct ui_window* win;
    struct web_backend_data* bdata;
    EmscriptenWebGLContextAttributes attr;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    (void)title;

    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    bdata = (struct web_backend_data*)backend->user_data;
    win = (struct ui_window*)UI_MALLOC(sizeof(struct ui_window));
    if (!win) {
        return UI_ERROR_OUT_OF_MEMORY;
    }

    win->canvas_width = width;
    win->canvas_height = height;
    win->is_closing = 0;

    emscripten_set_canvas_element_size("#canvas", width, height);

    emscripten_webgl_init_context_attributes(&attr);
    attr.alpha = 1;
    attr.depth = 1;
    attr.stencil = 1;
    attr.antialias = 1;
    attr.majorVersion = 2; /* Try WebGL 2 */
    attr.minorVersion = 0;
#if defined(__EMSCRIPTEN_PTHREADS__)
    attr.proxyContextToMainThread = EMSCRIPTEN_WEBGL_CONTEXT_PROXY_ALWAYS;
#endif

    ctx = emscripten_webgl_create_context("#canvas", &attr);
    if (!ctx) {
        /* Fallback to WebGL 1 */
        attr.majorVersion = 1;
        ctx = emscripten_webgl_create_context("#canvas", &attr);
        if (!ctx) {
            UI_FREE(win);
            return UI_ERROR_UNKNOWN;
        }
    }

    emscripten_webgl_make_context_current(ctx);
    win->gl_context = ctx;

    if (bdata) {
        bdata->active_window = win;
    }

    *out_window = win;
    return UI_ERROR_NONE;
}

static enum ui_error web_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    if (window->gl_context) {
        emscripten_webgl_destroy_context(window->gl_context);
    }
    UI_FREE(window);
    return UI_ERROR_NONE;
}

static enum ui_error web_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

static enum ui_error web_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

static enum ui_error web_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
    return UI_ERROR_NONE;
}

static enum ui_error web_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

static EM_BOOL web_resize_callback(int eventType, const EmscriptenUiEvent *uiEvent, void *userData) {
    struct ui_window_backend* backend = (struct ui_window_backend*)userData;
    struct web_backend_data* bdata;
    int w, h;
    (void)eventType;

    if (!backend || !backend->user_data) return EM_TRUE;

    bdata = (struct web_backend_data*)backend->user_data;
    if (bdata->resize_cb && bdata->active_window) {
        w = uiEvent->windowInnerWidth;
        h = uiEvent->windowInnerHeight;
        emscripten_set_canvas_element_size("#canvas", w, h);
        bdata->active_window->canvas_width = w;
        bdata->active_window->canvas_height = h;
        bdata->resize_cb(bdata->resize_user_data, w, h);
    }
    return EM_TRUE;
}

static enum ui_error web_set_on_resize_callback(struct ui_window_backend* backend, struct ui_window* window, enum ui_error (*cb)(void*, int, int), void* user_data) {
    struct web_backend_data* bdata;
    (void)window;
    if (!backend || !backend->user_data) return UI_ERROR_INVALID_ARGUMENT;
    bdata = (struct web_backend_data*)backend->user_data;
    bdata->resize_cb = cb;
    bdata->resize_user_data = user_data;
    return UI_ERROR_NONE;
}

enum ui_error ui_window_backend_web_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;
    struct web_backend_data* bdata;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    bdata = (struct web_backend_data*)UI_MALLOC(sizeof(struct web_backend_data));
    if (!bdata) {
        return UI_ERROR_OUT_OF_MEMORY;
    }
    bdata->resize_cb = NULL;
    bdata->resize_user_data = NULL;
    bdata->active_window = NULL;

    backend = (struct ui_window_backend*)UI_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        UI_FREE(bdata);
        return UI_ERROR_OUT_OF_MEMORY;
    }

    backend->create_window = web_create_window;
    backend->destroy_window = web_destroy_window;
    backend->show_window = web_show_window;
    backend->hide_window = web_hide_window;
    backend->poll_events = web_poll_events;
    backend->swap_buffers = web_swap_buffers;
    backend->push_deep_link = NULL;
    backend->get_os_handle = NULL;
    backend->set_on_resize_callback = web_set_on_resize_callback;
    backend->user_data = bdata;

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, backend, 1, web_resize_callback);

    *out_backend = backend;
    return UI_ERROR_NONE;
}

enum ui_error ui_window_backend_web_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    if (backend->user_data) {
        UI_FREE(backend->user_data);
    }
    UI_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Non-Emscripten Platform Stub */
#include "../include/ui_window_backend_web.h"
#include <stddef.h>
/* clang-format on */

enum ui_error
ui_window_backend_web_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

enum ui_error ui_window_backend_web_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
