#include "m3/m3_backend_null.h"

#include "m3/m3_log.h"
#include "m3/m3_object.h"

#include <string.h>

#define M3_NULL_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != M3_OK) { \
            return (rc); \
        } \
    } while (0)

#define M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, cleanup) \
    do { \
        if ((rc) != M3_OK) { \
            cleanup; \
            return (rc); \
        } \
    } while (0)

#define M3_NULL_DEFAULT_HANDLE_CAPACITY 64

#define M3_NULL_TYPE_WINDOW 1
#define M3_NULL_TYPE_TEXTURE 2
#define M3_NULL_TYPE_FONT 3

typedef struct M3NullWindow {
    M3ObjectHeader header;
    struct M3NullBackend *backend;
    m3_i32 width;
    m3_i32 height;
    M3Scalar dpi_scale;
    m3_u32 flags;
    M3Bool visible;
} M3NullWindow;

typedef struct M3NullTexture {
    M3ObjectHeader header;
    struct M3NullBackend *backend;
    m3_i32 width;
    m3_i32 height;
    m3_u32 format;
} M3NullTexture;

typedef struct M3NullFont {
    M3ObjectHeader header;
    struct M3NullBackend *backend;
    m3_i32 size_px;
    m3_i32 weight;
    M3Bool italic;
} M3NullFont;

struct M3NullBackend {
    M3Allocator allocator;
    M3HandleSystem handles;
    M3WS ws;
    M3Gfx gfx;
    M3Env env;
    M3IO io;
    M3Sensors sensors;
    M3Camera camera;
    M3Network network;
    M3Tasks tasks;
    M3Bool initialized;
    M3Bool log_enabled;
    M3Bool log_owner;
    M3Bool inline_tasks;
    m3_u32 time_ms;
    char *clipboard;
    m3_usize clipboard_capacity;
    m3_usize clipboard_length;
    m3_usize clipboard_limit;
};

static int m3_null_backend_log(struct M3NullBackend *backend, M3LogLevel level, const char *message)
{
    if (!backend->log_enabled) {
        return M3_OK;
    }
    return m3_log_write(level, "m3.null", message);
}

static int m3_null_backend_resolve(struct M3NullBackend *backend, M3Handle handle, m3_u32 type_id, void **out_obj)
{
    void *resolved;
    m3_u32 actual_type;
    int rc;

    if (out_obj != NULL) {
        *out_obj = NULL;
    }

    rc = backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_object_get_type_id((const M3ObjectHeader *)resolved, &actual_type);
    M3_NULL_RETURN_IF_ERROR(rc);
    if (actual_type != type_id) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (out_obj != NULL) {
        *out_obj = resolved;
    }
    return M3_OK;
}

static int m3_null_object_retain(void *obj)
{
    return m3_object_retain((M3ObjectHeader *)obj);
}

static int m3_null_object_release(void *obj)
{
    return m3_object_release((M3ObjectHeader *)obj);
}

static int m3_null_object_get_type_id(void *obj, m3_u32 *out_type_id)
{
    return m3_object_get_type_id((const M3ObjectHeader *)obj, out_type_id);
}

static int m3_null_window_destroy(void *obj)
{
    M3NullWindow *window;
    struct M3NullBackend *backend;
    int rc;

    window = (M3NullWindow *)obj;
    backend = window->backend;

    rc = backend->handles.vtable->unregister_object(backend->handles.ctx, window->header.handle);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = backend->allocator.free(backend->allocator.ctx, window);
    M3_NULL_RETURN_IF_ERROR(rc);

    return M3_OK;
}

static int m3_null_texture_destroy(void *obj)
{
    M3NullTexture *texture;
    struct M3NullBackend *backend;
    int rc;

    texture = (M3NullTexture *)obj;
    backend = texture->backend;

    rc = backend->handles.vtable->unregister_object(backend->handles.ctx, texture->header.handle);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = backend->allocator.free(backend->allocator.ctx, texture);
    M3_NULL_RETURN_IF_ERROR(rc);

    return M3_OK;
}

