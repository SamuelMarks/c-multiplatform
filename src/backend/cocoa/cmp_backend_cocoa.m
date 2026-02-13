#include "cmpc/cmp_backend_cocoa.h"

#include "cmpc/cmp_log.h"
#include "cmpc/cmp_object.h"

#include <limits.h>
#include <string.h>
#include <errno.h>

#if defined(CMP_COCOA_AVAILABLE)
#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import <CoreText/CoreText.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CFNetwork/CFNetwork.h>
#endif

#define CMP_COCOA_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != CMP_OK) { \
            return (rc); \
        } \
    } while (0)

#define CMP_COCOA_RETURN_IF_ERROR_CLEANUP(rc, cleanup) \
    do { \
        if ((rc) != CMP_OK) { \
            cleanup; \
            return (rc); \
        } \
    } while (0)

#define CMP_COCOA_DEFAULT_HANDLE_CAPACITY 64
#define CMP_COCOA_EVENT_CAPACITY 256u
#define CMP_COCOA_CLIP_STACK_CAPACITY 32u
#define CMP_COCOA_CAMERA_DEFAULT_WIDTH 640u
#define CMP_COCOA_CAMERA_DEFAULT_HEIGHT 480u

#define CMP_COCOA_TYPE_WINDOW 1
#define CMP_COCOA_TYPE_TEXTURE 2
#define CMP_COCOA_TYPE_FONT 3

#if defined(CMP_COCOA_AVAILABLE)
struct CMPCocoaBackend;

@interface CMPCocoaCameraDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
    struct CMPCocoaBackend *backend;
}
- (id)initWithBackend:(struct CMPCocoaBackend *)backend;
@end

static int cmp_cocoa_camera_update_frame(struct CMPCocoaBackend *backend, CMSampleBufferRef sample);
#endif

#if defined(__has_feature)
#if __has_feature(objc_arc)
#define CMP_COCOA_ARC 1
#endif
#endif
#ifndef CMP_COCOA_ARC
#define CMP_COCOA_ARC 0
#endif

#if defined(CMP_COCOA_AVAILABLE)

typedef struct CMPCocoaWindow {
    CMPObjectHeader header;
    struct CMPCocoaBackend *backend;
    NSWindow *window;
    NSView *content_view;
    cmp_i32 width;
    cmp_i32 height;
    CMPScalar dpi_scale;
    CMPScalar dpi_scale_override;
    CMPBool has_scale_override;
    cmp_u32 flags;
    CMPBool visible;
} CMPCocoaWindow;

typedef struct CMPCocoaTexture {
    CMPObjectHeader header;
    struct CMPCocoaBackend *backend;
    cmp_i32 width;
    cmp_i32 height;
    cmp_u32 format;
    cmp_u32 bytes_per_pixel;
    cmp_i32 stride;
    cmp_usize size;
    unsigned char *pixels;
} CMPCocoaTexture;

typedef struct CMPCocoaFont {
    CMPObjectHeader header;
    struct CMPCocoaBackend *backend;
    cmp_i32 size_px;
    cmp_i32 weight;
    CMPBool italic;
    CTFontRef font;
} CMPCocoaFont;

struct CMPCocoaBackend {
    CMPAllocator allocator;
    CMPHandleSystem handles;
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPIO io;
    CMPSensors sensors;
    CMPCamera camera;
    CMPNetwork network;
    CMPTasks tasks;
    CMPBool camera_opened;
    CMPBool camera_streaming;
    CMPBool camera_has_frame;
    cmp_u32 camera_format;
    cmp_u32 camera_width;
    cmp_u32 camera_height;
    cmp_u32 camera_requested_format;
    CMPBool camera_swizzle;
    cmp_usize camera_frame_capacity;
    cmp_usize camera_frame_size;
    unsigned char *camera_frame;
    int camera_error;
    AVCaptureSession *camera_session;
    AVCaptureDeviceInput *camera_input;
    AVCaptureVideoDataOutput *camera_output;
    dispatch_queue_t camera_queue;
    CMPCocoaCameraDelegate *camera_delegate;
    CMPBool initialized;
    CMPBool log_enabled;
    CMPBool log_owner;
    CMPBool inline_tasks;
    cmp_usize clipboard_limit;
    CMPCocoaWindow *active_window;
    CMPBool in_frame;
    CGContextRef frame_ctx;
    void *frame_data;
    cmp_usize frame_data_size;
    cmp_i32 frame_width;
    cmp_i32 frame_height;
    cmp_usize frame_stride;
    CMPMat3 transform;
    CMPBool has_transform;
    CMPRect clip_stack[CMP_COCOA_CLIP_STACK_CAPACITY];
    cmp_usize clip_depth;
    CMPInputEvent event_queue[CMP_COCOA_EVENT_CAPACITY];
    cmp_usize event_head;
    cmp_usize event_tail;
    cmp_usize event_count;
    char *last_text;
    double time_origin;
    NSApplication *app;
};

#if defined(CMP_COCOA_AVAILABLE)
@implementation CMPCocoaCameraDelegate
- (id)initWithBackend:(struct CMPCocoaBackend *)backend_ptr
{
    self = [super init];
    if (self) {
        backend = backend_ptr;
    }
    return self;
}

- (void)captureOutput:(AVCaptureOutput *)output
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection
{
    int rc;

    CMP_UNUSED(output);
    CMP_UNUSED(connection);

    if (backend == NULL) {
        return;
    }

    rc = cmp_cocoa_camera_update_frame(backend, sampleBuffer);
    CMP_UNUSED(rc);
}
@end
#endif

static void cmp_cocoa_event_queue_init(struct CMPCocoaBackend *backend)
{
    if (backend == NULL) {
        return;
    }
    backend->event_head = 0u;
    backend->event_tail = 0u;
    backend->event_count = 0u;
}

static int cmp_cocoa_event_queue_push(struct CMPCocoaBackend *backend, const CMPInputEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (backend->event_count >= CMP_COCOA_EVENT_CAPACITY) {
        return CMP_ERR_OVERFLOW;
    }
    backend->event_queue[backend->event_tail] = *event;
    backend->event_tail = (backend->event_tail + 1u) % CMP_COCOA_EVENT_CAPACITY;
    backend->event_count += 1u;
    return CMP_OK;
}

