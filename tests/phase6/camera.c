#include "test_utils.h"
#include "m3/m3_camera.h"

#include <string.h>

typedef struct TestCameraState {
    int fail_open;
    int fail_close;
    int fail_start;
    int fail_stop;
    int fail_read;
    int open_calls;
    int open_with_config_calls;
    int close_calls;
    int start_calls;
    int stop_calls;
    int read_calls;
    M3CameraConfig last_config;
    M3Bool opened;
    M3Bool streaming;
    M3CameraFrame frame;
    M3Bool has_frame;
} TestCameraState;

static int test_camera_state_reset(TestCameraState *state)
{
    if (state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }
    memset(state, 0, sizeof(*state));
    return M3_OK;
}

static int test_camera_open(void *camera, m3_u32 camera_id)
{
    TestCameraState *state;

    M3_UNUSED(camera_id);

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestCameraState *)camera;
    state->open_calls += 1;
    if (state->fail_open) {
        return M3_ERR_IO;
    }
    if (state->opened == M3_TRUE) {
        return M3_ERR_STATE;
    }

    state->opened = M3_TRUE;
    return M3_OK;
}

static int test_camera_open_with_config(void *camera, const M3CameraConfig *config)
{
    TestCameraState *state;

    if (camera == NULL || config == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestCameraState *)camera;
    state->open_with_config_calls += 1;
    state->last_config = *config;

    return test_camera_open(camera, config->camera_id);
}

static int test_camera_close(void *camera)
{
    TestCameraState *state;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestCameraState *)camera;
    state->close_calls += 1;
    if (state->fail_close) {
        return M3_ERR_IO;
    }
    if (state->opened != M3_TRUE) {
        return M3_ERR_STATE;
    }

    state->opened = M3_FALSE;
    state->streaming = M3_FALSE;
    return M3_OK;
}

static int test_camera_start(void *camera)
{
    TestCameraState *state;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestCameraState *)camera;
    state->start_calls += 1;
    if (state->fail_start) {
        return M3_ERR_IO;
    }
    if (state->opened != M3_TRUE) {
        return M3_ERR_STATE;
    }
    if (state->streaming == M3_TRUE) {
        return M3_ERR_STATE;
    }

    state->streaming = M3_TRUE;
    return M3_OK;
}

static int test_camera_stop(void *camera)
{
    TestCameraState *state;

    if (camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestCameraState *)camera;
    state->stop_calls += 1;
    if (state->fail_stop) {
        return M3_ERR_IO;
    }
    if (state->streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }

    state->streaming = M3_FALSE;
    return M3_OK;
}

static int test_camera_read(void *camera, M3CameraFrame *out_frame, M3Bool *out_has_frame)
{
    TestCameraState *state;

    if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestCameraState *)camera;
    state->read_calls += 1;
    if (state->fail_read) {
        return M3_ERR_IO;
    }
    if (state->streaming != M3_TRUE) {
        return M3_ERR_STATE;
    }

    if (state->has_frame == M3_TRUE) {
        *out_frame = state->frame;
        *out_has_frame = M3_TRUE;
    } else {
        memset(out_frame, 0, sizeof(*out_frame));
        *out_has_frame = M3_FALSE;
    }

    return M3_OK;
}

static const M3CameraVTable g_test_camera_vtable = {
    test_camera_open,
    test_camera_open_with_config,
    test_camera_close,
    test_camera_start,
    test_camera_stop,
    test_camera_read
};

typedef struct TestEnvState {
    M3Env env;
    M3Camera camera;
    int fail_get_camera;
} TestEnvState;

