#include "cmpc/cmp_camera.h"

#include <string.h>

#ifdef CMP_TESTING
static CMPBool g_cmp_camera_skip_vtable_check = CMP_FALSE;
#endif

#define CMP_CAMERA_VTABLE_HAS_OPEN(vtable)                                     \
  ((vtable)->open != NULL || (vtable)->open_with_config != NULL)
#define CMP_CAMERA_VTABLE_COMPLETE(vtable)                                     \
  (CMP_CAMERA_VTABLE_HAS_OPEN(vtable) && (vtable)->close != NULL &&            \
   (vtable)->start != NULL && (vtable)->stop != NULL &&                        \
   (vtable)->read_frame != NULL)

int CMP_CALL cmp_camera_config_init(CMPCameraSessionConfig *config) {
  if (config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  config->env = NULL;
  config->config.camera_id = CMP_CAMERA_ID_DEFAULT;
  config->config.facing = CMP_CAMERA_FACING_UNSPECIFIED;
  config->config.width = 0u;
  config->config.height = 0u;
  config->config.format = CMP_CAMERA_FORMAT_ANY;
  return CMP_OK;
}

int CMP_CALL cmp_camera_init(CMPCameraSession *session,
                             const CMPCameraSessionConfig *config) {
  CMPCamera camera;
  int rc;

  if (session == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable != NULL || session->camera.ctx != NULL ||
      session->opened != CMP_FALSE || session->streaming != CMP_FALSE) {
    return CMP_ERR_STATE;
  }
  if (config->env == NULL || config->env->vtable == NULL ||
      config->env->vtable->get_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(&camera, 0, sizeof(camera));
  rc = config->env->vtable->get_camera(config->env->ctx, &camera);
  if (rc != CMP_OK) {
    return rc;
  }

  if (camera.ctx == NULL || camera.vtable == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
#ifdef CMP_TESTING
  if (g_cmp_camera_skip_vtable_check == CMP_FALSE) {
    if (!CMP_CAMERA_VTABLE_COMPLETE(camera.vtable)) {
      return CMP_ERR_UNSUPPORTED;
    }
  }
#else  /* GCOVR_EXCL_LINE */
  if (!CMP_CAMERA_VTABLE_COMPLETE(camera.vtable)) { /* GCOVR_EXCL_LINE */
    return CMP_ERR_UNSUPPORTED;
  }
#endif /* GCOVR_EXCL_LINE */

  if (camera.vtable->open_with_config != NULL) {
    rc = camera.vtable->open_with_config(camera.ctx, &config->config);
    if (rc != CMP_OK) {
      return rc;
    }
  } else if (camera.vtable->open != NULL) {
    rc = camera.vtable->open(camera.ctx, config->config.camera_id);
    if (rc != CMP_OK) {
      return rc;
    }
  } else {
    return CMP_ERR_UNSUPPORTED;
  }

  session->camera = camera;
  session->camera_id = config->config.camera_id;
  session->facing = config->config.facing;
  session->width = config->config.width;
  session->height = config->config.height;
  session->format = config->config.format;
  session->opened = CMP_TRUE;
  session->streaming = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_camera_shutdown(CMPCameraSession *session) {
  int rc;

  if (session == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable == NULL || session->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (session->camera.ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable->close == NULL ||
      session->camera.vtable->stop == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  if (session->streaming == CMP_TRUE) {
    rc = session->camera.vtable->stop(session->camera.ctx);
    if (rc != CMP_OK) {
      return rc;
    }
    session->streaming = CMP_FALSE;
  }

  rc = session->camera.vtable->close(session->camera.ctx);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(session, 0, sizeof(*session));
  return CMP_OK;
}

int CMP_CALL cmp_camera_start(CMPCameraSession *session) {
  int rc;

  if (session == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable == NULL || session->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (session->streaming != CMP_FALSE) {
    return CMP_ERR_STATE;
  }
  if (session->camera.ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable->start == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = session->camera.vtable->start(session->camera.ctx);
  if (rc != CMP_OK) {
    return rc;
  }

  session->streaming = CMP_TRUE;
  return CMP_OK;
}

int CMP_CALL cmp_camera_stop(CMPCameraSession *session) {
  int rc;

  if (session == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable == NULL || session->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (session->streaming != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (session->camera.ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable->stop == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = session->camera.vtable->stop(session->camera.ctx);
  if (rc != CMP_OK) {
    return rc;
  }

  session->streaming = CMP_FALSE;
  return CMP_OK;
}

int CMP_CALL cmp_camera_read_frame(CMPCameraSession *session,
                                   CMPCameraFrame *out_frame,
                                   CMPBool *out_has_frame) {
  int rc;

  if (session == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable == NULL || session->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (session->streaming != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (session->camera.ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (session->camera.vtable->read_frame == NULL) {
    return CMP_ERR_UNSUPPORTED;
  }

  rc = session->camera.vtable->read_frame(session->camera.ctx, out_frame,
                                          out_has_frame);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

int CMP_CALL cmp_camera_copy_frame(const CMPCameraFrame *frame, void *dst,
                                   cmp_usize dst_capacity,
                                   cmp_usize *out_size) {
  if (frame == NULL || out_size == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (frame->size > 0 && frame->data == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (frame->size > 0 && dst == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (frame->size > dst_capacity) {
    return CMP_ERR_RANGE;
  }

  if (frame->size > 0) {
    memcpy(dst, frame->data, (size_t)frame->size);
  }
  *out_size = frame->size;
  return CMP_OK;
}

#ifdef CMP_TESTING
int CMP_CALL cmp_camera_test_set_skip_vtable_check(CMPBool enable) {
  if (enable != CMP_FALSE && enable != CMP_TRUE) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  g_cmp_camera_skip_vtable_check = enable ? CMP_TRUE : CMP_FALSE;
  return CMP_OK;
}
#endif