static int cmp_cocoa_event_queue_pop(struct CMPCocoaBackend *backend, CMPInputEvent *out_event, CMPBool *out_has_event)
{
    if (backend == NULL || out_event == NULL || out_has_event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (backend->event_count == 0u) {
        memset(out_event, 0, sizeof(*out_event));
        *out_has_event = CMP_FALSE;
        return CMP_OK;
    }
    *out_event = backend->event_queue[backend->event_head];
    backend->event_head = (backend->event_head + 1u) % CMP_COCOA_EVENT_CAPACITY;
    backend->event_count -= 1u;
    *out_has_event = CMP_TRUE;
    return CMP_OK;
}

static int cmp_cocoa_backend_validate_config(const CMPCocoaBackendConfig *config)
{
    if (config == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (config->handle_capacity == 0) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (config->allocator != NULL) {
        if (config->allocator->alloc == NULL || config->allocator->realloc == NULL || config->allocator->free == NULL) {
            return CMP_ERR_INVALID_ARGUMENT;
        }
    }
    return CMP_OK;
}

static int cmp_cocoa_backend_log(struct CMPCocoaBackend *backend, CMPLogLevel level, const char *message)
{
    if (!backend->log_enabled) {
        return CMP_OK;
    }
    return cmp_log_write(level, "m3.cocoa", message);
}

static int cmp_cocoa_backend_resolve(struct CMPCocoaBackend *backend, CMPHandle handle, cmp_u32 type_id, void **out_obj)
{
    void *resolved;
    cmp_u32 actual_type;
    int rc;

    if (out_obj != NULL) {
        *out_obj = NULL;
    }

    rc = backend->handles.vtable->resolve(backend->handles.ctx, handle, &resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_object_get_type_id((const CMPObjectHeader *)resolved, &actual_type);
    CMP_COCOA_RETURN_IF_ERROR(rc);
    if (actual_type != type_id) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    if (out_obj != NULL) {
        *out_obj = resolved;
    }
    return CMP_OK;
}

static int cmp_cocoa_object_retain(void *obj)
{
    return cmp_object_retain((CMPObjectHeader *)obj);
}

static int cmp_cocoa_object_release(void *obj)
{
    return cmp_object_release((CMPObjectHeader *)obj);
}

static int cmp_cocoa_object_get_type_id(void *obj, cmp_u32 *out_type_id)
{
    return cmp_object_get_type_id((const CMPObjectHeader *)obj, out_type_id);
}

static int cmp_cocoa_window_destroy(void *obj)
{
    CMPCocoaWindow *window;
    struct CMPCocoaBackend *backend;
    int rc;

    if (obj == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    window = (CMPCocoaWindow *)obj;
    backend = window->backend;

    if (backend != NULL && backend->active_window == window) {
        backend->active_window = NULL;
        backend->in_frame = CMP_FALSE;
    }

    if (backend != NULL) {
        rc = backend->handles.vtable->unregister_object(backend->handles.ctx, window->header.handle);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }

    if (window->window != NULL) {
        [window->window orderOut:nil];
#if !CMP_COCOA_ARC
        [window->window release];
#endif
        window->window = nil;
    }
    window->content_view = nil;

    if (backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, window);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }
    return CMP_OK;
}

static int cmp_cocoa_texture_destroy(void *obj)
{
    CMPCocoaTexture *texture;
    struct CMPCocoaBackend *backend;
    int rc;

    if (obj == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    texture = (CMPCocoaTexture *)obj;
    backend = texture->backend;

    if (backend != NULL) {
        rc = backend->handles.vtable->unregister_object(backend->handles.ctx, texture->header.handle);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }

    if (texture->pixels != NULL && backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, texture->pixels);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        texture->pixels = NULL;
    }

    if (backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, texture);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }
    return CMP_OK;
}

static int cmp_cocoa_font_destroy(void *obj)
{
    CMPCocoaFont *font;
    struct CMPCocoaBackend *backend;
    int rc;

    if (obj == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    font = (CMPCocoaFont *)obj;
    backend = font->backend;

    if (backend != NULL) {
        rc = backend->handles.vtable->unregister_object(backend->handles.ctx, font->header.handle);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }

    if (font->font != NULL) {
        CFRelease(font->font);
        font->font = NULL;
    }

    if (backend != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, font);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }
    return CMP_OK;
}

static const CMPObjectVTable g_cmp_cocoa_window_vtable = {
    cmp_cocoa_object_retain,
    cmp_cocoa_object_release,
    cmp_cocoa_window_destroy,
    cmp_cocoa_object_get_type_id
};

static const CMPObjectVTable g_cmp_cocoa_texture_vtable = {
    cmp_cocoa_object_retain,
    cmp_cocoa_object_release,
    cmp_cocoa_texture_destroy,
    cmp_cocoa_object_get_type_id
};

static const CMPObjectVTable g_cmp_cocoa_font_vtable = {
    cmp_cocoa_object_retain,
    cmp_cocoa_object_release,
    cmp_cocoa_font_destroy,
    cmp_cocoa_object_get_type_id
};

static int cmp_cocoa_color_validate(CMPColor color)
{
    if (color.r < 0.0f || color.r > 1.0f) {
        return CMP_ERR_RANGE;
    }
    if (color.g < 0.0f || color.g > 1.0f) {
        return CMP_ERR_RANGE;
    }
    if (color.b < 0.0f || color.b > 1.0f) {
        return CMP_ERR_RANGE;
    }
    if (color.a < 0.0f || color.a > 1.0f) {
        return CMP_ERR_RANGE;
    }
    return CMP_OK;
}

static int cmp_cocoa_get_active_context(struct CMPCocoaBackend *backend, CGContextRef *out_ctx)
{
    if (backend == NULL || out_ctx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (backend->frame_ctx == NULL || backend->active_window == NULL || !backend->in_frame) {
        return CMP_ERR_STATE;
    }
    *out_ctx = backend->frame_ctx;
    return CMP_OK;
}

static int cmp_cocoa_mat3_to_cgaffine(const CMPMat3 *mat, CGAffineTransform *out_transform)
{
    if (mat == NULL || out_transform == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    out_transform->a = (CGFloat)mat->m[0];
    out_transform->b = (CGFloat)mat->m[1];
    out_transform->c = (CGFloat)mat->m[3];
    out_transform->d = (CGFloat)mat->m[4];
    out_transform->tx = (CGFloat)mat->m[6];
    out_transform->ty = (CGFloat)mat->m[7];
    return CMP_OK;
}

static int cmp_cocoa_time_now_ms(struct CMPCocoaBackend *backend, cmp_u32 *out_time_ms)
{
    double now;
    double elapsed;
    double max_ms;

    if (backend == NULL || out_time_ms == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    now = CFAbsoluteTimeGetCurrent();
    elapsed = now - backend->time_origin;
    if (elapsed < 0.0) {
        elapsed = 0.0;
    }
    max_ms = (double)((cmp_u32)~(cmp_u32)0);
    if (elapsed * 1000.0 > max_ms) {
        *out_time_ms = (cmp_u32)~(cmp_u32)0;
        return CMP_OK;
    }
    *out_time_ms = (cmp_u32)(elapsed * 1000.0);
    return CMP_OK;
}

static void cmp_cocoa_release_last_text(struct CMPCocoaBackend *backend)
{
    int rc;

    if (backend == NULL || backend->last_text == NULL) {
        return;
    }

    rc = backend->allocator.free(backend->allocator.ctx, backend->last_text);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.text.free_failed");
    }
    backend->last_text = NULL;
}

static void cmp_cocoa_free_event_text(struct CMPCocoaBackend *backend, CMPInputEvent *event)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return;
    }

    if (event->type == CMP_INPUT_TEXT_UTF8) {
        if (event->data.text_utf8.utf8 != NULL) {
            rc = backend->allocator.free(backend->allocator.ctx, (void *)event->data.text_utf8.utf8);
            if (rc != CMP_OK) {
                cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.text_utf8.free_failed");
            }
            event->data.text_utf8.utf8 = NULL;
        }
    } else if (event->type == CMP_INPUT_TEXT_EDIT) {
        if (event->data.text_edit.utf8 != NULL) {
            rc = backend->allocator.free(backend->allocator.ctx, (void *)event->data.text_edit.utf8);
            if (rc != CMP_OK) {
                cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.text_edit.free_failed");
            }
            event->data.text_edit.utf8 = NULL;
        }
    }
}

static int cmp_cocoa_backend_push_event(struct CMPCocoaBackend *backend, const CMPInputEvent *event)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = cmp_cocoa_event_queue_push(backend, event);
    if (rc == CMP_ERR_OVERFLOW) {
        CMPInputEvent local_event = *event;
        cmp_cocoa_free_event_text(backend, &local_event);
        return CMP_OK;
    }
    return rc;
}

static cmp_u32 cmp_cocoa_modifiers_from_flags(NSEventModifierFlags flags)
{
    cmp_u32 mods = 0u;

#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)
    if ((flags & NSEventModifierFlagShift) != 0u) {
        mods |= CMP_MOD_SHIFT;
    }
    if ((flags & NSEventModifierFlagControl) != 0u) {
        mods |= CMP_MOD_CTRL;
    }
    if ((flags & NSEventModifierFlagOption) != 0u) {
        mods |= CMP_MOD_ALT;
    }
    if ((flags & NSEventModifierFlagCommand) != 0u) {
        mods |= CMP_MOD_META;
    }
    if ((flags & NSEventModifierFlagCapsLock) != 0u) {
        mods |= CMP_MOD_CAPS;
    }
    if ((flags & NSEventModifierFlagNumericPad) != 0u) {
        mods |= CMP_MOD_NUM;
    }
#else
    if ((flags & NSShiftKeyMask) != 0u) {
        mods |= CMP_MOD_SHIFT;
    }
    if ((flags & NSControlKeyMask) != 0u) {
        mods |= CMP_MOD_CTRL;
    }
    if ((flags & NSAlternateKeyMask) != 0u) {
        mods |= CMP_MOD_ALT;
    }
    if ((flags & NSCommandKeyMask) != 0u) {
        mods |= CMP_MOD_META;
    }
    if ((flags & NSAlphaShiftKeyMask) != 0u) {
        mods |= CMP_MOD_CAPS;
    }
    if ((flags & NSNumericPadKeyMask) != 0u) {
        mods |= CMP_MOD_NUM;
    }
#endif

    return mods;
}

static cmp_i32 cmp_cocoa_buttons_from_mask(NSUInteger mask)
{
    cmp_i32 buttons = 0;

    if ((mask & (1u << 0)) != 0u) {
        buttons |= 1;
    }
    if ((mask & (1u << 1)) != 0u) {
        buttons |= 2;
    }
    if ((mask & (1u << 2)) != 0u) {
        buttons |= 4;
    }
    if ((mask & (1u << 3)) != 0u) {
        buttons |= 8;
    }
    if ((mask & (1u << 4)) != 0u) {
        buttons |= 16;
    }
    return buttons;
}

static cmp_i32 cmp_cocoa_button_mask(NSInteger button_number)
{
    if (button_number == 0) {
        return 1;
    }
    if (button_number == 1) {
        return 2;
    }
    if (button_number == 2) {
        return 4;
    }
    if (button_number == 3) {
        return 8;
    }
    if (button_number == 4) {
        return 16;
    }
    return 0;
}

static int cmp_cocoa_fill_event_header(struct CMPCocoaBackend *backend, CMPInputEvent *event, cmp_u32 type, cmp_u32 modifiers,
                                      CMPHandle window)
{
    int rc;

    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    event->type = type;
    event->modifiers = modifiers;
    event->reserved = 0u;
    event->window = window;
    rc = cmp_cocoa_time_now_ms(backend, &event->time_ms);
    return rc;
}

static int cmp_cocoa_emit_pointer_event(struct CMPCocoaBackend *backend, CMPHandle window, cmp_u32 type, cmp_i32 x, cmp_i32 y,
                                       cmp_i32 buttons, cmp_i32 scroll_x, cmp_i32 scroll_y, cmp_u32 modifiers)
{
    CMPInputEvent event;
    int rc;

    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(&event, 0, sizeof(event));
    rc = cmp_cocoa_fill_event_header(backend, &event, type, modifiers, window);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    event.data.pointer.pointer_id = 0;
    event.data.pointer.x = x;
    event.data.pointer.y = y;
    event.data.pointer.buttons = buttons;
    event.data.pointer.scroll_x = scroll_x;
    event.data.pointer.scroll_y = scroll_y;
    return cmp_cocoa_backend_push_event(backend, &event);
}

static int cmp_cocoa_emit_key_event(struct CMPCocoaBackend *backend, CMPHandle window, cmp_u32 type, cmp_u32 modifiers,
                                   cmp_u32 key_code, cmp_u32 native_code, CMPBool is_repeat)
{
    CMPInputEvent event;
    int rc;

    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(&event, 0, sizeof(event));
    rc = cmp_cocoa_fill_event_header(backend, &event, type, modifiers, window);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    event.data.key.key_code = key_code;
    event.data.key.native_code = native_code;
    event.data.key.is_repeat = is_repeat;
    return cmp_cocoa_backend_push_event(backend, &event);
}

static int cmp_cocoa_copy_utf8(struct CMPCocoaBackend *backend, NSString *string, char **out_text, cmp_usize *out_len)
{
    const char *utf8;
    cmp_usize length;
    char *buffer;
    int rc;

    if (backend == NULL || out_text == NULL || out_len == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    *out_text = NULL;
    *out_len = 0u;

    if (string == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    utf8 = [string UTF8String];
    if (utf8 == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    length = 0u;
    while (utf8[length] != '\0') {
        length += 1u;
    }

    if (length == 0u) {
        return CMP_OK;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, length + 1u, (void **)&buffer);
    CMP_COCOA_RETURN_IF_ERROR(rc);
    memcpy(buffer, utf8, length + 1u);

    *out_text = buffer;
    *out_len = length;
    return CMP_OK;
}

static int cmp_cocoa_emit_text_event(struct CMPCocoaBackend *backend, CMPHandle window, cmp_u32 modifiers, NSString *string)
{
    CMPInputEvent event;
    const char *utf8;
    cmp_usize length;
    int rc;

    if (backend == NULL || string == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    utf8 = [string UTF8String];
    if (utf8 == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    length = 0u;
    while (utf8[length] != '\0') {
        length += 1u;
    }

    if (length == 0u) {
        return CMP_OK;
    }

    if (length <= (cmp_usize)(sizeof(event.data.text.utf8) - 1u)) {
        memset(&event, 0, sizeof(event));
        rc = cmp_cocoa_fill_event_header(backend, &event, CMP_INPUT_TEXT, modifiers, window);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        memcpy(event.data.text.utf8, utf8, length);
        event.data.text.utf8[length] = '\0';
        event.data.text.length = (cmp_u32)length;
        return cmp_cocoa_backend_push_event(backend, &event);
    }

    {
        char *buffer;
        cmp_usize out_len;

        rc = cmp_cocoa_copy_utf8(backend, string, &buffer, &out_len);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        if (buffer == NULL || out_len == 0u) {
            return CMP_OK;
        }

        memset(&event, 0, sizeof(event));
        rc = cmp_cocoa_fill_event_header(backend, &event, CMP_INPUT_TEXT_UTF8, modifiers, window);
        if (rc != CMP_OK) {
            backend->allocator.free(backend->allocator.ctx, buffer);
            return rc;
        }
        event.data.text_utf8.utf8 = buffer;
        event.data.text_utf8.length = out_len;
        return cmp_cocoa_backend_push_event(backend, &event);
    }
}

static int cmp_cocoa_utf8_bytes_for_range(NSString *string, NSRange range, cmp_i32 *out_offset, cmp_i32 *out_length)
{
    NSUInteger string_length;
    NSString *prefix;
    NSString *selection;
    NSUInteger offset_bytes;
    NSUInteger length_bytes;

    if (string == nil || out_offset == NULL || out_length == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    string_length = [string length];
    if (range.location == NSNotFound || range.location > string_length) {
        *out_offset = 0;
        *out_length = 0;
        return CMP_OK;
    }

    if (range.location + range.length > string_length) {
        range.length = string_length - range.location;
    }

    prefix = [string substringToIndex:range.location];
    selection = [string substringWithRange:range];
    offset_bytes = [prefix lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
    length_bytes = [selection lengthOfBytesUsingEncoding:NSUTF8StringEncoding];

    if (offset_bytes > (NSUInteger)INT_MAX || length_bytes > (NSUInteger)INT_MAX) {
        return CMP_ERR_OVERFLOW;
    }

    *out_offset = (cmp_i32)offset_bytes;
    *out_length = (cmp_i32)length_bytes;
    return CMP_OK;
}

static int cmp_cocoa_emit_text_edit_event(struct CMPCocoaBackend *backend, CMPHandle window, cmp_u32 modifiers, NSString *string,
                                         NSRange selected_range)
{
    CMPInputEvent event;
    char *buffer;
    cmp_usize out_len;
    cmp_i32 cursor;
    cmp_i32 selection_len;
    int rc;

    if (backend == NULL || string == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = cmp_cocoa_copy_utf8(backend, string, &buffer, &out_len);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_utf8_bytes_for_range(string, selected_range, &cursor, &selection_len);
    if (rc != CMP_OK) {
        if (buffer != NULL) {
            backend->allocator.free(backend->allocator.ctx, buffer);
        }
        return rc;
    }

    memset(&event, 0, sizeof(event));
    rc = cmp_cocoa_fill_event_header(backend, &event, CMP_INPUT_TEXT_EDIT, modifiers, window);
    if (rc != CMP_OK) {
        if (buffer != NULL) {
            backend->allocator.free(backend->allocator.ctx, buffer);
        }
        return rc;
    }

    event.data.text_edit.utf8 = buffer;
    event.data.text_edit.length = out_len;
    event.data.text_edit.cursor = cursor;
    event.data.text_edit.selection_length = selection_len;
    return cmp_cocoa_backend_push_event(backend, &event);
}

@interface CMPCocoaView : NSView <NSTextInputClient>
{
@public
    struct CMPCocoaBackend *backend;
    CMPHandle window_handle;
    NSString *marked_text;
    NSRange marked_range;
    NSRange selected_range;
}
@end

@implementation CMPCocoaView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self != nil) {
        backend = NULL;
        window_handle.id = 0u;
        window_handle.generation = 0u;
        marked_text = nil;
        marked_range = NSMakeRange(NSNotFound, 0);
        selected_range = NSMakeRange(0, 0);
    }
    return self;
}

- (BOOL)isFlipped
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)dealloc
{
#if !CMP_COCOA_ARC
    if (marked_text != nil) {
        [marked_text release];
        marked_text = nil;
    }
    [super dealloc];
#endif
}

- (void)cmp_update_marked_text:(NSString *)text selectedRange:(NSRange)range
{
    if (marked_text != nil) {
#if !CMP_COCOA_ARC
        [marked_text release];
#endif
        marked_text = nil;
    }

    if (text != nil && [text length] > 0) {
#if CMP_COCOA_ARC
        marked_text = text;
#else
        marked_text = [text copy];
#endif
        marked_range = NSMakeRange(0, [text length]);
        selected_range = range;
    } else {
        marked_range = NSMakeRange(NSNotFound, 0);
        selected_range = NSMakeRange(0, 0);
    }
}

- (cmp_u32)cmp_current_modifiers
{
    return cmp_cocoa_modifiers_from_flags([NSEvent modifierFlags]);
}

- (int)cmp_event_location:(NSEvent *)event outX:(cmp_i32 *)out_x outY:(cmp_i32 *)out_y
{
    NSPoint point;

    if (event == nil || out_x == NULL || out_y == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    point = [self convertPoint:[event locationInWindow] fromView:nil];
    *out_x = (cmp_i32)point.x;
    *out_y = (cmp_i32)point.y;
    return CMP_OK;
}

- (void)cmp_post_pointer_event:(NSEvent *)event type:(cmp_u32)type includeButton:(CMPBool)include_button
{
    cmp_i32 x;
    cmp_i32 y;
    cmp_i32 buttons;
    cmp_i32 button_mask;
    cmp_u32 modifiers;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    rc = [self cmp_event_location:event outX:&x outY:&y];
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.location_failed");
        return;
    }

    buttons = cmp_cocoa_buttons_from_mask([NSEvent pressedMouseButtons]);
    button_mask = cmp_cocoa_button_mask([event buttonNumber]);
    if (include_button) {
        if (type == CMP_INPUT_POINTER_UP) {
            buttons &= ~button_mask;
        } else if (type == CMP_INPUT_POINTER_DOWN) {
            buttons |= button_mask;
        }
    }

    modifiers = cmp_cocoa_modifiers_from_flags([event modifierFlags]);
    rc = cmp_cocoa_emit_pointer_event(backend, window_handle, type, x, y, buttons, 0, 0, modifiers);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.pointer.queue_failed");
    }
}

- (void)mouseDown:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_DOWN includeButton:CMP_TRUE];
}

- (void)mouseUp:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_UP includeButton:CMP_TRUE];
}

- (void)rightMouseDown:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_DOWN includeButton:CMP_TRUE];
}

- (void)rightMouseUp:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_UP includeButton:CMP_TRUE];
}

- (void)otherMouseDown:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_DOWN includeButton:CMP_TRUE];
}

- (void)otherMouseUp:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_UP includeButton:CMP_TRUE];
}

- (void)mouseMoved:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_MOVE includeButton:CMP_FALSE];
}

- (void)mouseDragged:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_MOVE includeButton:CMP_FALSE];
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_MOVE includeButton:CMP_FALSE];
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self cmp_post_pointer_event:event type:CMP_INPUT_POINTER_MOVE includeButton:CMP_FALSE];
}

- (void)scrollWheel:(NSEvent *)event
{
    cmp_i32 x;
    cmp_i32 y;
    cmp_i32 buttons;
    cmp_i32 scroll_x;
    cmp_i32 scroll_y;
    cmp_u32 modifiers;
    double dx;
    double dy;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    rc = [self cmp_event_location:event outX:&x outY:&y];
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.location_failed");
        return;
    }

    buttons = cmp_cocoa_buttons_from_mask([NSEvent pressedMouseButtons]);
    modifiers = cmp_cocoa_modifiers_from_flags([event modifierFlags]);

    dx = [event scrollingDeltaX];
    dy = [event scrollingDeltaY];
    scroll_x = (cmp_i32)dx;
    scroll_y = (cmp_i32)dy;

    rc = cmp_cocoa_emit_pointer_event(backend, window_handle, CMP_INPUT_POINTER_SCROLL, x, y, buttons, scroll_x, scroll_y, modifiers);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.scroll.queue_failed");
    }
}

- (void)keyDown:(NSEvent *)event
{
    NSArray *events;
    cmp_u32 modifiers;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    modifiers = cmp_cocoa_modifiers_from_flags([event modifierFlags]);
    rc = cmp_cocoa_emit_key_event(backend, window_handle, CMP_INPUT_KEY_DOWN, modifiers, (cmp_u32)[event keyCode],
                                 (cmp_u32)[event keyCode], event.isARepeat ? CMP_TRUE : CMP_FALSE);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.key_down.queue_failed");
    }

    events = [NSArray arrayWithObject:event];
    [self interpretKeyEvents:events];
}

- (void)keyUp:(NSEvent *)event
{
    cmp_u32 modifiers;
    int rc;

    if (backend == NULL || event == nil) {
        return;
    }

    modifiers = cmp_cocoa_modifiers_from_flags([event modifierFlags]);
    rc = cmp_cocoa_emit_key_event(backend, window_handle, CMP_INPUT_KEY_UP, modifiers, (cmp_u32)[event keyCode],
                                 (cmp_u32)[event keyCode], CMP_FALSE);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.key_up.queue_failed");
    }
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    NSString *text;
    cmp_u32 modifiers;
    int rc;

    CMP_UNUSED(replacementRange);

    if (backend == NULL || string == nil) {
        return;
    }

    if ([string isKindOfClass:[NSAttributedString class]]) {
        text = [string string];
    } else {
        text = (NSString *)string;
    }

    if (text == nil) {
        return;
    }

    [self cmp_update_marked_text:nil selectedRange:NSMakeRange(0, 0)];
    modifiers = [self cmp_current_modifiers];
    rc = cmp_cocoa_emit_text_event(backend, window_handle, modifiers, text);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.text.queue_failed");
    }
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
    NSString *text;
    cmp_u32 modifiers;
    int rc;

    CMP_UNUSED(replacementRange);

    if (backend == NULL || string == nil) {
        return;
    }

    if ([string isKindOfClass:[NSAttributedString class]]) {
        text = [string string];
    } else {
        text = (NSString *)string;
    }

    if (text == nil) {
        return;
    }

    [self cmp_update_marked_text:text selectedRange:selectedRange];
    modifiers = [self cmp_current_modifiers];
    rc = cmp_cocoa_emit_text_edit_event(backend, window_handle, modifiers, text, selectedRange);
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.text_edit.queue_failed");
    }
}

- (void)unmarkText
{
    cmp_u32 modifiers;
    int rc;

    if (backend == NULL) {
        return;
    }

    if (marked_text == nil || [marked_text length] == 0) {
        return;
    }

    [self cmp_update_marked_text:nil selectedRange:NSMakeRange(0, 0)];
    modifiers = [self cmp_current_modifiers];
    rc = cmp_cocoa_emit_text_edit_event(backend, window_handle, modifiers, @"", NSMakeRange(0, 0));
    if (rc != CMP_OK) {
        cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_ERROR, "event.text_edit.queue_failed");
    }
}

