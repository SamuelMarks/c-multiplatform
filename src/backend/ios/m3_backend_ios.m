#include "m3/m3_backend_ios.h"

#include "m3/m3_backend_null.h"

#include <string.h>
#include <errno.h>
#include <limits.h>

#define M3_IOS_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != M3_OK) { \
            return (rc); \
        } \
    } while (0)

#define M3_IOS_DEFAULT_HANDLE_CAPACITY 64u
#define M3_IOS_CAMERA_DEFAULT_WIDTH 640u
#define M3_IOS_CAMERA_DEFAULT_HEIGHT 480u

#if defined(__has_feature)
#if __has_feature(objc_arc)
#define M3_IOS_ARC 1
#endif
#endif

#if defined(M3_IOS_AVAILABLE)
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CFNetwork/CFNetwork.h>
#endif

static int m3_ios_backend_validate_config(const M3IOSBackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->handle_capacity == 0) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (config->allocator != NULL) {
        if (config->allocator->alloc == NULL || config->allocator->realloc == NULL || config->allocator->free == NULL) {
            return M3_ERR_INVALID_ARGUMENT;
        }
    }
    if (config->predictive_back != NULL && config->predictive_back->initialized != M3_TRUE) {
        return M3_ERR_STATE;
    }
    return M3_OK;
}

int M3_CALL m3_ios_backend_is_available(M3Bool *out_available)
{
    if (out_available == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#if defined(M3_IOS_AVAILABLE)
    *out_available = M3_TRUE;
#else
    *out_available = M3_FALSE;
#endif
    return M3_OK;
}

int M3_CALL m3_ios_backend_config_init(M3IOSBackendConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->handle_capacity = M3_IOS_DEFAULT_HANDLE_CAPACITY;
    config->clipboard_limit = (m3_usize)~(m3_usize)0;
    config->enable_logging = M3_TRUE;
    config->inline_tasks = M3_TRUE;
    config->predictive_back = NULL;
    return M3_OK;
}

#if defined(M3_IOS_AVAILABLE)

struct M3IOSBackend;

@interface M3IOSCameraDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
    struct M3IOSBackend *backend;
}
- (id)initWithBackend:(struct M3IOSBackend *)backend_ptr;
@end

typedef struct M3IOSBackend {
    M3Allocator allocator;
    M3NullBackend *null_backend;
    M3WS ws;
    M3Gfx gfx;
    M3Env env;
    M3Env null_env;
    M3Camera camera;
    M3Network network;
    M3PredictiveBack *predictive_back;
    M3Bool initialized;
    M3Bool camera_opened;
    M3Bool camera_streaming;
    M3Bool camera_has_frame;
    m3_u32 camera_format;
    m3_u32 camera_width;
    m3_u32 camera_height;
    m3_u32 camera_requested_format;
    M3Bool camera_swizzle;
    m3_usize camera_frame_capacity;
    m3_usize camera_frame_size;
    unsigned char *camera_frame;
    int camera_error;
    AVCaptureSession *camera_session;
    AVCaptureDeviceInput *camera_input;
    AVCaptureVideoDataOutput *camera_output;
    dispatch_queue_t camera_queue;
    M3IOSCameraDelegate *camera_delegate;
} M3IOSBackend;

static int m3_ios_camera_mul_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    m3_usize max_value;

    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    max_value = (m3_usize)~(m3_usize)0;
    if (a != 0u && b > max_value / a) {
        return M3_ERR_OVERFLOW;
    }

    *out_value = a * b;
    return M3_OK;
}

