#include "m3/m3_backend_android.h"

#include <string.h>

#if defined(M3_ANDROID_AVAILABLE)
#include "m3/m3_backend_null.h"
#if defined(__ANDROID_API__) && (__ANDROID_API__ >= 24)
#include <android/native_window.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <errno.h>
#include <media/NdkImageReader.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#define M3_ANDROID_CAMERA2_AVAILABLE 1
#ifndef ACAMERA_LENS_FACING_EXTERNAL
#define ACAMERA_LENS_FACING_EXTERNAL 2
#endif
#else
#define M3_ANDROID_CAMERA2_AVAILABLE 0
#endif
#endif

#define M3_ANDROID_RETURN_IF_ERROR(rc)                                         \
  do {                                                                         \
    if ((rc) != M3_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define M3_ANDROID_DEFAULT_HANDLE_CAPACITY 64u

static int
m3_android_backend_validate_config(const M3AndroidBackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
  }
  if (config->predictive_back != NULL &&
      config->predictive_back->initialized != M3_TRUE) {
    return M3_ERR_STATE;
  }
#if defined(M3_ANDROID_AVAILABLE)
  if (config->java_vm == NULL || config->activity == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#endif
  return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL
m3_android_backend_test_validate_config(const M3AndroidBackendConfig *config) {
  return m3_android_backend_validate_config(config);
}
#endif

int M3_CALL m3_android_backend_is_available(M3Bool *out_available) {
  if (out_available == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
#if defined(M3_ANDROID_AVAILABLE)
  *out_available = M3_TRUE;
#else
  *out_available = M3_FALSE;
#endif
  return M3_OK;
}

int M3_CALL m3_android_backend_config_init(M3AndroidBackendConfig *config) {
  if (config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = M3_ANDROID_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (m3_usize) ~(m3_usize)0;
  config->enable_logging = M3_TRUE;
  config->inline_tasks = M3_TRUE;
  config->predictive_back = NULL;
  config->java_vm = NULL;
  config->jni_env = NULL;
  config->activity = NULL;
  config->asset_manager = NULL;
  config->native_window = NULL;
  config->input_queue = NULL;
  config->looper = NULL;
  return M3_OK;
}

#if defined(M3_ANDROID_AVAILABLE)

#define M3_ANDROID_CAMERA_DEFAULT_WIDTH 640u
#define M3_ANDROID_CAMERA_DEFAULT_HEIGHT 480u

typedef struct M3AndroidCameraState M3AndroidCameraState;

struct M3AndroidBackend;

struct M3AndroidCameraState {
  struct M3AndroidBackend *backend;
  m3_u32 camera_id;
  m3_u32 width;
  m3_u32 height;
  m3_u32 format;
  m3_usize frame_capacity;
  m3_usize frame_size;
  m3_u8 *frame_data;
  M3Bool has_frame;
  M3Bool opened;
  M3Bool streaming;
  int last_error;
#if M3_ANDROID_CAMERA2_AVAILABLE
  ACameraManager *manager;
  ACameraDevice *device;
  ACameraCaptureSession *session;
  ACaptureRequest *request;
  AImageReader *reader;
  ANativeWindow *reader_window;
  ACameraOutputTarget *output_target;
  ACaptureSessionOutputContainer *output_container;
  ACaptureSessionOutput *session_output;
  pthread_mutex_t mutex;
  M3Bool mutex_initialized;
#endif
};

struct M3AndroidBackend {
  M3Allocator allocator;
  M3NullBackend *null_backend;
  M3WS ws;
  M3Gfx gfx;
  M3Env env;
  M3Env null_env;
  M3Camera camera;
  M3AndroidCameraState camera_state;
  M3PredictiveBack *predictive_back;
  M3Bool initialized;
  void *java_vm;
  void *jni_env;
  void *activity;
  void *asset_manager;
  void *native_window;
  void *input_queue;
  void *looper;
};

#if M3_ANDROID_CAMERA2_AVAILABLE
static int m3_android_camera_mul_overflow(m3_usize a, m3_usize b,
                                          m3_usize *out_value) {
  m3_usize max_value;

  if (out_value == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  max_value = (m3_usize) ~(m3_usize)0;
  if (a != 0u && b > max_value / a) {
    return M3_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return M3_OK;
}
#endif

#if M3_ANDROID_CAMERA2_AVAILABLE
static int m3_android_camera_status_to_error(camera_status_t status) {
  switch (status) {
  case ACAMERA_OK:
    return M3_OK;
  case ACAMERA_ERROR_PERMISSION_DENIED:
    return M3_ERR_PERMISSION;
  case ACAMERA_ERROR_CAMERA_IN_USE:
  case ACAMERA_ERROR_MAX_CAMERAS_IN_USE:
    return M3_ERR_BUSY;
  case ACAMERA_ERROR_CAMERA_DISCONNECTED:
    return M3_ERR_NOT_READY;
  case ACAMERA_ERROR_INVALID_PARAMETER:
    return M3_ERR_INVALID_ARGUMENT;
  default:
    return M3_ERR_UNKNOWN;
  }
}

static int m3_android_camera_lock(M3AndroidCameraState *state) {
  if (state == NULL || state->mutex_initialized != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (pthread_mutex_lock(&state->mutex) != 0) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int m3_android_camera_unlock(M3AndroidCameraState *state) {
  if (state == NULL || state->mutex_initialized != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (pthread_mutex_unlock(&state->mutex) != 0) {
    return M3_ERR_UNKNOWN;
  }
  return M3_OK;
}

static int m3_android_camera_copy_image(M3AndroidCameraState *state,
                                        AImage *image) {
  unsigned char *y_plane;
  unsigned char *u_plane;
  unsigned char *v_plane;
  int y_len;
  int u_len;
  int v_len;
  int y_row_stride;
  int u_row_stride;
  int v_row_stride;
  int u_pixel_stride;
  int v_pixel_stride;
  int width;
  int height;
  m3_usize needed;
  m3_usize y_size;
  m3_usize uv_size;
  m3_usize row_bytes;
  m3_usize row;
  m3_usize col;
  int rc;

  if (state == NULL || image == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (AImage_getWidth(image, &width) != AMEDIA_OK ||
      AImage_getHeight(image, &height) != AMEDIA_OK) {
    return M3_ERR_IO;
  }

  rc = m3_android_camera_mul_overflow((m3_usize)width, (m3_usize)height,
                                      &y_size);
  if (rc != M3_OK) {
    return rc;
  }
  uv_size = y_size / 2u;
  needed = y_size + uv_size;

  if (AImage_getPlaneData(image, 0, (uint8_t **)&y_plane, &y_len) !=
      AMEDIA_OK) {
    return M3_ERR_IO;
  }
  if (AImage_getPlaneData(image, 1, (uint8_t **)&u_plane, &u_len) !=
      AMEDIA_OK) {
    return M3_ERR_IO;
  }
  if (AImage_getPlaneData(image, 2, (uint8_t **)&v_plane, &v_len) !=
      AMEDIA_OK) {
    return M3_ERR_IO;
  }
  if (AImage_getPlaneRowStride(image, 0, &y_row_stride) != AMEDIA_OK ||
      AImage_getPlaneRowStride(image, 1, &u_row_stride) != AMEDIA_OK ||
      AImage_getPlaneRowStride(image, 2, &v_row_stride) != AMEDIA_OK ||
      AImage_getPlanePixelStride(image, 1, &u_pixel_stride) != AMEDIA_OK ||
      AImage_getPlanePixelStride(image, 2, &v_pixel_stride) != AMEDIA_OK) {
    return M3_ERR_IO;
  }

  if (state->backend == NULL) {
    return M3_ERR_STATE;
  }
  if (state->backend->allocator.alloc == NULL ||
      state->backend->allocator.realloc == NULL ||
      state->backend->allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (state->frame_capacity < needed) {
    m3_u8 *new_frame;
    rc = state->backend->allocator.realloc(state->backend->allocator.ctx,
                                           state->frame_data, needed,
                                           (void **)&new_frame);
    if (rc != M3_OK) {
      return rc;
    }
    state->frame_data = new_frame;
    state->frame_capacity = needed;
  }

  row_bytes = (m3_usize)width;
  for (row = 0u; row < (m3_usize)height; row += 1u) {
    memcpy(state->frame_data + row * row_bytes,
           y_plane + row * (m3_usize)y_row_stride, (size_t)row_bytes);
  }

  for (row = 0u; row < (m3_usize)(height / 2); row += 1u) {
    for (col = 0u; col < (m3_usize)(width / 2); col += 1u) {
      m3_usize uv_index = y_size + row * (m3_usize)width + col * 2u;
      state->frame_data[uv_index] = u_plane[row * (m3_usize)u_row_stride +
                                            col * (m3_usize)u_pixel_stride];
      state->frame_data[uv_index + 1u] =
          v_plane[row * (m3_usize)v_row_stride +
                  col * (m3_usize)v_pixel_stride];
    }
  }

  state->frame_size = needed;
  state->format = M3_CAMERA_FORMAT_NV12;
  state->width = (m3_u32)width;
  state->height = (m3_u32)height;
  state->has_frame = M3_TRUE;
  return M3_OK;
}

static void m3_android_camera_on_image(void *context, AImageReader *reader) {
  M3AndroidCameraState *state;
  AImage *image;
  int rc;
  int lock_rc;

  (void)reader;

  if (context == NULL) {
    return;
  }

  state = (M3AndroidCameraState *)context;
  if (AImageReader_acquireLatestImage(reader, &image) != AMEDIA_OK) {
    return;
  }

  lock_rc = m3_android_camera_lock(state);
  if (lock_rc != M3_OK) {
    AImage_delete(image);
    return;
  }

  rc = m3_android_camera_copy_image(state, image);
  if (rc != M3_OK) {
    state->last_error = rc;
  } else {
    state->last_error = M3_OK;
  }

  m3_android_camera_unlock(state);
  AImage_delete(image);
}

static void m3_android_camera_on_opened(void *context, ACameraDevice *device) {
  M3AndroidCameraState *state;

  if (context == NULL) {
    return;
  }
  state = (M3AndroidCameraState *)context;
  if (m3_android_camera_lock(state) != M3_OK) {
    return;
  }
  state->device = device;
  state->opened = M3_TRUE;
  state->last_error = M3_OK;
  m3_android_camera_unlock(state);
}

static void m3_android_camera_on_disconnected(void *context,
                                              ACameraDevice *device) {
  M3AndroidCameraState *state;

  (void)device;

  if (context == NULL) {
    return;
  }
  state = (M3AndroidCameraState *)context;
  if (m3_android_camera_lock(state) != M3_OK) {
    return;
  }
  state->opened = M3_FALSE;
  state->streaming = M3_FALSE;
  state->last_error = M3_ERR_NOT_READY;
  m3_android_camera_unlock(state);
}

static void m3_android_camera_on_error(void *context, ACameraDevice *device,
                                       int error) {
  M3AndroidCameraState *state;

  (void)device;
  (void)error;

  if (context == NULL) {
    return;
  }
  state = (M3AndroidCameraState *)context;
  if (m3_android_camera_lock(state) != M3_OK) {
    return;
  }
  state->opened = M3_FALSE;
  state->streaming = M3_FALSE;
  state->last_error = M3_ERR_IO;
  m3_android_camera_unlock(state);
}
#endif

#if M3_ANDROID_CAMERA2_AVAILABLE
static int m3_android_camera_select_index(ACameraManager *manager,
                                          const M3CameraConfig *config,
                                          const ACameraIdList *id_list,
                                          m3_u32 *out_index) {
  m3_u32 desired;
  m3_u32 i;

  if (manager == NULL || config == NULL || id_list == NULL ||
      out_index == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (id_list->numCameras <= 0) {
    return M3_ERR_NOT_FOUND;
  }

  if (config->camera_id != M3_CAMERA_ID_DEFAULT) {
    if (config->camera_id >= (m3_u32)id_list->numCameras) {
      return M3_ERR_NOT_FOUND;
    }
    *out_index = config->camera_id;
    return M3_OK;
  }

  if (config->facing == M3_CAMERA_FACING_UNSPECIFIED) {
    *out_index = 0u;
    return M3_OK;
  }

  switch (config->facing) {
  case M3_CAMERA_FACING_FRONT:
    desired = (m3_u32)ACAMERA_LENS_FACING_FRONT;
    break;
  case M3_CAMERA_FACING_BACK:
    desired = (m3_u32)ACAMERA_LENS_FACING_BACK;
    break;
  case M3_CAMERA_FACING_EXTERNAL:
    desired = (m3_u32)ACAMERA_LENS_FACING_EXTERNAL;
    break;
  case M3_CAMERA_FACING_UNSPECIFIED:
  default:
    desired = (m3_u32)ACAMERA_LENS_FACING_BACK;
    break;
  }

  for (i = 0u; i < (m3_u32)id_list->numCameras; i += 1u) {
    ACameraMetadata *metadata;
    ACameraMetadata_const_entry entry;
    camera_status_t status;

    metadata = NULL;
    status = ACameraManager_getCameraCharacteristics(
        manager, id_list->cameraIds[i], &metadata);
    if (status != ACAMERA_OK || metadata == NULL) {
      if (metadata != NULL) {
        ACameraMetadata_free(metadata);
      }
      continue;
    }

    if (ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &entry) ==
            ACAMERA_OK &&
        entry.count > 0u) {
      m3_u32 lens_facing = (m3_u32)entry.data.u8[0];
      if (lens_facing == desired) {
        ACameraMetadata_free(metadata);
        *out_index = i;
        return M3_OK;
      }
    }

    ACameraMetadata_free(metadata);
  }

  return M3_ERR_NOT_FOUND;
}
#endif

static int m3_android_camera_open_with_config(void *camera,
                                              const M3CameraConfig *config);

static int m3_android_camera_open(void *camera, m3_u32 camera_id) {
  M3CameraConfig config;

  config.camera_id = camera_id;
  config.facing = M3_CAMERA_FACING_UNSPECIFIED;
  config.width = 0u;
  config.height = 0u;
  config.format = M3_CAMERA_FORMAT_ANY;

  return m3_android_camera_open_with_config(camera, &config);
}

static int m3_android_camera_open_with_config(void *camera,
                                              const M3CameraConfig *config) {
  struct M3AndroidBackend *backend;
  M3AndroidCameraState *state;

  if (camera == NULL || config == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)camera;
  state = &backend->camera_state;

#if M3_ANDROID_CAMERA2_AVAILABLE
  if (state->opened == M3_TRUE || state->streaming == M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (config->format != M3_CAMERA_FORMAT_ANY &&
      config->format != M3_CAMERA_FORMAT_NV12) {
    return M3_ERR_UNSUPPORTED;
  }

  state->backend = backend;
  state->width =
      (config->width == 0u) ? M3_ANDROID_CAMERA_DEFAULT_WIDTH : config->width;
  state->height = (config->height == 0u) ? M3_ANDROID_CAMERA_DEFAULT_HEIGHT
                                         : config->height;
  state->format = M3_CAMERA_FORMAT_NV12;
  state->has_frame = M3_FALSE;
  state->frame_size = 0u;
  state->last_error = M3_OK;

  if (state->mutex_initialized != M3_TRUE) {
    if (pthread_mutex_init(&state->mutex, NULL) != 0) {
      return M3_ERR_UNKNOWN;
    }
    state->mutex_initialized = M3_TRUE;
  }

  state->manager = ACameraManager_create();
  if (state->manager == NULL) {
    return M3_ERR_UNKNOWN;
  }

  {
    ACameraIdList *id_list;
    AImageReader_ImageListener listener;
    camera_status_t status;
    m3_u32 selected_index;
    int rc;

    status = ACameraManager_getCameraIdList(state->manager, &id_list);
    if (status != ACAMERA_OK) {
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return m3_android_camera_status_to_error(status);
    }

    rc = m3_android_camera_select_index(state->manager, config, id_list,
                                        &selected_index);
    if (rc != M3_OK) {
      ACameraManager_deleteCameraIdList(id_list);
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return rc;
    }

    status = AImageReader_new((int)state->width, (int)state->height,
                              AIMAGE_FORMAT_YUV_420_888, 2, &state->reader);
    if (status != ACAMERA_OK) {
      ACameraManager_deleteCameraIdList(id_list);
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return m3_android_camera_status_to_error(status);
    }

    status = AImageReader_getWindow(state->reader, &state->reader_window);
    if (status != AMEDIA_OK || state->reader_window == NULL) {
      AImageReader_delete(state->reader);
      state->reader = NULL;
      ACameraManager_deleteCameraIdList(id_list);
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return M3_ERR_IO;
    }

    listener.context = state;
    listener.onImageAvailable = m3_android_camera_on_image;
    if (AImageReader_setImageListener(state->reader, &listener) != AMEDIA_OK) {
      AImageReader_delete(state->reader);
      state->reader = NULL;
      ACameraManager_deleteCameraIdList(id_list);
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return M3_ERR_IO;
    }

    {
      ACameraDevice_StateCallbacks callbacks;
      callbacks.context = state;
      callbacks.onOpened = m3_android_camera_on_opened;
      callbacks.onDisconnected = m3_android_camera_on_disconnected;
      callbacks.onError = m3_android_camera_on_error;

      status = ACameraManager_openCamera(state->manager,
                                         id_list->cameraIds[selected_index],
                                         &callbacks, &state->device);
      rc = m3_android_camera_status_to_error(status);
      ACameraManager_deleteCameraIdList(id_list);
      if (rc != M3_OK) {
        if (state->reader != NULL) {
          AImageReader_delete(state->reader);
          state->reader = NULL;
        }
        if (state->manager != NULL) {
          ACameraManager_delete(state->manager);
          state->manager = NULL;
        }
        return rc;
      }
    }

    state->camera_id = selected_index;
  }

  state->opened = M3_TRUE;
  return M3_OK;
#else
  M3_UNUSED(config);
  M3_UNUSED(state);
  M3_UNUSED(backend);
  return M3_ERR_UNSUPPORTED;
#endif
}

#if M3_ANDROID_CAMERA2_AVAILABLE
static int m3_android_camera_stop_internal(M3AndroidCameraState *state) {
  if (state == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  if (state->session != NULL) {
    ACameraCaptureSession_stopRepeating(state->session);
    ACameraCaptureSession_close(state->session);
    state->session = NULL;
  }
  if (state->request != NULL) {
    ACaptureRequest_free(state->request);
    state->request = NULL;
  }
  if (state->output_target != NULL) {
    ACameraOutputTarget_free(state->output_target);
    state->output_target = NULL;
  }
  if (state->session_output != NULL) {
    ACaptureSessionOutput_free(state->session_output);
    state->session_output = NULL;
  }
  if (state->output_container != NULL) {
    ACaptureSessionOutputContainer_free(state->output_container);
    state->output_container = NULL;
  }

  state->streaming = M3_FALSE;
  return M3_OK;
}
#endif

static int m3_android_camera_close(void *camera) {
  struct M3AndroidBackend *backend;
  M3AndroidCameraState *state;
  int rc = 0;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)camera;
  state = &backend->camera_state;

#if M3_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != M3_TRUE) {
    return M3_ERR_STATE;
  }

  if (state->streaming == M3_TRUE) {
    rc = m3_android_camera_stop_internal(state);
    if (rc != M3_OK) {
      return rc;
    }
  }

  if (state->device != NULL) {
    ACameraDevice_close(state->device);
    state->device = NULL;
  }
  if (state->reader != NULL) {
    AImageReader_delete(state->reader);
    state->reader = NULL;
  }
  if (state->manager != NULL) {
    ACameraManager_delete(state->manager);
    state->manager = NULL;
  }

  if (state->frame_data != NULL) {
    if (backend->allocator.free == NULL) {
      return M3_ERR_INVALID_ARGUMENT;
    }
    rc = backend->allocator.free(backend->allocator.ctx, state->frame_data);
    if (rc != M3_OK) {
      return rc;
    }
    state->frame_data = NULL;
    state->frame_capacity = 0u;
    state->frame_size = 0u;
  }

  if (state->mutex_initialized == M3_TRUE) {
    pthread_mutex_destroy(&state->mutex);
    state->mutex_initialized = M3_FALSE;
  }

  state->opened = M3_FALSE;
  state->streaming = M3_FALSE;
  state->has_frame = M3_FALSE;
  state->last_error = M3_OK;
  return M3_OK;
#else
  M3_UNUSED(state);
  M3_UNUSED(backend);
  M3_UNUSED(rc);
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_android_camera_start(void *camera) {
  struct M3AndroidBackend *backend;
  M3AndroidCameraState *state;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)camera;
  state = &backend->camera_state;

#if M3_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (state->streaming == M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (state->device == NULL || state->reader_window == NULL) {
    return M3_ERR_NOT_READY;
  }

  {
    camera_status_t status;
    ACameraCaptureSession_stateCallbacks callbacks;

    status = ACaptureSessionOutputContainer_create(&state->output_container);
    if (status != ACAMERA_OK) {
      return m3_android_camera_status_to_error(status);
    }

    status = ACaptureSessionOutput_create(state->reader_window,
                                          &state->session_output);
    if (status != ACAMERA_OK) {
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return m3_android_camera_status_to_error(status);
    }

    status = ACaptureSessionOutputContainer_add(state->output_container,
                                                state->session_output);
    if (status != ACAMERA_OK) {
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return m3_android_camera_status_to_error(status);
    }

    status =
        ACameraOutputTarget_create(state->reader_window, &state->output_target);
    if (status != ACAMERA_OK) {
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return m3_android_camera_status_to_error(status);
    }

    status = ACameraDevice_createCaptureRequest(state->device, TEMPLATE_PREVIEW,
                                                &state->request);
    if (status != ACAMERA_OK) {
      ACameraOutputTarget_free(state->output_target);
      state->output_target = NULL;
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return m3_android_camera_status_to_error(status);
    }

    status = ACaptureRequest_addTarget(state->request, state->output_target);
    if (status != ACAMERA_OK) {
      ACaptureRequest_free(state->request);
      state->request = NULL;
      ACameraOutputTarget_free(state->output_target);
      state->output_target = NULL;
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return m3_android_camera_status_to_error(status);
    }

    callbacks.context = state;
    callbacks.onActive = NULL;
    callbacks.onReady = NULL;
    callbacks.onClosed = NULL;

    status = ACameraDevice_createCaptureSession(
        state->device, state->output_container, &callbacks, &state->session);
    if (status != ACAMERA_OK) {
      ACaptureRequest_free(state->request);
      state->request = NULL;
      ACameraOutputTarget_free(state->output_target);
      state->output_target = NULL;
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return m3_android_camera_status_to_error(status);
    }

    status = ACameraCaptureSession_setRepeatingRequest(state->session, NULL, 1,
                                                       &state->request, NULL);
    if (status != ACAMERA_OK) {
      m3_android_camera_stop_internal(state);
      return m3_android_camera_status_to_error(status);
    }
  }

  state->streaming = M3_TRUE;
  return M3_OK;
#else
  M3_UNUSED(state);
  M3_UNUSED(backend);
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_android_camera_stop(void *camera) {
  struct M3AndroidBackend *backend;
  M3AndroidCameraState *state;

  if (camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)camera;
  state = &backend->camera_state;

#if M3_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (state->streaming != M3_TRUE) {
    return M3_ERR_STATE;
  }

  return m3_android_camera_stop_internal(state);
#else
  M3_UNUSED(state);
  M3_UNUSED(backend);
  return M3_ERR_UNSUPPORTED;
#endif
}

static int m3_android_camera_read_frame(void *camera, M3CameraFrame *out_frame,
                                        M3Bool *out_has_frame) {
  struct M3AndroidBackend *backend;
  M3AndroidCameraState *state;
  int rc = 0;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)camera;
  state = &backend->camera_state;

#if M3_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != M3_TRUE) {
    return M3_ERR_STATE;
  }
  if (state->streaming != M3_TRUE) {
    return M3_ERR_STATE;
  }

  rc = m3_android_camera_lock(state);
  if (rc != M3_OK) {
    return rc;
  }

  if (state->last_error != M3_OK) {
    rc = state->last_error;
    state->last_error = M3_OK;
    m3_android_camera_unlock(state);
    return rc;
  }

  if (state->has_frame != M3_TRUE || state->frame_data == NULL) {
    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = M3_FALSE;
    m3_android_camera_unlock(state);
    return M3_OK;
  }

  out_frame->format = state->format;
  out_frame->width = state->width;
  out_frame->height = state->height;
  out_frame->data = state->frame_data;
  out_frame->size = state->frame_size;
  *out_has_frame = M3_TRUE;
  m3_android_camera_unlock(state);
  return M3_OK;
#else
  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = M3_FALSE;
  M3_UNUSED(state);
  M3_UNUSED(backend);
  M3_UNUSED(rc);
  return M3_ERR_UNSUPPORTED;
#endif
}

static const M3CameraVTable g_m3_android_camera_vtable = {
    m3_android_camera_open,  m3_android_camera_open_with_config,
    m3_android_camera_close, m3_android_camera_start,
    m3_android_camera_stop,  m3_android_camera_read_frame};

static int m3_android_env_get_io(void *env, M3IO *out_io) {
  struct M3AndroidBackend *backend;

  if (env == NULL || out_io == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_io == NULL) {
    return M3_ERR_STATE;
  }
  return backend->null_env.vtable->get_io(backend->null_env.ctx, out_io);
}

static int m3_android_env_get_sensors(void *env, M3Sensors *out_sensors) {
  struct M3AndroidBackend *backend;

  if (env == NULL || out_sensors == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_sensors == NULL) {
    return M3_ERR_STATE;
  }
  return backend->null_env.vtable->get_sensors(backend->null_env.ctx,
                                               out_sensors);
}

static int m3_android_env_get_camera(void *env, M3Camera *out_camera) {
  struct M3AndroidBackend *backend;

  if (env == NULL || out_camera == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)env;
  *out_camera = backend->camera;
  return M3_OK;
}

static int m3_android_env_get_network(void *env, M3Network *out_network) {
  struct M3AndroidBackend *backend;

  if (env == NULL || out_network == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_network == NULL) {
    return M3_ERR_STATE;
  }
  return backend->null_env.vtable->get_network(backend->null_env.ctx,
                                               out_network);
}

static int m3_android_env_get_tasks(void *env, M3Tasks *out_tasks) {
  struct M3AndroidBackend *backend;

  if (env == NULL || out_tasks == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_tasks == NULL) {
    return M3_ERR_STATE;
  }
  return backend->null_env.vtable->get_tasks(backend->null_env.ctx, out_tasks);
}

static int m3_android_env_get_time_ms(void *env, m3_u32 *out_time_ms) {
  struct M3AndroidBackend *backend;

  if (env == NULL || out_time_ms == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  backend = (struct M3AndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_time_ms == NULL) {
    return M3_ERR_STATE;
  }
  return backend->null_env.vtable->get_time_ms(backend->null_env.ctx,
                                               out_time_ms);
}

static const M3EnvVTable g_m3_android_env_vtable = {
    m3_android_env_get_io,     m3_android_env_get_sensors,
    m3_android_env_get_camera, m3_android_env_get_network,
    m3_android_env_get_tasks,  m3_android_env_get_time_ms};

int M3_CALL m3_android_backend_create(const M3AndroidBackendConfig *config,
                                      M3AndroidBackend **out_backend) {
  M3AndroidBackendConfig local_config;
  M3NullBackendConfig null_config;
  M3Allocator allocator;
  struct M3AndroidBackend *backend;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_android_backend_config_init(&local_config);
    M3_ANDROID_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_android_backend_validate_config(config);
  M3_ANDROID_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = m3_get_default_allocator(&allocator);
    M3_ANDROID_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(*backend), (void **)&backend);
  M3_ANDROID_RETURN_IF_ERROR(rc);

  memset(backend, 0, sizeof(*backend));
  backend->allocator = allocator;
  backend->java_vm = config->java_vm;
  backend->jni_env = config->jni_env;
  backend->activity = config->activity;
  backend->asset_manager = config->asset_manager;
  backend->native_window = config->native_window;
  backend->input_queue = config->input_queue;
  backend->looper = config->looper;
  backend->predictive_back = config->predictive_back;

  null_config.allocator = &backend->allocator;
  null_config.handle_capacity = config->handle_capacity;
  null_config.clipboard_limit = config->clipboard_limit;
  null_config.enable_logging = config->enable_logging;
  null_config.inline_tasks = config->inline_tasks;

  rc = m3_null_backend_create(&null_config, &backend->null_backend);
  if (rc != M3_OK) {
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = m3_null_backend_get_ws(backend->null_backend, &backend->ws);
  if (rc != M3_OK) {
    m3_null_backend_destroy(backend->null_backend);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = m3_null_backend_get_gfx(backend->null_backend, &backend->gfx);
  if (rc != M3_OK) {
    m3_null_backend_destroy(backend->null_backend);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = m3_null_backend_get_env(backend->null_backend, &backend->null_env);
  if (rc != M3_OK) {
    m3_null_backend_destroy(backend->null_backend);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  backend->env.ctx = backend;
  backend->env.vtable = &g_m3_android_env_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_m3_android_camera_vtable;
  backend->camera_state.backend = backend;
  backend->camera_state.frame_data = NULL;
  backend->camera_state.frame_capacity = 0u;
  backend->camera_state.frame_size = 0u;
  backend->camera_state.has_frame = M3_FALSE;
  backend->camera_state.opened = M3_FALSE;
  backend->camera_state.streaming = M3_FALSE;
  backend->camera_state.last_error = M3_OK;

  backend->initialized = M3_TRUE;
  *out_backend = backend;
  return M3_OK;
}

int M3_CALL m3_android_backend_destroy(M3AndroidBackend *backend) {
  M3Allocator allocator;
  int first_error;
  int rc;

  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  allocator = backend->allocator;
  first_error = M3_OK;

  if (backend->camera_state.opened == M3_TRUE) {
    rc = m3_android_camera_close(backend);
    if (rc != M3_OK) {
      first_error = rc;
    }
  }

  if (backend->null_backend != NULL) {
    rc = m3_null_backend_destroy(backend->null_backend);
    if (rc != M3_OK) {
      first_error = rc;
    }
    backend->null_backend = NULL;
  }
  backend->predictive_back = NULL;

  if (allocator.free == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  rc = allocator.free(allocator.ctx, backend);
  if (rc != M3_OK && first_error == M3_OK) {
    first_error = rc;
  }

  return first_error;
}

int M3_CALL m3_android_backend_get_ws(M3AndroidBackend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_ws = backend->ws;
  return M3_OK;
}

int M3_CALL m3_android_backend_get_gfx(M3AndroidBackend *backend,
                                       M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_gfx = backend->gfx;
  return M3_OK;
}

int M3_CALL m3_android_backend_get_env(M3AndroidBackend *backend,
                                       M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_env = backend->env;
  return M3_OK;
}

int M3_CALL m3_android_backend_set_predictive_back(
    M3AndroidBackend *backend, M3PredictiveBack *predictive) {
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

int M3_CALL m3_android_backend_get_predictive_back(
    M3AndroidBackend *backend, M3PredictiveBack **out_predictive) {
  if (backend == NULL || out_predictive == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return M3_ERR_STATE;
  }
  *out_predictive = backend->predictive_back;
  return M3_OK;
}

int M3_CALL m3_android_backend_predictive_back_start(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return M3_ERR_STATE;
  }
  return m3_predictive_back_start(backend->predictive_back, event);
}

int M3_CALL m3_android_backend_predictive_back_progress(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return M3_ERR_STATE;
  }
  return m3_predictive_back_progress(backend->predictive_back, event);
}

int M3_CALL m3_android_backend_predictive_back_commit(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return M3_ERR_STATE;
  }
  return m3_predictive_back_commit(backend->predictive_back, event);
}

int M3_CALL m3_android_backend_predictive_back_cancel(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return M3_ERR_STATE;
  }
  return m3_predictive_back_cancel(backend->predictive_back, event);
}

#else

int M3_CALL m3_android_backend_create(const M3AndroidBackendConfig *config,
                                      M3AndroidBackend **out_backend) {
  M3AndroidBackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = m3_android_backend_config_init(&local_config);
    M3_ANDROID_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = m3_android_backend_validate_config(config);
  M3_ANDROID_RETURN_IF_ERROR(rc);

  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_destroy(M3AndroidBackend *backend) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_get_ws(M3AndroidBackend *backend, M3WS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_get_gfx(M3AndroidBackend *backend,
                                       M3Gfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_get_env(M3AndroidBackend *backend,
                                       M3Env *out_env) {
  if (backend == NULL || out_env == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_set_predictive_back(
    M3AndroidBackend *backend, M3PredictiveBack *predictive) {
  if (backend == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  M3_UNUSED(predictive);
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_get_predictive_back(
    M3AndroidBackend *backend, M3PredictiveBack **out_predictive) {
  if (backend == NULL || out_predictive == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_predictive = NULL;
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_predictive_back_start(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_predictive_back_progress(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_predictive_back_commit(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

int M3_CALL m3_android_backend_predictive_back_cancel(
    M3AndroidBackend *backend, const M3PredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  return M3_ERR_UNSUPPORTED;
}

#endif