static int m3_null_font_destroy(void *obj)
{
    M3NullFont *font;
    struct M3NullBackend *backend;
    int rc;

    font = (M3NullFont *)obj;
    backend = font->backend;

    rc = backend->handles.vtable->unregister_object(backend->handles.ctx, font->header.handle);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = backend->allocator.free(backend->allocator.ctx, font);
    M3_NULL_RETURN_IF_ERROR(rc);

    return M3_OK;
}

static const M3ObjectVTable g_m3_null_window_vtable = {
    m3_null_object_retain,
    m3_null_object_release,
    m3_null_window_destroy,
    m3_null_object_get_type_id
};

static const M3ObjectVTable g_m3_null_texture_vtable = {
    m3_null_object_retain,
    m3_null_object_release,
    m3_null_texture_destroy,
    m3_null_object_get_type_id
};

static const M3ObjectVTable g_m3_null_font_vtable = {
    m3_null_object_retain,
    m3_null_object_release,
    m3_null_font_destroy,
    m3_null_object_get_type_id
};

static int m3_null_ws_init(void *ws, const M3WSConfig *config)
{
    struct M3NullBackend *backend;
    int rc;

    if (ws == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.init");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_ws_shutdown(void *ws)
{
    struct M3NullBackend *backend;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.shutdown");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_ws_create_window(void *ws, const M3WSWindowConfig *config, M3Handle *out_window)
{
    struct M3NullBackend *backend;
    M3NullWindow *window;
    int rc;

    if (ws == NULL || config == NULL || out_window == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_title == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->width <= 0 || config->height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)ws;
    out_window->id = 0u;
    out_window->generation = 0u;

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.create_window");
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3NullWindow), (void **)&window);
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(window, 0, sizeof(*window));
    window->backend = backend;
    window->width = config->width;
    window->height = config->height;
    window->dpi_scale = 1.0f;
    window->flags = config->flags;
    window->visible = M3_FALSE;

    rc = m3_object_header_init(&window->header, M3_NULL_TYPE_WINDOW, 0, &g_m3_null_window_vtable);
    M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, window));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &window->header);
    M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, window));

    *out_window = window->header.handle;
    return M3_OK;
}

static int m3_null_ws_destroy_window(void *ws, M3Handle window)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.destroy_window");
    M3_NULL_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_null_ws_show_window(void *ws, M3Handle window)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.show_window");
    M3_NULL_RETURN_IF_ERROR(rc);

    resolved->visible = M3_TRUE;
    return M3_OK;
}

static int m3_null_ws_hide_window(void *ws, M3Handle window)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.hide_window");
    M3_NULL_RETURN_IF_ERROR(rc);

    resolved->visible = M3_FALSE;
    return M3_OK;
}

static int m3_null_ws_set_window_title(void *ws, M3Handle window, const char *utf8_title)
{
    struct M3NullBackend *backend;
    int rc;

    if (ws == NULL || utf8_title == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, NULL);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_title");
    M3_NULL_RETURN_IF_ERROR(rc);

    return M3_OK;
}

static int m3_null_ws_set_window_size(void *ws, M3Handle window, m3_i32 width, m3_i32 height)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_size");
    M3_NULL_RETURN_IF_ERROR(rc);

    resolved->width = width;
    resolved->height = height;
    return M3_OK;
}

static int m3_null_ws_get_window_size(void *ws, M3Handle window, m3_i32 *out_width, m3_i32 *out_height)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL || out_width == NULL || out_height == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_size");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_width = resolved->width;
    *out_height = resolved->height;
    return M3_OK;
}

static int m3_null_ws_set_window_dpi_scale(void *ws, M3Handle window, M3Scalar scale)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (scale <= 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_window_dpi_scale");
    M3_NULL_RETURN_IF_ERROR(rc);

    resolved->dpi_scale = scale;
    return M3_OK;
}

