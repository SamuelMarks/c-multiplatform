#include "cmpc/cmp_backend_ios.h"

#include "cmpc/cmp_backend_null.h"

#include <string.h>
#include <errno.h>
#include <limits.h>

#define CMP_IOS_RETURN_IF_ERROR(rc) \
    do { \
        if ((rc) != CMP_OK) { \
            return (rc); \
        } \
    } while (0)

#define CMP_IOS_DEFAULT_HANDLE_CAPACITY 64u
#define CMP_IOS_CAMERA_DEFAULT_WIDTH 640u
#define CMP_IOS_CAMERA_DEFAULT_HEIGHT 480u

#if defined(__has_feature)
#if __has_feature(objc_arc)
#define CMP_IOS_ARC 1
#endif
#endif

#if defined(CMP_IOS_AVAILABLE)
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <CFNetwork/CFNetwork.h>
#endif

static int cmp_ios_backend_validate_config(const CMPIOSBackendConfig *config)
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
    if (config->predictive_back != NULL && config->predictive_back->initialized != CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    return CMP_OK;
}

int CMP_CALL cmp_ios_backend_is_available(CMPBool *out_available)
{
    if (out_available == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
#if defined(CMP_IOS_AVAILABLE)
    *out_available = CMP_TRUE;
#else
    *out_available = CMP_FALSE;
#endif
    return CMP_OK;
}

int CMP_CALL cmp_ios_backend_config_init(CMPIOSBackendConfig *config)
{
    if (config == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    config->allocator = NULL;
    config->handle_capacity = CMP_IOS_DEFAULT_HANDLE_CAPACITY;
    config->clipboard_limit = (cmp_usize)~(cmp_usize)0;
    config->enable_logging = CMP_TRUE;
    config->inline_tasks = CMP_TRUE;
    config->predictive_back = NULL;
    return CMP_OK;
}

#if defined(CMP_IOS_AVAILABLE)

struct CMPIOSBackend;

@interface CMPIOSCameraDelegate : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate> {
    struct CMPIOSBackend *backend;
}
- (id)initWithBackend:(struct CMPIOSBackend *)backend_ptr;
@end

typedef struct CMPIOSBackend {
    CMPAllocator allocator;
    CMPNullBackend *null_backend;
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPEnv null_env;
    CMPCamera camera;
    CMPNetwork network;
    CMPPredictiveBack *predictive_back;
    CMPBool initialized;
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
    CMPIOSCameraDelegate *camera_delegate;
} CMPIOSBackend;

static int cmp_ios_camera_mul_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value)
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

static int cmp_ios_camera_select_device(const CMPCameraConfig *config, AVCaptureDevice **out_device)
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

static int cmp_ios_camera_apply_preset(AVCaptureSession *session, cmp_u32 width, cmp_u32 height)
{
    NSString *preset;
    cmp_u32 target_width;
    cmp_u32 target_height;
    CMPBool allow_fallback;

    if (session == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    allow_fallback = (width == 0u || height == 0u) ? CMP_TRUE : CMP_FALSE;
    target_width = (width == 0u) ? CMP_IOS_CAMERA_DEFAULT_WIDTH : width;
    target_height = (height == 0u) ? CMP_IOS_CAMERA_DEFAULT_HEIGHT : height;

    preset = nil;
    if (target_width == CMP_IOS_CAMERA_DEFAULT_WIDTH && target_height == CMP_IOS_CAMERA_DEFAULT_HEIGHT) {
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

static CMPBool cmp_ios_camera_output_supports_format(AVCaptureVideoDataOutput *output, OSType pixel_format)
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

static int cmp_ios_camera_pick_pixel_format(AVCaptureVideoDataOutput *output, cmp_u32 requested_format,
    OSType *out_format, CMPBool *out_swizzle)
{
    if (output == nil || out_format == NULL || out_swizzle == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    *out_swizzle = CMP_FALSE;

    if (requested_format == CMP_CAMERA_FORMAT_ANY) {
        if (cmp_ios_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            *out_format = kCVPixelFormatType_32BGRA;
            return CMP_OK;
        }
        if (cmp_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return CMP_OK;
        }
        if (cmp_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return CMP_OK;
        }
        return CMP_ERR_UNSUPPORTED;
    }

    if (requested_format == CMP_CAMERA_FORMAT_BGRA8 || requested_format == CMP_CAMERA_FORMAT_RGBA8) {
        if (!cmp_ios_camera_output_supports_format(output, kCVPixelFormatType_32BGRA)) {
            return CMP_ERR_UNSUPPORTED;
        }
        *out_format = kCVPixelFormatType_32BGRA;
        *out_swizzle = (requested_format == CMP_CAMERA_FORMAT_RGBA8) ? CMP_TRUE : CMP_FALSE;
        return CMP_OK;
    }

    if (requested_format == CMP_CAMERA_FORMAT_NV12) {
        if (cmp_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            return CMP_OK;
        }
        if (cmp_ios_camera_output_supports_format(output, kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange)) {
            *out_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            return CMP_OK;
        }
        return CMP_ERR_UNSUPPORTED;
    }

    return CMP_ERR_UNSUPPORTED;
}

static int cmp_ios_camera_update_frame(CMPIOSBackend *backend, CMSampleBufferRef sample)
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

        rc = cmp_ios_camera_mul_overflow(width, (cmp_usize)4u, &row_bytes);
        if (rc == CMP_OK) {
            rc = cmp_ios_camera_mul_overflow(row_bytes, height, &needed);
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

            rc = cmp_ios_camera_mul_overflow(width, height, &y_size);
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

@implementation CMPIOSCameraDelegate
- (id)initWithBackend:(CMPIOSBackend *)backend_ptr
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

    rc = cmp_ios_camera_update_frame(backend, sampleBuffer);
    CMP_UNUSED(rc);
}
@end

static int cmp_ios_camera_open_with_config(void *camera, const CMPCameraConfig *config);

static int cmp_ios_camera_open(void *camera, cmp_u32 camera_id)
{
    CMPCameraConfig config;

    config.camera_id = camera_id;
    config.facing = CMP_CAMERA_FACING_UNSPECIFIED;
    config.width = 0u;
    config.height = 0u;
    config.format = CMP_CAMERA_FORMAT_ANY;

    return cmp_ios_camera_open_with_config(camera, &config);
}

static int cmp_ios_camera_open_with_config(void *camera, const CMPCameraConfig *config)
{
    CMPIOSBackend *backend;
    int rc;

    if (camera == NULL || config == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)camera;
    if (backend->camera_opened == CMP_TRUE) {
        return CMP_ERR_STATE;
    }

    @autoreleasepool {
        AVCaptureDevice *device;
        AVCaptureSession *session;
        AVCaptureDeviceInput *input;
        AVCaptureVideoDataOutput *output;
        NSDictionary *settings;
        CMPIOSCameraDelegate *delegate;
        dispatch_queue_t queue;
        NSError *error;
        OSType pixel_format;
        CMPBool swizzle;

        rc = cmp_ios_camera_select_device(config, &device);
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
#if !defined(CMP_IOS_ARC)
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        rc = cmp_ios_camera_apply_preset(session, config->width, config->height);
        if (rc != CMP_OK) {
#if !defined(CMP_IOS_ARC)
            [session release];
            [input release];
#endif
            return rc;
        }

        output = [[AVCaptureVideoDataOutput alloc] init];
        if (output == nil) {
#if !defined(CMP_IOS_ARC)
            [session release];
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        rc = cmp_ios_camera_pick_pixel_format(output, config->format, &pixel_format, &swizzle);
        if (rc != CMP_OK) {
#if !defined(CMP_IOS_ARC)
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

        delegate = [[CMPIOSCameraDelegate alloc] initWithBackend:backend];
        if (delegate == nil) {
#if !defined(CMP_IOS_ARC)
            [output release];
            [session release];
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        queue = dispatch_queue_create("m3.ios.camera", DISPATCH_QUEUE_SERIAL);
        if (queue == NULL) {
#if !defined(CMP_IOS_ARC)
            [delegate release];
            [output release];
            [session release];
            [input release];
#endif
            return CMP_ERR_OUT_OF_MEMORY;
        }

        if (![session canAddInput:input] || ![session canAddOutput:output]) {
#if !defined(CMP_IOS_ARC)
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

static int cmp_ios_camera_close(void *camera)
{
    CMPIOSBackend *backend;
    int rc;

    if (camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)camera;
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
#if !defined(CMP_IOS_ARC)
        [backend->camera_delegate release];
#endif
        backend->camera_delegate = nil;
    }

    if (backend->camera_output != nil) {
#if !defined(CMP_IOS_ARC)
        [backend->camera_output release];
#endif
        backend->camera_output = nil;
    }

    if (backend->camera_input != nil) {
#if !defined(CMP_IOS_ARC)
        [backend->camera_input release];
#endif
        backend->camera_input = nil;
    }

    if (backend->camera_session != nil) {
#if !defined(CMP_IOS_ARC)
        [backend->camera_session release];
#endif
        backend->camera_session = nil;
    }

    if (backend->camera_queue != NULL) {
#if !defined(CMP_IOS_ARC)
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

static int cmp_ios_camera_start(void *camera)
{
    CMPIOSBackend *backend;

    if (camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)camera;
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

static int cmp_ios_camera_stop(void *camera)
{
    CMPIOSBackend *backend;

    if (camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)camera;
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

static int cmp_ios_camera_read_frame(void *camera, CMPCameraFrame *out_frame, CMPBool *out_has_frame)
{
    CMPIOSBackend *backend;
    int rc;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)camera;
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

static const CMPCameraVTable g_cmp_ios_camera_vtable = {
    cmp_ios_camera_open,
    cmp_ios_camera_open_with_config,
    cmp_ios_camera_close,
    cmp_ios_camera_start,
    cmp_ios_camera_stop,
    cmp_ios_camera_read_frame
};

#if !defined(CMP_APPLE_USE_CFNETWORK_C)
static int cmp_ios_network_error_from_ns_error(NSError *error)
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
static int cmp_ios_network_apply_headers(NSMutableURLRequest *request, const char *headers)
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
#if !defined(CMP_IOS_ARC)
                [key release];
#endif
                return CMP_ERR_INVALID_ARGUMENT;
            }

            [request setValue:value forHTTPHeaderField:key];

#if !defined(CMP_IOS_ARC)
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
static int cmp_ios_network_add_overflow(cmp_usize a, cmp_usize b, cmp_usize *out_value)
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

static int cmp_ios_network_error_from_cf_stream(CFStreamError error)
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

static int cmp_ios_network_apply_cf_headers(CFHTTPMessageRef message, const char *headers)
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

static int cmp_ios_network_request_cfnetwork(void *net, const CMPNetworkRequest *request, const CMPAllocator *allocator,
    CMPNetworkResponse *out_response)
{
    CMPIOSBackend *backend;
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

    backend = (CMPIOSBackend *)net;
    CMP_UNUSED(backend);

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

    rc = cmp_ios_network_apply_cf_headers(message, request->headers);
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
        return cmp_ios_network_error_from_cf_stream(stream_error);
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
            rc = cmp_ios_network_error_from_cf_stream(stream_error);
            goto cleanup;
        }
        if (bytes_read == 0) {
            rc = CMP_OK;
            break;
        }

        rc = cmp_ios_network_add_overflow(total_size, (cmp_usize)bytes_read, &alloc_size);
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
static int cmp_ios_network_request_foundation(void *net, const CMPNetworkRequest *request, const CMPAllocator *allocator,
    CMPNetworkResponse *out_response)
{
    CMPIOSBackend *backend;
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

    backend = (CMPIOSBackend *)net;
    CMP_UNUSED(backend);

    memset(out_response, 0, sizeof(*out_response));

    url_string = [[NSString alloc] initWithBytes:request->url
                                          length:(NSUInteger)strlen(request->url)
                                        encoding:NSUTF8StringEncoding];
    if (url_string == nil) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    url = [NSURL URLWithString:url_string];
    if (url == nil) {
#if !defined(CMP_IOS_ARC)
        [url_string release];
#endif
        return CMP_ERR_INVALID_ARGUMENT;
    }

    url_request = [[NSMutableURLRequest alloc] initWithURL:url];
    if (url_request == nil) {
#if !defined(CMP_IOS_ARC)
        [url_string release];
#endif
        return CMP_ERR_OUT_OF_MEMORY;
    }

    method = [[NSString alloc] initWithBytes:request->method
                                      length:(NSUInteger)strlen(request->method)
                                    encoding:NSUTF8StringEncoding];
    if (method == nil) {
#if !defined(CMP_IOS_ARC)
        [url_request release];
        [url_string release];
#endif
        return CMP_ERR_INVALID_ARGUMENT;
    }
    [url_request setHTTPMethod:method];

    if (request->timeout_ms > 0u) {
        [url_request setTimeoutInterval:((NSTimeInterval)request->timeout_ms) / 1000.0];
    }

    rc = cmp_ios_network_apply_headers(url_request, request->headers);
    if (rc != CMP_OK) {
#if !defined(CMP_IOS_ARC)
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
#if !defined(CMP_IOS_ARC)
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

#if !defined(CMP_IOS_ARC)
    if (body_data != nil) {
        [body_data release];
    }
    [method release];
    [url_request release];
    [url_string release];
#endif

    if (error != nil) {
        return cmp_ios_network_error_from_ns_error(error);
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

static int cmp_ios_network_request(void *net, const CMPNetworkRequest *request, const CMPAllocator *allocator,
    CMPNetworkResponse *out_response)
{
#if defined(CMP_APPLE_USE_CFNETWORK_C)
    return cmp_ios_network_request_cfnetwork(net, request, allocator, out_response);
#else
    return cmp_ios_network_request_foundation(net, request, allocator, out_response);
#endif
}

static int cmp_ios_network_free_response(void *net, const CMPAllocator *allocator, CMPNetworkResponse *response)
{
    CMPIOSBackend *backend;
    int rc;

    if (net == NULL || allocator == NULL || response == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (allocator->alloc == NULL || allocator->realloc == NULL || allocator->free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (response->body_size > 0 && response->body == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)net;
    CMP_UNUSED(backend);

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

static const CMPNetworkVTable g_cmp_ios_network_vtable = {
    cmp_ios_network_request,
    cmp_ios_network_free_response
};

static int cmp_ios_env_get_io(void *env, CMPIO *out_io)
{
    CMPIOSBackend *backend;

    if (env == NULL || out_io == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_io == NULL) {
        return CMP_ERR_STATE;
    }
    return backend->null_env.vtable->get_io(backend->null_env.ctx, out_io);
}

static int cmp_ios_env_get_sensors(void *env, CMPSensors *out_sensors)
{
    CMPIOSBackend *backend;

    if (env == NULL || out_sensors == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_sensors == NULL) {
        return CMP_ERR_STATE;
    }
    return backend->null_env.vtable->get_sensors(backend->null_env.ctx, out_sensors);
}

static int cmp_ios_env_get_camera(void *env, CMPCamera *out_camera)
{
    CMPIOSBackend *backend;

    if (env == NULL || out_camera == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)env;
    *out_camera = backend->camera;
    return CMP_OK;
}

static int cmp_ios_env_get_image(void *env, CMPImage *out_image)
{
    if (env == NULL || out_image == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    out_image->ctx = NULL;
    out_image->vtable = NULL;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_ios_env_get_video(void *env, CMPVideo *out_video)
{
    if (env == NULL || out_video == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    out_video->ctx = NULL;
    out_video->vtable = NULL;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_ios_env_get_audio(void *env, CMPAudio *out_audio)
{
    if (env == NULL || out_audio == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    out_audio->ctx = NULL;
    out_audio->vtable = NULL;
    return CMP_ERR_UNSUPPORTED;
}

static int cmp_ios_env_get_network(void *env, CMPNetwork *out_network)
{
    CMPIOSBackend *backend;

    if (env == NULL || out_network == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)env;
    *out_network = backend->network;
    return CMP_OK;
}

static int cmp_ios_env_get_tasks(void *env, CMPTasks *out_tasks)
{
    CMPIOSBackend *backend;

    if (env == NULL || out_tasks == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_tasks == NULL) {
        return CMP_ERR_STATE;
    }
    return backend->null_env.vtable->get_tasks(backend->null_env.ctx, out_tasks);
}

static int cmp_ios_env_get_time_ms(void *env, cmp_u32 *out_time_ms)
{
    CMPIOSBackend *backend;

    if (env == NULL || out_time_ms == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    backend = (CMPIOSBackend *)env;
    if (backend->null_env.vtable == NULL || backend->null_env.vtable->get_time_ms == NULL) {
        return CMP_ERR_STATE;
    }
    return backend->null_env.vtable->get_time_ms(backend->null_env.ctx, out_time_ms);
}

static const CMPEnvVTable g_cmp_ios_env_vtable = {
    cmp_ios_env_get_io,
    cmp_ios_env_get_sensors,
    cmp_ios_env_get_camera,
    cmp_ios_env_get_image,
    cmp_ios_env_get_video,
    cmp_ios_env_get_audio,
    cmp_ios_env_get_network,
    cmp_ios_env_get_tasks,
    cmp_ios_env_get_time_ms
};

static int cmp_ios_backend_cleanup(CMPAllocator *allocator, CMPIOSBackend *backend, int primary_error)
{
    int rc;
    int result;

    if (allocator == NULL || backend == NULL) {
        return primary_error;
    }

    result = primary_error;

    if (backend->camera_opened == CMP_TRUE) {
        rc = cmp_ios_camera_close(backend);
        if (result == CMP_OK && rc != CMP_OK) {
            result = rc;
        }
    }

    if (backend->null_backend != NULL) {
        rc = cmp_null_backend_destroy(backend->null_backend);
        if (result == CMP_OK && rc != CMP_OK) {
            result = rc;
        }
    }

    rc = allocator->free(allocator->ctx, backend);
    if (result == CMP_OK && rc != CMP_OK) {
        result = rc;
    }

    return result;
}

int CMP_CALL cmp_ios_backend_create(const CMPIOSBackendConfig *config, CMPIOSBackend **out_backend)
{
    CMPIOSBackendConfig local_config;
    CMPAllocator allocator;
    CMPNullBackendConfig null_config;
    CMPIOSBackend *backend;
    int rc;

    if (out_backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = cmp_ios_backend_config_init(&local_config);
        CMP_IOS_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = cmp_ios_backend_validate_config(config);
    CMP_IOS_RETURN_IF_ERROR(rc);

    if (config->allocator == NULL) {
        rc = cmp_get_default_allocator(&allocator);
        CMP_IOS_RETURN_IF_ERROR(rc);
    } else {
        allocator = *config->allocator;
    }

    if (allocator.alloc == NULL || allocator.realloc == NULL || allocator.free == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }

    rc = allocator.alloc(allocator.ctx, sizeof(*backend), (void **)&backend);
    CMP_IOS_RETURN_IF_ERROR(rc);

    memset(backend, 0, sizeof(*backend));
    backend->allocator = allocator;

    rc = cmp_null_backend_config_init(&null_config);
    if (rc != CMP_OK) {
        return cmp_ios_backend_cleanup(&allocator, backend, rc);
    }

    null_config.allocator = &allocator;
    null_config.handle_capacity = config->handle_capacity;
    null_config.clipboard_limit = config->clipboard_limit;
    null_config.enable_logging = config->enable_logging;
    null_config.inline_tasks = config->inline_tasks;

    rc = cmp_null_backend_create(&null_config, &backend->null_backend);
    if (rc != CMP_OK) {
        return cmp_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = cmp_null_backend_get_ws(backend->null_backend, &backend->ws);
    if (rc != CMP_OK) {
        return cmp_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = cmp_null_backend_get_gfx(backend->null_backend, &backend->gfx);
    if (rc != CMP_OK) {
        return cmp_ios_backend_cleanup(&allocator, backend, rc);
    }

    rc = cmp_null_backend_get_env(backend->null_backend, &backend->null_env);
    if (rc != CMP_OK) {
        return cmp_ios_backend_cleanup(&allocator, backend, rc);
    }

    backend->env.ctx = backend;
    backend->env.vtable = &g_cmp_ios_env_vtable;
    backend->camera.ctx = backend;
    backend->camera.vtable = &g_cmp_ios_camera_vtable;
    backend->network.ctx = backend;
    backend->network.vtable = &g_cmp_ios_network_vtable;
    backend->camera_opened = CMP_FALSE;
    backend->camera_streaming = CMP_FALSE;
    backend->camera_has_frame = CMP_FALSE;
    backend->camera_frame = NULL;
    backend->camera_frame_capacity = 0u;
    backend->camera_frame_size = 0u;
    backend->camera_error = CMP_OK;

    backend->predictive_back = config->predictive_back;
    backend->initialized = CMP_TRUE;
    *out_backend = backend;
    return CMP_OK;
}

int CMP_CALL cmp_ios_backend_destroy(CMPIOSBackend *backend)
{
    CMPAllocator allocator;
    int rc;
    int first_error;

    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }

    allocator = backend->allocator;
    first_error = CMP_OK;

    if (backend->camera_opened == CMP_TRUE) {
        rc = cmp_ios_camera_close(backend);
        if (rc != CMP_OK) {
            first_error = rc;
        }
    }

    if (backend->null_backend != NULL) {
        rc = cmp_null_backend_destroy(backend->null_backend);
        if (rc != CMP_OK && first_error == CMP_OK) {
            first_error = rc;
        }
        backend->null_backend = NULL;
    }

    backend->initialized = CMP_FALSE;
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
    if (rc != CMP_OK && first_error == CMP_OK) {
        first_error = rc;
    }

    return first_error;
}

int CMP_CALL cmp_ios_backend_get_ws(CMPIOSBackend *backend, CMPWS *out_ws)
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

int CMP_CALL cmp_ios_backend_get_gfx(CMPIOSBackend *backend, CMPGfx *out_gfx)
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

int CMP_CALL cmp_ios_backend_get_env(CMPIOSBackend *backend, CMPEnv *out_env)
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

int CMP_CALL cmp_ios_backend_set_predictive_back(CMPIOSBackend *backend, CMPPredictiveBack *predictive)
{
    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }
    if (predictive != NULL && predictive->initialized != CMP_TRUE) {
        return CMP_ERR_STATE;
    }
    backend->predictive_back = predictive;
    return CMP_OK;
}

int CMP_CALL cmp_ios_backend_get_predictive_back(CMPIOSBackend *backend, CMPPredictiveBack **out_predictive)
{
    if (backend == NULL || out_predictive == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized) {
        return CMP_ERR_STATE;
    }
    *out_predictive = backend->predictive_back;
    return CMP_OK;
}

int CMP_CALL cmp_ios_backend_predictive_back_start(CMPIOSBackend *backend, const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return CMP_ERR_STATE;
    }
    return cmp_predictive_back_start(backend->predictive_back, event);
}

int CMP_CALL cmp_ios_backend_predictive_back_progress(CMPIOSBackend *backend, const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return CMP_ERR_STATE;
    }
    return cmp_predictive_back_progress(backend->predictive_back, event);
}

int CMP_CALL cmp_ios_backend_predictive_back_commit(CMPIOSBackend *backend, const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return CMP_ERR_STATE;
    }
    return cmp_predictive_back_commit(backend->predictive_back, event);
}

int CMP_CALL cmp_ios_backend_predictive_back_cancel(CMPIOSBackend *backend, const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    if (!backend->initialized || backend->predictive_back == NULL) {
        return CMP_ERR_STATE;
    }
    return cmp_predictive_back_cancel(backend->predictive_back, event);
}

#else

int CMP_CALL cmp_ios_backend_create(const CMPIOSBackendConfig *config, CMPIOSBackend **out_backend)
{
    CMPIOSBackendConfig local_config;
    int rc;

    if (out_backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    *out_backend = NULL;

    if (config == NULL) {
        rc = cmp_ios_backend_config_init(&local_config);
        CMP_IOS_RETURN_IF_ERROR(rc);
        config = &local_config;
    }

    rc = cmp_ios_backend_validate_config(config);
    CMP_IOS_RETURN_IF_ERROR(rc);

    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_destroy(CMPIOSBackend *backend)
{
    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_ws(CMPIOSBackend *backend, CMPWS *out_ws)
{
    if (backend == NULL || out_ws == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(out_ws, 0, sizeof(*out_ws));
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_gfx(CMPIOSBackend *backend, CMPGfx *out_gfx)
{
    if (backend == NULL || out_gfx == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(out_gfx, 0, sizeof(*out_gfx));
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_env(CMPIOSBackend *backend, CMPEnv *out_env)
{
    if (backend == NULL || out_env == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    memset(out_env, 0, sizeof(*out_env));
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_set_predictive_back(CMPIOSBackend *backend,
                                               CMPPredictiveBack *predictive)
{
    if (backend == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    CMP_UNUSED(predictive);
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_get_predictive_back(CMPIOSBackend *backend,
                                               CMPPredictiveBack **out_predictive)
{
    if (backend == NULL || out_predictive == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    *out_predictive = NULL;
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_start(CMPIOSBackend *backend,
                                                 const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_progress(CMPIOSBackend *backend,
                                                    const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_commit(CMPIOSBackend *backend,
                                                  const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_ios_backend_predictive_back_cancel(CMPIOSBackend *backend,
                                                  const CMPPredictiveBackEvent *event)
{
    if (backend == NULL || event == NULL) {
        return CMP_ERR_INVALID_ARGUMENT;
    }
    return CMP_ERR_UNSUPPORTED;
}

#endif