- (BOOL)hasMarkedText
{
    return (marked_text != nil && [marked_text length] > 0);
}

- (NSRange)markedRange
{
    if (marked_text == nil || [marked_text length] == 0) {
        return NSMakeRange(NSNotFound, 0);
    }
    return marked_range;
}

- (NSRange)selectedRange
{
    return selected_range;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    CMP_UNUSED(range);
    if (actualRange != NULL) {
        *actualRange = NSMakeRange(NSNotFound, 0);
    }
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    CMP_UNUSED(point);
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange
{
    NSRect rect;
    NSWindow *window;

    if (actualRange != NULL) {
        *actualRange = range;
    }

    rect = [self bounds];
    window = [self window];
    if (window != nil) {
        rect = [self convertRect:rect toView:nil];
        rect = [window convertRectToScreen:rect];
    }
    return rect;
}

- (void)doCommandBySelector:(SEL)selector
{
    CMP_UNUSED(selector);
}

@end

static int cmp_cocoa_ws_init(void *ws, const CMPWSConfig *config)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (ws == NULL || config == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_app_name == NULL || config->utf8_app_id == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.init");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    backend->app = [NSApplication sharedApplication];
    if (backend->app != nil) {
        [backend->app setActivationPolicy:NSApplicationActivationPolicyRegular];
    }
    return CMP_OK;
}

static int cmp_cocoa_ws_shutdown(void *ws)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.shutdown");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_OK;
}

static int cmp_cocoa_ws_create_window(void *ws, const CMPWSWindowConfig *config, CMPHandle *out_window)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *window;
    NSWindowStyleMask style_mask;
    NSRect rect;
    NSString *title;
    NSView *content_view;
    int rc;

    if (ws == NULL || config == NULL || out_window == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (config->utf8_title == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (config->width <= 0 || config->height <= 0) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)ws;
    out_window->id = 0u;
    out_window->generation = 0u;

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.create_window");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    style_mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if (config->flags & CMP_WS_WINDOW_RESIZABLE) {
        style_mask |= NSWindowStyleMaskResizable;
    }
    if (config->flags & CMP_WS_WINDOW_BORDERLESS) {
        style_mask = NSWindowStyleMaskBorderless;
    }

    rect = NSMakeRect(0.0, 0.0, (CGFloat)config->width, (CGFloat)config->height);
    title = [[NSString alloc] initWithUTF8String:config->utf8_title];
    if (title == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPCocoaWindow), (void **)&window);
    if (rc != CMP_OK) {
#if !CMP_COCOA_ARC
        [title release];
#endif
        return rc;
    }

    memset(window, 0, sizeof(*window));
    window->backend = backend;
    window->width = config->width;
    window->height = config->height;
    window->dpi_scale = 1.0f;
    window->flags = config->flags;
    window->visible = CMP_FALSE;

    window->window = [[NSWindow alloc] initWithContentRect:rect
                                                 styleMask:style_mask
                                                   backing:NSBackingStoreBuffered
                                                     defer:NO];
    if (window->window == nil) {
#if !CMP_COCOA_ARC
        [title release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return CMP_ERR_UNKNOWN;
    }

    content_view = [[CMPCocoaView alloc] initWithFrame:rect];
    if (content_view == nil) {
#if !CMP_COCOA_ARC
        [window->window release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return CMP_ERR_OUT_OF_MEMORY;
    }
    [content_view setWantsLayer:YES];
    [window->window setContentView:content_view];
    window->content_view = content_view;
#if !CMP_COCOA_ARC
    [content_view release];
#endif
    [window->window setAcceptsMouseMovedEvents:YES];

    [window->window setReleasedWhenClosed:NO];
    [window->window setTitle:title];
#if !CMP_COCOA_ARC
    [title release];
#endif

    if ((config->flags & CMP_WS_WINDOW_HIGH_DPI) != 0u) {
        window->dpi_scale = (CMPScalar)[window->window backingScaleFactor];
    }

    rc = cmp_object_header_init(&window->header, CMP_COCOA_TYPE_WINDOW, 0, &g_cmp_cocoa_window_vtable);
    if (rc != CMP_OK) {
        [window->window orderOut:nil];
#if !CMP_COCOA_ARC
        [window->window release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return rc;
    }

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &window->header);
    if (rc != CMP_OK) {
        [window->window orderOut:nil];
#if !CMP_COCOA_ARC
        [window->window release];
#endif
        backend->allocator.free(backend->allocator.ctx, window);
        return rc;
    }

    if (window->content_view != nil) {
        CMPCocoaView *view = (CMPCocoaView *)window->content_view;
        view->backend = backend;
        view->window_handle = window->header.handle;
        [window->window makeFirstResponder:window->content_view];
    }

    if ((config->flags & CMP_WS_WINDOW_FULLSCREEN) != 0u) {
        [window->window toggleFullScreen:nil];
    }

    *out_window = window->header.handle;
    return CMP_OK;
}

static int cmp_cocoa_ws_destroy_window(void *ws, CMPHandle window)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.destroy_window");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    return cmp_object_release(&resolved->header);
}

static int cmp_cocoa_ws_show_window(void *ws, CMPHandle window)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.show_window");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->window != nil) {
        [resolved->window makeKeyAndOrderFront:nil];
        if (resolved->content_view != nil) {
            [resolved->window makeFirstResponder:resolved->content_view];
        }
        if (backend->app != nil) {
            [backend->app activateIgnoringOtherApps:YES];
        }
    }
    resolved->visible = CMP_TRUE;
    return CMP_OK;
}

static int cmp_cocoa_ws_hide_window(void *ws, CMPHandle window)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.hide_window");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->window != nil) {
        [resolved->window orderOut:nil];
    }
    resolved->visible = CMP_FALSE;
    return CMP_OK;
}

static int cmp_cocoa_ws_set_window_title(void *ws, CMPHandle window, const char *utf8_title)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    NSString *title;
    int rc;

    if (ws == NULL || utf8_title == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_title");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    title = [[NSString alloc] initWithUTF8String:utf8_title];
    if (title == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (resolved->window != nil) {
        [resolved->window setTitle:title];
    }
#if !CMP_COCOA_ARC
    [title release];
#endif
    return CMP_OK;
}

static int cmp_cocoa_ws_set_window_size(void *ws, CMPHandle window, cmp_i32 width, cmp_i32 height)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    NSSize size;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_size");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    size.width = (CGFloat)width;
    size.height = (CGFloat)height;
    if (resolved->window != nil) {
        [resolved->window setContentSize:size];
    }
    resolved->width = width;
    resolved->height = height;
    return CMP_OK;
}

static int cmp_cocoa_ws_get_window_size(void *ws, CMPHandle window, cmp_i32 *out_width, cmp_i32 *out_height)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    NSRect rect;
    int rc;

    if (ws == NULL || out_width == NULL || out_height == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_size");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->window != nil) {
        rect = [resolved->window contentRectForFrameRect:[resolved->window frame]];
        resolved->width = (cmp_i32)rect.size.width;
        resolved->height = (cmp_i32)rect.size.height;
    }

    *out_width = resolved->width;
    *out_height = resolved->height;
    return CMP_OK;
}

static int cmp_cocoa_ws_set_window_dpi_scale(void *ws, CMPHandle window, CMPScalar scale)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (scale <= 0.0f) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_window_dpi_scale");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    resolved->dpi_scale_override = scale;
    resolved->has_scale_override = CMP_TRUE;
    return CMP_OK;
}

static int cmp_cocoa_ws_get_window_dpi_scale(void *ws, CMPHandle window, CMPScalar *out_scale)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    int rc;

    if (ws == NULL || out_scale == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_window_dpi_scale");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->has_scale_override) {
        *out_scale = resolved->dpi_scale_override;
        return CMP_OK;
    }

    if (resolved->window != nil) {
        resolved->dpi_scale = (CMPScalar)[resolved->window backingScaleFactor];
    }
    *out_scale = resolved->dpi_scale;
    return CMP_OK;
}

static int cmp_cocoa_ws_set_clipboard_text(void *ws, const char *utf8_text)
{
    struct CMPCocoaBackend *backend;
    cmp_usize length;
    NSString *text;
    NSPasteboard *pasteboard;
    int rc;

    if (ws == NULL || utf8_text == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.set_clipboard_text");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    length = 0;
    while (utf8_text[length] != '\0') {
        length += 1;
    }
    if (length > backend->clipboard_limit) {
        return CMP_ERR_RANGE;
    }

    text = [[NSString alloc] initWithBytes:utf8_text length:length encoding:NSUTF8StringEncoding];
    if (text == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard == nil) {
#if !CMP_COCOA_ARC
        [text release];
#endif
        return CMP_ERR_STATE;
    }

    [pasteboard clearContents];
    if (![pasteboard setString:text forType:NSPasteboardTypeString]) {
#if !CMP_COCOA_ARC
        [text release];
#endif
        return CMP_ERR_UNKNOWN;
    }

#if !CMP_COCOA_ARC
    [text release];
#endif
    return CMP_OK;
}

static int cmp_cocoa_ws_get_clipboard_text(void *ws, char *buffer, cmp_usize buffer_size, cmp_usize *out_length)
{
    struct CMPCocoaBackend *backend;
    NSPasteboard *pasteboard;
    NSString *text;
    const char *utf8;
    cmp_usize length;
    int rc;

    if (ws == NULL || buffer == NULL || out_length == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "ws.get_clipboard_text");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    pasteboard = [NSPasteboard generalPasteboard];
    if (pasteboard == nil) {
        return CMP_ERR_STATE;
    }

    text = [pasteboard stringForType:NSPasteboardTypeString];
    if (text == nil) {
        if (buffer_size == 0) {
            return CMP_ERR_RANGE;
        }
        buffer[0] = '\0';
        *out_length = 0;
        return CMP_OK;
    }

    utf8 = [text UTF8String];
    if (utf8 == NULL) {
        if (buffer_size == 0) {
            return CMP_ERR_RANGE;
        }
        buffer[0] = '\0';
        *out_length = 0;
        return CMP_OK;
    }

    length = 0;
    while (utf8[length] != '\0') {
        length += 1;
    }

    if (buffer_size < length + 1) {
        *out_length = length;
        return CMP_ERR_RANGE;
    }

    memcpy(buffer, utf8, length + 1);
    *out_length = length;
    return CMP_OK;
}

static int cmp_cocoa_ws_pump_events(void *ws);

static int cmp_cocoa_ws_poll_event(void *ws, CMPInputEvent *out_event, CMPBool *out_has_event)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (ws == NULL || out_event == NULL || out_has_event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.poll_event");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    cmp_cocoa_release_last_text(backend);

    if (backend->event_count == 0u) {
        rc = cmp_cocoa_ws_pump_events(ws);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    }

    rc = cmp_cocoa_event_queue_pop(backend, out_event, out_has_event);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (*out_has_event) {
        if (out_event->type == CMP_INPUT_TEXT_UTF8 && out_event->data.text_utf8.utf8 != NULL) {
            backend->last_text = (char *)out_event->data.text_utf8.utf8;
        } else if (out_event->type == CMP_INPUT_TEXT_EDIT && out_event->data.text_edit.utf8 != NULL) {
            backend->last_text = (char *)out_event->data.text_edit.utf8;
        }
    }
    return CMP_OK;
}

static int cmp_cocoa_ws_pump_events(void *ws)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.pump_events");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->app != nil) {
        NSEvent *event;
        do {
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101200)
            event = [backend->app nextEventMatchingMask:NSEventMaskAny
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES];
#else
            event = [backend->app nextEventMatchingMask:NSAnyEventMask
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES];
#endif
            if (event != nil) {
                [backend->app sendEvent:event];
            }
        } while (event != nil);
    }

    return CMP_OK;
}

static int cmp_cocoa_ws_get_time_ms(void *ws, cmp_u32 *out_time_ms)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (ws == NULL || out_time_ms == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)ws;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "ws.get_time_ms");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    return cmp_cocoa_time_now_ms(backend, out_time_ms);
}

static const CMPWSVTable g_cmp_cocoa_ws_vtable = {
    cmp_cocoa_ws_init,
    cmp_cocoa_ws_shutdown,
    cmp_cocoa_ws_create_window,
    cmp_cocoa_ws_destroy_window,
    cmp_cocoa_ws_show_window,
    cmp_cocoa_ws_hide_window,
    cmp_cocoa_ws_set_window_title,
    cmp_cocoa_ws_set_window_size,
    cmp_cocoa_ws_get_window_size,
    cmp_cocoa_ws_set_window_dpi_scale,
    cmp_cocoa_ws_get_window_dpi_scale,
    cmp_cocoa_ws_set_clipboard_text,
    cmp_cocoa_ws_get_clipboard_text,
    cmp_cocoa_ws_poll_event,
    cmp_cocoa_ws_pump_events,
    cmp_cocoa_ws_get_time_ms
};

static int cmp_cocoa_gfx_begin_frame(void *gfx, CMPHandle window, cmp_i32 width, cmp_i32 height, CMPScalar dpi_scale)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    CGColorSpaceRef color_space;
    CGContextRef context;
    cmp_usize row_bytes;
    cmp_usize total_bytes;
    void *data;
    int rc;

    if (gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return CMP_ERR_RANGE;
    }
    if (dpi_scale <= 0.0f) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    if (backend->active_window != NULL) {
        return CMP_ERR_STATE;
    }

    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.begin_frame");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    row_bytes = (cmp_usize)width * 4u;
    if (row_bytes / 4u != (cmp_usize)width) {
        return CMP_ERR_OVERFLOW;
    }
    total_bytes = row_bytes * (cmp_usize)height;
    if (total_bytes / (cmp_usize)height != row_bytes) {
        return CMP_ERR_OVERFLOW;
    }

    if (backend->frame_data == NULL || backend->frame_data_size < total_bytes) {
        if (backend->frame_data != NULL) {
            rc = backend->allocator.free(backend->allocator.ctx, backend->frame_data);
            CMP_COCOA_RETURN_IF_ERROR(rc);
            backend->frame_data = NULL;
            backend->frame_data_size = 0u;
        }
        rc = backend->allocator.alloc(backend->allocator.ctx, total_bytes, &data);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        backend->frame_data = data;
        backend->frame_data_size = total_bytes;
    } else {
        data = backend->frame_data;
    }
    memset(data, 0, total_bytes);

    color_space = CGColorSpaceCreateDeviceRGB();
    if (color_space == NULL) {
        backend->allocator.free(backend->allocator.ctx, data);
        return CMP_ERR_UNKNOWN;
    }

    context = CGBitmapContextCreate(data, (size_t)width, (size_t)height, 8, (size_t)row_bytes, color_space,
                                    kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(color_space);
    if (context == NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, data);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        if (backend->frame_data == data) {
            backend->frame_data = NULL;
            backend->frame_data_size = 0u;
        }
        return CMP_ERR_UNKNOWN;
    }

    CGContextTranslateCTM(context, 0.0, (CGFloat)height);
    CGContextScaleCTM(context, 1.0, -1.0);

    backend->frame_ctx = context;
    backend->frame_data = data;
    backend->frame_data_size = total_bytes;
    backend->frame_width = width;
    backend->frame_height = height;
    backend->frame_stride = row_bytes;
    backend->active_window = resolved;
    backend->clip_depth = 0u;
    backend->has_transform = CMP_FALSE;
    backend->in_frame = CMP_TRUE;

    resolved->width = width;
    resolved->height = height;
    resolved->dpi_scale = dpi_scale;
    return CMP_OK;
}