static int m3_null_ws_get_window_dpi_scale(void *ws, M3Handle window, M3Scalar *out_scale)
{
    struct M3NullBackend *backend;
    M3NullWindow *resolved;
    int rc;

    if (ws == NULL || out_scale == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_window_dpi_scale");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_scale = resolved->dpi_scale;
    return M3_OK;
}

static int m3_null_ws_set_clipboard_text(void *ws, const char *utf8_text)
{
    struct M3NullBackend *backend;
    m3_usize length;
    m3_usize required;
    int rc;

    if (ws == NULL || utf8_text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.set_clipboard_text");
    M3_NULL_RETURN_IF_ERROR(rc);

    length = 0;
    while (utf8_text[length] != '\0') {
        length += 1;
    }
    if (length > backend->clipboard_limit) {
        return M3_ERR_RANGE;
    }

    required = length + 1;
    if (required > backend->clipboard_capacity) {
        void *mem = NULL;
        if (backend->clipboard == NULL) {
            rc = backend->allocator.alloc(backend->allocator.ctx, required, &mem);
        } else {
            rc = backend->allocator.realloc(backend->allocator.ctx, backend->clipboard, required, &mem);
        }
        M3_NULL_RETURN_IF_ERROR(rc);
        backend->clipboard = (char *)mem;
        backend->clipboard_capacity = required;
    }

    memcpy(backend->clipboard, utf8_text, required);
    backend->clipboard_length = length;
    return M3_OK;
}

static int m3_null_ws_get_clipboard_text(void *ws, char *buffer, m3_usize buffer_size, m3_usize *out_length)
{
    struct M3NullBackend *backend;
    m3_usize required;
    int rc;

    if (ws == NULL || buffer == NULL || out_length == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "ws.get_clipboard_text");
    M3_NULL_RETURN_IF_ERROR(rc);

    required = backend->clipboard_length + 1;
    if (buffer_size < required) {
        *out_length = backend->clipboard_length;
        return M3_ERR_RANGE;
    }

    if (backend->clipboard_length == 0) {
        buffer[0] = '\0';
    } else {
        memcpy(buffer, backend->clipboard, required);
    }
    *out_length = backend->clipboard_length;
    return M3_OK;
}

static int m3_null_ws_poll_event(void *ws, M3InputEvent *out_event, M3Bool *out_has_event)
{
    struct M3NullBackend *backend;
    int rc;

    if (ws == NULL || out_event == NULL || out_has_event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.poll_event");
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(out_event, 0, sizeof(*out_event));
    *out_has_event = M3_FALSE;
    return M3_OK;
}

static int m3_null_ws_pump_events(void *ws)
{
    struct M3NullBackend *backend;
    int rc;

    if (ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.pump_events");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_ws_get_time_ms(void *ws, m3_u32 *out_time_ms)
{
    struct M3NullBackend *backend;
    int rc;

    if (ws == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)ws;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "ws.get_time_ms");
    M3_NULL_RETURN_IF_ERROR(rc);

    backend->time_ms += 16u;
    *out_time_ms = backend->time_ms;
    return M3_OK;
}

static const M3WSVTable g_m3_null_ws_vtable = {
    m3_null_ws_init,
    m3_null_ws_shutdown,
    m3_null_ws_create_window,
    m3_null_ws_destroy_window,
    m3_null_ws_show_window,
    m3_null_ws_hide_window,
    m3_null_ws_set_window_title,
    m3_null_ws_set_window_size,
    m3_null_ws_get_window_size,
    m3_null_ws_set_window_dpi_scale,
    m3_null_ws_get_window_dpi_scale,
    m3_null_ws_set_clipboard_text,
    m3_null_ws_get_clipboard_text,
    m3_null_ws_poll_event,
    m3_null_ws_pump_events,
    m3_null_ws_get_time_ms
};

static int m3_null_gfx_begin_frame(void *gfx, M3Handle window, m3_i32 width, m3_i32 height, M3Scalar dpi_scale)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (dpi_scale <= 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, NULL);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.begin_frame");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_end_frame(void *gfx, M3Handle window)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_resolve(backend, window, M3_NULL_TYPE_WINDOW, NULL);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.end_frame");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_clear(void *gfx, M3Color color)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(color);

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.clear");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_draw_rect(void *gfx, const M3Rect *rect, M3Color color, M3Scalar corner_radius)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(color);

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }
    if (corner_radius < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_rect");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_draw_line(void *gfx, M3Scalar x0, M3Scalar y0, M3Scalar x1, M3Scalar y1, M3Color color, M3Scalar thickness)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(x0);
    M3_UNUSED(y0);
    M3_UNUSED(x1);
    M3_UNUSED(y1);
    M3_UNUSED(color);

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (thickness < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_line");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_push_clip(void *gfx, const M3Rect *rect)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL || rect == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.push_clip");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_pop_clip(void *gfx)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.pop_clip");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_set_transform(void *gfx, const M3Mat3 *transform)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL || transform == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.set_transform");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_create_texture(void *gfx, m3_i32 width, m3_i32 height, m3_u32 format, const void *pixels, m3_usize size,
                                      M3Handle *out_texture)
{
    struct M3NullBackend *backend;
    M3NullTexture *texture;
    int rc;

    if (gfx == NULL || out_texture == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (format != M3_TEX_FORMAT_RGBA8 && format != M3_TEX_FORMAT_BGRA8 && format != M3_TEX_FORMAT_A8) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (pixels == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    out_texture->id = 0u;
    out_texture->generation = 0u;

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.create_texture");
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3NullTexture), (void **)&texture);
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(texture, 0, sizeof(*texture));
    texture->backend = backend;
    texture->width = width;
    texture->height = height;
    texture->format = format;

    rc = m3_object_header_init(&texture->header, M3_NULL_TYPE_TEXTURE, 0, &g_m3_null_texture_vtable);
    M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, texture));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &texture->header);
    M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, texture));

    *out_texture = texture->header.handle;
    return M3_OK;
}

