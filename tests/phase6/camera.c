#include "cmpc/cmp_camera.h"
#include "test_utils.h"

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
  CMPCameraConfig last_config;
  CMPBool opened;
  CMPBool streaming;
  CMPCameraFrame frame;
  CMPBool has_frame;
} TestCameraState;

static int test_camera_state_reset(TestCameraState *state) {
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(state, 0, sizeof(*state));
  return CMP_OK;
}

static int test_camera_open(void *camera, cmp_u32 camera_id) {
  TestCameraState *state;

  CMP_UNUSED(camera_id);

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestCameraState *)camera;
  state->open_calls += 1;
  if (state->fail_open) {
    return CMP_ERR_IO;
  }
  if (state->opened == CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  state->opened = CMP_TRUE;
  return CMP_OK;
}

static int test_camera_open_with_config(void *camera,
                                        const CMPCameraConfig *config) {
  TestCameraState *state;

  if (camera == NULL || config == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestCameraState *)camera;
  state->open_with_config_calls += 1;
  state->last_config = *config;

  return test_camera_open(camera, config->camera_id);
}

static int test_camera_close(void *camera) {
  TestCameraState *state;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestCameraState *)camera;
  state->close_calls += 1;
  if (state->fail_close) {
    return CMP_ERR_IO;
  }
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  state->opened = CMP_FALSE;
  state->streaming = CMP_FALSE;
  return CMP_OK;
}

static int test_camera_start(void *camera) {
  TestCameraState *state;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestCameraState *)camera;
  state->start_calls += 1;
  if (state->fail_start) {
    return CMP_ERR_IO;
  }
  if (state->opened != CMP_TRUE) {
    return CMP_ERR_STATE;
  }
  if (state->streaming == CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  state->streaming = CMP_TRUE;
  return CMP_OK;
}