static int cmp_cocoa_gfx_end_frame(void *gfx, CMPHandle window)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaWindow *resolved;
    int result;
    int rc;

    if (gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_resolve(backend, window, CMP_COCOA_TYPE_WINDOW, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->active_window != resolved) {
        return CMP_ERR_STATE;
    }

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.end_frame");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    result = CMP_OK;

    if (backend->frame_ctx != NULL) {
        CGImageRef image;
        NSView *view;
        id layer;

        CGContextFlush(backend->frame_ctx);
        image = CGBitmapContextCreateImage(backend->frame_ctx);
        if (image == NULL) {
            result = CMP_ERR_UNKNOWN;
        } else {
            view = resolved->content_view;
            if (view == nil && resolved->window != nil) {
                view = [resolved->window contentView];
            }
            if (view != nil) {
                layer = [view layer];
                if (layer != nil) {
                    [layer setContents:(id)image];
                }
            }
            CGImageRelease(image);
        }
        CGContextRelease(backend->frame_ctx);
        backend->frame_ctx = NULL;
    }
    backend->frame_width = 0;
    backend->frame_height = 0;
    backend->frame_stride = 0u;
    backend->active_window = NULL;
    backend->in_frame = CMP_FALSE;
    return result;
}

static int cmp_cocoa_gfx_clear(void *gfx, CMPColor color)
{
    struct CMPCocoaBackend *backend;
    CGContextRef ctx;
    CGRect rect;
    int rc;

    if (gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.clear");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_color_validate(color);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rect = CGRectMake(0.0, 0.0, (CGFloat)backend->frame_width, (CGFloat)backend->frame_height);
    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextFillRect(ctx, rect);
    return CMP_OK;
}

static int cmp_cocoa_gfx_draw_rect(void *gfx, const CMPRect *rect, CMPColor color, CMPScalar corner_radius)
{
    struct CMPCocoaBackend *backend;
    CGContextRef ctx;
    CGRect cg_rect;
    CGPathRef path;
    CGAffineTransform transform;
    CMPBool apply_transform;
    CMPScalar radius;
    int rc;

    if (gfx == NULL || rect == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return CMP_ERR_RANGE;
    }
    if (corner_radius < 0.0f) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_rect");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_color_validate(color);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    cg_rect = CGRectMake((CGFloat)rect->x, (CGFloat)rect->y, (CGFloat)rect->width, (CGFloat)rect->height);
    radius = corner_radius;
    if (radius > 0.0f) {
        if (radius * 2.0f > rect->width) {
            radius = rect->width * 0.5f;
        }
        if (radius * 2.0f > rect->height) {
            radius = rect->height * 0.5f;
        }
    }

    apply_transform = backend->has_transform ? CMP_TRUE : CMP_FALSE;
    if (apply_transform) {
        rc = cmp_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    if (radius > 0.0f) {
        path = CGPathCreateWithRoundedRect(cg_rect, (CGFloat)radius, (CGFloat)radius, NULL);
        if (path != NULL) {
            CGContextAddPath(ctx, path);
            CGContextFillPath(ctx);
            CGPathRelease(path);
        } else {
            CGContextFillRect(ctx, cg_rect);
        }
    } else {
        CGContextFillRect(ctx, cg_rect);
    }

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }
    return CMP_OK;
}

static int cmp_cocoa_gfx_draw_line(void *gfx, CMPScalar x0, CMPScalar y0, CMPScalar x1, CMPScalar y1, CMPColor color, CMPScalar thickness)
{
    struct CMPCocoaBackend *backend;
    CGContextRef ctx;
    CGAffineTransform transform;
    CMPBool apply_transform;
    int rc;

    if (gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (thickness < 0.0f) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_line");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_color_validate(color);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    apply_transform = backend->has_transform ? CMP_TRUE : CMP_FALSE;
    if (apply_transform) {
        rc = cmp_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBStrokeColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextSetLineWidth(ctx, (CGFloat)thickness);
    CGContextMoveToPoint(ctx, (CGFloat)x0, (CGFloat)y0);
    CGContextAddLineToPoint(ctx, (CGFloat)x1, (CGFloat)y1);
    CGContextStrokePath(ctx);

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }
    return CMP_OK;
}

static int cmp_cocoa_path_build(const CMPPath *path, CGMutablePathRef cg_path)
{
    CMPScalar current_x;
    CMPScalar current_y;
    CMPScalar start_x;
    CMPScalar start_y;
    CMPBool has_current;
    cmp_usize i;

    if (path == NULL || cg_path == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (path->commands == NULL) {
        return CMP_ERR_STATE;
    }
    if (path->count == 0) {
        return CMP_OK;
    }

    has_current = CMP_FALSE;
    current_x = 0.0f;
    current_y = 0.0f;
    start_x = 0.0f;
    start_y = 0.0f;

    for (i = 0; i < path->count; ++i) {
        const CMPPathCmd *cmd = &path->commands[i];
        switch (cmd->type) {
        case CMP_PATH_CMD_MOVE_TO:
            current_x = cmd->data.move_to.x;
            current_y = cmd->data.move_to.y;
            start_x = current_x;
            start_y = current_y;
            has_current = CMP_TRUE;
            CGPathMoveToPoint(cg_path, NULL, (CGFloat)current_x, (CGFloat)current_y);
            break;
        case CMP_PATH_CMD_LINE_TO:
            if (!has_current) {
                return CMP_ERR_STATE;
            }
            current_x = cmd->data.line_to.x;
            current_y = cmd->data.line_to.y;
            CGPathAddLineToPoint(cg_path, NULL, (CGFloat)current_x, (CGFloat)current_y);
            break;
        case CMP_PATH_CMD_QUAD_TO:
            if (!has_current) {
                return CMP_ERR_STATE;
            }
            current_x = cmd->data.quad_to.x;
            current_y = cmd->data.quad_to.y;
            CGPathAddQuadCurveToPoint(cg_path, NULL,
                                      (CGFloat)cmd->data.quad_to.cx,
                                      (CGFloat)cmd->data.quad_to.cy,
                                      (CGFloat)current_x, (CGFloat)current_y);
            break;
        case CMP_PATH_CMD_CUBIC_TO:
            if (!has_current) {
                return CMP_ERR_STATE;
            }
            current_x = cmd->data.cubic_to.x;
            current_y = cmd->data.cubic_to.y;
            CGPathAddCurveToPoint(cg_path, NULL,
                                  (CGFloat)cmd->data.cubic_to.cx1,
                                  (CGFloat)cmd->data.cubic_to.cy1,
                                  (CGFloat)cmd->data.cubic_to.cx2,
                                  (CGFloat)cmd->data.cubic_to.cy2,
                                  (CGFloat)current_x, (CGFloat)current_y);
            break;
        case CMP_PATH_CMD_CLOSE:
            if (!has_current) {
                return CMP_ERR_STATE;
            }
            CGPathCloseSubpath(cg_path);
            current_x = start_x;
            current_y = start_y;
            has_current = CMP_FALSE;
            break;
        default:
            return CMP_ERR_INVALID_ARGUMENT;
        }
    }
    return CMP_OK;
}

static int cmp_cocoa_gfx_draw_path(void *gfx, const CMPPath *path, CMPColor color)
{
    struct CMPCocoaBackend *backend;
    CGContextRef ctx;
    CGMutablePathRef cg_path;
    CGAffineTransform transform;
    CMPBool apply_transform;
    int rc;

    if (gfx == NULL || path == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (path->commands == NULL) {
        return CMP_ERR_STATE;
    }
    if (path->count > path->capacity) {
        return CMP_ERR_STATE;
    }
    if (path->count == 0) {
        return CMP_OK;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_path");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_color_validate(color);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    cg_path = CGPathCreateMutable();
    if (cg_path == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    rc = cmp_cocoa_path_build(path, cg_path);
    if (rc != CMP_OK) {
        CGPathRelease(cg_path);
        return rc;
    }

    apply_transform = backend->has_transform ? CMP_TRUE : CMP_FALSE;
    if (apply_transform) {
        rc = cmp_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        CMP_COCOA_RETURN_IF_ERROR_CLEANUP(rc, CGPathRelease(cg_path));
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextAddPath(ctx, cg_path);
    CGContextFillPath(ctx);

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }

    CGPathRelease(cg_path);
    return CMP_OK;
}

static int cmp_cocoa_gfx_push_clip(void *gfx, const CMPRect *rect)
{
    struct CMPCocoaBackend *backend;
    CGContextRef ctx;
    CGRect cg_rect;
    int rc;

    if (gfx == NULL || rect == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (rect->width < 0.0f || rect->height < 0.0f) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.push_clip");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->clip_depth >= CMP_COCOA_CLIP_STACK_CAPACITY) {
        return CMP_ERR_OVERFLOW;
    }

    cg_rect = CGRectMake((CGFloat)rect->x, (CGFloat)rect->y, (CGFloat)rect->width, (CGFloat)rect->height);
    CGContextSaveGState(ctx);
    CGContextClipToRect(ctx, cg_rect);
    backend->clip_stack[backend->clip_depth] = *rect;
    backend->clip_depth += 1u;
    return CMP_OK;
}

static int cmp_cocoa_gfx_pop_clip(void *gfx)
{
    struct CMPCocoaBackend *backend;
    CGContextRef ctx;
    int rc;

    if (gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.pop_clip");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->clip_depth == 0u) {
        return CMP_ERR_STATE;
    }

    CGContextRestoreGState(ctx);
    backend->clip_depth -= 1u;
    return CMP_OK;
}

static int cmp_cocoa_gfx_set_transform(void *gfx, const CMPMat3 *transform)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (gfx == NULL || transform == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.set_transform");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    backend->transform = *transform;
    backend->has_transform = CMP_TRUE;
    return CMP_OK;
}

static int cmp_cocoa_texture_format_info(cmp_u32 format, cmp_u32 *out_bpp, CGBitmapInfo *out_info, CGColorSpaceRef *out_space)
{
    CGColorSpaceRef color_space;
    CGBitmapInfo info;
    cmp_u32 bpp;

    if (out_bpp == NULL || out_info == NULL || out_space == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    color_space = NULL;
    info = 0u;
    bpp = 0u;

    if (format == CMP_TEX_FORMAT_RGBA8) {
        color_space = CGColorSpaceCreateDeviceRGB();
        info = kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast;
        bpp = 4u;
    } else if (format == CMP_TEX_FORMAT_BGRA8) {
        color_space = CGColorSpaceCreateDeviceRGB();
        info = kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
        bpp = 4u;
    } else if (format == CMP_TEX_FORMAT_A8) {
        color_space = CGColorSpaceCreateDeviceGray();
        info = (CGBitmapInfo)kCGImageAlphaOnly;
        bpp = 1u;
    } else {
        return CMP_ERR_UNSUPPORTED;
    }

    if (color_space == NULL) {
        return CMP_ERR_UNKNOWN;
    }

    *out_bpp = bpp;
    *out_info = info;
    *out_space = color_space;
    return CMP_OK;
}

static int cmp_cocoa_gfx_create_texture(void *gfx, cmp_i32 width, cmp_i32 height, cmp_u32 format, const void *pixels, cmp_usize size,
                                      CMPHandle *out_texture)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaTexture *texture;
    cmp_u32 bpp;
    CGBitmapInfo info;
    CGColorSpaceRef color_space;
    cmp_usize row_bytes;
    cmp_usize total_bytes;
    int rc;

    if (gfx == NULL || out_texture == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return CMP_ERR_RANGE;
    }
    if (pixels == NULL && size != 0u) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    out_texture->id = 0u;
    out_texture->generation = 0u;

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.create_texture");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    color_space = NULL;
    rc = cmp_cocoa_texture_format_info(format, &bpp, &info, &color_space);
    if (rc != CMP_OK) {
        if (color_space != NULL) {
            CGColorSpaceRelease(color_space);
        }
        return rc;
    }
    CGColorSpaceRelease(color_space);

    row_bytes = (cmp_usize)width * (cmp_usize)bpp;
    if (row_bytes / (cmp_usize)bpp != (cmp_usize)width) {
        return CMP_ERR_OVERFLOW;
    }
    total_bytes = row_bytes * (cmp_usize)height;
    if (total_bytes / (cmp_usize)height != row_bytes) {
        return CMP_ERR_OVERFLOW;
    }
    if (pixels != NULL && size < total_bytes) {
        return CMP_ERR_RANGE;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPCocoaTexture), (void **)&texture);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    memset(texture, 0, sizeof(*texture));
    texture->backend = backend;
    texture->width = width;
    texture->height = height;
    texture->format = format;
    texture->bytes_per_pixel = bpp;
    texture->stride = (cmp_i32)row_bytes;
    texture->size = total_bytes;

    rc = backend->allocator.alloc(backend->allocator.ctx, total_bytes, (void **)&texture->pixels);
    if (rc != CMP_OK) {
        backend->allocator.free(backend->allocator.ctx, texture);
        return rc;
    }

    if (pixels != NULL && total_bytes > 0u) {
        memcpy(texture->pixels, pixels, total_bytes);
    } else if (total_bytes > 0u) {
        memset(texture->pixels, 0, total_bytes);
    }

    rc = cmp_object_header_init(&texture->header, CMP_COCOA_TYPE_TEXTURE, 0, &g_cmp_cocoa_texture_vtable);
    CMP_COCOA_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, texture->pixels);
                                     backend->allocator.free(backend->allocator.ctx, texture));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &texture->header);
    CMP_COCOA_RETURN_IF_ERROR_CLEANUP(rc, backend->allocator.free(backend->allocator.ctx, texture->pixels);
                                     backend->allocator.free(backend->allocator.ctx, texture));

    *out_texture = texture->header.handle;
    return CMP_OK;
}

static int cmp_cocoa_gfx_update_texture(void *gfx, CMPHandle texture, cmp_i32 x, cmp_i32 y, cmp_i32 width, cmp_i32 height, const void *pixels,
                                      cmp_usize size)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaTexture *resolved;
    cmp_usize row_bytes;
    cmp_usize total_bytes;
    cmp_usize row;
    const unsigned char *src;
    unsigned char *dst;
    int rc;

    if (gfx == NULL || pixels == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (width <= 0 || height <= 0) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_resolve(backend, texture, CMP_COCOA_TYPE_TEXTURE, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.update_texture");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (x < 0 || y < 0 || x + width > resolved->width || y + height > resolved->height) {
        return CMP_ERR_RANGE;
    }

    row_bytes = (cmp_usize)width * (cmp_usize)resolved->bytes_per_pixel;
    if (row_bytes / (cmp_usize)resolved->bytes_per_pixel != (cmp_usize)width) {
        return CMP_ERR_OVERFLOW;
    }
    total_bytes = row_bytes * (cmp_usize)height;
    if (total_bytes / (cmp_usize)height != row_bytes) {
        return CMP_ERR_OVERFLOW;
    }
    if (size < total_bytes) {
        return CMP_ERR_RANGE;
    }

    src = (const unsigned char *)pixels;
    dst = resolved->pixels + ((cmp_usize)y * (cmp_usize)resolved->stride) + ((cmp_usize)x * (cmp_usize)resolved->bytes_per_pixel);
    for (row = 0; row < (cmp_usize)height; row += 1u) {
        memcpy(dst, src, row_bytes);
        src += row_bytes;
        dst += (cmp_usize)resolved->stride;
    }

    return CMP_OK;
}

static int cmp_cocoa_gfx_destroy_texture(void *gfx, CMPHandle texture)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaTexture *resolved;
    int rc;

    if (gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_resolve(backend, texture, CMP_COCOA_TYPE_TEXTURE, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.destroy_texture");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    return cmp_object_release(&resolved->header);
}

static int cmp_cocoa_gfx_draw_texture(void *gfx, CMPHandle texture, const CMPRect *src, const CMPRect *dst, CMPScalar opacity)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaTexture *resolved;
    CGContextRef ctx;
    CGColorSpaceRef color_space;
    CGDataProviderRef provider;
    CGImageRef image;
    CGImageRef sub_image;
    CGBitmapInfo info;
    CGRect src_rect;
    CGRect dst_rect;
    CGAffineTransform transform;
    CMPBool apply_transform;
    cmp_u32 bpp;
    int rc;

    if (gfx == NULL || src == NULL || dst == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (opacity < 0.0f || opacity > 1.0f) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)gfx;
    rc = cmp_cocoa_backend_resolve(backend, texture, CMP_COCOA_TYPE_TEXTURE, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "gfx.draw_texture");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->pixels == NULL) {
        return CMP_ERR_STATE;
    }

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (src->width < 0.0f || src->height < 0.0f || dst->width < 0.0f || dst->height < 0.0f) {
        return CMP_ERR_RANGE;
    }

    color_space = NULL;
    rc = cmp_cocoa_texture_format_info(resolved->format, &bpp, &info, &color_space);
    if (rc != CMP_OK) {
        if (color_space != NULL) {
            CGColorSpaceRelease(color_space);
        }
        return rc;
    }

    provider = CGDataProviderCreateWithData(NULL, resolved->pixels, (size_t)resolved->size, NULL);
    if (provider == NULL) {
        CGColorSpaceRelease(color_space);
        return CMP_ERR_UNKNOWN;
    }

    image = CGImageCreate((size_t)resolved->width, (size_t)resolved->height, 8, (size_t)(bpp * 8u),
                          (size_t)resolved->stride, color_space, info, provider, NULL, 0, kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(color_space);
    if (image == NULL) {
        return CMP_ERR_UNKNOWN;
    }

    src_rect = CGRectMake((CGFloat)src->x, (CGFloat)src->y, (CGFloat)src->width, (CGFloat)src->height);
    if (src_rect.size.width <= 0.0 || src_rect.size.height <= 0.0) {
        CGImageRelease(image);
        return CMP_OK;
    }

    sub_image = CGImageCreateWithImageInRect(image, src_rect);
    CGImageRelease(image);
    if (sub_image == NULL) {
        return CMP_ERR_UNKNOWN;
    }

    dst_rect = CGRectMake((CGFloat)dst->x, (CGFloat)dst->y, (CGFloat)dst->width, (CGFloat)dst->height);

    apply_transform = backend->has_transform ? CMP_TRUE : CMP_FALSE;
    CGContextSaveGState(ctx);
    if (apply_transform) {
        rc = cmp_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        if (rc != CMP_OK) {
            CGContextRestoreGState(ctx);
            CGImageRelease(sub_image);
            return rc;
        }
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetAlpha(ctx, (CGFloat)opacity);
    CGContextDrawImage(ctx, dst_rect, sub_image);
    CGContextRestoreGState(ctx);

    CGImageRelease(sub_image);
    return CMP_OK;
}

static const CMPGfxVTable g_cmp_cocoa_gfx_vtable = {
    cmp_cocoa_gfx_begin_frame,
    cmp_cocoa_gfx_end_frame,
    cmp_cocoa_gfx_clear,
    cmp_cocoa_gfx_draw_rect,
    cmp_cocoa_gfx_draw_line,
    cmp_cocoa_gfx_draw_path,
    cmp_cocoa_gfx_push_clip,
    cmp_cocoa_gfx_pop_clip,
    cmp_cocoa_gfx_set_transform,
    cmp_cocoa_gfx_create_texture,
    cmp_cocoa_gfx_update_texture,
    cmp_cocoa_gfx_destroy_texture,
    cmp_cocoa_gfx_draw_texture
};

static int cmp_cocoa_text_create_font(void *text, const char *utf8_family, cmp_i32 size_px, cmp_i32 weight, CMPBool italic, CMPHandle *out_font)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaFont *font;
    CFStringRef family;
    CFMutableDictionaryRef traits;
    CFMutableDictionaryRef attributes;
    CTFontDescriptorRef descriptor;
    CTFontRef ct_font;
    float weight_norm;
    int trait_value;
    CFNumberRef weight_number;
    CFNumberRef trait_number;
    int rc;

    if (text == NULL || out_font == NULL || utf8_family == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (size_px <= 0) {
        return CMP_ERR_RANGE;
    }
    if (weight < 100 || weight > 900) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)text;
    out_font->id = 0u;
    out_font->generation = 0u;

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.create_font");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    family = CFStringCreateWithCString(kCFAllocatorDefault, utf8_family, kCFStringEncodingUTF8);
    if (family == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    traits = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (traits == NULL) {
        CFRelease(family);
        return CMP_ERR_OUT_OF_MEMORY;
    }

    weight_norm = ((float)weight - 400.0f) / 500.0f;
    if (weight_norm < -1.0f) {
        weight_norm = -1.0f;
    }
    if (weight_norm > 1.0f) {
        weight_norm = 1.0f;
    }
    weight_number = CFNumberCreate(kCFAllocatorDefault, kCFNumberFloatType, &weight_norm);
    if (weight_number == NULL) {
        CFRelease(traits);
        CFRelease(family);
        return CMP_ERR_OUT_OF_MEMORY;
    }
    CFDictionarySetValue(traits, kCTFontWeightTrait, weight_number);
    CFRelease(weight_number);

    if (italic) {
        trait_value = (int)kCTFontItalicTrait;
        trait_number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &trait_value);
        if (trait_number == NULL) {
            CFRelease(traits);
            CFRelease(family);
            return CMP_ERR_OUT_OF_MEMORY;
        }
        CFDictionarySetValue(traits, kCTFontSymbolicTrait, trait_number);
        CFRelease(trait_number);
    }

    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attributes == NULL) {
        CFRelease(traits);
        CFRelease(family);
        return CMP_ERR_OUT_OF_MEMORY;
    }

    CFDictionarySetValue(attributes, kCTFontFamilyNameAttribute, family);
    CFDictionarySetValue(attributes, kCTFontTraitsAttribute, traits);
    CFRelease(traits);
    CFRelease(family);

    descriptor = CTFontDescriptorCreateWithAttributes(attributes);
    CFRelease(attributes);
    if (descriptor == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    ct_font = CTFontCreateWithFontDescriptor(descriptor, (CGFloat)size_px, NULL);
    CFRelease(descriptor);
    if (ct_font == NULL) {
        return CMP_ERR_NOT_FOUND;
    }

    rc = backend->allocator.alloc(backend->allocator.ctx, sizeof(CMPCocoaFont), (void **)&font);
    if (rc != CMP_OK) {
        CFRelease(ct_font);
        return rc;
    }

    memset(font, 0, sizeof(*font));
    font->backend = backend;
    font->size_px = size_px;
    font->weight = weight;
    font->italic = italic ? CMP_TRUE : CMP_FALSE;
    font->font = ct_font;

    rc = cmp_object_header_init(&font->header, CMP_COCOA_TYPE_FONT, 0, &g_cmp_cocoa_font_vtable);
    CMP_COCOA_RETURN_IF_ERROR_CLEANUP(rc, CFRelease(ct_font);
                                     backend->allocator.free(backend->allocator.ctx, font));

    rc = backend->handles.vtable->register_object(backend->handles.ctx, &font->header);
    CMP_COCOA_RETURN_IF_ERROR_CLEANUP(rc, CFRelease(ct_font);
                                     backend->allocator.free(backend->allocator.ctx, font));

    *out_font = font->header.handle;
    return CMP_OK;
}

static int cmp_cocoa_text_destroy_font(void *text, CMPHandle font)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaFont *resolved;
    int rc;

    if (text == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)text;
    rc = cmp_cocoa_backend_resolve(backend, font, CMP_COCOA_TYPE_FONT, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.destroy_font");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    return cmp_object_release(&resolved->header);
}

static int cmp_cocoa_text_measure_text(void *text, CMPHandle font, const char *utf8, cmp_usize utf8_len, CMPScalar *out_width, CMPScalar *out_height,
                                     CMPScalar *out_baseline)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaFont *resolved;
    CFStringRef string_ref;
    CFDictionaryRef attributes;
    CFAttributedStringRef attr_string;
    CTLineRef line;
    CFMutableDictionaryRef attrs_mut;
    double width;
    double ascent;
    double descent;
    double leading;
    int rc;

    if (text == NULL || out_width == NULL || out_height == NULL || out_baseline == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0u) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)text;
    rc = cmp_cocoa_backend_resolve(backend, font, CMP_COCOA_TYPE_FONT, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.measure_text");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (resolved->font == NULL) {
        return CMP_ERR_STATE;
    }

    if (utf8_len == 0u) {
        *out_width = 0.0f;
        *out_height = (CMPScalar)(CTFontGetAscent(resolved->font) + CTFontGetDescent(resolved->font));
        *out_baseline = (CMPScalar)CTFontGetAscent(resolved->font);
        return CMP_OK;
    }

    string_ref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)utf8, (CFIndex)utf8_len, kCFStringEncodingUTF8, false);
    if (string_ref == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    attrs_mut = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attrs_mut == NULL) {
        CFRelease(string_ref);
        return CMP_ERR_OUT_OF_MEMORY;
    }
    CFDictionarySetValue(attrs_mut, kCTFontAttributeName, resolved->font);
    attributes = attrs_mut;

    attr_string = CFAttributedStringCreate(kCFAllocatorDefault, string_ref, attributes);
    CFRelease(string_ref);
    CFRelease(attrs_mut);
    if (attr_string == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    line = CTLineCreateWithAttributedString(attr_string);
    CFRelease(attr_string);
    if (line == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    width = CTLineGetTypographicBounds(line, &ascent, &descent, &leading);
    CFRelease(line);

    *out_width = (CMPScalar)width;
    *out_height = (CMPScalar)(ascent + descent);
    *out_baseline = (CMPScalar)ascent;
    return CMP_OK;
}

static int cmp_cocoa_text_draw_text(void *text, CMPHandle font, const char *utf8, cmp_usize utf8_len, CMPScalar x, CMPScalar y, CMPColor color)
{
    struct CMPCocoaBackend *backend;
    CMPCocoaFont *resolved;
    CGContextRef ctx;
    CFStringRef string_ref;
    CFDictionaryRef attributes;
    CFAttributedStringRef attr_string;
    CFMutableDictionaryRef attrs_mut;
    CTLineRef line;
    CGAffineTransform transform;
    CMPBool apply_transform;
    int rc;

    if (text == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (utf8 == NULL && utf8_len != 0u) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)text;
    rc = cmp_cocoa_backend_resolve(backend, font, CMP_COCOA_TYPE_FONT, (void **)&resolved);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "text.draw_text");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    rc = cmp_cocoa_color_validate(color);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (utf8_len == 0u) {
        return CMP_OK;
    }

    if (resolved->font == NULL) {
        return CMP_ERR_STATE;
    }

    rc = cmp_cocoa_get_active_context(backend, &ctx);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    string_ref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)utf8, (CFIndex)utf8_len, kCFStringEncodingUTF8, false);
    if (string_ref == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    attrs_mut = CFDictionaryCreateMutable(kCFAllocatorDefault, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attrs_mut == NULL) {
        CFRelease(string_ref);
        return CMP_ERR_OUT_OF_MEMORY;
    }
    CFDictionarySetValue(attrs_mut, kCTFontAttributeName, resolved->font);
    attributes = attrs_mut;

    attr_string = CFAttributedStringCreate(kCFAllocatorDefault, string_ref, attributes);
    CFRelease(string_ref);
    CFRelease(attrs_mut);
    if (attr_string == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    line = CTLineCreateWithAttributedString(attr_string);
    CFRelease(attr_string);
    if (line == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    apply_transform = backend->has_transform ? CMP_TRUE : CMP_FALSE;
    if (apply_transform) {
        rc = cmp_cocoa_mat3_to_cgaffine(&backend->transform, &transform);
        if (rc != CMP_OK) {
            CFRelease(line);
            return rc;
        }
        CGContextSaveGState(ctx);
        CGContextConcatCTM(ctx, transform);
    }

    CGContextSetRGBFillColor(ctx, (CGFloat)color.r, (CGFloat)color.g, (CGFloat)color.b, (CGFloat)color.a);
    CGContextSetTextMatrix(ctx, CGAffineTransformIdentity);
    CGContextSetTextPosition(ctx, (CGFloat)x, (CGFloat)(y + CTFontGetAscent(resolved->font)));
    CTLineDraw(line, ctx);

    if (apply_transform) {
        CGContextRestoreGState(ctx);
    }

    CFRelease(line);
    return CMP_OK;
}

static const CMPTextVTable g_cmp_cocoa_text_vtable = {
    cmp_cocoa_text_create_font,
    cmp_cocoa_text_destroy_font,
    cmp_cocoa_text_measure_text,
    cmp_cocoa_text_draw_text
};

static int cmp_cocoa_io_read_file(void *io, const char *utf8_path, void *buffer, cmp_usize buffer_size, cmp_usize *out_read)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_read == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (buffer == NULL && buffer_size != 0u) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)io;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_read = 0u;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_io_read_file_alloc(void *io, const char *utf8_path, const CMPAllocator *allocator, void **out_data, cmp_usize *out_size)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || allocator == NULL || out_data == NULL || out_size == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)io;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.read_file_alloc");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_data = NULL;
    *out_size = 0u;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_io_write_file(void *io, const char *utf8_path, const void *data, cmp_usize size, CMPBool overwrite)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(overwrite);

    if (io == NULL || utf8_path == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (data == NULL && size != 0u) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)io;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.write_file");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_io_file_exists(void *io, const char *utf8_path, CMPBool *out_exists)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_exists == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)io;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.file_exists");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_exists = CMP_FALSE;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_io_delete_file(void *io, const char *utf8_path)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)io;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.delete_file");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_io_stat_file(void *io, const char *utf8_path, CMPFileInfo *out_info)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (io == NULL || utf8_path == NULL || out_info == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)io;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "io.stat_file");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    memset(out_info, 0, sizeof(*out_info));
    return CMP_ERR_UNSUPPORTED;
}