static int m3_null_gfx_update_texture(void *gfx, M3Handle texture, m3_i32 x, m3_i32 y, m3_i32 width, m3_i32 height, const void *pixels,
                                      m3_usize size)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (x < 0 || y < 0 || width <= 0 || height <= 0) {
        return M3_ERR_RANGE;
    }
    if (pixels == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_resolve(backend, texture, M3_NULL_TYPE_TEXTURE, NULL);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.update_texture");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static int m3_null_gfx_destroy_texture(void *gfx, M3Handle texture)
{
    struct M3NullBackend *backend;
    M3NullTexture *resolved;
    int rc;

    if (gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_resolve(backend, texture, M3_NULL_TYPE_TEXTURE, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
    M3_NULL_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_null_gfx_draw_texture(void *gfx, M3Handle texture, const M3Rect *src, const M3Rect *dst, M3Scalar opacity)
{
    struct M3NullBackend *backend;
    int rc;

    if (gfx == NULL || src == NULL || dst == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (opacity < 0.0f || opacity > 1.0f) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)gfx;
    rc = m3_null_backend_resolve(backend, texture, M3_NULL_TYPE_TEXTURE, NULL);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "gfx.draw_texture");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_OK;
}

static const M3GfxVTable g_m3_null_gfx_vtable = {
    m3_null_gfx_begin_frame,
    m3_null_gfx_end_frame,
    m3_null_gfx_clear,
    m3_null_gfx_draw_rect,
    m3_null_gfx_draw_line,
    m3_null_gfx_push_clip,
    m3_null_gfx_pop_clip,
    m3_null_gfx_set_transform,
    m3_null_gfx_create_texture,
    m3_null_gfx_update_texture,
    m3_null_gfx_destroy_texture,
    m3_null_gfx_draw_texture
};

static int m3_null_text_create_font(void *text, const char *utf8_family, m3_i32 size_px, m3_i32 weight, M3Bool italic, M3Handle *out_font)
{
    struct M3NullBackend *backend;
    M3NullFont *font;
    int rc;

    if (text == NULL || out_font == NULL || utf8_family == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (size_px <= 0) {
        return M3_ERR_RANGE;
    }
    if (weight < 100 || weight > 900) {
        return M3_ERR_RANGE;
    }

    backend = (struct M3NullBackend *)text;
    out_font->id = 0u;
    out_font->generation = 0u;

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.create_font");
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(M3NullFont), (void **)&font);
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(font, 0, sizeof(*font));
    font->backend = backend;
    font->size_px = size_px;
    font->weight = weight;
    font->italic = italic ? M3_TRUE : M3_FALSE;

    rc = m3_object_header_init(&font->header, M3_NULL_TYPE_FONT, 0, &g_m3_null_font_vtable);
    M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, font));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &font->header);
    M3_NULL_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, font));

    *out_font = font->header.handle;
    return M3_OK;
}