static int m3_ios_camera_select_device(const M3CameraConfig *config, AVCaptureDevice **out_device)
{
    NSArray *devices;
    AVCaptureDevice *device;
    AVCaptureDevicePosition desired;
    NSUInteger count;
    NSUInteger i;

    if (config == NULL || out_device == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_device = nil;
    devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    if (devices == nil) {
        return M3_ERR_NOT_FOUND;
    }

    count = [devices count];
    if (count == 0u) {
        return M3_ERR_NOT_FOUND;
    }

    if (config->camera_id != M3_CAMERA_ID_DEFAULT) {
        if (config->camera_id >= (m3_u32)count) {
            return M3_ERR_NOT_FOUND;
        }
        *out_device = [devices objectAtIndex:(NSUInteger)config->camera_id];
        return M3_OK;
    }

    if (config->facing == M3_CAMERA_FACING_UNSPECIFIED) {
        *out_device = [devices objectAtIndex:0u];
        return M3_OK;
    }

    desired = AVCaptureDevicePositionUnspecified;
    switch (config->facing) {
    case M3_CAMERA_FACING_FRONT:
        desired = AVCaptureDevicePositionFront;
        break;
    case M3_CAMERA_FACING_BACK:
        desired = AVCaptureDevicePositionBack;
        break;
    case M3_CAMERA_FACING_EXTERNAL:
    default:
        desired = AVCaptureDevicePositionUnspecified;
        break;
    }

    for (i = 0u; i < count; i += 1u) {
        device = [devices objectAtIndex:i];
        if (device != nil && [device position] == desired) {
            *out_device = device;
            return M3_OK;
        }
    }

    return M3_ERR_NOT_FOUND;
}

static int m3_ios_camera_apply_preset(AVCaptureSession *session, m3_u32 width, m3_u32 height)
{
    NSString *preset;
    m3_u32 target_width;
    m3_u32 target_height;
    M3Bool allow_fallback;

    if (session == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    allow_fallback = (width == 0u || height == 0u) ? M3_TRUE : M3_FALSE;
    target_width = (width == 0u) ? M3_IOS_CAMERA_DEFAULT_WIDTH : width;
    target_height = (height == 0u) ? M3_IOS_CAMERA_DEFAULT_HEIGHT : height;

    preset = nil;
    if (target_width == M3_IOS_CAMERA_DEFAULT_WIDTH && target_height == M3_IOS_CAMERA_DEFAULT_HEIGHT) {
        if ([session canSetSessionPreset:AVCaptureSessionPreset640x480]) {
            session.sessionPreset = AVCaptureSessionPreset640x480;
            return M3_OK;
        }
        if (allow_fallback && [session canSetSessionPreset:AVCaptureSessionPresetMedium]) {
            session.sessionPreset = AVCaptureSessionPresetMedium;
            return M3_OK;
        }
        return M3_ERR_UNSUPPORTED;
    } else if (target_width == 1280u && target_height == 720u) {
        preset = AVCaptureSessionPreset1280x720;
    } else if (target_width == 1920u && target_height == 1080u) {
        preset = AVCaptureSessionPreset1920x1080;
    } else if (target_width == 3840u && target_height == 2160u) {
        preset = AVCaptureSessionPreset3840x2160;
    } else {
        return M3_ERR_UNSUPPORTED;
    }

    if ([session canSetSessionPreset:preset]) {
        session.sessionPreset = preset;
        return M3_OK;
    }

    return M3_ERR_UNSUPPORTED;
}

static M3Bool m3_ios_camera_output_supports_format(AVCaptureVideoDataOutput *output, OSType pixel_format)
{
    NSArray *formats;
    NSNumber *value;
    NSUInteger count;
    NSUInteger i;

    if (output == nil) {
        return M3_FALSE;
    }

    formats = [output availableVideoCVPixelFormatTypes];
    if (formats == nil) {
        return M3_FALSE;
    }

    count = [formats count];
    for (i = 0u; i < count; i += 1u) {
        value = [formats objectAtIndex:i];
        if (value != nil && [value unsignedIntValue] == (unsigned int)pixel_format) {
            return M3_TRUE;
        }
    }

    return M3_FALSE;
}

static int m3_ios_camera_pick_pixel_format(AVCaptureVideoDataOutput *output, m3_u32 requested_format,
    OSType *out_format, M3Bool *out_swizzle)
{
    if (output == nil || out_format == NULL || out_swizzle == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    *out_swizzle = M3_FALSE;

    if (requested_format == M3_CAMERA_FORMAT_ANY) {
        if (m3_ios_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            *out_format = kCVPixelFormatType_32BGRA;
            return M3_OK;
        }
        if (m3_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return M3_OK;
        }
        if (m3_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return M3_OK;
        }
        return M3_ERR_UNSUPPORTED;
    }

    if (requested_format == M3_CAMERA_FORMAT_BGRA8 || requested_format == M3_CAMERA_FORMAT_RGBA8) {
        if (!m3_ios_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            return M3_ERR_UNSUPPORTED;
        }
        *out_format = kCVPixelFormatType_32BGRA;
        *out_swizzle = (requested_format == M3_CAMERA_FORMAT_RGBA8) ? M3_TRUE : M3_FALSE;
        return M3_OK;
    }

    if (requested_format == M3_CAMERA_FORMAT_NV12) {
        if (m3_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return M3_OK;
        }
        if (m3_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return M3_OK;
        }
        return M3_ERR_UNSUPPORTED;
    }

    return M3_ERR_UNSUPPORTED;
}

static int m3_ios_camera_update_frame(M3IOSBackend *backend, CMSampleBufferRef sample)
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
    m3_usize width;
    m3_usize height;
    m3_usize row_bytes;
    m3_usize needed;
    m3_usize y_size;
    m3_usize uv_size;
    m3_usize col;
    int rc;
    int frame_rc;
    M3Bool swizzle;

    if (backend == NULL || sample == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    image = CMSampleBufferGetImageBuffer(sample);
    if (image == NULL) {
        return M3_ERR_NOT_READY;
    }

    pixel = (CVPixelBufferRef)image;
    pixel_format = CVPixelBufferGetPixelFormatType(pixel);

    if (CVPixelBufferLockBaseAddress(pixel, kCVPixelBufferLock_ReadOnly) != kCVReturnSuccess) {
        return M3_ERR_IO;
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
    frame_rc = M3_OK;
    if (pixel_format == kCVPixelFormatType_32BGRA) {
        base_address = (unsigned char *)CVPixelBufferGetBaseAddress(pixel);
        bytes_per_row = CVPixelBufferGetBytesPerRow(pixel);
        width = (m3_usize)CVPixelBufferGetWidth(pixel);
        height = (m3_usize)CVPixelBufferGetHeight(pixel);

        rc = m3_ios_camera_mul_overflow(width, (m3_usize)4u, &row_bytes);
        if (rc == M3_OK) {
            rc = m3_ios_camera_mul_overflow(row_bytes, height, &needed);
        }

        if (rc != M3_OK || base_address == NULL) {
            frame_rc = (rc == M3_OK) ? M3_ERR_IO : rc;
        } else {
            @synchronized (backend) {
                if (backend->allocator.alloc == NULL || backend->allocator.realloc == NULL
                    || backend->allocator.free == NULL) {
                    frame_rc = M3_ERR_INVALID_ARGUMENT;
                } else if (backend->camera_frame_capacity < needed) {
                    unsigned char *new_frame;
                    int alloc_rc = backend->allocator.realloc(backend->allocator.ctx, backend->camera_frame, needed,
                        (void **)&new_frame);
                    if (alloc_rc != M3_OK) {
                        frame_rc = alloc_rc;
                    } else {
                        backend->camera_frame = new_frame;
                        backend->camera_frame_capacity = needed;
                    }
                }

                if (frame_rc == M3_OK) {
                    if (swizzle == M3_TRUE) {
                        unsigned char *src;
                        unsigned char *dst;
                        m3_usize idx;

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
                        backend->camera_format = M3_CAMERA_FORMAT_RGBA8;
                    } else {
                        if (bytes_per_row == (size_t)row_bytes) {
                            memcpy(backend->camera_frame, base_address, (size_t)needed);
                        } else {
                            for (row = 0u; row < height; row += 1u) {
                                memcpy(backend->camera_frame + row * row_bytes, base_address + row * bytes_per_row,
                                    (size_t)row_bytes);
                            }
                        }
                        backend->camera_format = M3_CAMERA_FORMAT_BGRA8;
                    }

                    backend->camera_width = (m3_u32)width;
                    backend->camera_height = (m3_u32)height;
                    backend->camera_frame_size = needed;
                    backend->camera_has_frame = M3_TRUE;
                }
                backend->camera_error = frame_rc;
            }
        }
    } else if (pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
        || pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
        if (!CVPixelBufferIsPlanar(pixel) || CVPixelBufferGetPlaneCount(pixel) < 2u) {
            frame_rc = M3_ERR_UNSUPPORTED;
        } else {
            y_plane = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(pixel, 0);
            uv_plane = (unsigned char *)CVPixelBufferGetBaseAddressOfPlane(pixel, 1);
            y_bytes_per_row = CVPixelBufferGetBytesPerRowOfPlane(pixel, 0);
            uv_bytes_per_row = CVPixelBufferGetBytesPerRowOfPlane(pixel, 1);
            width = (m3_usize)CVPixelBufferGetWidth(pixel);
            height = (m3_usize)CVPixelBufferGetHeight(pixel);

            rc = m3_ios_camera_mul_overflow(width, height, &y_size);
            if (rc == M3_OK) {
                uv_size = y_size / 2u;
                needed = y_size + uv_size;
            }

            if (rc != M3_OK || y_plane == NULL || uv_plane == NULL) {
                frame_rc = (rc == M3_OK) ? M3_ERR_IO : rc;
            } else {
                @synchronized (backend) {
                    if (backend->allocator.alloc == NULL || backend->allocator.realloc == NULL
                        || backend->allocator.free == NULL) {
                        frame_rc = M3_ERR_INVALID_ARGUMENT;
                    } else if (backend->camera_frame_capacity < needed) {
                        unsigned char *new_frame;
                        int alloc_rc = backend->allocator.realloc(backend->allocator.ctx, backend->camera_frame, needed,
                            (void **)&new_frame);
                        if (alloc_rc != M3_OK) {
                            frame_rc = alloc_rc;
                        } else {
                            backend->camera_frame = new_frame;
                            backend->camera_frame_capacity = needed;
                        }
                    }

                    if (frame_rc == M3_OK) {
                        for (row = 0u; row < height; row += 1u) {
                            memcpy(backend->camera_frame + row * width,
                                y_plane + row * (m3_usize)y_bytes_per_row, (size_t)width);
                        }
                        for (row = 0u; row < height / 2u; row += 1u) {
                            memcpy(backend->camera_frame + y_size + row * width,
                                uv_plane + row * (m3_usize)uv_bytes_per_row, (size_t)width);
                        }

                        backend->camera_format = M3_CAMERA_FORMAT_NV12;
                        backend->camera_width = (m3_u32)width;
                        backend->camera_height = (m3_u32)height;
                        backend->camera_frame_size = needed;
                        backend->camera_has_frame = M3_TRUE;
                    }
                    backend->camera_error = frame_rc;
                }
            }
        }
    } else {
        frame_rc = M3_ERR_UNSUPPORTED;
        @synchronized (backend) {
            backend->camera_error = frame_rc;
        }
    }

    if (frame_rc != M3_OK) {
        @synchronized (backend) {
            backend->camera_error = frame_rc;
        }
    }

    CVPixelBufferUnlockBaseAddress(pixel, kCVPixelBufferLock_ReadOnly);
    return frame_rc;
}

@implementation M3IOSCameraDelegate
- (id)initWithBackend:(M3IOSBackend *)backend_ptr
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

    M3_UNUSED(output);
    M3_UNUSED(connection);

    if (backend == NULL) {
        return;
    }

    rc = m3_ios_camera_update_frame(backend, sampleBuffer);
    M3_UNUSED(rc);
}
@end

static int m3_ios_camera_open_with_config(void *camera, const M3CameraConfig *config);

static int m3_ios_camera_open(void *camera, m3_u32 camera_id)
{
    M3CameraConfig config;

    config.camera_id = camera_id;
    config.facing = M3_CAMERA_FACING_UNSPECIFIED;
    config.width = 0u;
    config.height = 0u;
    config.format = M3_CAMERA_FORMAT_ANY;

    return m3_ios_camera_open_with_config(camera, &config);
}

static int m3_ios_camera_open_with_config(void *camera, const M3CameraConfig *config)
{
    M3IOSBackend *backend;
    int rc;

    if (camera == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)camera;
    if (backend->camera_opened == M3_TRUE) {
        return M3_ERR_STATE;
    }

    @autoreleasepool {
        AVCaptureDevice *device;
        AVCaptureSession *session;
        AVCaptureDeviceInput *input;
        AVCaptureVideoDataOutput *output;
        NSDictionary *settings;
        M3IOSCameraDelegate *delegate;
        dispatch_queue_t queue;
        NSError *error;
        OSType pixel_format;
        M3Bool swizzle;

        rc = m3_ios_camera_select_device(config, &device);
        if (rc != M3_OK) {
            return rc;
        }

        error = nil;
        input = [[AVCaptureDeviceInput alloc] initWithDevice:device error:&error];
        if (input == nil) {
            if (error != nil && [[error domain] isEqualToString:AVFoundationErrorDomain]) {
                if ([error code] == AVErrorApplicationIsNotAuthorizedToUseDevice) {
                    return M3_ERR_PERMISSION;
                }
                if ([error code] == AVErrorDeviceNotConnected) {
                    return M3_ERR_NOT_FOUND;
                }
                if ([error code] == AVErrorDeviceInUseByAnotherApplication) {
                    return M3_ERR_BUSY;
                }
            }
            return M3_ERR_UNKNOWN;
        }

        session = [[AVCaptureSession alloc] init];
        if (session == nil) {
#if !defined(M3_IOS_ARC)
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        rc = m3_ios_camera_apply_preset(session, config->width, config->height);
        if (rc != M3_OK) {
#if !defined(M3_IOS_ARC)
            [session release];
            [input release];
#endif
            return rc;
        }

        output = [[AVCaptureVideoDataOutput alloc] init];
        if (output == nil) {
#if !defined(M3_IOS_ARC)
            [session release];
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        rc = m3_ios_camera_pick_pixel_format(output, config->format, &pixel_format, &swizzle);
        if (rc != M3_OK) {
#if !defined(M3_IOS_ARC)
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

        delegate = [[M3IOSCameraDelegate alloc] initWithBackend:backend];
        if (delegate == nil) {
#if !defined(M3_IOS_ARC)
            [output release];
            [session release];
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        queue = dispatch_queue_create("m3.ios.camera", DISPATCH_QUEUE_SERIAL);
        if (queue == NULL) {
#if !defined(M3_IOS_ARC)
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }

        if (![session canAddInput:input] || ![session canAddOutput:output]) {
#if !defined(M3_IOS_ARC)
            dispatch_release(queue);
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return M3_ERR_UNSUPPORTED;
        }

        [session addInput:input];
        [session addOutput:output];
        [output setSampleBufferDelegate:delegate queue:queue];

        backend->camera_session = session;
        backend->camera_input = input;
        backend->camera_output = output;
        backend->camera_delegate = delegate;
        backend->camera_queue = queue;
        backend->camera_opened = M3_TRUE;
        backend->camera_streaming = M3_FALSE;
        backend->camera_has_frame = M3_FALSE;
        backend->camera_error = M3_OK;
        backend->camera_requested_format = config->format;
        backend->camera_swizzle = swizzle;
    }

    return M3_OK;
}

static int m3_ios_camera_close(void *camera)
{
    M3IOSBackend *backend;
    int rc;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)camera;
    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }

    if (backend->camera_streaming == M3_TRUE && backend->camera_session != nil) {
        [backend->camera_session stopRunning];
        backend->camera_streaming = M3_FALSE;
    }

    if (backend->camera_output != nil) {
        [backend->camera_output setSampleBufferDelegate:nil queue:NULL];
    }

    if (backend->camera_frame != NULL) {
        if (backend->allocator.free == NULL) {
            return M3_ERR_INVALID_ARGUMENT;
        }
        rc = backend->allocator.free(backend->allocator.ctx, backend->camera_frame);
        if (rc != M3_OK) {
            return rc;
        }
        backend->camera_frame = NULL;
        backend->camera_frame_capacity = 0u;
        backend->camera_frame_size = 0u;
    }

    if (backend->camera_delegate != nil) {
#if !defined(M3_IOS_ARC)
        [backend->camera_delegate release];
#endif
        backend->camera_delegate = nil;
    }

    if (backend->camera_output != nil) {
#if !defined(M3_IOS_ARC)
        [backend->camera_output release];
#endif
        backend->camera_output = nil;
    }

    if (backend->camera_input != nil) {
#if !defined(M3_IOS_ARC)
        [backend->camera_input release];
#endif
        backend->camera_input = nil;
    }

    if (backend->camera_session != nil) {
#if !defined(M3_IOS_ARC)
        [backend->camera_session release];
#endif
        backend->camera_session = nil;
    }

    if (backend->camera_queue != NULL) {
#if !defined(M3_IOS_ARC)
        dispatch_release(backend->camera_queue);
#endif
        backend->camera_queue = NULL;
    }

    backend->camera_opened = M3_FALSE;
    backend->camera_streaming = M3_FALSE;
    backend->camera_has_frame = M3_FALSE;
    backend->camera_format = 0u;
    backend->camera_width = 0u;
    backend->camera_height = 0u;
    backend->camera_requested_format = 0u;
    backend->camera_swizzle = M3_FALSE;
    backend->camera_error = M3_OK;
    return M3_OK;
}

static int m3_ios_camera_start(void *camera)
{
    M3IOSBackend *backend;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)camera;
    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_streaming == M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_session == nil) {
        return M3_ERR_STATE;
    }

    [backend->camera_session startRunning];
    if (![backend->camera_session isRunning]) {
        return M3_ERR_UNKNOWN;
    }

    backend->camera_streaming = M3_TRUE;
    return M3_OK;
}

static int m3_ios_camera_stop(void *camera)
{
    M3IOSBackend *backend;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)camera;
    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_session == nil) {
        return M3_ERR_STATE;
    }

    [backend->camera_session stopRunning];
    backend->camera_streaming = M3_FALSE;
    return M3_OK;
}

static int m3_ios_camera_read_frame(void *camera, M3CameraFrame *out_frame, M3Bool *out_has_frame)
{
    M3IOSBackend *backend;
    int rc;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)camera;
    if (backend->camera_opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (backend->camera_streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }

    @synchronized (backend) {
        if (backend->camera_error != M3_OK) {
            rc = backend->camera_error;
            backend->camera_error = M3_OK;
            return rc;
        }

        if (backend->camera_has_frame != M3_TRUE || backend->camera_frame == NULL) {
            memset(out_frame, 0, sizeof(*out_frame));
            *out_has_frame = M3_FALSE;
            return M3_OK;
        }

        out_frame->format = backend->camera_format;
        out_frame->width = backend->camera_width;
        out_frame->height = backend->camera_height;
        out_frame->data = backend->camera_frame;
        out_frame->size = backend->camera_frame_size;
        *out_has_frame = M3_TRUE;
    }

    return M3_OK;
}

static const M3CameraVTable g_m3_ios_camera_vtable = {
    m3_ios_camera_open,
    m3_ios_camera_open_with_config,
    m3_ios_camera_close,
    m3_ios_camera_start,
    m3_ios_camera_stop,
    m3_ios_camera_read_frame
};

#if !defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_ios_network_error_from_ns_error(NSError *error)
{
    NSString *domain;
    NSInteger code;

    if (error == nil) {
        return M3_OK;
    }

    domain = [error domain];
    code = [error code];
    if ([domain isEqualToString:NSURLErrorDomain]) {
        switch (code) {
        case NSURLErrorTimedOut:
            return M3_ERR_TIMEOUT;
        case NSURLErrorBadURL:
            return M3_ERR_INVALID_ARGUMENT;
        case NSURLErrorUnsupportedURL:
            return M3_ERR_UNSUPPORTED;
        case NSURLErrorCannotFindHost:
        case NSURLErrorDNSLookupFailed:
            return M3_ERR_NOT_FOUND;
        case NSURLErrorUserAuthenticationRequired:
        case NSURLErrorUserCancelledAuthentication:
        case NSURLErrorSecureConnectionFailed:
            return M3_ERR_PERMISSION;
        case NSURLErrorCannotConnectToHost:
        case NSURLErrorNetworkConnectionLost:
        case NSURLErrorNotConnectedToInternet:
            return M3_ERR_IO;
        default:
            return M3_ERR_IO;
        }
    }

    return M3_ERR_IO;
}

#endif

#if !defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_ios_network_apply_headers(NSMutableURLRequest *request, const char *headers)
{
    const char *cursor;

    if (request == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (headers == NULL || headers[0] == '\0') {
        return M3_OK;
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
                return M3_ERR_INVALID_ARGUMENT;
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
                return M3_ERR_INVALID_ARGUMENT;
            }

            key = [[NSString alloc] initWithBytes:key_start
                                           length:(NSUInteger)(key_end - key_start)
                                         encoding:NSUTF8StringEncoding];
            if (key == nil) {
                return M3_ERR_INVALID_ARGUMENT;
            }
            value = [[NSString alloc] initWithBytes:value_start
                                             length:(NSUInteger)(value_end - value_start)
                                           encoding:NSUTF8StringEncoding];
            if (value == nil) {
#if !defined(M3_IOS_ARC)
                [key release];
#endif
                return M3_ERR_INVALID_ARGUMENT;
            }

            [request setValue:value forHTTPHeaderField:key];

#if !defined(M3_IOS_ARC)
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

    return M3_OK;
}

#endif

#if defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_ios_network_add_overflow(m3_usize a, m3_usize b, m3_usize *out_value)
{
    if (out_value == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (b > ((m3_usize)~(m3_usize)0) - a) {
        return M3_ERR_OVERFLOW;
    }
    *out_value = a + b;
    return M3_OK;
}

static int m3_ios_network_error_from_cf_stream(CFStreamError error)
{
    if (error.domain == 0) {
        return M3_ERR_IO;
    }
    if (error.domain == kCFStreamErrorDomainPOSIX) {
        if (error.error == ETIMEDOUT) {
            return M3_ERR_TIMEOUT;
        }
        return M3_ERR_IO;
    }
    return M3_ERR_IO;
}

static int m3_ios_network_apply_cf_headers(CFHTTPMessageRef message, const char *headers)
{
    const char *cursor;

    if (message == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (headers == NULL || headers[0] == '\0') {
        return M3_OK;
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
                return M3_ERR_INVALID_ARGUMENT;
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
                return M3_ERR_INVALID_ARGUMENT;
            }

            if ((m3_usize)(key_end - key_start) > (m3_usize)LONG_MAX
                    || (m3_usize)(value_end - value_start) > (m3_usize)LONG_MAX) {
                return M3_ERR_RANGE;
            }

            key = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)key_start,
                (CFIndex)(key_end - key_start), kCFStringEncodingUTF8, false);
            if (key == NULL) {
                return M3_ERR_INVALID_ARGUMENT;
            }
            value = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)value_start,
                (CFIndex)(value_end - value_start), kCFStringEncodingUTF8, false);
            if (value == NULL) {
                CFRelease(key);
                return M3_ERR_INVALID_ARGUMENT;
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

    return M3_OK;
}

static int m3_ios_network_request_cfnetwork(void *net, const M3NetworkRequest *request, const M3Allocator *allocator,
    M3NetworkResponse *out_response)
{
    M3IOSBackend *backend;
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
    m3_usize total_size;
    m3_usize capacity;
    m3_usize alloc_size;
    m3_usize url_len;
    m3_usize method_len;
    int rc;

    if (net == NULL || request == NULL || allocator == NULL || out_response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method == NULL || request->url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    url_len = (m3_usize)strlen(request->url);
    if (url_len > (m3_usize)LONG_MAX) {
        return M3_ERR_RANGE;
    }
    method_len = (m3_usize)strlen(request->method);
    if (method_len > (m3_usize)LONG_MAX) {
        return M3_ERR_RANGE;
    }
    if (request->body_size > (m3_usize)LONG_MAX) {
        return M3_ERR_RANGE;
    }

    backend = (M3IOSBackend *)net;
    M3_UNUSED(backend);

    memset(out_response, 0, sizeof(*out_response));

    url_string = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)request->url, (CFIndex)url_len,
        kCFStringEncodingUTF8, false);
    if (url_string == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    url = CFURLCreateWithString(kCFAllocatorDefault, url_string, NULL);
    CFRelease(url_string);
    if (url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    method = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)request->method, (CFIndex)method_len,
        kCFStringEncodingUTF8, false);
    if (method == NULL) {
        CFRelease(url);
        return M3_ERR_INVALID_ARGUMENT;
    }

    message = CFHTTPMessageCreateRequest(kCFAllocatorDefault, method, url, kCFHTTPVersion1_1);
    CFRelease(method);
    CFRelease(url);
    if (message == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
    }

    rc = m3_ios_network_apply_cf_headers(message, request->headers);
    if (rc != M3_OK) {
        CFRelease(message);
        return rc;
    }

    if (request->body_size > 0) {
        body_data = CFDataCreate(kCFAllocatorDefault, (const UInt8 *)request->body, (CFIndex)request->body_size);
        if (body_data == NULL) {
            CFRelease(message);
            return M3_ERR_OUT_OF_MEMORY;
        }
        CFHTTPMessageSetBody(message, body_data);
        CFRelease(body_data);
    }

    stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, message);
    CFRelease(message);
    if (stream == NULL) {
        return M3_ERR_OUT_OF_MEMORY;
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
        return m3_ios_network_error_from_cf_stream(stream_error);
    }

    response_message = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
    if (response_message != NULL) {
        long status_code = CFHTTPMessageGetResponseStatusCode(response_message);
        if (status_code > 0) {
            out_response->status_code = (m3_u32)status_code;
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
            rc = m3_ios_network_error_from_cf_stream(stream_error);
            goto cleanup;
        }
        if (bytes_read == 0) {
            rc = M3_OK;
            break;
        }

        rc = m3_ios_network_add_overflow(total_size, (m3_usize)bytes_read, &alloc_size);
        if (rc != M3_OK) {
            goto cleanup;
        }

        if (alloc_size > capacity) {
            new_body = NULL;
            if (body == NULL) {
                rc = allocator->alloc(allocator->ctx, alloc_size, &new_body);
            } else {
                rc = allocator->realloc(allocator->ctx, body, alloc_size, &new_body);
            }
            if (rc != M3_OK) {
                goto cleanup;
            }
            body = new_body;
            capacity = alloc_size;
        }

        memcpy((m3_u8 *)body + total_size, buffer, (size_t)bytes_read);
        total_size += (m3_usize)bytes_read;
    }