static int test_camera_stop(void *camera) {
  TestCameraState *state;

  if (camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestCameraState *)camera;
  state->stop_calls += 1;
  if (state->fail_stop) {
    return CMP_ERR_IO;
  }
  if (state->streaming != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  state->streaming = CMP_FALSE;
  return CMP_OK;
}

static int test_camera_read(void *camera, CMPCameraFrame *out_frame,
                            CMPBool *out_has_frame) {
  TestCameraState *state;

  if (camera == NULL || out_frame == NULL || out_has_frame == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestCameraState *)camera;
  state->read_calls += 1;
  if (state->fail_read) {
    return CMP_ERR_IO;
  }
  if (state->streaming != CMP_TRUE) {
    return CMP_ERR_STATE;
  }

  if (state->has_frame == CMP_TRUE) {
    *out_frame = state->frame;
    *out_has_frame = CMP_TRUE;
  } else {
    memset(out_frame, 0, sizeof(*out_frame));
    *out_has_frame = CMP_FALSE;
  }

  return CMP_OK;
}

static const CMPCameraVTable g_test_camera_vtable = {
    test_camera_open,  test_camera_open_with_config,
    test_camera_close, test_camera_start,
    test_camera_stop,  test_camera_read};

typedef struct TestEnvState {
  CMPEnv env;
  CMPCamera camera;
  int fail_get_camera;
} TestEnvState;

static int test_env_get_camera(void *env, CMPCamera *out_camera) {
  TestEnvState *state;

  if (env == NULL || out_camera == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestEnvState *)env;
  if (state->fail_get_camera) {
    return CMP_ERR_IO;
  }

  *out_camera = state->camera;
  return CMP_OK;
}

static const CMPEnvVTable g_test_env_vtable = {NULL, NULL, test_env_get_camera,
                                              NULL, NULL, NULL, NULL, NULL,
                                              NULL};

static const CMPEnvVTable g_test_env_vtable_no_camera = {NULL, NULL, NULL,
                                                        NULL, NULL, NULL, NULL,
                                                        NULL, NULL};

static int test_env_reset(TestEnvState *env_state,
                          TestCameraState *camera_state,
                          const CMPCameraVTable *vtable) {
  if (env_state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(env_state, 0, sizeof(*env_state));
  env_state->env.ctx = env_state;
  env_state->env.vtable = &g_test_env_vtable;

  if (camera_state != NULL && vtable != NULL) {
    env_state->camera.ctx = camera_state;
    env_state->camera.vtable = vtable;
  }

  return CMP_OK;
}

static int test_session_reset(CMPCameraSession *session,
                              TestCameraState *camera_state,
                              const CMPCameraVTable *vtable, CMPBool opened,
                              CMPBool streaming) {
  if (session == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(session, 0, sizeof(*session));
  if (vtable != NULL) {
    session->camera.ctx = camera_state;
    session->camera.vtable = vtable;
  }
  session->opened = opened;
  session->streaming = streaming;
  return CMP_OK;
}

static int test_camera_config_init(void) {
  CMPCameraSessionConfig config;

  CMP_TEST_EXPECT(cmp_camera_config_init(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_camera_config_init(&config));
  CMP_TEST_ASSERT(config.env == NULL);
  CMP_TEST_ASSERT(config.config.camera_id == CMP_CAMERA_ID_DEFAULT);
  CMP_TEST_ASSERT(config.config.facing == CMP_CAMERA_FACING_UNSPECIFIED);
  CMP_TEST_ASSERT(config.config.width == 0u);
  CMP_TEST_ASSERT(config.config.height == 0u);
  CMP_TEST_ASSERT(config.config.format == CMP_CAMERA_FORMAT_ANY);
  return 0;
}

static int test_camera_init_errors(void) {
  CMPCameraSession session;
  CMPCameraSessionConfig config;
  TestCameraState cam_state;
  TestEnvState env_state;
  CMPCameraVTable vtable_missing;
  CMPEnv env;

  memset(&session, 0, sizeof(session));
  CMP_TEST_OK(cmp_camera_config_init(&config));

  CMP_TEST_EXPECT(cmp_camera_init(NULL, &config), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_camera_init(&session, NULL), CMP_ERR_INVALID_ARGUMENT);

  session.camera.vtable = &g_test_camera_vtable;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_STATE);
  memset(&session, 0, sizeof(session));

  config.env = NULL;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_INVALID_ARGUMENT);

  env.ctx = NULL;
  env.vtable = NULL;
  config.env = &env;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_INVALID_ARGUMENT);

  env.vtable = &g_test_env_vtable_no_camera;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
  config.env = &env_state.env;

  env_state.fail_get_camera = 1;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_IO);
  env_state.fail_get_camera = 0;

  env_state.camera.ctx = NULL;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_INVALID_ARGUMENT);

  env_state.camera.ctx = &cam_state;
  env_state.camera.vtable = NULL;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_INVALID_ARGUMENT);

  vtable_missing = g_test_camera_vtable;
  vtable_missing.open = NULL;
  vtable_missing.open_with_config = NULL;
  env_state.camera.vtable = &vtable_missing;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_UNSUPPORTED);

  env_state.camera.vtable = &g_test_camera_vtable;
  cam_state.fail_open = 1;
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_IO);
  cam_state.fail_open = 0;

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
  config.env = &env_state.env;

  vtable_missing = g_test_camera_vtable;
  vtable_missing.open_with_config = NULL;
  env_state.camera.vtable = &vtable_missing;
  memset(&session, 0, sizeof(session));
  CMP_TEST_OK(cmp_camera_init(&session, &config));
  CMP_TEST_ASSERT(cam_state.open_calls == 1);
  CMP_TEST_OK(cmp_camera_shutdown(&session));

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_env_reset(&env_state, &cam_state, &vtable_missing));
  config.env = &env_state.env;
  cam_state.fail_open = 1;
  memset(&session, 0, sizeof(session));
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_IO);
  cam_state.fail_open = 0;

  vtable_missing = g_test_camera_vtable;
  vtable_missing.open = NULL;
  vtable_missing.open_with_config = NULL;
  env_state.camera.vtable = &vtable_missing;
  memset(&session, 0, sizeof(session));
  CMP_TEST_EXPECT(cmp_camera_test_set_skip_vtable_check(2),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_camera_test_set_skip_vtable_check(CMP_TRUE));
  CMP_TEST_EXPECT(cmp_camera_init(&session, &config), CMP_ERR_UNSUPPORTED);
  CMP_TEST_OK(cmp_camera_test_set_skip_vtable_check(CMP_FALSE));

  return 0;
}