static const CMPIOVTable g_cmp_cocoa_io_vtable = {
    cmp_cocoa_io_read_file,
    cmp_cocoa_io_read_file_alloc,
    cmp_cocoa_io_write_file,
    cmp_cocoa_io_file_exists,
    cmp_cocoa_io_delete_file,
    cmp_cocoa_io_stat_file
};

static int cmp_cocoa_sensors_is_available(void *sensors, cmp_u32 type, CMPBool *out_available)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(type);

    if (sensors == NULL || out_available == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)sensors;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.is_available");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_available = CMP_FALSE;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_sensors_start(void *sensors, cmp_u32 type)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(type);

    if (sensors == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)sensors;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.start");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_sensors_stop(void *sensors, cmp_u32 type)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(type);

    if (sensors == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)sensors;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.stop");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_sensors_read(void *sensors, cmp_u32 type, CMPSensorReading *out_reading, CMPBool *out_has_reading)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(type);

    if (sensors == NULL || out_reading == NULL || out_has_reading == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)sensors;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "sensors.read");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    memset(out_reading, 0, sizeof(*out_reading));
    *out_has_reading = CMP_FALSE;
    return CMP_ERR_UNSUPPORTED;
}

static const CMPSensorsVTable g_cmp_cocoa_sensors_vtable = {
    cmp_cocoa_sensors_is_available,
    cmp_cocoa_sensors_start,
    cmp_cocoa_sensors_stop,
    cmp_cocoa_sensors_read
};