cleanup:
    CFReadStreamClose(stream);
    CFRelease(stream);

    if (rc != M3_OK) {
        if (body != NULL) {
            allocator->free(allocator->ctx, body);
        }
        memset(out_response, 0, sizeof(*out_response));
        return rc;
    }

    out_response->body = body;
    out_response->body_size = total_size;
    return M3_OK;
}

#endif

#if !defined(M3_APPLE_USE_CFNETWORK_C)
static int m3_ios_network_request_foundation(void *net, const M3NetworkRequest *request, const M3Allocator *allocator,
    M3NetworkResponse *out_response)
{
    M3IOSBackend *backend;
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
    m3_usize body_size;
    int rc;

    if (net == NULL || request == NULL || allocator == NULL || out_response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method == NULL || request->url == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->method[0] == '\0' || request->url[0] == '\0') {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (request->body_size > 0 && request->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)net;
    M3_UNUSED(backend);

    memset(out_response, 0, sizeof(*out_response));

    url_string = [[NSString alloc] initWithBytes:request->url
                                          length:(NSUInteger)strlen(request->url)
                                        encoding:NSUTF8StringEncoding];
    if (url_string == nil) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    url = [NSURL URLWithString:url_string];
    if (url == nil) {
#if !defined(M3_IOS_ARC)
        [url_string release];
#endif
        return M3_ERR_INVALID_ARGUMENT;
    }

    url_request = [[NSMutableURLRequest alloc] initWithURL:url];
    if (url_request == nil) {
#if !defined(M3_IOS_ARC)
        [url_string release];
#endif
        return M3_ERR_OUT_OF_MEMORY;
    }

    method = [[NSString alloc] initWithBytes:request->method
                                      length:(NSUInteger)strlen(request->method)
                                    encoding:NSUTF8StringEncoding];
    if (method == nil) {
#if !defined(M3_IOS_ARC)
        [url_request release];
        [url_string release];
#endif
        return M3_ERR_INVALID_ARGUMENT;
    }
    [url_request setHTTPMethod:method];

    if (request->timeout_ms > 0u) {
        [url_request setTimeoutInterval:((NSTimeInterval)request->timeout_ms) / 1000.0];
    }

    rc = m3_ios_network_apply_headers(url_request, request->headers);
    if (rc != M3_OK) {
#if !defined(M3_IOS_ARC)
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
#if !defined(M3_IOS_ARC)
            [method release];
            [url_request release];
            [url_string release];
#endif
            return M3_ERR_OUT_OF_MEMORY;
        }
        [url_request setHTTPBody:body_data];
    }

    response = nil;
    error = nil;
    data = [NSURLConnection sendSynchronousRequest:url_request returningResponse:&response error:&error];

#if !defined(M3_IOS_ARC)
    if (body_data != nil) {
        [body_data release];
    }
    [method release];
    [url_request release];
    [url_string release];
#endif

    if (error != nil) {
        return m3_ios_network_error_from_ns_error(error);
    }

    out_response->status_code = 0u;
    if (response != nil && [response isKindOfClass:[NSHTTPURLResponse class]]) {
        http_response = (NSHTTPURLResponse *)response;
        if ([http_response statusCode] > 0) {
            out_response->status_code = (m3_u32)[http_response statusCode];
        }
    }

    body = NULL;
    body_size = 0u;
    if (data != nil && [data length] > 0) {
        body_size = (m3_usize)[data length];
        rc = allocator->alloc(allocator->ctx, body_size, &body);
        if (rc != M3_OK) {
            out_response->status_code = 0u;
            out_response->body = NULL;
            out_response->body_size = 0u;
            return rc;
        }
        memcpy(body, [data bytes], (size_t)body_size);
    }

    out_response->body = body;
    out_response->body_size = body_size;
    return M3_OK;
}