static int test_camera_shutdown_errors(void) {
  CMPCameraSession session;
  TestCameraState cam_state;
  CMPCameraVTable vtable_missing;

  CMP_TEST_EXPECT(cmp_camera_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);

  memset(&session, 0, sizeof(session));
  CMP_TEST_EXPECT(cmp_camera_shutdown(&session), CMP_ERR_STATE);

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_FALSE));
  session.camera.ctx = NULL;
  CMP_TEST_EXPECT(cmp_camera_shutdown(&session), CMP_ERR_INVALID_ARGUMENT);

  vtable_missing = g_test_camera_vtable;
  vtable_missing.close = NULL;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, CMP_TRUE,
                                CMP_FALSE));
  CMP_TEST_EXPECT(cmp_camera_shutdown(&session), CMP_ERR_UNSUPPORTED);

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_TRUE));
  cam_state.fail_stop = 1;
  CMP_TEST_EXPECT(cmp_camera_shutdown(&session), CMP_ERR_IO);
  cam_state.fail_stop = 0;

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_FALSE));
  cam_state.fail_close = 1;
  CMP_TEST_EXPECT(cmp_camera_shutdown(&session), CMP_ERR_IO);
  cam_state.fail_close = 0;

  return 0;
}

static int test_camera_shutdown_no_stream(void) {
  CMPCameraSession session;
  CMPCameraSessionConfig config;
  TestCameraState cam_state;
  TestEnvState env_state;

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
  CMP_TEST_OK(cmp_camera_config_init(&config));
  config.env = &env_state.env;
  config.config.camera_id = 7u;
  config.config.facing = CMP_CAMERA_FACING_BACK;
  config.config.width = 640u;
  config.config.height = 480u;
  config.config.format = CMP_CAMERA_FORMAT_BGRA8;

  memset(&session, 0, sizeof(session));
  CMP_TEST_OK(cmp_camera_init(&session, &config));
  CMP_TEST_ASSERT(session.opened == CMP_TRUE);
  CMP_TEST_ASSERT(session.streaming == CMP_FALSE);
  CMP_TEST_ASSERT(session.camera_id == 7u);
  CMP_TEST_ASSERT(cam_state.open_with_config_calls == 1);
  CMP_TEST_ASSERT(cam_state.last_config.camera_id == 7u);
  CMP_TEST_ASSERT(cam_state.last_config.facing == CMP_CAMERA_FACING_BACK);
  CMP_TEST_ASSERT(cam_state.last_config.width == 640u);
  CMP_TEST_ASSERT(cam_state.last_config.height == 480u);
  CMP_TEST_ASSERT(cam_state.last_config.format == CMP_CAMERA_FORMAT_BGRA8);

  CMP_TEST_OK(cmp_camera_shutdown(&session));
  CMP_TEST_ASSERT(session.camera.vtable == NULL);
  CMP_TEST_ASSERT(cam_state.stop_calls == 0);
  CMP_TEST_ASSERT(cam_state.close_calls == 1);
  return 0;
}

static int test_camera_shutdown_with_stream(void) {
  CMPCameraSession session;
  CMPCameraSessionConfig config;
  TestCameraState cam_state;
  TestEnvState env_state;

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
  CMP_TEST_OK(cmp_camera_config_init(&config));
  config.env = &env_state.env;

  memset(&session, 0, sizeof(session));
  CMP_TEST_OK(cmp_camera_init(&session, &config));
  CMP_TEST_OK(cmp_camera_start(&session));

  CMP_TEST_OK(cmp_camera_shutdown(&session));
  CMP_TEST_ASSERT(cam_state.stop_calls == 1);
  CMP_TEST_ASSERT(cam_state.close_calls == 1);
  return 0;
}