static int m3_null_text_destroy_font(void *text, M3Handle font)
{
    struct M3NullBackend *backend;
    M3NullFont *resolved;
    int rc;

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)text;
    rc = m3_null_backend_resolve(backend, font, M3_NULL_TYPE_FONT, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.destroy_font");
    M3_NULL_RETURN_IF_ERROR(rc);

    return m3_object_release(&resolved->header);
}

static int m3_null_text_measure_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar *out_width, M3Scalar *out_height,
                                     M3Scalar *out_baseline)
{
    struct M3NullBackend *backend;
    M3NullFont *resolved;
    M3Scalar size;
    int rc;

    if (text == NULL || out_width == NULL || out_height == NULL || out_baseline == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)text;
    rc = m3_null_backend_resolve(backend, font, M3_NULL_TYPE_FONT, (void **)&resolved);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.measure_text");
    M3_NULL_RETURN_IF_ERROR(rc);

    size = (M3Scalar)resolved->size_px;
    *out_width = size * (M3Scalar)utf8_len * 0.5f;
    *out_height = size;
    *out_baseline = size * 0.8f;
    return M3_OK;
}

static int m3_null_text_draw_text(void *text, M3Handle font, const char *utf8, m3_usize utf8_len, M3Scalar x, M3Scalar y, M3Color color)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(x);
    M3_UNUSED(y);
    M3_UNUSED(color);

    if (text == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)text;
    rc = m3_null_backend_resolve(backend, font, M3_NULL_TYPE_FONT, NULL);
    M3_NULL_RETURN_IF_ERROR(rc);

    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "text.draw_text");
    M3_NULL_RETURN_IF_ERROR(rc);

    return M3_OK;
}

static const M3TextVTable g_m3_null_text_vtable = {
    m3_null_text_create_font,
    m3_null_text_destroy_font,
    m3_null_text_measure_text,
    m3_null_text_draw_text
};

static int m3_null_io_read_file(void *io, const char *utf8_path, void *buffer, m3_usize buffer_size, m3_usize *out_read)
{
    struct M3NullBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_read == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (buffer == NULL && buffer_size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)io;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_read = 0;
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_io_read_file_alloc(void *io, const char *utf8_path, const M3Allocator *allocator, void **out_data, m3_usize *out_size)
{
    struct M3NullBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || allocator == NULL || out_data == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)io;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.read_file_alloc");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_data = NULL;
    *out_size = 0;
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_io_write_file(void *io, const char *utf8_path, const void *data, m3_usize size, M3Bool overwrite)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(overwrite);

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (data == NULL && size != 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)io;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.write_file");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_io_file_exists(void *io, const char *utf8_path, M3Bool *out_exists)
{
    struct M3NullBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_exists == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)io;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.file_exists");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_exists = M3_FALSE;
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_io_delete_file(void *io, const char *utf8_path)
{
    struct M3NullBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)io;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.delete_file");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_io_stat_file(void *io, const char *utf8_path, M3FileInfo *out_info)
{
    struct M3NullBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_info == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)io;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "io.stat_file");
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(out_info, 0, sizeof(*out_info));
    return M3_ERR_UNSUPPORTED;
}

static const M3IOVTable g_m3_null_io_vtable = {
    m3_null_io_read_file,
    m3_null_io_read_file_alloc,
    m3_null_io_write_file,
    m3_null_io_file_exists,
    m3_null_io_delete_file,
    m3_null_io_stat_file
};

