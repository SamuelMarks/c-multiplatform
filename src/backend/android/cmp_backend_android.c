#include "cmpc/cmp_backend_android.h"

#include <string.h>

#if defined(CMP_ANDROID_AVAILABLE)
#include "cmpc/cmp_backend_null.h"
#if defined(__ANDROID_API__) && (__ANDROID_API__ >= 24)
#include <android/native_window.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <errno.h>
#include <media/NdkImageReader.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#define CMP_ANDROID_CAMERA2_AVAILABLE 1
#ifndef ACAMERA_LENS_FACING_EXTERNAL
#define ACAMERA_LENS_FACING_EXTERNAL 2
#endif
#else
#define CMP_ANDROID_CAMERA2_AVAILABLE 0
#endif
#endif

#define CMP_ANDROID_RETURN_IF_ERROR(rc)                                         \
  do {                                                                         \
    if ((rc) != CMP_OK) {                                                       \
      return (rc);                                                             \
    }                                                                          \
  } while (0)

#define CMP_ANDROID_DEFAULT_HANDLE_CAPACITY 64u

static int
cmp_android_backend_validate_config(const CMPAndroidBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->handle_capacity == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (config->allocator != NULL) {
    if (config->allocator->alloc == NULL ||
        config->allocator->realloc == NULL || config->allocator->free == NULL) {
      return CMP_ERR_INVALID_ARGUMENT;
    }
  }
  if (config->predictive_back != NULL &&
      config->predictive_back->initialized != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
/* GCOVR_EXCL_START */
#if defined(CMP_ANDROID_AVAILABLE)
  if (config->java_vm == NULL || config->activity == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#endif
  /* GCOVR_EXCL_STOP */
  return CMP_OK; /* GCOVR_EXCL_LINE */
}

#ifdef CMP_TESTING
int CMP_CALL
cmp_android_backend_test_validate_config(const CMPAndroidBackendConfig *config) {
  return cmp_android_backend_validate_config(config);
}
#endif

int CMP_CALL cmp_android_backend_is_available(CMPBool *out_available) {
  if (out_available == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#if defined(CMP_ANDROID_AVAILABLE)
  *out_available = CMP_TRUE;
#else
  *out_available = CMP_FALSE;
#endif
  return CMP_OK;
}

int CMP_CALL cmp_android_backend_config_init(CMPAndroidBackendConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->allocator = NULL;
  config->handle_capacity = CMP_ANDROID_DEFAULT_HANDLE_CAPACITY;
  config->clipboard_limit = (cmp_usize) ~(cmp_usize)0;
  config->enable_logging = CMP_TRUE;
  config->inline_tasks = CMP_TRUE;
  config->predictive_back = NULL;
  config->java_vm = NULL;
  config->jni_env = NULL;
  config->activity = NULL;
  config->asset_manager = NULL;
  config->native_window = NULL;
  config->input_queue = NULL;
  config->looper = NULL;
  return CMP_OK;
}

/* GCOVR_EXCL_START */
#if defined(CMP_ANDROID_AVAILABLE)

#define CMP_ANDROID_CAMERA_DEFAULT_WIDTH 640u
#define CMP_ANDROID_CAMERA_DEFAULT_HEIGHT 480u

typedef struct CMPAndroidCameraState CMPAndroidCameraState;

struct CMPAndroidBackend;

struct CMPAndroidCameraState {
  struct CMPAndroidBackend *backend;
  cmp_u32 camera_id;
  cmp_u32 width;
  cmp_u32 height;
  cmp_u32 format;
  cmp_usize frame_capacity;
  cmp_usize frame_size;
  cmp_u8 *frame_data;
  CMPBool has_frame;
  CMPBool opened;
  CMPBool streaming;
  int last_error;
#if CMP_ANDROID_CAMERA2_AVAILABLE
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
  CMPBool mutex_initialized;
#endif
};

struct CMPAndroidBackend {
  CMPAllocator allocator;
  CMPNullBackend *null_backend;
  CMPWS ws;
  CMPGfx gfx;
  CMPEnv env;
  CMPEnv null_env;
  CMPCamera camera;
  CMPAndroidCameraState camera_state;
  CMPPredictiveBack *predictive_back;
  CMPBool initialized;
  void *java_vm;
  void *jni_env;
  void *activity;
  void *asset_manager;
  void *native_window;
  void *input_queue;
  void *looper;
};

#if CMP_ANDROID_CAMERA2_AVAILABLE
static int cmp_android_camera_mul_overflow(cmp_usize a, cmp_usize b,
                                          cmp_usize *out_value) {
  cmp_usize max_value;

  if (out_value == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  max_value = (cmp_usize) ~(cmp_usize)0;
  if (a != 0u && b > max_value / a) {
    return CMP_ERR_OVERFLOW;
  }

  *out_value = a * b;
  return CMP_OK;
}
#endif

#if CMP_ANDROID_CAMERA2_AVAILABLE
static int cmp_android_camera_status_to_error(camera_status_t status) {
  switch (status) {
  case ACAMERA_OK:
    return CMP_OK;
  case ACAMERA_ERROR_PERMISSION_DENIED:
    return CMP_ERR_PERMISSION;
  case ACAMERA_ERROR_CAMERA_IN_USE:
  case ACAMERA_ERROR_MAX_CAMERAS_IN_USE:
    return CMP_ERR_BUSY;
  case ACAMERA_ERROR_CAMERA_DISCONNECTED:
    return CMP_ERR_NOT_READY;
  case ACAMERA_ERROR_INVALID_PARAMETER:
    return CMP_ERR_INVALID_ARGUMENT;
  default:
    return CMP_ERR_UNKNOWN;
  }
}

static int cmp_android_camera_lock(CMPAndroidCameraState *state) {
  if (state == NULL || state->mutex_initialized != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (pthread_mutex_lock(&state->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int cmp_android_camera_unlock(CMPAndroidCameraState *state) {
  if (state == NULL || state->mutex_initialized != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (pthread_mutex_unlock(&state->mutex) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

static int cmp_android_camera_copy_image(CMPAndroidCameraState *state,
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
  cmp_usize needed;
  cmp_usize y_size;
  cmp_usize uv_size;
  cmp_usize row_bytes;
  cmp_usize row;
  cmp_usize col;
  int rc;

  if (state == NULL || image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (AImage_getWidth(image, &width) != AMEDIA_OK ||
      AImage_getHeight(image, &height) != AMEDIA_OK) {
    return CMP_ERR_IO;
  }

  rc = cmp_android_camera_mul_overflow((cmp_usize)width, (cmp_usize)height,
                                      &y_size);
  if (rc != CMP_OK) {
    return rc;
  }
  uv_size = y_size / 2u;
  needed = y_size + uv_size;

  if (AImage_getPlaneData(image, 0, (uint8_t **)&y_plane, &y_len) !=
      AMEDIA_OK) {
    return CMP_ERR_IO;
  }
  if (AImage_getPlaneData(image, 1, (uint8_t **)&u_plane, &u_len) !=
      AMEDIA_OK) {
    return CMP_ERR_IO;
  }
  if (AImage_getPlaneData(image, 2, (uint8_t **)&v_plane, &v_len) !=
      AMEDIA_OK) {
    return CMP_ERR_IO;
  }
  if (AImage_getPlaneRowStride(image, 0, &y_row_stride) != AMEDIA_OK ||
      AImage_getPlaneRowStride(image, 1, &u_row_stride) != AMEDIA_OK ||
      AImage_getPlaneRowStride(image, 2, &v_row_stride) != AMEDIA_OK ||
      AImage_getPlanePixelStride(image, 1, &u_pixel_stride) != AMEDIA_OK ||
      AImage_getPlanePixelStride(image, 2, &v_pixel_stride) != AMEDIA_OK) {
    return CMP_ERR_IO;
  }

  if (state->backend == NULL) {
    return CMP_ERR_STATE;
  }
  if (state->backend->allocator.alloc == NULL ||
      state->backend->allocator.realloc == NULL ||
      state->backend->allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (state->frame_capacity < needed) {
    cmp_u8 *new_frame;
    rc = state->backend->allocator.realloc(state->backend->allocator.ctx,
                                           state->frame_data, needed,
                                           (void **)&new_frame);
    if (rc != CMP_OK) {
      return rc;
    }
    state->frame_data = new_frame;
    state->frame_capacity = needed;
  }

  row_bytes = (cmp_usize)width;
  for (row = 0u; row < (cmp_usize)height; row += 1u) {
    memcpy(state->frame_data + row * row_bytes,
           y_plane + row * (cmp_usize)y_row_stride, (size_t)row_bytes);
  }

  for (row = 0u; row < (cmp_usize)(height / 2); row += 1u) {
    for (col = 0u; col < (cmp_usize)(width / 2); col += 1u) {
      cmp_usize uv_index = y_size + row * (cmp_usize)width + col * 2u;
      state->frame_data[uv_index] = u_plane[row * (cmp_usize)u_row_stride +
                                            col * (cmp_usize)u_pixel_stride];
      state->frame_data[uv_index + 1u] =
          v_plane[row * (cmp_usize)v_row_stride +
                  col * (cmp_usize)v_pixel_stride];
    }
  }

  state->frame_size = needed;
  state->format = CMP_CAMERA_FORMAT_NV12;
  state->width = (cmp_u32)width;
  state->height = (cmp_u32)height;
  state->has_frame = CMP_TRUE;
  return CMP_OK;
}

static void cmp_android_camera_on_image(void *context, AImageReader *reader) {
  CMPAndroidCameraState *state;
  AImage *image;
  int rc;
  int lock_rc;

  (void)reader;

  if (context == NULL) {
    return;
  }

  state = (CMPAndroidCameraState *)context;
  if (AImageReader_acquireLatestImage(reader, &image) != AMEDIA_OK) {
    return;
  }

  lock_rc = cmp_android_camera_lock(state);
  if (lock_rc != CMP_OK) {
    AImage_delete(image);
    return;
  }

  rc = cmp_android_camera_copy_image(state, image);
  if (rc != CMP_OK) {
    state->last_error = rc;
  } else {
    state->last_error = CMP_OK;
  }

  cmp_android_camera_unlock(state);
  AImage_delete(image);
}

static void cmp_android_camera_on_opened(void *context, ACameraDevice *device) {
  CMPAndroidCameraState *state;

  if (context == NULL) {
    return;
  }
  state = (CMPAndroidCameraState *)context;
  if (cmp_android_camera_lock(state) != CMP_OK) {
    return;
  }
  state->device = device;
  state->opened = CMP_TRUE;
  state->last_error = CMP_OK;
  cmp_android_camera_unlock(state);
}

static void cmp_android_camera_on_disconnected(void *context,
                                              ACameraDevice *device) {
  CMPAndroidCameraState *state;

  (void)device;

  if (context == NULL) {
    return;
  }
  state = (CMPAndroidCameraState *)context;
  if (cmp_android_camera_lock(state) != CMP_OK) {
    return;
  }
  state->opened = CMP_FALSE;
  state->streaming = CMP_FALSE;
  state->last_error = CMP_ERR_NOT_READY;
  cmp_android_camera_unlock(state);
}

static void cmp_android_camera_on_error(void *context, ACameraDevice *device,
                                       int error) {
  CMPAndroidCameraState *state;

  (void)device;
  (void)error;

  if (context == NULL) {
    return;
  }
  state = (CMPAndroidCameraState *)context;
  if (cmp_android_camera_lock(state) != CMP_OK) {
    return;
  }
  state->opened = CMP_FALSE;
  state->streaming = CMP_FALSE;
  state->last_error = CMP_ERR_IO;
  cmp_android_camera_unlock(state);
}
#endif

#if CMP_ANDROID_CAMERA2_AVAILABLE
static int cmp_android_camera_select_index(ACameraManager *manager,
                                          const CMPCameraConfig *config,
                                          const ACameraIdList *id_list,
                                          cmp_u32 *out_index) {
  cmp_u32 desired;
  cmp_u32 i;

  if (manager == NULL || config == NULL || id_list == NULL ||
      out_index == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (id_list->numCameras <= 0) {
    return CMP_ERR_NOT_FOUND;
  }

  if (config->camera_id != CMP_CAMERA_ID_DEFAULT) {
    if (config->camera_id >= (cmp_u32)id_list->numCameras) {
      return CMP_ERR_NOT_FOUND;
    }
    *out_index = config->camera_id;
    return CMP_OK;
  }

  if (config->facing == CMP_CAMERA_FACING_UNSPECIFIED) {
    *out_index = 0u;
    return CMP_OK;
  }

  switch (config->facing) {
  case CMP_CAMERA_FACING_FRONT:
    desired = (cmp_u32)ACAMERA_LENS_FACING_FRONT;
    break;
  case CMP_CAMERA_FACING_BACK:
    desired = (cmp_u32)ACAMERA_LENS_FACING_BACK;
    break;
  case CMP_CAMERA_FACING_EXTERNAL:
    desired = (cmp_u32)ACAMERA_LENS_FACING_EXTERNAL;
    break;
  case CMP_CAMERA_FACING_UNSPECIFIED:
  default:
    desired = (cmp_u32)ACAMERA_LENS_FACING_BACK;
    break;
  }

  for (i = 0u; i < (cmp_u32)id_list->numCameras; i += 1u) {
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
      cmp_u32 lens_facing = (cmp_u32)entry.data.u8[0];
      if (lens_facing == desired) {
        ACameraMetadata_free(metadata);
        *out_index = i;
        return CMP_OK;
      }
    }

    ACameraMetadata_free(metadata);
  }

  return CMP_ERR_NOT_FOUND;
}
#endif

static int cmp_android_camera_open_with_config(void *camera,
                                              const CMPCameraConfig *config);

static int cmp_android_camera_open(void *camera, cmp_u32 camera_id) {
  CMPCameraConfig config;

  config.camera_id = camera_id;
  config.facing = CMP_CAMERA_FACING_UNSPECIFIED;
  config.width = 0u;
  config.height = 0u;
  config.format = CMP_CAMERA_FORMAT_ANY;

  return cmp_android_camera_open_with_config(camera, &config);
}

static int cmp_android_camera_open_with_config(void *camera,
                                              const CMPCameraConfig *config) {
  struct CMPAndroidBackend *backend;
  CMPAndroidCameraState *state;

  if (camera == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)camera;
  state = &backend->camera_state;

#if CMP_ANDROID_CAMERA2_AVAILABLE
  if (state->opened == CMP_TRUE || state->streaming == CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (config->format != CMP_CAMERA_FORMAT_ANY &&
      config->format != CMP_CAMERA_FORMAT_NV12) {
    return CMP_ERR_UNSUPPORTED;
  }

  state->backend = backend;
  state->width =
      (config->width == 0u) ? CMP_ANDROID_CAMERA_DEFAULT_WIDTH : config->width;
  state->height = (config->height == 0u) ? CMP_ANDROID_CAMERA_DEFAULT_HEIGHT
                                         : config->height;
  state->format = CMP_CAMERA_FORMAT_NV12;
  state->has_frame = CMP_FALSE;
  state->frame_size = 0u;
  state->last_error = CMP_OK;

  if (state->mutex_initialized != CMP_TRUE) {
    if (pthread_mutex_init(&state->mutex, NULL) != 0) {
      return CMP_ERR_UNKNOWN;
    }
    state->mutex_initialized = CMP_TRUE;
  }

  state->manager = ACameraManager_create();
  if (state->manager == NULL) {
    return CMP_ERR_UNKNOWN;
  }

  {
    ACameraIdList *id_list;
    AImageReader_ImageListener listener;
    camera_status_t status;
    cmp_u32 selected_index;
    int rc;

    status = ACameraManager_getCameraIdList(state->manager, &id_list);
    if (status != ACAMERA_OK) {
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return cmp_android_camera_status_to_error(status);
    }

    rc = cmp_android_camera_select_index(state->manager, config, id_list,
                                        &selected_index);
    if (rc != CMP_OK) {
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
      return cmp_android_camera_status_to_error(status);
    }

    status = AImageReader_getWindow(state->reader, &state->reader_window);
    if (status != AMEDIA_OK || state->reader_window == NULL) {
      AImageReader_delete(state->reader);
      state->reader = NULL;
      ACameraManager_deleteCameraIdList(id_list);
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return CMP_ERR_IO;
    }

    listener.context = state;
    listener.onImageAvailable = cmp_android_camera_on_image;
    if (AImageReader_setImageListener(state->reader, &listener) != AMEDIA_OK) {
      AImageReader_delete(state->reader);
      state->reader = NULL;
      ACameraManager_deleteCameraIdList(id_list);
      ACameraManager_delete(state->manager);
      state->manager = NULL;
      return CMP_ERR_IO;
    }

    {
      ACameraDevice_StateCallbacks callbacks;
      callbacks.context = state;
      callbacks.onOpened = cmp_android_camera_on_opened;
      callbacks.onDisconnected = cmp_android_camera_on_disconnected;
      callbacks.onError = cmp_android_camera_on_error;

      status = ACameraManager_openCamera(state->manager,
                                         id_list->cameraIds[selected_index],
                                         &callbacks, &state->device);
      rc = cmp_android_camera_status_to_error(status);
      ACameraManager_deleteCameraIdList(id_list);
      if (rc != CMP_OK) {
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

  state->opened = CMP_TRUE;
  return CMP_OK;
#else
  CMP_UNUSED(config);
  CMP_UNUSED(state);
  CMP_UNUSED(backend);
  return CMP_ERR_UNSUPPORTED;
#endif
}

#if CMP_ANDROID_CAMERA2_AVAILABLE
static int cmp_android_camera_stop_internal(CMPAndroidCameraState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
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

  state->streaming = CMP_FALSE;
  return CMP_OK;
}
#endif

static int cmp_android_camera_close(void *camera) {
  struct CMPAndroidBackend *backend;
  CMPAndroidCameraState *state;
  int rc = 0;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)camera;
  state = &backend->camera_state;

#if CMP_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  if (state->streaming == CMP_TRUE) {
    rc = cmp_android_camera_stop_internal(state);
    if (rc != CMP_OK) {
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
      return CMP_ERR_INVALID_ARGUMENT;
    }
    rc = backend->allocator.free(backend->allocator.ctx, state->frame_data);
    if (rc != CMP_OK) {
      return rc;
    }
    state->frame_data = NULL;
    state->frame_capacity = 0u;
    state->frame_size = 0u;
  }

  if (state->mutex_initialized == CMP_TRUE) {
    pthread_mutex_destroy(&state->mutex);
    state->mutex_initialized = CMP_FALSE;
  }

  state->opened = CMP_FALSE;
  state->streaming = CMP_FALSE;
  state->has_frame = CMP_FALSE;
  state->last_error = CMP_OK;
  return CMP_OK;
#else
  CMP_UNUSED(state);
  CMP_UNUSED(backend);
  CMP_UNUSED(rc);
  return CMP_ERR_UNSUPPORTED;
#endif
}

static int cmp_android_camera_start(void *camera) {
  struct CMPAndroidBackend *backend;
  CMPAndroidCameraState *state;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)camera;
  state = &backend->camera_state;

#if CMP_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (state->streaming == CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (state->device == NULL || state->reader_window == NULL) {
    return CMP_ERR_NOT_READY;
  }

  {
    camera_status_t status;
    ACameraCaptureSession_stateCallbacks callbacks;

    status = ACaptureSessionOutputContainer_create(&state->output_container);
    if (status != ACAMERA_OK) {
      return cmp_android_camera_status_to_error(status);
    }

    status = ACaptureSessionOutput_create(state->reader_window,
                                          &state->session_output);
    if (status != ACAMERA_OK) {
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return cmp_android_camera_status_to_error(status);
    }

    status = ACaptureSessionOutputContainer_add(state->output_container,
                                                state->session_output);
    if (status != ACAMERA_OK) {
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return cmp_android_camera_status_to_error(status);
    }

    status =
        ACameraOutputTarget_create(state->reader_window, &state->output_target);
    if (status != ACAMERA_OK) {
      ACaptureSessionOutput_free(state->session_output);
      state->session_output = NULL;
      ACaptureSessionOutputContainer_free(state->output_container);
      state->output_container = NULL;
      return cmp_android_camera_status_to_error(status);
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
      return cmp_android_camera_status_to_error(status);
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
      return cmp_android_camera_status_to_error(status);
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
      return cmp_android_camera_status_to_error(status);
    }

    status = ACameraCaptureSession_setRepeatingRequest(state->session, NULL, 1,
                                                       &state->request, NULL);
    if (status != ACAMERA_OK) {
      cmp_android_camera_stop_internal(state);
      return cmp_android_camera_status_to_error(status);
    }
  }

  state->streaming = CMP_TRUE;
  return CMP_OK;
#else
  CMP_UNUSED(state);
  CMP_UNUSED(backend);
  return CMP_ERR_UNSUPPORTED;
#endif
}

static int cmp_android_camera_stop(void *camera) {
  struct CMPAndroidBackend *backend;
  CMPAndroidCameraState *state;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)camera;
  state = &backend->camera_state;

#if CMP_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (state->streaming != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  return cmp_android_camera_stop_internal(state);
#else
  CMP_UNUSED(state);
  CMP_UNUSED(backend);
  return CMP_ERR_UNSUPPORTED;
#endif
}

static int cmp_android_camera_read_frame(void *camera, CMPCameraFrame *out_frame,
                                        CMPBool *out_has_frame) {
  struct CMPAndroidBackend *backend;
  CMPAndroidCameraState *state;
  int rc = 0;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)camera;
  state = &backend->camera_state;

#if CMP_ANDROID_CAMERA2_AVAILABLE
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (state->streaming != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  rc = cmp_android_camera_lock(state);
  if (rc != CMP_OK) {
    return rc;
  }

  if (state->last_error != CMP_OK) {
    rc = state->last_error;
    state->last_error = CMP_OK;
    cmp_android_camera_unlock(state);
    return rc;
  }

  if (state->has_frame != CMP_TRUE || state->frame_data == NULL) {
    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = CMP_FALSE;
    cmp_android_camera_unlock(state);
    return CMP_OK;
  }

  out_frame->format = state->format;
  out_frame->width = state->width;
  out_frame->height = state->height;
  out_frame->data = state->frame_data;
  out_frame->size = state->frame_size;
  *out_has_frame = CMP_TRUE;
  cmp_android_camera_unlock(state);
  return CMP_OK;
#else
  memset(out_frame, 0, sizeof(*out_frame));
  *out_has_frame = CMP_FALSE;
  CMP_UNUSED(state);
  CMP_UNUSED(backend);
  CMP_UNUSED(rc);
  return CMP_ERR_UNSUPPORTED;
#endif
}

static const CMPCameraVTable g_cmp_android_camera_vtable = {
    cmp_android_camera_open,  cmp_android_camera_open_with_config,
    cmp_android_camera_close, cmp_android_camera_start,
    cmp_android_camera_stop,  cmp_android_camera_read_frame};

static int cmp_android_env_get_io(void *env, CMPIO *out_io) {
  struct CMPAndroidBackend *backend;

  if (env == NULL || out_io == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_io == NULL) {
    return CMP_ERR_STATE;
  }
  return backend->null_env.vtable->get_io(backend->null_env.ctx, out_io);
}

static int cmp_android_env_get_sensors(void *env, CMPSensors *out_sensors) {
  struct CMPAndroidBackend *backend;

  if (env == NULL || out_sensors == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_sensors == NULL) {
    return CMP_ERR_STATE;
  }
  return backend->null_env.vtable->get_sensors(backend->null_env.ctx,
                                               out_sensors);
}

static int cmp_android_env_get_camera(void *env, CMPCamera *out_camera) {
  struct CMPAndroidBackend *backend;

  if (env == NULL || out_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)env;
  *out_camera = backend->camera;
  return CMP_OK;
}

static int cmp_android_env_get_image(void *env, CMPImage *out_image) {
  if (env == NULL || out_image == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_image->ctx = NULL;
  out_image->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_android_env_get_video(void *env, CMPVideo *out_video) {
  if (env == NULL || out_video == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_video->ctx = NULL;
  out_video->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_android_env_get_audio(void *env, CMPAudio *out_audio) {
  if (env == NULL || out_audio == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_audio->ctx = NULL;
  out_audio->vtable = NULL;
  return CMP_ERR_UNSUPPORTED;
}

static int cmp_android_env_get_network(void *env, CMPNetwork *out_network) {
  struct CMPAndroidBackend *backend;

  if (env == NULL || out_network == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_network == NULL) {
    return CMP_ERR_STATE;
  }
  return backend->null_env.vtable->get_network(backend->null_env.ctx,
                                               out_network);
}

static int cmp_android_env_get_tasks(void *env, CMPTasks *out_tasks) {
  struct CMPAndroidBackend *backend;

  if (env == NULL || out_tasks == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_tasks == NULL) {
    return CMP_ERR_STATE;
  }
  return backend->null_env.vtable->get_tasks(backend->null_env.ctx, out_tasks);
}

static int cmp_android_env_get_time_ms(void *env, cmp_u32 *out_time_ms) {
  struct CMPAndroidBackend *backend;

  if (env == NULL || out_time_ms == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  backend = (struct CMPAndroidBackend *)env;
  if (backend->null_env.vtable == NULL ||
      backend->null_env.vtable->get_time_ms == NULL) {
    return CMP_ERR_STATE;
  }
  return backend->null_env.vtable->get_time_ms(backend->null_env.ctx,
                                               out_time_ms);
}

static const CMPEnvVTable g_cmp_android_env_vtable = {
    cmp_android_env_get_io,     cmp_android_env_get_sensors,
    cmp_android_env_get_camera, cmp_android_env_get_image,
    cmp_android_env_get_video,  cmp_android_env_get_audio,
    cmp_android_env_get_network, cmp_android_env_get_tasks,
    cmp_android_env_get_time_ms};

int CMP_CALL cmp_android_backend_create(const CMPAndroidBackendConfig *config,
                                      CMPAndroidBackend **out_backend) {
  CMPAndroidBackendConfig local_config;
  CMPNullBackendConfig null_config;
  CMPAllocator allocator;
  struct CMPAndroidBackend *backend;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_android_backend_config_init(&local_config);
    CMP_ANDROID_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_android_backend_validate_config(config);
  CMP_ANDROID_RETURN_IF_ERROR(rc);

  if (config->allocator == NULL) {
    rc = cmp_get_default_allocator(&allocator);
    CMP_ANDROID_RETURN_IF_ERROR(rc);
  } else {
    allocator = *config->allocator;
  }

  if (allocator.alloc == NULL || allocator.realloc == NULL ||
      allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = allocator.alloc(allocator.ctx, sizeof(*backend), (void **)&backend);
  CMP_ANDROID_RETURN_IF_ERROR(rc);

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

  rc = cmp_null_backend_create(&null_config, &backend->null_backend);
  if (rc != CMP_OK) {
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = cmp_null_backend_get_ws(backend->null_backend, &backend->ws);
  if (rc != CMP_OK) {
    cmp_null_backend_destroy(backend->null_backend);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = cmp_null_backend_get_gfx(backend->null_backend, &backend->gfx);
  if (rc != CMP_OK) {
    cmp_null_backend_destroy(backend->null_backend);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  rc = cmp_null_backend_get_env(backend->null_backend, &backend->null_env);
  if (rc != CMP_OK) {
    cmp_null_backend_destroy(backend->null_backend);
    allocator.free(allocator.ctx, backend);
    return rc;
  }

  backend->env.ctx = backend;
  backend->env.vtable = &g_cmp_android_env_vtable;
  backend->camera.ctx = backend;
  backend->camera.vtable = &g_cmp_android_camera_vtable;
  backend->camera_state.backend = backend;
  backend->camera_state.frame_data = NULL;
  backend->camera_state.frame_capacity = 0u;
  backend->camera_state.frame_size = 0u;
  backend->camera_state.has_frame = CMP_FALSE;
  backend->camera_state.opened = CMP_FALSE;
  backend->camera_state.streaming = CMP_FALSE;
  backend->camera_state.last_error = CMP_OK;

  backend->initialized = CMP_TRUE;
  *out_backend = backend;
  return CMP_OK;
}

int CMP_CALL cmp_android_backend_destroy(CMPAndroidBackend *backend) {
  CMPAllocator allocator;
  int first_error;
  int rc;

  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  allocator = backend->allocator;
  first_error = CMP_OK;

  if (backend->camera_state.opened == CMP_TRUE) {
    rc = cmp_android_camera_close(backend);
    if (rc != CMP_OK) {
      first_error = rc;
    }
  }

  if (backend->null_backend != NULL) {
    rc = cmp_null_backend_destroy(backend->null_backend);
    if (rc != CMP_OK) {
      first_error = rc;
    }
    backend->null_backend = NULL;
  }
  backend->predictive_back = NULL;

  if (allocator.free == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  rc = allocator.free(allocator.ctx, backend);
  if (rc != CMP_OK && first_error == CMP_OK) {
    first_error = rc;
  }

  return first_error;
}

int CMP_CALL cmp_android_backend_get_ws(CMPAndroidBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_ws = backend->ws;
  return CMP_OK;
}

int CMP_CALL cmp_android_backend_get_gfx(CMPAndroidBackend *backend,
                                       CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_gfx = backend->gfx;
  return CMP_OK;
}

int CMP_CALL cmp_android_backend_get_env(CMPAndroidBackend *backend,
                                       CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_env = backend->env;
  return CMP_OK;
}

int CMP_CALL cmp_android_backend_set_predictive_back(
    CMPAndroidBackend *backend, CMPPredictiveBack *predictive) {
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

int CMP_CALL cmp_android_backend_get_predictive_back(
    CMPAndroidBackend *backend, CMPPredictiveBack **out_predictive) {
  if (backend == NULL || out_predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized) {
    return CMP_ERR_STATE;
  }
  *out_predictive = backend->predictive_back;
  return CMP_OK;
}

int CMP_CALL cmp_android_backend_predictive_back_start(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_start(backend->predictive_back, event);
}

int CMP_CALL cmp_android_backend_predictive_back_progress(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_progress(backend->predictive_back, event);
}

int CMP_CALL cmp_android_backend_predictive_back_commit(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_commit(backend->predictive_back, event);
}

int CMP_CALL cmp_android_backend_predictive_back_cancel(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (!backend->initialized || backend->predictive_back == NULL) {
    return CMP_ERR_STATE;
  }
  return cmp_predictive_back_cancel(backend->predictive_back, event);
}

/* GCOVR_EXCL_STOP */
#else

int CMP_CALL cmp_android_backend_create(const CMPAndroidBackendConfig *config,
                                      CMPAndroidBackend **out_backend) {
  CMPAndroidBackendConfig local_config;
  int rc;

  if (out_backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_backend = NULL;

  if (config == NULL) {
    rc = cmp_android_backend_config_init(&local_config);
    CMP_ANDROID_RETURN_IF_ERROR(rc);
    config = &local_config;
  }

  rc = cmp_android_backend_validate_config(config);
  CMP_ANDROID_RETURN_IF_ERROR(rc);

  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_destroy(CMPAndroidBackend *backend) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_get_ws(CMPAndroidBackend *backend, CMPWS *out_ws) {
  if (backend == NULL || out_ws == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_ws, 0, sizeof(*out_ws));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_get_gfx(CMPAndroidBackend *backend,
                                       CMPGfx *out_gfx) {
  if (backend == NULL || out_gfx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_gfx, 0, sizeof(*out_gfx));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_get_env(CMPAndroidBackend *backend,
                                       CMPEnv *out_env) {
  if (backend == NULL || out_env == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(out_env, 0, sizeof(*out_env));
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_set_predictive_back(
    CMPAndroidBackend *backend, CMPPredictiveBack *predictive) {
  if (backend == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  CMP_UNUSED(predictive);
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_get_predictive_back(
    CMPAndroidBackend *backend, CMPPredictiveBack **out_predictive) {
  if (backend == NULL || out_predictive == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_predictive = NULL;
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_predictive_back_start(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_predictive_back_progress(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_predictive_back_commit(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

int CMP_CALL cmp_android_backend_predictive_back_cancel(
    CMPAndroidBackend *backend, const CMPPredictiveBackEvent *event) {
  if (backend == NULL || event == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  return CMP_ERR_UNSUPPORTED;
}

#endif