static int test_camera_start_errors(void) {
  CMPCameraSession session;
  TestCameraState cam_state;
  CMPCameraVTable vtable_missing;

  CMP_TEST_EXPECT(cmp_camera_start(NULL), CMP_ERR_INVALID_ARGUMENT);

  memset(&session, 0, sizeof(session));
  CMP_TEST_EXPECT(cmp_camera_start(&session), CMP_ERR_STATE);

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_TRUE));
  CMP_TEST_EXPECT(cmp_camera_start(&session), CMP_ERR_STATE);

  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_FALSE));
  session.camera.ctx = NULL;
  CMP_TEST_EXPECT(cmp_camera_start(&session), CMP_ERR_INVALID_ARGUMENT);

  vtable_missing = g_test_camera_vtable;
  vtable_missing.start = NULL;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, CMP_TRUE,
                                CMP_FALSE));
  CMP_TEST_EXPECT(cmp_camera_start(&session), CMP_ERR_UNSUPPORTED);

  cam_state.fail_start = 1;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_FALSE));
  CMP_TEST_EXPECT(cmp_camera_start(&session), CMP_ERR_IO);
  cam_state.fail_start = 0;

  return 0;
}

static int test_camera_stop_errors(void) {
  CMPCameraSession session;
  TestCameraState cam_state;
  CMPCameraVTable vtable_missing;

  CMP_TEST_EXPECT(cmp_camera_stop(NULL), CMP_ERR_INVALID_ARGUMENT);

  memset(&session, 0, sizeof(session));
  CMP_TEST_EXPECT(cmp_camera_stop(&session), CMP_ERR_STATE);

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_FALSE));
  CMP_TEST_EXPECT(cmp_camera_stop(&session), CMP_ERR_STATE);

  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_TRUE));
  session.camera.ctx = NULL;
  CMP_TEST_EXPECT(cmp_camera_stop(&session), CMP_ERR_INVALID_ARGUMENT);

  vtable_missing = g_test_camera_vtable;
  vtable_missing.stop = NULL;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, CMP_TRUE,
                                CMP_TRUE));
  CMP_TEST_EXPECT(cmp_camera_stop(&session), CMP_ERR_UNSUPPORTED);

  cam_state.fail_stop = 1;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_TRUE));
  CMP_TEST_EXPECT(cmp_camera_stop(&session), CMP_ERR_IO);
  cam_state.fail_stop = 0;

  return 0;
}

static int test_camera_read_errors(void) {
  CMPCameraSession session;
  TestCameraState cam_state;
  CMPCameraVTable vtable_missing;
  CMPCameraFrame frame;
  CMPBool has_frame;

  CMP_TEST_EXPECT(cmp_camera_read_frame(NULL, &frame, &has_frame),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, NULL, &has_frame),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, &frame, NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&session, 0, sizeof(session));
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, &frame, &has_frame),
                 CMP_ERR_STATE);

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_FALSE));
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, &frame, &has_frame),
                 CMP_ERR_STATE);

  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_TRUE));
  session.camera.ctx = NULL;
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, &frame, &has_frame),
                 CMP_ERR_INVALID_ARGUMENT);

  vtable_missing = g_test_camera_vtable;
  vtable_missing.read_frame = NULL;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &vtable_missing, CMP_TRUE,
                                CMP_TRUE));
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, &frame, &has_frame),
                 CMP_ERR_UNSUPPORTED);

  cam_state.fail_read = 1;
  CMP_TEST_OK(test_session_reset(&session, &cam_state, &g_test_camera_vtable,
                                CMP_TRUE, CMP_TRUE));
  CMP_TEST_EXPECT(cmp_camera_read_frame(&session, &frame, &has_frame), CMP_ERR_IO);
  cam_state.fail_read = 0;

  return 0;
}