static int m3_null_sensors_is_available(void *sensors, m3_u32 type, M3Bool *out_available)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL || out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)sensors;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.is_available");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_available = M3_FALSE;
    return M3_OK;
}

static int m3_null_sensors_start(void *sensors, m3_u32 type)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)sensors;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.start");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_sensors_stop(void *sensors, m3_u32 type)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)sensors;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.stop");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_sensors_read(void *sensors, m3_u32 type, M3SensorReading *out_reading, M3Bool *out_has_reading)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(type);

    if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)sensors;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "sensors.read");
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(out_reading, 0, sizeof(*out_reading));
    *out_has_reading = M3_FALSE;
    return M3_OK;
}

static const M3SensorsVTable g_m3_null_sensors_vtable = {
    m3_null_sensors_is_available,
    m3_null_sensors_start,
    m3_null_sensors_stop,
    m3_null_sensors_read
};

static int m3_null_camera_open(void *camera, m3_u32 camera_id)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(camera_id);

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)camera;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.open");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_camera_open_with_config(void *camera, const M3CameraConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return m3_null_camera_open(camera, config->camera_id);
}

static int m3_null_camera_close(void *camera)
{
    struct M3NullBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)camera;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.close");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_camera_start(void *camera)
{
    struct M3NullBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)camera;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.start");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_camera_stop(void *camera)
{
    struct M3NullBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)camera;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.stop");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_camera_read_frame(void *camera, M3CameraFrame *out_frame, M3Bool *out_has_frame)
{
    struct M3NullBackend *backend;
    int rc;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)camera;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "camera.read_frame");
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = M3_FALSE;
    return M3_OK;
}

static const M3CameraVTable g_m3_null_camera_vtable = {
    m3_null_camera_open,
    m3_null_camera_open_with_config,
    m3_null_camera_close,
    m3_null_camera_start,
    m3_null_camera_stop,
    m3_null_camera_read_frame
};