static int test_env_get_camera(void *env, M3Camera *out_camera)
{
    TestEnvState *state;

    if (env == NULL || out_camera == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    state = (TestEnvState *)env;
    if (state->fail_get_camera) {
        return M3_ERR_IO;
    }

    *out_camera = state->camera;
    return M3_OK;
}

static const M3EnvVTable g_test_env_vtable = {
    NULL,
    NULL,
    test_env_get_camera,
    NULL,
    NULL,
    NULL
};

static const M3EnvVTable g_test_env_vtable_no_camera = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static int test_env_reset(TestEnvState *env_state, TestCameraState *camera_state, const M3CameraVTable *vtable)
{
    if (env_state == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(env_state, 0, sizeof(*env_state));
    env_state->env.ctx = env_state;
    env_state->env.vtable = &g_test_env_vtable;

    if (camera_state != NULL && vtable != NULL) {
        env_state->camera.ctx = camera_state;
        env_state->camera.vtable = vtable;
    }

    return M3_OK;
}

static int test_session_reset(M3CameraSession *session, TestCameraState *camera_state, const M3CameraVTable *vtable,
    M3Bool opened, M3Bool streaming)
{
    if (session == NULL) {
        return M3_ERR_INVALID_ARGUMENT;
    }

    memset(session, 0, sizeof(*session));
    if (vtable != NULL) {
        session->camera.ctx = camera_state;
        session->camera.vtable = vtable;
    }
    session->opened = opened;
    session->streaming = streaming;
    return M3_OK;
}

static int test_camera_config_init(void)
{
    M3CameraSessionConfig config;

    M3_TEST_EXPECT(m3_camera_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_camera_config_init(&config));
    M3_TEST_ASSERT(config.env == NULL);
    M3_TEST_ASSERT(config.config.camera_id == M3_CAMERA_ID_DEFAULT);
    M3_TEST_ASSERT(config.config.facing == M3_CAMERA_FACING_UNSPECIFIED);
    M3_TEST_ASSERT(config.config.width == 0u);
    M3_TEST_ASSERT(config.config.height == 0u);
    M3_TEST_ASSERT(config.config.format == M3_CAMERA_FORMAT_ANY);
    return 0;
}

static int test_camera_init_errors(void)
{
    M3CameraSession session;
    M3CameraSessionConfig config;
    TestCameraState cam_state;
    TestEnvState env_state;
    M3CameraVTable vtable_missing;
    M3Env env;

    memset(&session, 0, sizeof(session));
    M3_TEST_OK(m3_camera_config_init(&config));

    M3_TEST_EXPECT(m3_camera_init(NULL, &config), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_camera_init(&session, NULL), M3_ERR_INVALID_ARGUMENT);

    session.camera.vtable = &g_test_camera_vtable;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_STATE);
    memset(&session, 0, sizeof(session));

    config.env = NULL;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_INVALID_ARGUMENT);

    env.ctx = NULL;
    env.vtable = NULL;
    config.env = &env;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_INVALID_ARGUMENT);

    env.vtable = &g_test_env_vtable_no_camera;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
    config.env = &env_state.env;

    env_state.fail_get_camera = 1;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_IO);
    env_state.fail_get_camera = 0;

    env_state.camera.ctx = NULL;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_INVALID_ARGUMENT);

    env_state.camera.ctx = &cam_state;
    env_state.camera.vtable = NULL;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_INVALID_ARGUMENT);

    vtable_missing = g_test_camera_vtable;
    vtable_missing.open = NULL;
    vtable_missing.open_with_config = NULL;
    env_state.camera.vtable = &vtable_missing;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_UNSUPPORTED);

    env_state.camera.vtable = &g_test_camera_vtable;
    cam_state.fail_open = 1;
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_IO);
    cam_state.fail_open = 0;

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
    config.env = &env_state.env;

    vtable_missing = g_test_camera_vtable;
    vtable_missing.open_with_config = NULL;
    env_state.camera.vtable = &vtable_missing;
    memset(&session, 0, sizeof(session));
    M3_TEST_OK(m3_camera_init(&session, &config));
    M3_TEST_ASSERT(cam_state.open_calls == 1);
    M3_TEST_OK(m3_camera_shutdown(&session));

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_env_reset(&env_state, &cam_state, &vtable_missing));
    config.env = &env_state.env;
    cam_state.fail_open = 1;
    memset(&session, 0, sizeof(session));
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_IO);
    cam_state.fail_open = 0;

    vtable_missing = g_test_camera_vtable;
    vtable_missing.open = NULL;
    vtable_missing.open_with_config = NULL;
    env_state.camera.vtable = &vtable_missing;
    memset(&session, 0, sizeof(session));
    M3_TEST_EXPECT(m3_camera_test_set_skip_vtable_check(2), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_camera_test_set_skip_vtable_check(M3_TRUE));
    M3_TEST_EXPECT(m3_camera_init(&session, &config), M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_camera_test_set_skip_vtable_check(M3_FALSE));

    return 0;
}

