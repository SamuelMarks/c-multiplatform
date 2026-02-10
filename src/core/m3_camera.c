#include "m3/m3_camera.h"

#include <string.h>

#ifdef M3_TESTING
static M3Bool g_m3_camera_skip_vtable_check = M3_FALSE;
#endif

#define M3_CAMERA_VTABLE_HAS_OPEN(vtable) \
    ((vtable)->open != NULL || (vtable)->open_with_config != NULL)
#define M3_CAMERA_VTABLE_COMPLETE(vtable) \
    (M3_CAMERA_VTABLE_HAS_OPEN(vtable) && (vtable)->close != NULL && (vtable)->start != NULL \
        && (vtable)->stop != NULL && (vtable)->read_frame != NULL)

int M3_CALL m3_camera_config_init(M3CameraSessionConfig *config)
{
    if (config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    config->env = NULL;
    config->config.camera_id = M3_CAMERA_ID_DEFAULT;
    config->config.facing = M3_CAMERA_FACING_UNSPECIFIED;
    config->config.width = 0u;
    config->config.height = 0u;
    config->config.format = M3_CAMERA_FORMAT_ANY;
    return M3_OK;
}

int M3_CALL m3_camera_init(M3CameraSession *session, const M3CameraSessionConfig *config)
{
    M3Camera camera;
    int rc;

    if (session == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable != NULL || session->camera.ctx != NULL
            || session->opened != M3_FALSE || session->streaming != M3_FALSE) {
        return M3_ERR_STATE;
    }
    if (config->env == NULL || config->env->vtable == NULL || config->env->vtable->get_camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(&camera, 0, sizeof(camera));
    rc = config->env->vtable->get_camera(config->env->ctx, &camera);
    if (rc != M3_OK) {
        return rc;
    }

    if (camera.ctx == NULL || camera.vtable == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
#ifdef M3_TESTING
    if (g_m3_camera_skip_vtable_check == M3_FALSE) {
        if (!M3_CAMERA_VTABLE_COMPLETE(camera.vtable)) {
            return M3_ERR_UNSUPPORTED;
        }
    }
#else
    if (!M3_CAMERA_VTABLE_COMPLETE(camera.vtable)) {
        return M3_ERR_UNSUPPORTED;
    }
#endif

    if (camera.vtable->open_with_config != NULL) {
        rc = camera.vtable->open_with_config(camera.ctx, &config->config);
        if (rc != M3_OK) {
            return rc;
        }
    } else if (camera.vtable->open != NULL) {
        rc = camera.vtable->open(camera.ctx, config->config.camera_id);
        if (rc != M3_OK) {
            return rc;
        }
    } else {
        return M3_ERR_UNSUPPORTED;
    }

    session->camera = camera;
    session->camera_id = config->config.camera_id;
    session->facing = config->config.facing;
    session->width = config->config.width;
    session->height = config->config.height;
    session->format = config->config.format;
    session->opened = M3_TRUE;
    session->streaming = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_camera_shutdown(M3CameraSession *session)
{
    int rc;

    if (session == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable == NULL || session->opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (session->camera.ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable->close == NULL || session->camera.vtable->stop == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    if (session->streaming == M3_TRUE) {
        rc = session->camera.vtable->stop(session->camera.ctx);
        if (rc != M3_OK) {
            return rc;
        }
        session->streaming = M3_FALSE;
    }

    rc = session->camera.vtable->close(session->camera.ctx);
    if (rc != M3_OK) {
        return rc;
    }

    memset(session, 0, sizeof(*session));
    return M3_OK;
}

int M3_CALL m3_camera_start(M3CameraSession *session)
{
    int rc;

    if (session == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable == NULL || session->opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (session->streaming != M3_FALSE) {
        return M3_ERR_STATE;
    }
    if (session->camera.ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable->start == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    rc = session->camera.vtable->start(session->camera.ctx);
    if (rc != M3_OK) {
        return rc;
    }

    session->streaming = M3_TRUE;
    return M3_OK;
}

int M3_CALL m3_camera_stop(M3CameraSession *session)
{
    int rc;

    if (session == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable == NULL || session->opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (session->streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (session->camera.ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable->stop == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    rc = session->camera.vtable->stop(session->camera.ctx);
    if (rc != M3_OK) {
        return rc;
    }

    session->streaming = M3_FALSE;
    return M3_OK;
}

int M3_CALL m3_camera_read_frame(M3CameraSession *session, M3CameraFrame *out_frame, M3Bool *out_has_frame)
{
    int rc;

    if (session == NULL || out_frame == NULL || out_has_frame == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable == NULL || session->opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (session->streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (session->camera.ctx == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (session->camera.vtable->read_frame == NULL) {
        return M3_ERR_UNSUPPORTED;
    }

    rc = session->camera.vtable->read_frame(session->camera.ctx, out_frame, out_has_frame);
    if (rc != M3_OK) {
        return rc;
    }

    return M3_OK;
}

int M3_CALL m3_camera_copy_frame(const M3CameraFrame *frame, void *dst, m3_usize dst_capacity, m3_usize *out_size)
{
    if (frame == NULL || out_size == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (frame->size > 0 && frame->data == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (frame->size > 0 && dst == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    if (frame->size > dst_capacity) {
        return M3_ERR_RANGE;
    }

    if (frame->size > 0) {
        memcpy(dst, frame->data, (size_t)frame->size);
    }
    *out_size = frame->size;
    return M3_OK;
}

#ifdef M3_TESTING
int M3_CALL m3_camera_test_set_skip_vtable_check(M3Bool enable)
{
    if (enable != M3_FALSE && enable != M3_TRUE) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    g_m3_camera_skip_vtable_check = enable ? M3_TRUE : M3_FALSE;
    return M3_OK;
}
#endif