static int cmp_cocoa_camera_mul_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value)
{
    cmp_usize max_value;

    if (out_value == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    max_value = (cmp_usize)~(cmp_usize)0;
    if (a != 0u && b > max_value / a) {
        return CMP_ERR_OVERFLOW;
    }

    *out_value = a * b;
    return CMP_OK;
}

static int cmp_cocoa_camera_select_device(const CMPCameraConfig *config, AVCaptureDevice **out_device)
{
    NSArray *devices;
    AVCaptureDevice *device;
    AVCaptureDevicePosition desired;
    NSUInteger count;
    NSUInteger i;

    if (config == NULL || out_device == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    *out_device = nil;
    devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    if (devices == nil) {
        return CMP_ERR_NOT_FOUND;
    }

    count = [devices count];
    if (count == 0u) {
        return CMP_ERR_NOT_FOUND;
    }

    if (config->camera_id != CMP_CAMERA_ID_DEFAULT) {
        if (config->camera_id >= (cmp_u32)count) {
            return CMP_ERR_NOT_FOUND;
        }
        *out_device = [devices objectAtIndex:(NSUInteger)config->camera_id];
        return CMP_OK;
    }

    if (config->facing == CMP_CAMERA_FACING_UNSPECIFIED) {
        *out_device = [devices objectAtIndex:0u];
        return CMP_OK;
    }

    desired = AVCaptureDevicePositionUnspecified;
    switch (config->facing) {
    case CMP_CAMERA_FACING_FRONT:
        desired = AVCaptureDevicePositionFront;
        break;
    case CMP_CAMERA_FACING_BACK:
        desired = AVCaptureDevicePositionBack;
        break;
    case CMP_CAMERA_FACING_EXTERNAL:
    default:
        desired = AVCaptureDevicePositionUnspecified;
        break;
    }

    for (i = 0u; i < count; i += 1u) {
        device = [devices objectAtIndex:i];
        if (device != nil && [device position] == desired) {
            *out_device = device;
            return CMP_OK;
        }
    }

    return CMP_ERR_NOT_FOUND;
}

static int cmp_cocoa_camera_apply_preset(AVCaptureSession *session, cmp_u32 width, cmp_u32 height)
{
    NSString *preset;
    cmp_u32 target_width;
    cmp_u32 target_height;
    CMPBool allow_fallback;

    if (session == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    allow_fallback = (width == 0u || height == 0u) ? CMP_TRUE : CMP_FALSE;
    target_width = (width == 0u) ? CMP_COCOA_CAMERA_DEFAULT_WIDTH : width;
    target_height = (height == 0u) ? CMP_COCOA_CAMERA_DEFAULT_HEIGHT : height;

    preset = nil;
    if (target_width == CMP_COCOA_CAMERA_DEFAULT_WIDTH && target_height == CMP_COCOA_CAMERA_DEFAULT_HEIGHT) {
        if ([session canSetSessionPreset:AVCaptureSessionPreset640x480]) {
            session.sessionPreset = AVCaptureSessionPreset640x480;
            return CMP_OK;
        }
        if (allow_fallback && [session canSetSessionPreset:AVCaptureSessionPresetMedium]) {
            session.sessionPreset = AVCaptureSessionPresetMedium;
            return CMP_OK;
        }
        return CMP_ERR_UNSUPPORTED;
    } else if (target_width == 1280u && target_height == 720u) {
        preset = AVCaptureSessionPreset1280x720;
    } else if (target_width == 1920u && target_height == 1080u) {
        preset = AVCaptureSessionPreset1920x1080;
    } else if (target_width == 3840u && target_height == 2160u) {
        preset = AVCaptureSessionPreset3840x2160;
    } else {
        return CMP_ERR_UNSUPPORTED;
    }

    if ([session canSetSessionPreset:preset]) {
        session.sessionPreset = preset;
        return CMP_OK;
    }

    return CMP_ERR_UNSUPPORTED;
}

static CMPBool cmp_cocoa_camera_output_supports_format(AVCaptureVideoDataOutput *output, OSType pixel_format)
{
    NSArray *formats;
    NSNumber *value;
    NSUInteger count;
    NSUInteger i;

    if (output == nil) {
        return CMP_FALSE;
    }

    formats = [output availableVideoCVPixelFormatTypes];
    if (formats == nil) {
        return CMP_FALSE;
    }

    count = [formats count];
    for (i = 0u; i < count; i += 1u) {
        value = [formats objectAtIndex:i];
        if (value != nil && [value unsignedIntValue] == (unsigned int)pixel_format) {
            return CMP_TRUE;
        }
    }

    return CMP_FALSE;
}

static int cmp_cocoa_camera_pick_pixel_format(AVCaptureVideoDataOutput *output, cmp_u32 requested_format,
    OSType *out_format, CMPBool *out_swizzle)
{
    if (output == nil || out_format == NULL || out_swizzle == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    *out_swizzle = CMP_FALSE;

    if (requested_format == CMP_CAMERA_FORMAT_ANY) {
        if (cmp_cocoa_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            *out_format = kCVPixelFormatType_32BGRA;
            return CMP_OK;
        }
        if (cmp_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return CMP_OK;
        }
        if (cmp_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return CMP_OK;
        }
        return CMP_ERR_UNSUPPORTED;
    }

    if (requested_format == CMP_CAMERA_FORMAT_BGRA8 || requested_format == CMP_CAMERA_FORMAT_RGBA8) {
        if (!cmp_cocoa_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            return CMP_ERR_UNSUPPORTED;
        }
        *out_format = kCVPixelFormatType_32BGRA;
        *out_swizzle = (requested_format == CMP_CAMERA_FORMAT_RGBA8) ? CMP_TRUE : CMP_FALSE;
        return CMP_OK;
    }

    if (requested_format == CMP_CAMERA_FORMAT_NV12) {
        if (cmp_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return CMP_OK;
        }
        if (cmp_cocoa_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return CMP_OK;
        }
        return CMP_ERR_UNSUPPORTED;
    }

    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_camera_update_frame(struct CMPCocoaBackend *backend, CMSampleBufferRef sample)
{
    CVImageBufferRef image;
    CVPixelBufferRef pixel;
    OSType pixel_format;
    unsigned char *base_address;
    unsigned char *y_plane;
    unsigned char *uv_plane;
    size_t bytes_per_row;
    size_t row;
    size_t y_bytes_per_row;
    size_t uv_bytes_per_row;
    cmp_usize width;
    cmp_usize height;
    cmp_usize row_bytes;
    cmp_usize needed;
    cmp_usize y_size;
    cmp_usize uv_size;
    cmp_usize col;
    int rc;
    int frame_rc;
    CMPBool swizzle;

    if (backend == NULL || sample == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    image = CMSampleBufferGetImageBuffer(sample);
    if (image == NULL) {
        return CMP_ERR_NOT_READY;
    }

    pixel = (CVPixelBufferRef)image;
    pixel_format = CVPixelBufferGetPixelFormatType(pixel);

    if (CVPixelBufferLockBaseAddress(pixel, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess) {
        return CMP_ERR_IO;
    }

    base_address = NULL;
    y_plane = NULL;
    uv_plane = NULL;
    bytes_per_row = 0u;
    y_bytes_per_row = 0u;
    uv_bytes_per_row = 0u;
    width = 0u;
    height = 0u;
    row_bytes = 0u;
    needed = 0u;
    y_size = 0u;
    uv_size = 0u;
    swizzle = backend->camera_swizzle;
    frame_rc = CMP_OK;
    if (pixel_format == kCVPixelFormatType_32BGRA) {
        base_address = (unsigned char *)CVPixelBufferGetBaseAddress(pixel);
        bytes_per_row = CVPixelBufferGetBytesPerRow(pixel);
        width = (cmp_usize)CVPixelBufferGetWidth(pixel);
        height = (cmp_usize)CVPixelBufferGetHeight(pixel);

        rc = cmp_cocoa_camera_mul_overflow(width, (cmp_usize)4u, &row_bytes);
        if (rc == CMP_OK) {
            rc = cmp_cocoa_camera_mul_overflow(row_bytes, height, &needed);
        }

        if (rc != CMP_OK || base_address == NULL) {
            frame_rc = (rc == CMP_OK) ? CMP_ERR_IO : rc;
        } else {
            @synchronized (backend) {
                if (backend->allocator.alloc == NULL || backend->allocator.realloc == NULL
                    || backend->allocator.free == NULL) {
                    frame_rc = CMP_ERR_INVALID_ARGUMENT;
                } else if (backend->camera_frame_capacity < needed) {
                    unsigned char *new_frame;
                    int alloc_rc = backend->allocator.realloc(backend->allocator.ctx, backend->camera_frame, needed,
                        (void **)&new_frame);
                    if (alloc_rc != CMP_OK) {
                        frame_rc = alloc_rc;
                    } else {
                        backend->camera_frame = new_frame;
                        backend->camera_frame_capacity = needed;
                    }
                }

                if (frame_rc == CMP_OK) {
                    if (swizzle == CMP_TRUE) {
                        unsigned char *src;
                        unsigned char *dst;
                        cmp_usize idx;

                        for (row = 0u; row < height; row += 1u) {
                            src = base_address + row * bytes_per_row;
                            dst = backend->camera_frame + row * row_bytes;
                            for (col = 0u; col < width; col += 1u) {
                                idx = col * 4u;
                                dst[idx] = src[idx + 2u];
                                dst[idx + 1u] = src[idx + 1u];
                                dst[idx + 2u] = src[idx];
                                dst[idx + 3u] = src[idx + 3u];
                            }
                        }
                        backend->camera_format = CMP_CAMERA_FORMAT_RGBA8;
                    } else {
                        if (bytes_per_row == (size_t)row_bytes) {
                            memcpy(backend->camera_frame, base_address, (size_t)needed);
                        } else {
                            for (row = 0u; row < height; row += 1u) {
                                memcpy(backend->camera_frame + row * row_bytes, base_address + row * bytes_per_row,
                                    (size_t)row_bytes);
                            }
                        }
                        backend->camera_format = CMP_CAMERA_FORMAT_BGRA8;
                    }

                    backend->camera_width = (cmp_u32)width;
                    backend->camera_height = (cmp_u32)height;
                    backend->camera_frame_size = needed;
                    backend->camera_has_frame = CMP_TRUE;
                }
                backend->camera_error = frame_rc;
            }
        }
    } else if (pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
        || pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
        if (!CVPixelBufferIsPlanar(pixel) || CVPixelBufferGetPlaneCount(pixel) < 2u) {
            frame_rc = CMP_ERR_UNSUPPORTED;
        } else {
            y_plane = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(pixel, 0);
            uv_plane = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(pixel, 1);
            y_bytes_per_row = CVPixelBufferGetBytesPerRowOfPlane(pixel, 0);
            uv_bytes_per_row = CVPixelBufferGetBytesPerRowOfPlane(pixel, 1);
            width = (cmp_usize)CVPixelBufferGetWidth(pixel);
            height = (cmp_usize)CVPixelBufferGetHeight(pixel);

            rc = cmp_cocoa_camera_mul_overflow(width, height, &y_size);
            if (rc == CMP_OK) {
                uv_size = y_size / 2u;
                needed = y_size + uv_size;
            }

            if (rc != CMP_OK || y_plane == NULL || uv_plane == NULL) {
                frame_rc = (rc == CMP_OK) ? CMP_ERR_IO : rc;
            } else {
                @synchronized (backend) {
                    if (backend->allocator.alloc == NULL || backend->allocator.realloc == NULL
                        || backend->allocator.free == NULL) {
                        frame_rc = CMP_ERR_INVALID_ARGUMENT;
                    } else if (backend->camera_frame_capacity < needed) {
                        unsigned char *new_frame;
                        int alloc_rc = backend->allocator.realloc(backend->allocator.ctx, backend->camera_frame, needed,
                            (void **)&new_frame);
                        if (alloc_rc != CMP_OK) {
                            frame_rc = alloc_rc;
                        } else {
                            backend->camera_frame = new_frame;
                            backend->camera_frame_capacity = needed;
                        }
                    }

                    if (frame_rc == CMP_OK) {
                        for (row = 0u; row < height; row += 1u) {
                            memcpy(backend->camera_frame + row * width,
                                y_plane + row * (cmp_usize)y_bytes_per_row, (size_t)width);
                        }
                        for (row = 0u; row < height / 2u; row += 1u) {
                            memcpy(backend->camera_frame + y_size + row * width,
                                uv_plane + row * (cmp_usize)uv_bytes_per_row, (size_t)width);
                        }

                        backend->camera_format = CMP_CAMERA_FORMAT_NV12;
                        backend->camera_width = (cmp_u32)width;
                        backend->camera_height = (cmp_u32)height;
                        backend->camera_frame_size = needed;
                        backend->camera_has_frame = CMP_TRUE;
                    }
                    backend->camera_error = frame_rc;
                }
            }
        }
    } else {
        frame_rc = CMP_ERR_UNSUPPORTED;
        @synchronized (backend) {
            backend->camera_error = frame_rc;
        }
    }

    if (frame_rc != CMP_OK) {
        @synchronized (backend) {
            backend->camera_error = frame_rc;
        }
    }

    CVPixelBufferUnlockBaseAddress(pixel, kCVPixelBufferLock_ReadOnly);
    return frame_rc;
}

static int cmp_cocoa_camera_open_with_config(void *camera, const CMPCameraConfig *config);

static int cmp_cocoa_camera_open(void *camera, cmp_u32 camera_id)
{
    CMPCameraConfig config;

    config.camera_id = camera_id;
    config.facing = CMP_CAMERA_FACING_UNSPECIFIED;
    config.width = 0u;
    config.height = 0u;
    config.format = CMP_CAMERA_FORMAT_ANY;

    return cmp_cocoa_camera_open_with_config(camera, &config);
}

static int cmp_cocoa_camera_open_with_config(void *camera, const CMPCameraConfig *config)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (camera == NULL || config == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)camera;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.open");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened == CMP_TRUE) {
        return CMP_ERR_STATE;
    }

    @autoreleasepool {
        AVCaptureDevice *device;
        AVCaptureSession *session;
        AVCaptureDeviceInput *input;
        AVCaptureVideoDataOutput *output;
        NSDictionary *settings;
        CMPCocoaCameraDelegate *delegate;
        dispatch_queue_t queue;
        NSError *error;
        OSType pixel_format;
        CMPBool swizzle;

        rc = cmp_cocoa_camera_select_device(config, &device);
        if (rc != CMP_OK) {
            return rc;
        }

        error = nil;
        input = [[AVCaptureDeviceInput alloc] initWithDevice:device error:&error];
        if (input == nil) {
            if (error != nil && [[error domain] isEqualToString:AVFoundationErrorDomain]) {
                if ([error code] == AVErrorApplicationIsNotAuthorizedToUseDevice) {
                    return CMP_ERR_PERMISSION;
                }
                if ([error code] == AVErrorDeviceNotConnected) {
                    return CMP_ERR_NOT_FOUND;
                }
                if ([error code] == AVErrorDeviceInUseByAnotherApplication) {
                    return CMP_ERR_BUSY;
                }
            }
            return CMP_ERR_UNKNOWN;
        }

        session = [[AVCaptureSession alloc] init];
        if (session == nil) {
#if !defined(CMP_COCOA_ARC)
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        rc = cmp_cocoa_camera_apply_preset(session, config->width, config->height);
        if (rc != CMP_OK) {
#if !defined(CMP_COCOA_ARC)
            [session release];
            [input release];
#endif
            return rc;
        }

        output = [[AVCaptureVideoDataOutput alloc] init];
        if (output == nil) {
#if !defined(CMP_COCOA_ARC)
            [session release];
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        rc = cmp_cocoa_camera_pick_pixel_format(output, config->format, &pixel_format, &swizzle);
        if (rc != CMP_OK) {
#if !defined(CMP_COCOA_ARC)
            [output release];
            [session release];
            [input release];
#endif
            return rc;
        }

        settings = [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedInt:(unsigned int)pixel_format]
                                               forKey:(id)kCVPixelBufferPixelFormatTypeKey];
        output.videoSettings = settings;
        output.alwaysDiscardsLateVideoFrames = YES;

        delegate = [[CMPCocoaCameraDelegate alloc] initWithBackend:backend];
        if (delegate == nil) {
#if !defined(CMP_COCOA_ARC)
            [output release];
            [session release];
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        queue = dispatch_queue_create("m3.cocoa.camera", DISPATCH_QUEUE_SERIAL);
        if (queue == NULL) {
#if !defined(CMP_COCOA_ARC)
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        if (![session canAddInput:input] || ![session canAddOutput:output]) {
#if !defined(CMP_COCOA_ARC)
            dispatch_release(queue);
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return CMP_ERR_UNSUPPORTED;
        }

        [session addInput:input];
        [session addOutput:output];
        [output setSampleBufferDelegate:delegate queue:queue];

        backend->camera_session = session;
        backend->camera_input = input;
        backend->camera_output = output;
        backend->camera_delegate = delegate;
        backend->camera_queue = queue;
        backend->camera_opened = CMP_TRUE;
        backend->camera_streaming = CMP_FALSE;
        backend->camera_has_frame = CMP_FALSE;
        backend->camera_error = CMP_OK;
        backend->camera_requested_format = config->format;
        backend->camera_swizzle = swizzle;
    }

    return CMP_OK;
}

static int cmp_cocoa_camera_close(void *camera)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)camera;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.close");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != CMP_TRUE) {
        return CMP_ERR_STATE;
    }

    if (backend->camera_streaming == CMP_TRUE && backend->camera_session != nil) {
        [backend->camera_session stopRunning];
        backend->camera_streaming = CMP_FALSE;
    }

    if (backend->camera_output != nil) {
        [backend->camera_output setSampleBufferDelegate:nil queue:NULL];
    }

    if (backend->camera_frame != NULL) {
        if (backend->allocator.free == NULL) {
            return CMP_ERR_INVALID_ARGUMENT;
        }
        rc = backend->allocator.free(backend->allocator.ctx, backend->camera_frame);
        if (rc != CMP_OK) {
            return rc;
        }
        backend->camera_frame = NULL;
        backend->camera_frame_capacity = 0u;
        backend->camera_frame_size = 0u;
    }

    if (backend->camera_delegate != nil) {
#if !defined(CMP_COCOA_ARC)
        [backend->camera_delegate release];
#endif
        backend->camera_delegate = nil;
    }

    if (backend->camera_output != nil) {
#if !defined(CMP_COCOA_ARC)
        [backend->camera_output release];
#endif
        backend->camera_output = nil;
    }

    if (backend->camera_input != nil) {
#if !defined(CMP_COCOA_ARC)
        [backend->camera_input release];
#endif
        backend->camera_input = nil;
    }

    if (backend->camera_session != nil) {
#if !defined(CMP_COCOA_ARC)
        [backend->camera_session release];
#endif
        backend->camera_session = nil;
    }

    if (backend->camera_queue != NULL) {
#if !defined(CMP_COCOA_ARC)
        dispatch_release(backend->camera_queue);
#endif
        backend->camera_queue = NULL;
    }

    backend->camera_opened = CMP_FALSE;
    backend->camera_streaming = CMP_FALSE;
    backend->camera_has_frame = CMP_FALSE;
    backend->camera_format = 0u;
    backend->camera_width = 0u;
    backend->camera_height = 0u;
    backend->camera_requested_format = 0u;
    backend->camera_swizzle = CMP_FALSE;
    backend->camera_error = CMP_OK;
    return CMP_OK;
}

static int cmp_cocoa_camera_start(void *camera)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)camera;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.start");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    if (backend->camera_streaming == CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    if (backend->camera_session == nil) {
        return CMP_ERR_STATE;
    }

    [backend->camera_session startRunning];
    if (![backend->camera_session isRunning]) {
        return CMP_ERR_UNKNOWN;
    }

    backend->camera_streaming = CMP_TRUE;
    return CMP_OK;
}

static int cmp_cocoa_camera_stop(void *camera)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)camera;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.stop");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    if (backend->camera_streaming != CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    if (backend->camera_session == nil) {
        return CMP_ERR_STATE;
    }

    [backend->camera_session stopRunning];
    backend->camera_streaming = CMP_FALSE;
    return CMP_OK;
}