static int test_camera_shutdown_errors(void)
{
    M3CameraSession session;
    TestCameraState cam_state;
    M3CameraVTable vtable_missing;

    M3_TEST_EXPECT(m3_camera_shutdown(NULL), M3_ERR_INVALID_ARGUMENT);

    memset(&session, 0, sizeof(session));
    M3_TEST_EXPECT(m3_camera_shutdown(&session), M3_ERR_STATE);

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_FALSE));
    session.camera.ctx = NULL;
    M3_TEST_EXPECT(m3_camera_shutdown(&session), M3_ERR_INVALID_ARGUMENT);

    vtable_missing = g_test_camera_vtable;
    vtable_missing.close = NULL;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_camera_shutdown(&session), M3_ERR_UNSUPPORTED);

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_TRUE));
    cam_state.fail_stop = 1;
    M3_TEST_EXPECT(m3_camera_shutdown(&session), M3_ERR_IO);
    cam_state.fail_stop = 0;

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_FALSE));
    cam_state.fail_close = 1;
    M3_TEST_EXPECT(m3_camera_shutdown(&session), M3_ERR_IO);
    cam_state.fail_close = 0;

    return 0;
}

static int test_camera_shutdown_no_stream(void)
{
    M3CameraSession session;
    M3CameraSessionConfig config;
    TestCameraState cam_state;
    TestEnvState env_state;

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
    M3_TEST_OK(m3_camera_config_init(&config));
    config.env = &env_state.env;
    config.config.camera_id = 7u;
    config.config.facing = M3_CAMERA_FACING_BACK;
    config.config.width = 640u;
    config.config.height = 480u;
    config.config.format = M3_CAMERA_FORMAT_BGRA8;

    memset(&session, 0, sizeof(session));
    M3_TEST_OK(m3_camera_init(&session, &config));
    M3_TEST_ASSERT(session.opened == M3_TRUE);
    M3_TEST_ASSERT(session.streaming == M3_FALSE);
    M3_TEST_ASSERT(session.camera_id == 7u);
    M3_TEST_ASSERT(cam_state.open_with_config_calls == 1);
    M3_TEST_ASSERT(cam_state.last_config.camera_id == 7u);
    M3_TEST_ASSERT(cam_state.last_config.facing == M3_CAMERA_FACING_BACK);
    M3_TEST_ASSERT(cam_state.last_config.width == 640u);
    M3_TEST_ASSERT(cam_state.last_config.height == 480u);
    M3_TEST_ASSERT(cam_state.last_config.format == M3_CAMERA_FORMAT_BGRA8);

    M3_TEST_OK(m3_camera_shutdown(&session));
    M3_TEST_ASSERT(session.camera.vtable == NULL);
    M3_TEST_ASSERT(cam_state.stop_calls == 0);
    M3_TEST_ASSERT(cam_state.close_calls == 1);
    return 0;
}