static int test_camera_read_success(void) {
  CMPCameraSession session;
  CMPCameraSessionConfig config;
  TestCameraState cam_state;
  TestEnvState env_state;
  CMPCameraFrame frame;
  CMPBool has_frame;
  cmp_u8 data[4];

  data[0] = 1u;
  data[1] = 2u;
  data[2] = 3u;
  data[3] = 4u;

  CMP_TEST_OK(test_camera_state_reset(&cam_state));
  cam_state.frame.format = CMP_CAMERA_FORMAT_RGBA8;
  cam_state.frame.width = 2u;
  cam_state.frame.height = 2u;
  cam_state.frame.data = data;
  cam_state.frame.size = (cmp_usize)sizeof(data);
  cam_state.has_frame = CMP_TRUE;

  CMP_TEST_OK(test_env_reset(&env_state, &cam_state, &g_test_camera_vtable));
  CMP_TEST_OK(cmp_camera_config_init(&config));
  config.env = &env_state.env;

  memset(&session, 0, sizeof(session));
  CMP_TEST_OK(cmp_camera_init(&session, &config));
  CMP_TEST_OK(cmp_camera_start(&session));
  CMP_TEST_OK(cmp_camera_read_frame(&session, &frame, &has_frame));
  CMP_TEST_ASSERT(has_frame == CMP_TRUE);
  CMP_TEST_ASSERT(frame.size == (cmp_usize)sizeof(data));
  CMP_TEST_OK(cmp_camera_stop(&session));
  CMP_TEST_OK(cmp_camera_shutdown(&session));

  return 0;
}

static int test_camera_copy_frame(void) {
  CMPCameraFrame frame;
  cmp_u8 buffer[8];
  cmp_usize out_size;

  CMP_TEST_EXPECT(cmp_camera_copy_frame(NULL, buffer, sizeof(buffer), &out_size),
                 CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_camera_copy_frame(&frame, buffer, sizeof(buffer), NULL),
                 CMP_ERR_INVALID_ARGUMENT);

  memset(&frame, 0, sizeof(frame));
  frame.size = 4u;
  frame.data = NULL;
  CMP_TEST_EXPECT(
      cmp_camera_copy_frame(&frame, buffer, sizeof(buffer), &out_size),
      CMP_ERR_INVALID_ARGUMENT);

  frame.data = buffer;
  CMP_TEST_EXPECT(cmp_camera_copy_frame(&frame, NULL, sizeof(buffer), &out_size),
                 CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_EXPECT(cmp_camera_copy_frame(&frame, buffer, 2u, &out_size),
                 CMP_ERR_RANGE);

  buffer[0] = 9u;
  buffer[1] = 8u;
  buffer[2] = 7u;
  buffer[3] = 6u;
  CMP_TEST_OK(
      cmp_camera_copy_frame(&frame, buffer + 4, sizeof(buffer) - 4u, &out_size));
  CMP_TEST_ASSERT(out_size == 4u);
  CMP_TEST_ASSERT(buffer[4] == 9u);
  CMP_TEST_ASSERT(buffer[7] == 6u);

  frame.size = 0u;
  frame.data = NULL;
  CMP_TEST_OK(cmp_camera_copy_frame(&frame, NULL, 0u, &out_size));
  CMP_TEST_ASSERT(out_size == 0u);

  return 0;
}

int main(void) {
  CMP_TEST_ASSERT(test_camera_config_init() == 0);
  CMP_TEST_ASSERT(test_camera_init_errors() == 0);
  CMP_TEST_ASSERT(test_camera_shutdown_errors() == 0);
  CMP_TEST_ASSERT(test_camera_shutdown_no_stream() == 0);
  CMP_TEST_ASSERT(test_camera_shutdown_with_stream() == 0);
  CMP_TEST_ASSERT(test_camera_start_errors() == 0);
  CMP_TEST_ASSERT(test_camera_stop_errors() == 0);
  CMP_TEST_ASSERT(test_camera_read_errors() == 0);
  CMP_TEST_ASSERT(test_camera_read_success() == 0);
  CMP_TEST_ASSERT(test_camera_copy_frame() == 0);
  return 0;
}