static int cmp_cocoa_camera_read_frame(void *camera, CMPCameraFrame *out_frame, CMPBool *out_has_frame)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)camera;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "camera.read_frame");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (backend->camera_opened != CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    if (backend->camera_streaming != CMP_TRUE) {
        return CMP_ERR_STATE;
    }

    @synchronized (backend) {
        if (backend->camera_error != CMP_OK) {
            rc = backend->camera_error;
            backend->camera_error = CMP_OK;
            return rc;
        }

        if (backend->camera_has_frame != CMP_TRUE || backend->camera_frame == NULL) {
            memset(out_frame, 0, sizeof(*out_frame));
            *out_has_frame = CMP_FALSE;
            return CMP_OK;
        }

        out_frame->format = backend->camera_format;
        out_frame->width = backend->camera_width;
        out_frame->height = backend->camera_height;
        out_frame->data = backend->camera_frame;
        out_frame->size = backend->camera_frame_size;
        *out_has_frame = CMP_TRUE;
    }

    return CMP_OK;
}

static const CMPCameraVTable g_cmp_cocoa_camera_vtable = {
    cmp_cocoa_camera_open,
    cmp_cocoa_camera_open_with_config,
    cmp_cocoa_camera_close,
    cmp_cocoa_camera_start,
    cmp_cocoa_camera_stop,
    cmp_cocoa_camera_read_frame
};

#if !defined(CMP_APPLE_USE_CFNETWORK_C)
static int cmp_cocoa_network_error_from_ns_error(NSError *error)
{
    NSString *domain;
    NSInteger code;

    if (error == nil) {
        return CMP_OK;
    }

    domain = [error domain];
    code = [error code];
    if ([domain isEqualToString:NSURLErrorDomain]) {
        switch (code) {
        case NSURLErrorTimedOut:
            return CMP_ERR_TIMEOUT;
        case NSURLErrorBadURL:
            return CMP_ERR_INVALID_ARGUMENT;
        case NSURLErrorUnsupportedURL:
            return CMP_ERR_UNSUPPORTED;
        case NSURLErrorCannotFindHost:
        case NSURLErrorDNSLookupFailed:
            return CMP_ERR_NOT_FOUND;
        case NSURLErrorUserAuthenticationRequired:
        case NSURLErrorUserCancelledAuthentication:
        case NSURLErrorSecureConnectionFailed:
            return CMP_ERR_PERMISSION;
        case NSURLErrorCannotConnectToHost:
        case NSURLErrorNetworkConnectionLost:
        case NSURLErrorNotConnectedToInternet:
            return CMP_ERR_IO;
        default:
            return CMP_ERR_IO;
        }
    }

    return CMP_ERR_IO;
}

#endif

#if !defined(CMP_APPLE_USE_CFNETWORK_C)
static int cmp_cocoa_network_apply_headers(NSMutableURLRequest *request, const char *headers)
{
    const char *cursor;

    if (request == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (headers == NULL || headers[0] == '\0') {
        return CMP_OK;
    }

    cursor = headers;
    while (*cursor != '\0') {
        const char *line_start;
        const char *line_end;
        const char *colon;
        const char *key_start;
        const char *key_end;
        const char *value_start;
        const char *value_end;
        NSString *key;
        NSString *value;

        line_start = cursor;
        line_end = line_start;
        while (*line_end != '\0' && *line_end != '\r' && *line_end != '\n') {
            line_end++;
        }

        if (line_end != line_start) {
            colon = line_start;
            while (colon < line_end && *colon != ':') {
                colon++;
            }
            if (colon == line_end) {
                return CMP_ERR_INVALID_ARGUMENT;
            }

            key_start = line_start;
            key_end = colon;
            while (key_start < key_end && (*key_start == ' ' || *key_start == '\t')) {
                key_start++;
            }
            while (key_end > key_start && (key_end[-1] == ' ' || key_end[-1] == '\t')) {
                key_end--;
            }

            value_start = colon + 1;
            value_end = line_end;
            while (value_start < value_end && (*value_start == ' ' || *value_start == '\t')) {
                value_start++;
            }
            while (value_end > value_start && (value_end[-1] == ' ' || value_end[-1] == '\t')) {
                value_end--;
            }

            if (key_start == key_end) {
                return CMP_ERR_INVALID_ARGUMENT;
            }

            key = [[NSString alloc] initWithBytes:key_start
                                           length:(NSUInteger)(key_end - key_start)
                                         encoding:NSUTF8StringEncoding];
            if (key == nil) {
                return CMP_ERR_INVALID_ARGUMENT;
            }
            value = [[NSString alloc] initWithBytes:value_start
                                             length:(NSUInteger)(value_end - value_start)
                                           encoding:NSUTF8StringEncoding];
            if (value == nil) {
#if !CMP_COCOA_ARC
                [key release];
#endif
                return CMP_ERR_INVALID_ARGUMENT;
            }

            [request setValue:value forHTTPHeaderField:key];

#if !CMP_COCOA_ARC
            [value release];
            [key release];
#endif
        }

        if (*line_end == '\r' && line_end[1] == '\n') {
            cursor = line_end + 2;
        } else if (*line_end == '\r' || *line_end == '\n') {
            cursor = line_end + 1;
        } else {
            cursor = line_end;
        }
    }

    return CMP_OK;
}

#endif

#if defined(CMP_APPLE_USE_CFNETWORK_C)
static int cmp_cocoa_network_add_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value)
{
    if (out_value == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (b > ((cmp_usize)~(cmp_usize)0) - a) {
        return CMP_ERR_OVERFLOW;
    }
    *out_value = a + b;
    return CMP_OK;
}

static int cmp_cocoa_network_error_from_cf_stream(CFStreamError error)
{
    if (error.domain == 0) {
        return CMP_ERR_IO;
    }
    if (error.domain == kCFStreamErrorDomainPOSIX) {
        if (error.error == ETIMEDOUT) {
            return CMP_ERR_TIMEOUT;
        }
        return CMP_ERR_IO;
    }
    return CMP_ERR_IO;
}

static int cmp_cocoa_network_apply_cf_headers(CFHTTPMessageRef message, const char *headers)
{
    const char *cursor;

    if (message == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (headers == NULL || headers[0] == '\0') {
        return CMP_OK;
    }

    cursor = headers;
    while (*cursor != '\0') {
        const char *line_start;
        const char *line_end;
        const char *colon;
        const char *key_start;
        const char *key_end;
        const char *value_start;
        const char *value_end;
        CFStringRef key;
        CFStringRef value;

        line_start = cursor;
        line_end = line_start;
        while (*line_end != '\0' && *line_end != '\r' && *line_end != '\n') {
            line_end++;
        }

        if (line_end != line_start) {
            colon = line_start;
            while (colon < line_end && *colon != ':') {
                colon++;
            }
            if (colon == line_end) {
                return CMP_ERR_INVALID_ARGUMENT;
            }

            key_start = line_start;
            key_end = colon;
            while (key_start < key_end && (*key_start == ' ' || *key_start == '\t')) {
                key_start++;
            }
            while (key_end > key_start && (key_end[-1] == ' ' || key_end[-1] == '\t')) {
                key_end--;
            }

            value_start = colon + 1;
            value_end = line_end;
            while (value_start < value_end && (*value_start == ' ' || *value_start == '\t')) {
                value_start++;
            }
            while (value_end > value_start && (value_end[-1] == ' ' || value_end[-1] == '\t')) {
                value_end--;
            }

            if (key_start == key_end) {
                return CMP_ERR_INVALID_ARGUMENT;
            }

            if ((cmp_usize)(key_end - key_start) > (cmp_usize)LONG_MAX
                    || (cmp_usize)(value_end - value_start) > (cmp_usize)LONG_MAX) {
                return CMP_ERR_RANGE;
            }

            key = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)key_start,
                (CFIndex)(key_end - key_start), kCFStringEncodingUTF8, false);
            if (key == NULL) {
                return CMP_ERR_INVALID_ARGUMENT;
            }
            value = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)value_start,
                (CFIndex)(value_end - value_start), kCFStringEncodingUTF8, false);
            if (value == NULL) {
                CFRelease(key);
                return CMP_ERR_INVALID_ARGUMENT;
            }

            CFHTTPMessageSetHeaderFieldValue(message, key, value);
            CFRelease(value);
            CFRelease(key);
        }

        if (*line_end == '\r' && line_end[1] == '\n') {
            cursor = line_end + 2;
        } else if (*line_end == '\r' || *line_end == '\n') {
            cursor = line_end + 1;
        } else {
            cursor = line_end;
        }
    }

    return CMP_OK;
}

static int cmp_cocoa_network_request_cfnetwork(void *net, const CMPNetworkRequest *request, const CMPAllocator *allocator,
    CMPNetworkResponse *out_response)
{
    struct CMPCocoaBackend *backend;
    CFStringRef url_string;
    CFURLRef url;
    CFStringRef method;
    CFHTTPMessageRef message;
    CFDataRef body_data;
    CFReadStreamRef stream;
    CFHTTPMessageRef response_message;
    CFStreamError stream_error;
    CFIndex bytes_read;
    UInt8 buffer[4096];
    void *body;
    void *new_body;
    cmp_usize total_size;
    cmp_usize capacity;
    cmp_usize alloc_size;
    cmp_usize url_len;
    cmp_usize method_len;
    int rc;

    if (net == NULL || request == NULL || allocator == NULL || out_response == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (request->method == NULL || request->url == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    url_len = (cmp_usize)strlen(request->url);
    if (url_len > (cmp_usize)LONG_MAX) {
        return CMP_ERR_RANGE;
    }
    method_len = (cmp_usize)strlen(request->method);
    if (method_len > (cmp_usize)LONG_MAX) {
        return CMP_ERR_RANGE;
    }
    if (request->body_size > (cmp_usize)LONG_MAX) {
        return CMP_ERR_RANGE;
    }

    backend = (struct CMPCocoaBackend *)net;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.request");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    memset(out_response, 0, sizeof(*out_response));

    url_string = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)request->url, (CFIndex)url_len,
        kCFStringEncodingUTF8, false);
    if (url_string == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    url = CFURLCreateWithString(kCFAllocatorDefault, url_string, NULL);
    CFRelease(url_string);
    if (url == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    method = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)request->method, (CFIndex)method_len,
        kCFStringEncodingUTF8, false);
    if (method == NULL) {
        CFRelease(url);
        return CMP_ERR_INVALID_ARGUMENT;
    }

    message = CFHTTPMessageCreateRequest(kCFAllocatorDefault, method, url, kCFHTTPVersion1_1);
    CFRelease(method);
    CFRelease(url);
    if (message == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    rc = cmp_cocoa_network_apply_cf_headers(message, request->headers);
    if (rc != CMP_OK) {
        CFRelease(message);
        return rc;
    }

    if (request->body_size > 0) {
        body_data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)request->body, (CFIndex)request->body_size);
        if (body_data == NULL) {
            CFRelease(message);
            return CMP_ERR_OUT_OF_MEMORY;
        }
        CFHTTPMessageSetBody(message, body_data);
        CFRelease(body_data);
    }

    stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, message);
    CFRelease(message);
    if (stream == NULL) {
        return CMP_ERR_OUT_OF_MEMORY;
    }

    if (request->timeout_ms > 0u) {
        double seconds = ((double)request->timeout_ms) / 1000.0;
        CFNumberRef timeout = CFNumberCreate(kCFAllocatorDefault, kCFNumberDoubleType, &seconds);
        if (timeout != NULL) {
            CFReadStreamSetProperty(stream, kCFStreamPropertyReadTimeout, timeout);
            CFReadStreamSetProperty(stream, kCFStreamPropertyWriteTimeout, timeout);
            CFRelease(timeout);
        }
    }

    if (!CFReadStreamOpen(stream)) {
        stream_error = CFReadStreamGetError(stream);
        CFReadStreamClose(stream);
        CFRelease(stream);
        return cmp_cocoa_network_error_from_cf_stream(stream_error);
    }

    response_message = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
    if (response_message != NULL) {
        long status_code = CFHTTPMessageGetResponseStatusCode(response_message);
        if (status_code > 0) {
            out_response->status_code = (cmp_u32)status_code;
        }
        CFRelease(response_message);
    }

    body = NULL;
    new_body = NULL;
    total_size = 0;
    capacity = 0;

    for (;;) {
        bytes_read = CFReadStreamRead(stream, buffer, (CFIndex)sizeof(buffer));
        if (bytes_read < 0) {
            stream_error = CFReadStreamGetError(stream);
            rc = cmp_cocoa_network_error_from_cf_stream(stream_error);
            goto cleanup;
        }
        if (bytes_read == 0) {
            rc = CMP_OK;
            break;
        }

        rc = cmp_cocoa_network_add_overflow(total_size, (cmp_usize)bytes_read, &alloc_size);
        if (rc != CMP_OK) {
            goto cleanup;
        }

        if (alloc_size > capacity) {
            new_body = NULL;
            if (body == NULL) {
                rc = allocator->alloc(allocator->ctx, alloc_size, &new_body);
            } else {
                rc = allocator->realloc(allocator->ctx, body, alloc_size, &new_body);
            }
            if (rc != CMP_OK) {
                goto cleanup;
            }
            body = new_body;
            capacity = alloc_size;
        }

        memcpy((cmp_u8 *)body + total_size, buffer, (size_t)bytes_read);
        total_size += (cmp_usize)bytes_read;
    }