static int test_camera_shutdown_with_stream(void)
{
    M3CameraSession session;
    M3CameraSessionConfig config;
    TestCameraState cam_state;
    TestEnvState env_state;

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
    M3_TEST_OK(m3_camera_config_init(&config));
    config.env = &env_state.env;

    memset(&session, 0, sizeof(session));
    M3_TEST_OK(m3_camera_init(&session, &config));
    M3_TEST_OK(m3_camera_start(&session));

    M3_TEST_OK(m3_camera_shutdown(&session));
    M3_TEST_ASSERT(cam_state.stop_calls == 1);
    M3_TEST_ASSERT(cam_state.close_calls == 1);
    return 0;
}

static int test_camera_start_errors(void)
{
    M3CameraSession session;
    TestCameraState cam_state;
    M3CameraVTable vtable_missing;

    M3_TEST_EXPECT(m3_camera_start(NULL), M3_ERR_INVALID_ARGUMENT);

    memset(&session, 0, sizeof(session));
    M3_TEST_EXPECT(m3_camera_start(&session), M3_ERR_STATE);

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_TRUE));
    M3_TEST_EXPECT(m3_camera_start(&session), M3_ERR_STATE);

    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_FALSE));
    session.camera.ctx = NULL;
    M3_TEST_EXPECT(m3_camera_start(&session), M3_ERR_INVALID_ARGUMENT);

    vtable_missing = g_test_camera_vtable;
    vtable_missing.start = NULL;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_camera_start(&session), M3_ERR_UNSUPPORTED);

    cam_state.fail_start = 1;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_camera_start(&session), M3_ERR_IO);
    cam_state.fail_start = 0;

    return 0;
}

static int test_camera_stop_errors(void)
{
    M3CameraSession session;
    TestCameraState cam_state;
    M3CameraVTable vtable_missing;

    M3_TEST_EXPECT(m3_camera_stop(NULL), M3_ERR_INVALID_ARGUMENT);

    memset(&session, 0, sizeof(session));
    M3_TEST_EXPECT(m3_camera_stop(&session), M3_ERR_STATE);

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_camera_stop(&session), M3_ERR_STATE);

    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_TRUE));
    session.camera.ctx = NULL;
    M3_TEST_EXPECT(m3_camera_stop(&session), M3_ERR_INVALID_ARGUMENT);

    vtable_missing = g_test_camera_vtable;
    vtable_missing.stop = NULL;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, M3_TRUE, M3_TRUE));
    M3_TEST_EXPECT(m3_camera_stop(&session), M3_ERR_UNSUPPORTED);

    cam_state.fail_stop = 1;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_TRUE));
    M3_TEST_EXPECT(m3_camera_stop(&session), M3_ERR_IO);
    cam_state.fail_stop = 0;

    return 0;
}

static int test_camera_read_errors(void)
{
    M3CameraSession session;
    TestCameraState cam_state;
    M3CameraVTable vtable_missing;
    M3CameraFrame frame;
    M3Bool has_frame;

    M3_TEST_EXPECT(m3_camera_read_frame(NULL, &frame, &has_frame), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_camera_read_frame(&session, NULL, &has_frame), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_camera_read_frame(&session, &frame, NULL), M3_ERR_INVALID_ARGUMENT);

    memset(&session, 0, sizeof(session));
    M3_TEST_EXPECT(m3_camera_read_frame(&session, &frame, &has_frame), M3_ERR_STATE);

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_FALSE));
    M3_TEST_EXPECT(m3_camera_read_frame(&session, &frame, &has_frame), M3_ERR_STATE);

    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_TRUE));
    session.camera.ctx = NULL;
    M3_TEST_EXPECT(m3_camera_read_frame(&session, &frame, &has_frame), M3_ERR_INVALID_ARGUMENT);

    vtable_missing = g_test_camera_vtable;
    vtable_missing.read_frame = NULL;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, M3_TRUE, M3_TRUE));
    M3_TEST_EXPECT(m3_camera_read_frame(&session, &frame, &has_frame), M3_ERR_UNSUPPORTED);

    cam_state.fail_read = 1;
    M3_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable, M3_TRUE, M3_TRUE));
    M3_TEST_EXPECT(m3_camera_read_frame(&session, &frame, &has_frame), M3_ERR_IO);
    cam_state.fail_read = 0;

    return 0;
}