static int m3_null_network_request(void *net, const M3NetworkRequest *request, const M3Allocator *allocator, M3NetworkResponse *out_response)
{
    struct M3NullBackend *backend;
    int rc;

    if (net == NULL || request == NULL || allocator == NULL || out_response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)net;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.request");
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(out_response, 0, sizeof(*out_response));
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_network_free_response(void *net, const M3Allocator *allocator, M3NetworkResponse *response)
{
    struct M3NullBackend *backend;
    int rc;

    if (net == NULL || allocator == NULL || response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)net;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "network.free_response");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static const M3NetworkVTable g_m3_null_network_vtable = {
    m3_null_network_request,
    m3_null_network_free_response
};

static int m3_null_tasks_thread_create(void *tasks, M3ThreadFn entry, void *user, M3Handle *out_thread)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(entry);
    M3_UNUSED(user);

    if (tasks == NULL || out_thread == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_create");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_tasks_thread_join(void *tasks, M3Handle thread)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(thread);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.thread_join");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_tasks_mutex_create(void *tasks, M3Handle *out_mutex)
{
    struct M3NullBackend *backend;
    int rc;

    if (tasks == NULL || out_mutex == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_create");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_tasks_mutex_destroy(void *tasks, M3Handle mutex)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_tasks_mutex_lock(void *tasks, M3Handle mutex)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_tasks_mutex_unlock(void *tasks, M3Handle mutex)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(mutex);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
    M3_NULL_RETURN_IF_ERROR(rc);
    return M3_ERR_UNSUPPORTED;
}

static int m3_null_tasks_sleep_ms(void *tasks, m3_u32 ms)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(ms);

    if (tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
    M3_NULL_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return M3_OK;
}

static int m3_null_tasks_post(void *tasks, M3TaskFn fn, void *user)
{
    struct M3NullBackend *backend;
    int rc;

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post");
    M3_NULL_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static int m3_null_tasks_post_delayed(void *tasks, M3TaskFn fn, void *user, m3_u32 delay_ms)
{
    struct M3NullBackend *backend;
    int rc;

    M3_UNUSED(delay_ms);

    if (tasks == NULL || fn == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)tasks;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_DEBUG, "tasks.post_delayed");
    M3_NULL_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return M3_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static const M3TasksVTable g_m3_null_tasks_vtable = {
    m3_null_tasks_thread_create,
    m3_null_tasks_thread_join,
    m3_null_tasks_mutex_create,
    m3_null_tasks_mutex_destroy,
    m3_null_tasks_mutex_lock,
    m3_null_tasks_mutex_unlock,
    m3_null_tasks_sleep_ms,
    m3_null_tasks_post,
    m3_null_tasks_post_delayed
};

static int m3_null_env_get_io(void *env, M3IO *out_io)
{
    struct M3NullBackend *backend;
    int rc;

    if (env == NULL || out_io == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)env;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_io");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_io = backend->io;
    return M3_OK;
}

static int m3_null_env_get_sensors(void *env, M3Sensors *out_sensors)
{
    struct M3NullBackend *backend;
    int rc;

    if (env == NULL || out_sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)env;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_sensors");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_sensors = backend->sensors;
    return M3_OK;
}

static int m3_null_env_get_camera(void *env, M3Camera *out_camera)
{
    struct M3NullBackend *backend;
    int rc;

    if (env == NULL || out_camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)env;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_camera");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_camera = backend->camera;
    return M3_OK;
}

static int m3_null_env_get_network(void *env, M3Network *out_network)
{
    struct M3NullBackend *backend;
    int rc;

    if (env == NULL || out_network == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)env;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_network");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_network = backend->network;
    return M3_OK;
}

static int m3_null_env_get_tasks(void *env, M3Tasks *out_tasks)
{
    struct M3NullBackend *backend;
    int rc;

    if (env == NULL || out_tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)env;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_tasks");
    M3_NULL_RETURN_IF_ERROR(rc);

    *out_tasks = backend->tasks;
    return M3_OK;
}

static int m3_null_env_get_time_ms(void *env, m3_u32 *out_time_ms)
{
    struct M3NullBackend *backend;
    int rc;

    if (env == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (struct M3NullBackend *)env;
    rc = m3_null_backend_log(backend, M3_LOG_LEVEL_INFO, "env.get_time_ms");
    M3_NULL_RETURN_IF_ERROR(rc);

    backend->time_ms += 16u;
    *out_time_ms = backend->time_ms;
    return M3_OK;
}

static const M3EnvVTable g_m3_null_env_vtable = {
    m3_null_env_get_io,
    m3_null_env_get_sensors,
    m3_null_env_get_camera,
    m3_null_env_get_network,
    m3_null_env_get_tasks,
    m3_null_env_get_time_ms
};

int M3_CALL m3_null_backend_config_init(M3NullBackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->handle_capacity = M3_NULL_DEFAULT_HANDLE_CAPACITY;
    config->clipboard_limit = (m3_usize)~(m3_usize)0;
    config->enable_logging = M3_TRUE;
    config->inline_tasks = M3_TRUE;
    return M3_OK;
}

int M3_CALL m3_null_backend_create(const M3NullBackendConfig *config, M3NullBackend **out_backend)
{
    M3NullBackendConfig local_config;
    M3Allocator allocator;
    struct M3NullBackend *backend;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_null_backend_config_init(&local_config);
        M3_NULL_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    if (config->handle_capacity == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    if (config->allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        M3_NULL_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(M3NullBackend), (void **)&backend);
    M3_NULL_RETURN_IF_ERROR(rc);

    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;
    backend->log_enabled = config->enable_logging ? M3_TRUE : M3_FALSE;
    backend->inline_tasks = config->inline_tasks ? M3_TRUE : M3_FALSE;
    backend->clipboard_limit = config->clipboard_limit;

    if (backend->log_enabled) {
        rc = m3_log_init(&allocator);
        if (rc != M3_OK && rc != M3_ERR_STATE) {
            allocator.free(allocator.ctx, backend);
            return rc;
        }
        backend->log_owner = (rc == M3_OK) ? M3_TRUE : M3_FALSE;
    }

    rc = m3_handle_system_default_create(config->handle_capacity, &allocator, &backend->handles);
    if (rc != M3_OK) {
        if (backend->log_owner) {
            m3_log_shutdown();
        }
        allocator.free(allocator.ctx, backend);
        return rc;
    }

    backend->ws.ctx = backend;
    backend->ws.vtable = &g_m3_null_ws_vtable;
    backend->gfx.ctx = backend;
    backend->gfx.vtable = &g_m3_null_gfx_vtable;
    backend->gfx.text_vtable = &g_m3_null_text_vtable;
    backend->env.ctx = backend;
    backend->env.vtable = &g_m3_null_env_vtable;
    backend->io.ctx = backend;
    backend->io.vtable = &g_m3_null_io_vtable;
    backend->sensors.ctx = backend;
    backend->sensors.vtable = &g_m3_null_sensors_vtable;
    backend->camera.ctx = backend;
    backend->camera.vtable = &g_m3_null_camera_vtable;
    backend->network.ctx = backend;
    backend->network.vtable = &g_m3_null_network_vtable;
    backend->tasks.ctx = backend;
    backend->tasks.vtable = &g_m3_null_tasks_vtable;
    backend->initialized = M3_TRUE;

    *out_backend = backend;
    return M3_OK;
}

int M3_CALL m3_null_backend_destroy(M3NullBackend *backend)
{
    int rc;
    int first_error;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }

    rc = m3_handle_system_default_destroy(&backend->handles);
    M3_NULL_RETURN_IF_ERROR(rc);

    first_error = M3_OK;

    if (backend->clipboard != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, backend->clipboard);
        if (rc != M3_OK && first_error == M3_OK) {
            first_error = rc;
        }
        backend->clipboard = NULL;
        backend->clipboard_capacity = 0;
        backend->clipboard_length = 0;
    }

    if (backend->log_owner) {
        rc = m3_log_shutdown();
        if (rc != M3_OK && first_error == M3_OK) {
            first_error = rc;
        }
    }

    backend->initialized = M3_FALSE;
    backend->ws.ctx = NULL;
    backend->ws.vtable = NULL;
    backend->gfx.ctx = NULL;
    backend->gfx.vtable = NULL;
    backend->gfx.text_vtable = NULL;
    backend->env.ctx = NULL;
    backend->env.vtable = NULL;
    backend->io.ctx = NULL;
    backend->io.vtable = NULL;
    backend->sensors.ctx = NULL;
    backend->sensors.vtable = NULL;
    backend->camera.ctx = NULL;
    backend->camera.vtable = NULL;
    backend->network.ctx = NULL;
    backend->network.vtable = NULL;
    backend->tasks.ctx = NULL;
    backend->tasks.vtable = NULL;

    rc = backend->allocator.free(backend->allocator.ctx, backend);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    return first_error;
}

int M3_CALL m3_null_backend_get_ws(M3NullBackend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_ws = backend->ws;
    return M3_OK;
}

int M3_CALL m3_null_backend_get_gfx(M3NullBackend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_gfx = backend->gfx;
    return M3_OK;
}

int M3_CALL m3_null_backend_get_env(M3NullBackend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_env = backend->env;
    return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_null_backend_test_set_initialized(M3NullBackend *backend, M3Bool initialized)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend->initialized = initialized ? M3_TRUE : M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_null_backend_test_object_retain(M3ObjectHeader *obj)
{
    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    return m3_null_object_retain(obj);
}

int M3_CALL m3_null_backend_test_object_release(M3ObjectHeader *obj)
{
    if (obj == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    return m3_null_object_release(obj);
}

int M3_CALL m3_null_backend_test_object_get_type_id(M3ObjectHeader *obj, m3_u32 *out_type_id)
{
    if (obj == NULL || out_type_id == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    return m3_null_object_get_type_id(obj, out_type_id);
}
#endif
