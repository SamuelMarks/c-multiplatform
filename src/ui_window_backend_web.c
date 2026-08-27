#if defined(__EMSCRIPTEN__)

/* clang-format off */
#include "../include/ui_window_backend_web.h"
#include "../include/ui_event.h"
#include "ui_internal_mem.h"

#include <emscripten/html5.h>

/**
 * @struct ui_window
 * \brief ui_window
 */
struct ui_window {
    int canvas_width; /**< canvas_width */
    int canvas_height; /**< canvas_height */
    int is_closing; /**< is_closing */
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE gl_context; /**< gl_context */
};

/**
 * @struct web_backend_data
 * \brief web_backend_data
 */
struct web_backend_data {
    void (*resize_cb)(void*, int, int); /**< int) */
    void* resize_user_data; /**< resize_user_data */
    struct ui_window* active_window; /**< active_window */
};

/*
 * @brief web_create_window.
 * @param backend Parameter backend.
 * @param title Parameter title.
 * @param width Parameter width.
 * @param height Parameter height.
 * @param out_window Parameter out_window.
 * @return Return value.
 */
static ui_error_t web_create_window(struct ui_window_backend* backend, const char* title, int width, int height, struct ui_window** out_window) {
    struct ui_window* win;
    struct web_backend_data* bdata;
    EmscriptenWebGLContextAttributes attr;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
    (void)title;

    if (!backend || !title || !out_window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    bdata = (struct web_backend_data*)backend->user_data;
    win = (struct ui_window*)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window));
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
            C_MULTIPLATFORM_FREE(win);
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

/*
 * @brief web_destroy_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t web_destroy_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    if (window->gl_context) {
        emscripten_webgl_destroy_context(window->gl_context);
    }
    C_MULTIPLATFORM_FREE(window);
    return UI_ERROR_NONE;
}

/*
 * @brief web_show_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t web_show_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/*
 * @brief web_hide_window.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t web_hide_window(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/*
 * @brief web_poll_events.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @param out_event Parameter out_event.
 * @param out_has_event Parameter out_has_event.
 * @return Return value.
 */
static ui_error_t web_poll_events(struct ui_window_backend* backend, struct ui_window* window, struct ui_event* out_event, int* out_has_event) {
    if (!backend || !window || !out_event || !out_has_event) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    *out_has_event = 0;
    out_event->type = UI_EVENT_NONE;
    return UI_ERROR_NONE;
}

/*
 * @brief web_swap_buffers.
 * @param backend Parameter backend.
 * @param window Parameter window.
 * @return Return value.
 */
static ui_error_t web_swap_buffers(struct ui_window_backend* backend, struct ui_window* window) {
    if (!backend || !window) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    return UI_ERROR_NONE;
}

/*
 * @brief web_resize_callback.
 * @param eventType Parameter eventType.
 * @param uiEvent Parameter uiEvent.
 * @param userData Parameter userData.
 * @return Return value.
 */
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

static ui_error_t web_set_on_resize_callback(struct ui_window_backend* backend, struct ui_window* window, ui_error_t (*cb)(void*, int, int), void* user_data) {
    struct web_backend_data* bdata;
    (void)window;
    if (!backend || !backend->user_data) return UI_ERROR_INVALID_ARGUMENT;
    bdata = (struct web_backend_data*)backend->user_data;
    bdata->resize_cb = cb;
    bdata->resize_user_data = user_data;
    return UI_ERROR_NONE;
}

/*
 * @brief ui_window_backend_web_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_web_create(struct ui_window_backend** out_backend) {
    struct ui_window_backend* backend;
    struct web_backend_data* bdata;

    if (!out_backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }

    bdata = (struct web_backend_data*)C_MULTIPLATFORM_MALLOC(sizeof(struct web_backend_data));
    if (!bdata) {
        return UI_ERROR_OUT_OF_MEMORY;
    }
    bdata->resize_cb = NULL;
    bdata->resize_user_data = NULL;
    bdata->active_window = NULL;

    backend = (struct ui_window_backend*)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_window_backend));
    if (!backend) {
        C_MULTIPLATFORM_FREE(bdata);
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

/*
 * @brief ui_window_backend_web_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_web_destroy(struct ui_window_backend* backend) {
    if (!backend) {
        return UI_ERROR_INVALID_ARGUMENT;
    }
    if (backend->user_data) {
        C_MULTIPLATFORM_FREE(backend->user_data);
    }
    C_MULTIPLATFORM_FREE(backend);
    return UI_ERROR_NONE;
}

#else
/* Non-Emscripten Platform Stub */
#include "../include/ui_window_backend_web.h"
#include <stddef.h>
/* clang-format on */

/*
 * @brief ui_window_backend_web_create.
 * @param out_backend Parameter out_backend.
 * @return Return value.
 */
ui_error_t
ui_window_backend_web_create(struct ui_window_backend **out_backend) {
  if (!out_backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;
  return UI_ERROR_UNKNOWN;
}

/*
 * @brief ui_window_backend_web_destroy.
 * @param backend Parameter backend.
 * @return Return value.
 */
ui_error_t ui_window_backend_web_destroy(struct ui_window_backend *backend) {
  if (!backend) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return UI_ERROR_UNKNOWN;
}
#endif