static int test_camera_read_success(void)
{
    M3CameraSession session;
    M3CameraSessionConfig config;
    TestCameraState cam_state;
    TestEnvState env_state;
    M3CameraFrame frame;
    M3Bool has_frame;
    m3_u8 data[4];

    data[0] = 1u;
    data[1] = 2u;
    data[2] = 3u;
    data[3] = 4u;

    M3_TEST_OK(test_camera_state_reset(&cam_state));
    cam_state.frame.format = M3_CAMERA_FORMAT_RGBA8;
    cam_state.frame.width = 2u;
    cam_state.frame.height = 2u;
    cam_state.frame.data = data;
    cam_state.frame.size = (m3_usize)sizeof(data);
    cam_state.has_frame = M3_TRUE;

    M3_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
    M3_TEST_OK(m3_camera_config_init(&config));
    config.env = &env_state.env;

    memset(&session, 0, sizeof(session));
    M3_TEST_OK(m3_camera_init(&session, &config));
    M3_TEST_OK(m3_camera_start(&session));
    M3_TEST_OK(m3_camera_read_frame(&session, &frame, &has_frame));
    M3_TEST_ASSERT(has_frame == M3_TRUE);
    M3_TEST_ASSERT(frame.size == (m3_usize)sizeof(data));
    M3_TEST_OK(m3_camera_stop(&session));
    M3_TEST_OK(m3_camera_shutdown(&session));

    return 0;
}

static int test_camera_copy_frame(void)
{
    M3CameraFrame frame;
    m3_u8 buffer[8];
    m3_usize out_size;

    M3_TEST_EXPECT(m3_camera_copy_frame(NULL, buffer, sizeof(buffer), &out_size), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_camera_copy_frame(&frame, buffer, sizeof(buffer), NULL), M3_ERR_INVALID_ARGUMENT);

    memset(&frame, 0, sizeof(frame));
    frame.size = 4u;
    frame.data = NULL;
    M3_TEST_EXPECT(m3_camera_copy_frame(&frame, buffer, sizeof(buffer), &out_size), M3_ERR_INVALID_ARGUMENT);

    frame.data = buffer;
    M3_TEST_EXPECT(m3_camera_copy_frame(&frame, NULL, sizeof(buffer), &out_size), M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(m3_camera_copy_frame(&frame, buffer, 2u, &out_size), M3_ERR_RANGE);

    buffer[0] = 9u;
    buffer[1] = 8u;
    buffer[2] = 7u;
    buffer[3] = 6u;
    M3_TEST_OK(m3_camera_copy_frame(&frame, buffer + 4, sizeof(buffer) - 4u, &out_size));
    M3_TEST_ASSERT(out_size == 4u);
    M3_TEST_ASSERT(buffer[4] == 9u);
    M3_TEST_ASSERT(buffer[7] == 6u);

    frame.size = 0u;
    frame.data = NULL;
    M3_TEST_OK(m3_camera_copy_frame(&frame, NULL, 0u, &out_size));
    M3_TEST_ASSERT(out_size == 0u);

    return 0;
}

int main(void)
{
    M3_TEST_ASSERT(test_camera_config_init() == 0);
    M3_TEST_ASSERT(test_camera_init_errors() == 0);
    M3_TEST_ASSERT(test_camera_shutdown_errors() == 0);
    M3_TEST_ASSERT(test_camera_shutdown_no_stream() == 0);
    M3_TEST_ASSERT(test_camera_shutdown_with_stream() == 0);
    M3_TEST_ASSERT(test_camera_start_errors() == 0);
    M3_TEST_ASSERT(test_camera_stop_errors() == 0);
    M3_TEST_ASSERT(test_camera_read_errors() == 0);
    M3_TEST_ASSERT(test_camera_read_success() == 0);
    M3_TEST_ASSERT(test_camera_copy_frame() == 0);
    return 0;
}