#endif

static int m3_ios_network_request(void *net, const M3NetworkRequest *request, const M3Allocator *allocator,
    M3NetworkResponse *out_response)
{
#if defined(M3_APPLE_USE_CFNETWORK_C)
    return m3_ios_network_request_cfnetwork(net, request, allocator, out_response);
#else
    return m3_ios_network_request_foundation(net, request, allocator, out_response);
#endif
}

static int m3_ios_network_free_response(void *net, const M3Allocator *allocator, M3NetworkResponse *response)
{
    M3IOSBackend *backend;
    int rc;

    if (net == NULL || allocator == NULL || response == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (response->body_size > 0 && response->body == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)net;
    M3_UNUSED(backend);

    if (response->body != NULL) {
        rc = allocator->free(allocator->ctx, (void *)response->body);
        if (rc != M3_OK) {
            return rc;
        }
    }

    response->body = NULL;
    response->body_size = 0;
    response->status_code = 0;
    return M3_OK;
}

static const M3NetworkVTable g_m3_ios_network_vtable = {
    m3_ios_network_request,
    m3_ios_network_free_response
};

static int m3_ios_env_get_io(void *env, M3IO *out_io)
{
    M3IOSBackend *backend;

    if (env == NULL || out_io == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_io == NULL) {
        return M3_ERR_STATE;
    }
    return backend->null_env.vtable->get_io(backend->null_env.ctx, out_io);
}

static int m3_ios_env_get_sensors(void *env, M3Sensors *out_sensors)
{
    M3IOSBackend *backend;

    if (env == NULL || out_sensors == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_sensors == NULL) {
        return M3_ERR_STATE;
    }
    return backend->null_env.vtable->get_sensors(backend->null_env.ctx, out_sensors);
}

static int m3_ios_env_get_camera(void *env, M3Camera *out_camera)
{
    M3IOSBackend *backend;

    if (env == NULL || out_camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)env;
    *out_camera = backend->camera;
    return M3_OK;
}

static int m3_ios_env_get_network(void *env, M3Network *out_network)
{
    M3IOSBackend *backend;

    if (env == NULL || out_network == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)env;
    *out_network = backend->network;
    return M3_OK;
}

static int m3_ios_env_get_tasks(void *env, M3Tasks *out_tasks)
{
    M3IOSBackend *backend;

    if (env == NULL || out_tasks == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_tasks == NULL) {
        return M3_ERR_STATE;
    }
    return backend->null_env.vtable->get_tasks(backend->null_env.ctx, out_tasks);
}

static int m3_ios_env_get_time_ms(void *env, m3_u32 *out_time_ms)
{
    M3IOSBackend *backend;

    if (env == NULL || out_time_ms == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    backend = (M3IOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_time_ms == NULL) {
        return M3_ERR_STATE;
    }
    return backend->null_env.vtable->get_time_ms(backend->null_env.ctx, out_time_ms);
}

static const M3EnvVTable g_m3_ios_env_vtable = {
    m3_ios_env_get_io,
    m3_ios_env_get_sensors,
    m3_ios_env_get_camera,
    m3_ios_env_get_network,
    m3_ios_env_get_tasks,
    m3_ios_env_get_time_ms
};

static int m3_ios_backend_cleanup(M3Allocator *allocator, M3IOSBackend *backend, int primary_error)
{
    int rc;
    int result;

    if (allocator == NULL || backend == NULL) {
        return primary_error;
    }

    result = primary_error;

    if (backend->camera_opened == M3_TRUE) {
        rc = m3_ios_camera_close(backend);
        if (result == M3_OK && rc != M3_OK) {
            result = rc;
        }
    }

    if (backend->null_backend != NULL) {
        rc = m3_null_backend_destroy(backend->null_backend);
        if (result == M3_OK && rc != M3_OK) {
            result = rc;
        }
    }

    rc = allocator->free(allocator->ctx, backend);
    if (result == M3_OK && rc != M3_OK) {
        result = rc;
    }

    return result;
}

int M3_CALL m3_ios_backend_create(const M3IOSBackendConfig *config, M3IOSBackend **out_backend)
{
    M3IOSBackendConfig local_config;
    M3Allocator allocator;
    M3NullBackendConfig null_config;
    M3IOSBackend *backend;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_ios_backend_config_init(&local_config);
        M3_IOS_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_ios_backend_validate_config(config);
    M3_IOS_RETURN_IF_ERROR(rc);

    if (config->allocator == NULL) {
        rc = m3_get_default_allocator(&allocator);
        M3_IOS_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(*backend), (void **)&backend);
    M3_IOS_RETURN_IF_ERROR(rc);

    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;

    rc = m3_null_backend_config_init(&null_config);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    null_config.allocator = &allocator;
    null_config.handle_capacity = config->handle_capacity;
    null_config.clipboard_limit = config->clipboard_limit;
    null_config.enable_logging = config->enable_logging;
    null_config.inline_tasks = config->inline_tasks;

    rc = m3_null_backend_create(&null_config, &backend->null_backend);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = m3_null_backend_get_ws(backend->null_backend, &backend->ws);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = m3_null_backend_get_gfx(backend->null_backend, &backend->gfx);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = m3_null_backend_get_env(backend->null_backend, &backend->null_env);
    if (rc != M3_OK) {
        return m3_ios_backend_cleanup(&allocator, backend, rc);
    }

    backend->env.ctx = backend;
    backend->env.vtable = &g_m3_ios_env_vtable;
    backend->camera.ctx = backend;
    backend->camera.vtable = &g_m3_ios_camera_vtable;
    backend->network.ctx = backend;
    backend->network.vtable = &g_m3_ios_network_vtable;
    backend->camera_opened = M3_FALSE;
    backend->camera_streaming = M3_FALSE;
    backend->camera_has_frame = M3_FALSE;
    backend->camera_frame = NULL;
    backend->camera_frame_capacity = 0u;
    backend->camera_frame_size = 0u;
    backend->camera_error = M3_OK;

    backend->predictive_back = config->predictive_back;
    backend->initialized = M3_TRUE;
    *out_backend = backend;
    return M3_OK;
}

int M3_CALL m3_ios_backend_destroy(M3IOSBackend *backend)
{
    M3Allocator allocator;
    int rc;
    int first_error;

    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }

    allocator = backend->allocator;
    first_error = M3_OK;

    if (backend->camera_opened == M3_TRUE) {
        rc = m3_ios_camera_close(backend);
        if (rc != M3_OK) {
            first_error = rc;
        }
    }

    if (backend->null_backend != NULL) {
        rc = m3_null_backend_destroy(backend->null_backend);
        if (rc != M3_OK && first_error == M3_OK) {
            first_error = rc;
        }
        backend->null_backend = NULL;
    }

    backend->initialized = M3_FALSE;
    backend->env.ctx = NULL;
    backend->env.vtable = NULL;
    backend->null_env.ctx = NULL;
    backend->null_env.vtable = NULL;
    backend->camera.ctx = NULL;
    backend->camera.vtable = NULL;
    backend->network.ctx = NULL;
    backend->network.vtable = NULL;
    backend->predictive_back = NULL;

    rc = allocator.free(allocator.ctx, backend);
    if (rc != M3_OK && first_error == M3_OK) {
        first_error = rc;
    }

    return first_error;
}

int M3_CALL m3_ios_backend_get_ws(M3IOSBackend *backend, M3WS *out_ws)
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

int M3_CALL m3_ios_backend_get_gfx(M3IOSBackend *backend, M3Gfx *out_gfx)
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

int M3_CALL m3_ios_backend_get_env(M3IOSBackend *backend, M3Env *out_env)
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

int M3_CALL m3_ios_backend_set_predictive_back(M3IOSBackend *backend, M3PredictiveBack *predictive)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    if (predictive != NULL && predictive->initialized != M3_TRUE) {
        return M3_ERR_STATE;
    }
    backend->predictive_back = predictive;
    return M3_OK;
}