cleanup:
    CFReadStreamClose(stream);
    CFRelease(stream);

    if (rc != CMP_OK) {
        if (body != NULL) {
            allocator->free(allocator->ctx, body);
        }
        memset(out_response, 0, sizeof(*out_response));
        return rc;
    }

    out_response->body = body;
    out_response->body_size = total_size;
    return CMP_OK;
}

#endif

#if !defined(CMP_APPLE_USE_CFNETWORK_C)
static int cmp_cocoa_network_request_foundation(void *net, const CMPNetworkRequest *request, const CMPAllocator *allocator,
    CMPNetworkResponse *out_response)
{
    struct CMPCocoaBackend *backend;
    NSString *url_string;
    NSURL *url;
    NSMutableURLRequest *url_request;
    NSString *method;
    NSData *body_data;
    NSURLResponse *response;
    NSHTTPURLResponse *http_response;
    NSError *error;
    NSData *data;
    void *body;
    cmp_usize body_size;
    int rc;

    if (net == NULL || allocator == NULL || out_response == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)net;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.request");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (request == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (request->method == NULL || request->url == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    memset(out_response, 0, sizeof(*out_response));

    url_string = [[NSString alloc] initWithBytes:request->url
                                          length:(NSUInteger)strlen(request->url)
                                        encoding:NSUTF8StringEncoding];
    if (url_string == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    url = [NSURL URLWithString:url_string];
    if (url == nil) {
#if !CMP_COCOA_ARC
        [url_string release];
#endif
        return CMP_ERR_INVALID_ARGUMENT;
    }

    url_request = [[NSMutableURLRequest alloc] initWithURL:url];
    if (url_request == nil) {
#if !CMP_COCOA_ARC
        [url_string release];
#endif
        return CMP_ERR_OUT_OF_MEMORY;
    }

    method = [[NSString alloc] initWithBytes:request->method
                                      length:(NSUInteger)strlen(request->method)
                                    encoding:NSUTF8StringEncoding];
    if (method == nil) {
#if !CMP_COCOA_ARC
        [url_request release];
        [url_string release];
#endif
        return CMP_ERR_INVALID_ARGUMENT;
    }
    [url_request setHTTPMethod:method];

    if (request->timeout_ms > 0u) {
        [url_request setTimeoutInterval:((NSTimeInterval)request->timeout_ms) / 1000.0];
    }

    rc = cmp_cocoa_network_apply_headers(url_request, request->headers);
    if (rc != CMP_OK) {
#if !CMP_COCOA_ARC
        [method release];
        [url_request release];
        [url_string release];
#endif
        return rc;
    }

    body_data = nil;
    if (request->body_size > 0) {
        body_data = [[NSData alloc] initWithBytes:request->body length:(NSUInteger)request->body_size];
        if (body_data == nil) {
#if !CMP_COCOA_ARC
            [method release];
            [url_request release];
            [url_string release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }
        [url_request setHTTPBody:body_data];
    }

    response = nil;
    error = nil;
    data = [NSURLConnection sendSynchronousRequest:url_request returningResponse:&response error:&error];

#if !CMP_COCOA_ARC
    if (body_data != nil) {
        [body_data release];
    }
    [method release];
    [url_request release];
    [url_string release];
#endif

    if (error != nil) {
        return cmp_cocoa_network_error_from_ns_error(error);
    }

    out_response->status_code = 0u;
    if (response != nil && [response isKindOfClass:[NSHTTPURLResponse class]]) {
        http_response = (NSHTTPURLResponse *)response;
        if ([http_response statusCode] > 0) {
            out_response->status_code = (cmp_u32)[http_response statusCode];
        }
    }

    body = NULL;
    body_size = 0u;
    if (data != nil && [data length] > 0) {
        body_size = (cmp_usize)[data length];
        rc = allocator->alloc(allocator->ctx, body_size, &body);
        if (rc != CMP_OK) {
            out_response->status_code = 0u;
            out_response->body = NULL;
            out_response->body_size = 0u;
            return rc;
        }
        memcpy(body, [data bytes], (size_t)body_size);
    }

    out_response->body = body;
    out_response->body_size = body_size;
    return CMP_OK;
}

#endif

static int cmp_cocoa_network_request(void *net, const CMPNetworkRequest *request, const CMPAllocator *allocator,
    CMPNetworkResponse *out_response)
{
#if defined(CMP_APPLE_USE_CFNETWORK_C)
    return cmp_cocoa_network_request_cfnetwork(net, request, allocator, out_response);
#else
    return cmp_cocoa_network_request_foundation(net, request, allocator, out_response);
#endif
}

static int cmp_cocoa_network_free_response(void *net, const CMPAllocator *allocator, CMPNetworkResponse *response)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (net == NULL || allocator == NULL || response == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)net;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "network.free_response");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (response->body_size > 0 && response->body == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (response->body != NULL) {
        rc = allocator->free(allocator->ctx, (void *)response->body);
        if (rc != CMP_OK) {
            return rc;
        }
    }

    response->body = NULL;
    response->body_size = 0;
    response->status_code = 0;
    return CMP_OK;
}

static const CMPNetworkVTable g_cmp_cocoa_network_vtable = {
    cmp_cocoa_network_request,
    cmp_cocoa_network_free_response
};

static int cmp_cocoa_tasks_thread_create(void *tasks, CMPThreadFn entry, void *user, CMPHandle *out_thread)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(entry);
    CMP_UNUSED(user);

    if (tasks == NULL || out_thread == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_create");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_tasks_thread_join(void *tasks, CMPHandle thread)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(thread);

    if (tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.thread_join");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_tasks_mutex_create(void *tasks, CMPHandle *out_mutex)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (tasks == NULL || out_mutex == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_create");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_tasks_mutex_destroy(void *tasks, CMPHandle mutex)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(mutex);

    if (tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_destroy");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_tasks_mutex_lock(void *tasks, CMPHandle mutex)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(mutex);

    if (tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_lock");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_tasks_mutex_unlock(void *tasks, CMPHandle mutex)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(mutex);

    if (tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.mutex_unlock");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_tasks_sleep_ms(void *tasks, cmp_u32 ms)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(ms);

    if (tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.sleep_ms");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return CMP_ERR_UNSUPPORTED;
    }
    return CMP_OK;
}

static int cmp_cocoa_tasks_post(void *tasks, CMPTaskFn fn, void *user)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (tasks == NULL || fn == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return CMP_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static int cmp_cocoa_tasks_post_delayed(void *tasks, CMPTaskFn fn, void *user, cmp_u32 delay_ms)
{
    struct CMPCocoaBackend *backend;
    int rc;

    CMP_UNUSED(delay_ms);

    if (tasks == NULL || fn == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)tasks;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_DEBUG, "tasks.post_delayed");
    CMP_COCOA_RETURN_IF_ERROR(rc);
    if (!backend->inline_tasks) {
        return CMP_ERR_UNSUPPORTED;
    }
    return fn(user);
}

static const CMPTasksVTable g_cmp_cocoa_tasks_vtable = {
    cmp_cocoa_tasks_thread_create,
    cmp_cocoa_tasks_thread_join,
    cmp_cocoa_tasks_mutex_create,
    cmp_cocoa_tasks_mutex_destroy,
    cmp_cocoa_tasks_mutex_lock,
    cmp_cocoa_tasks_mutex_unlock,
    cmp_cocoa_tasks_sleep_ms,
    cmp_cocoa_tasks_post,
    cmp_cocoa_tasks_post_delayed
};

static int cmp_cocoa_env_get_io(void *env, CMPIO *out_io)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_io == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_io");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_io = backend->io;
    return CMP_OK;
}

static int cmp_cocoa_env_get_sensors(void *env, CMPSensors *out_sensors)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_sensors == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_sensors");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_sensors = backend->sensors;
    return CMP_OK;
}

static int cmp_cocoa_env_get_camera(void *env, CMPCamera *out_camera)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_camera");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_camera = backend->camera;
    return CMP_OK;
}

static int cmp_cocoa_env_get_image(void *env, CMPImage *out_image)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_image == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_image");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    out_image->ctx = NULL;
    out_image->vtable = NULL;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_env_get_video(void *env, CMPVideo *out_video)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_video == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_video");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    out_video->ctx = NULL;
    out_video->vtable = NULL;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_env_get_audio(void *env, CMPAudio *out_audio)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_audio == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_audio");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    out_audio->ctx = NULL;
    out_audio->vtable = NULL;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_cocoa_env_get_network(void *env, CMPNetwork *out_network)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_network == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_network");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_network = backend->network;
    return CMP_OK;
}

static int cmp_cocoa_env_get_tasks(void *env, CMPTasks *out_tasks)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_tasks");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    *out_tasks = backend->tasks;
    return CMP_OK;
}

static int cmp_cocoa_env_get_time_ms(void *env, cmp_u32 *out_time_ms)
{
    struct CMPCocoaBackend *backend;
    int rc;

    if (env == NULL || out_time_ms == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (struct CMPCocoaBackend *)env;
    rc = cmp_cocoa_backend_log(backend, CMP_LOG_LEVEL_INFO, "env.get_time_ms");
    CMP_COCOA_RETURN_IF_ERROR(rc);

    return cmp_cocoa_time_now_ms(backend, out_time_ms);
}

static const CMPEnvVTable g_cmp_cocoa_env_vtable = {
    cmp_cocoa_env_get_io,
    cmp_cocoa_env_get_sensors,
    cmp_cocoa_env_get_camera,
    cmp_cocoa_env_get_image,
    cmp_cocoa_env_get_video,
    cmp_cocoa_env_get_audio,
    cmp_cocoa_env_get_network,
    cmp_cocoa_env_get_tasks,
    cmp_cocoa_env_get_time_ms
};

int CMP_CALL cmp_cocoa_backend_create(const CMPCocoaBackendConfig *config, CMPCocoaBackend **out_backend)
{
    CMPCocoaBackendConfig local_config;
    CMPAllocator allocator;
    struct CMPCocoaBackend *backend;
    int rc;

    if (out_backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = cmp_cocoa_backend_config_init(&local_config);
        CMP_COCOA_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = cmp_cocoa_backend_validate_config(config);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    if (config->allocator == NULL) {
        rc = cmp_get_default_allocator(&allocator);
        CMP_COCOA_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(CMPCocoaBackend), (void **)&backend);
    CMP_COCOA_RETURN_IF_ERROR(rc);

    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;
    backend->log_enabled = config->enable_logging ? CMP_TRUE : CMP_FALSE;
    backend->inline_tasks = config->inline_tasks ? CMP_TRUE : CMP_FALSE;
    backend->clipboard_limit = config->clipboard_limit;
    cmp_cocoa_event_queue_init(backend);
    backend->last_text = NULL;
    backend->time_origin = CFAbsoluteTimeGetCurrent();

    if (backend->log_enabled) {
        rc = cmp_log_init(&allocator);
        if (rc != CMP_OK && rc != CMP_ERR_STATE) {
            allocator.free(allocator.ctx, backend);
            return rc;
        }
        backend->log_owner = (rc == CMP_OK) ? CMP_TRUE : CMP_FALSE;
    }

    rc = cmp_handle_system_default_create(config->handle_capacity, &allocator, &backend->handles);
    if (rc != CMP_OK) {
        if (backend->log_owner) {
            cmp_log_shutdown();
        }
        allocator.free(allocator.ctx, backend);
        return rc;
    }

    backend->ws.ctx = backend;
    backend->ws.vtable = &g_cmp_cocoa_ws_vtable;
    backend->gfx.ctx = backend;
    backend->gfx.vtable = &g_cmp_cocoa_gfx_vtable;
    backend->gfx.text_vtable = &g_cmp_cocoa_text_vtable;
    backend->env.ctx = backend;
    backend->env.vtable = &g_cmp_cocoa_env_vtable;
    backend->io.ctx = backend;
    backend->io.vtable = &g_cmp_cocoa_io_vtable;
    backend->sensors.ctx = backend;
    backend->sensors.vtable = &g_cmp_cocoa_sensors_vtable;
    backend->camera.ctx = backend;
    backend->camera.vtable = &g_cmp_cocoa_camera_vtable;
    backend->network.ctx = backend;
    backend->network.vtable = &g_cmp_cocoa_network_vtable;
    backend->tasks.ctx = backend;
    backend->tasks.vtable = &g_cmp_cocoa_tasks_vtable;
    backend->initialized = CMP_TRUE;

    *out_backend = backend;
    return CMP_OK;
}

int CMP_CALL cmp_cocoa_backend_destroy(CMPCocoaBackend *backend)
{
    int rc;
    int first_error;
    cmp_usize i;
    cmp_usize idx;

    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }

    first_error = CMP_OK;

    cmp_cocoa_release_last_text(backend);
    for (i = 0u; i < backend->event_count; i += 1u) {
        idx = (backend->event_head + i) % CMP_COCOA_EVENT_CAPACITY;
        cmp_cocoa_free_event_text(backend, &backend->event_queue[idx]);
    }

    rc = cmp_handle_system_default_destroy(&backend->handles);
    if (rc != CMP_OK && first_error == CMP_OK) {
        first_error = rc;
    }

    if (backend->frame_ctx != NULL) {
        CGContextRelease(backend->frame_ctx);
        backend->frame_ctx = NULL;
    }

    if (backend->frame_data != NULL) {
        rc = backend->allocator.free(backend->allocator.ctx, backend->frame_data);
        if (rc != CMP_OK && first_error == CMP_OK) {
            first_error = rc;
        }
        backend->frame_data = NULL;
    }

    if (backend->camera_opened == CMP_TRUE) {
        rc = cmp_cocoa_camera_close(backend);
        if (rc != CMP_OK && first_error == CMP_OK) {
            first_error = rc;
        }
    }

    if (backend->log_owner) {
        rc = cmp_log_shutdown();
        if (rc != CMP_OK && first_error == CMP_OK) {
            first_error = rc;
        }
    }

    backend->initialized = CMP_FALSE;
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
    if (rc != CMP_OK && first_error == CMP_OK) {
        first_error = rc;
    }

    return first_error;
}

int CMP_CALL cmp_cocoa_backend_get_ws(CMPCocoaBackend *backend, CMPWS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }
    *out_ws = backend->ws;
    return CMP_OK;
}

int CMP_CALL cmp_cocoa_backend_get_gfx(CMPCocoaBackend *backend, CMPGfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }
    *out_gfx = backend->gfx;
    return CMP_OK;
}

int CMP_CALL cmp_cocoa_backend_get_env(CMPCocoaBackend *backend, CMPEnv *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }
    *out_env = backend->env;
    return CMP_OK;
}

#endif