int M3_CALL m3_ios_backend_get_predictive_back(M3IOSBackend *backend, M3PredictiveBack **out_predictive)
{
    if (backend == NULL || out_predictive == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return M3_ERR_STATE;
    }
    *out_predictive = backend->predictive_back;
    return M3_OK;
}

int M3_CALL m3_ios_backend_predictive_back_start(M3IOSBackend *backend, const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return M3_ERR_STATE;
    }
    return m3_predictive_back_start(backend->predictive_back, event);
}

int M3_CALL m3_ios_backend_predictive_back_progress(M3IOSBackend *backend, const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return M3_ERR_STATE;
    }
    return m3_predictive_back_progress(backend->predictive_back, event);
}

int M3_CALL m3_ios_backend_predictive_back_commit(M3IOSBackend *backend, const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return M3_ERR_STATE;
    }
    return m3_predictive_back_commit(backend->predictive_back, event);
}

int M3_CALL m3_ios_backend_predictive_back_cancel(M3IOSBackend *backend, const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return M3_ERR_STATE;
    }
    return m3_predictive_back_cancel(backend->predictive_back, event);
}

#else

int M3_CALL m3_ios_backend_create(const M3IOSBackendConfig *config, M3IOSBackend **out_backend)
{
    M3IOSBackendConfig local_config;
    int rc;

    if (out_backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = m3_ios_backend_config_init(&local_config);
        M3_IOS_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = m3_ios_backend_validate_config(config);
    M3_IOS_RETURN_IF_ERROR(rc);

    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_destroy(M3IOSBackend *backend)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_ws(M3IOSBackend *backend, M3WS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_ws, 0, sizeof(*out_ws));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_gfx(M3IOSBackend *backend, M3Gfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_gfx, 0, sizeof(*out_gfx));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_env(M3IOSBackend *backend, M3Env *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(out_env, 0, sizeof(*out_env));
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_set_predictive_back(M3IOSBackend *backend,
                                               M3PredictiveBack *predictive)
{
    if (backend == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    M3_UNUSED(predictive);
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_get_predictive_back(M3IOSBackend *backend,
                                               M3PredictiveBack **out_predictive)
{
    if (backend == NULL || out_predictive == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    *out_predictive = NULL;
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_predictive_back_start(M3IOSBackend *backend,
                                                 const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_predictive_back_progress(M3IOSBackend *backend,
                                                    const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_predictive_back_commit(M3IOSBackend *backend,
                                                  const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_ios_backend_predictive_back_cancel(M3IOSBackend *backend,
                                                  const M3PredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    return M3_ERR_UNSUPPORTED;
}

#endif
