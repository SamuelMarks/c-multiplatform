#include "test_utils.h"

#include "cmpc/cmp_backend_null.h"
#include "cmpc/cmp_log.h"
#include "cmpc/cmp_object.h"
#include "cmpc/cmp_path.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  cmp_usize alloc_calls;
  cmp_usize realloc_calls;
  cmp_usize free_calls;
  cmp_usize fail_alloc_on;
  cmp_usize fail_realloc_on;
  cmp_usize fail_free_on;
  void *fail_free_ptr;
} TestAllocator;

static int test_allocator_reset(TestAllocator *alloc) {
  if (alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  alloc->alloc_calls = 0;
  alloc->realloc_calls = 0;
  alloc->free_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_realloc_on = 0;
  alloc->fail_free_on = 0;
  alloc->fail_free_ptr = NULL;
  return CMP_OK;
}

static int test_alloc(void *ctx, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }
  return CMP_OK;
}

static int test_realloc(void *ctx, void *ptr, cmp_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on != 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return CMP_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_ptr != NULL && alloc->fail_free_ptr == ptr) {
    return CMP_ERR_UNKNOWN;
  }
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return CMP_ERR_UNKNOWN;
  }

  free(ptr);
  return CMP_OK;
}

static int test_allocator_make(TestAllocator *state, CMPAllocator *out_alloc) {
  if (state == NULL || out_alloc == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  out_alloc->ctx = state;
  out_alloc->alloc = test_alloc;
  out_alloc->realloc = test_realloc;
  out_alloc->free = test_free;
  return CMP_OK;
}

typedef struct TestLogSink {
  int rc;
} TestLogSink;

static int test_log_sink(void *ctx, CMPLogLevel level, const char *tag,
                         const char *message, cmp_usize length) {
  TestLogSink *sink;

  CMP_UNUSED(level);
  CMP_UNUSED(tag);
  CMP_UNUSED(message);
  CMP_UNUSED(length);

  if (ctx == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  sink = (TestLogSink *)ctx;
  return sink->rc;
}

typedef struct TestTaskState {
  int calls;
  int last_value;
} TestTaskState;

static int test_task_ok(void *user) {
  TestTaskState *state;

  if (user == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestTaskState *)user;
  state->calls += 1;
  state->last_value = 1;
  return CMP_OK;
}

static int test_task_fail(void *user) {
  TestTaskState *state;

  if (user == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (TestTaskState *)user;
  state->calls += 1;
  state->last_value = -1;
  return CMP_ERR_UNKNOWN;
}

static int test_obj_destroy(void *obj) {
  CMP_UNUSED(obj);
  return CMP_OK;
}

static int test_obj_retain(void *obj) {
  CMP_UNUSED(obj);
  return CMP_OK;
}

static int test_obj_release(void *obj) {
  CMP_UNUSED(obj);
  return CMP_OK;
}

static int test_obj_get_type_id(void *obj, cmp_u32 *out_type_id) {
  if (obj == NULL || out_type_id == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  *out_type_id = 42u;
  return CMP_OK;
}

static const CMPObjectVTable g_test_object_vtable = {
    test_obj_retain, test_obj_release, test_obj_destroy, test_obj_get_type_id};

#define CMP_TEST_NULL_FAIL_SWEEP(call_expr, success_cleanup)                   \
  do {                                                                         \
    cmp_usize cmp_test_fail_index = 1u;                                        \
    int cmp_test_rc = CMP_OK;                                                  \
    CMPBool cmp_test_done = CMP_FALSE;                                         \
    for (; cmp_test_fail_index <= 64u; ++cmp_test_fail_index) {                \
      CMP_TEST_OK(cmp_null_backend_test_set_fail_after(cmp_test_fail_index));  \
      cmp_test_rc = (call_expr);                                               \
      if (cmp_test_rc == CMP_OK) {                                             \
        CMP_TEST_OK(cmp_null_backend_test_clear_fail_after());                 \
        success_cleanup;                                                       \
        cmp_test_done = CMP_TRUE;                                              \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    CMP_TEST_OK(cmp_null_backend_test_clear_fail_after());                     \
    CMP_TEST_ASSERT(cmp_test_done == CMP_TRUE);                                \
  } while (0)

#define CMP_TEST_NULL_FORCE_FAIL(call_expr)                                    \
  do {                                                                         \
    CMP_TEST_OK(cmp_null_backend_test_set_fail_after(1u));                     \
    CMP_TEST_EXPECT((call_expr), CMP_ERR_IO);                                  \
    CMP_TEST_OK(cmp_null_backend_test_clear_fail_after());                     \
  } while (0)

#define CMP_TEST_NULL_FORCE_FAIL_AT(call_expr, idx)                            \
  do {                                                                         \
    CMP_TEST_OK(cmp_null_backend_test_set_fail_after((idx)));                  \
    CMP_TEST_EXPECT((call_expr), CMP_ERR_IO);                                  \
    CMP_TEST_OK(cmp_null_backend_test_clear_fail_after());                     \
  } while (0)

int main(void) {
  {
    int rc;

    rc = cmp_log_shutdown();
    CMP_TEST_ASSERT(rc == CMP_OK || rc == CMP_ERR_STATE);
  }

  {
    CMPNullBackendConfig config;

    CMP_TEST_EXPECT(cmp_null_backend_config_init(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    CMP_TEST_ASSERT(config.handle_capacity != 0);
    CMP_TEST_ASSERT(config.clipboard_limit != 0);
    CMP_TEST_ASSERT(config.enable_logging == CMP_TRUE);
    CMP_TEST_ASSERT(config.inline_tasks == CMP_TRUE);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPAllocator bad_alloc;
    TestAllocator alloc_state;
    CMPAllocator alloc;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    backend = NULL;
    CMP_TEST_EXPECT(cmp_null_backend_create(&config, NULL),
                    CMP_ERR_INVALID_ARGUMENT);

    config.handle_capacity = 0;
    CMP_TEST_EXPECT(cmp_null_backend_create(&config, &backend),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_ASSERT(backend == NULL);

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    memset(&bad_alloc, 0, sizeof(bad_alloc));
    config.allocator = &bad_alloc;
    CMP_TEST_EXPECT(cmp_null_backend_create(&config, &backend),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_TRUE));
    CMP_TEST_EXPECT(cmp_null_backend_create(NULL, &backend), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_core_test_set_default_allocator_fail(CMP_FALSE));

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_alloc_on = 1;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_EXPECT(cmp_null_backend_create(&config, &backend),
                    CMP_ERR_OUT_OF_MEMORY);

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_alloc_on = 2;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_EXPECT(cmp_null_backend_create(&config, &backend),
                    CMP_ERR_OUT_OF_MEMORY);

    CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_TRUE, CMP_FALSE, CMP_FALSE,
                                                CMP_FALSE));
    CMP_TEST_EXPECT(cmp_null_backend_create(NULL, &backend), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                                CMP_FALSE));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    int rc;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.handle_capacity = 8;
    config.enable_logging = CMP_TRUE;

    rc = cmp_log_shutdown();
    CMP_TEST_ASSERT(rc == CMP_OK || rc == CMP_ERR_STATE);

    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_destroy(backend));

    rc = cmp_log_shutdown();
    CMP_TEST_EXPECT(rc, CMP_ERR_STATE);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    int rc;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.handle_capacity = 4;
    config.enable_logging = CMP_TRUE;

    rc = cmp_log_shutdown();
    CMP_TEST_ASSERT(rc == CMP_OK || rc == CMP_ERR_STATE);

    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                                CMP_TRUE));
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_log_test_set_mutex_failures(CMP_FALSE, CMP_FALSE, CMP_FALSE,
                                                CMP_FALSE));
    CMP_TEST_OK(cmp_log_shutdown());
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    config.inline_tasks = CMP_TRUE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_UNKNOWN);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestLogSink sink;
    CMPLogSink log_sink;
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPIO io;
    CMPSensors sensors;
    CMPCamera camera;
    CMPImage image;
    CMPVideo video;
    CMPAudio audio;
    CMPNetwork network;
    CMPTasks tasks;
    CMPAllocator default_alloc = {0};
    CMPAllocator bad_alloc = {0};
    CMPAllocator partial_alloc = {0};
    CMPWSConfig ws_config;
    CMPWSWindowConfig win_config;
    CMPHandle window;
    CMPHandle window2;
    CMPHandle texture;
    CMPHandle font;
    CMPHandle invalid_handle;
    CMPRect rect;
    CMPRect rect_small;
    CMPMat3 transform;
    CMPColor color;
    CMPPath path;
    char clip_buffer[16];
    cmp_usize clip_len;
    cmp_i32 width;
    cmp_i32 height;
    CMPScalar scale;
    CMPBool has_event;
    CMPInputEvent event;
    cmp_u32 time_ms;
    CMPFileInfo file_info;
    CMPSensorReading reading;
    CMPCameraFrame frame;
    CMPImageDecodeRequest image_req;
    CMPImageData image_data;
    CMPVideoOpenRequest video_req;
    CMPVideoFrame video_frame;
    CMPAudioDecodeRequest audio_req;
    CMPAudioData audio_data;
    CMPNetworkRequest net_req;
    CMPNetworkResponse net_resp;
    TestTaskState task_state;
    void *data;
    int rc;

    CMP_TEST_OK(cmp_log_init(NULL));
    sink.rc = CMP_OK;
    log_sink.ctx = &sink;
    log_sink.write = test_log_sink;
    CMP_TEST_OK(cmp_log_set_sink(&log_sink));

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.handle_capacity = 16;
    config.clipboard_limit = 8;
    config.enable_logging = CMP_TRUE;
    config.inline_tasks = CMP_TRUE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));

    CMP_TEST_EXPECT(cmp_null_backend_destroy(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_get_ws(NULL, &ws),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_get_ws(backend, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_null_backend_test_set_initialized(backend, CMP_FALSE));
    CMP_TEST_EXPECT(cmp_null_backend_get_ws(backend, &ws), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_null_backend_get_gfx(backend, &gfx), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_null_backend_get_env(backend, &env), CMP_ERR_STATE);
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_STATE);
    CMP_TEST_OK(cmp_null_backend_test_set_initialized(backend, CMP_TRUE));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));

    CMP_TEST_EXPECT(cmp_null_backend_get_gfx(NULL, &gfx),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_get_gfx(backend, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_null_backend_get_gfx(backend, &gfx));

    CMP_TEST_EXPECT(cmp_null_backend_get_env(NULL, &env),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_get_env(backend, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_null_backend_get_env(backend, &env));

    sink.rc = CMP_ERR_IO;
    CMP_TEST_EXPECT(ws.vtable->pump_events(ws.ctx), CMP_ERR_IO);
    sink.rc = CMP_OK;

    ws_config.utf8_app_name = "TestApp";
    ws_config.utf8_app_id = "com.example.test";
    ws_config.reserved = 0;
    CMP_TEST_EXPECT(ws.vtable->init(NULL, &ws_config),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->init(ws.ctx, NULL), CMP_ERR_INVALID_ARGUMENT);
    ws_config.utf8_app_name = NULL;
    CMP_TEST_EXPECT(ws.vtable->init(ws.ctx, &ws_config),
                    CMP_ERR_INVALID_ARGUMENT);
    ws_config.utf8_app_name = "TestApp";
    ws_config.utf8_app_id = NULL;
    CMP_TEST_EXPECT(ws.vtable->init(ws.ctx, &ws_config),
                    CMP_ERR_INVALID_ARGUMENT);
    ws_config.utf8_app_id = "com.example.test";
    CMP_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));

    win_config.width = 640;
    win_config.height = 480;
    win_config.utf8_title = "Main";
    win_config.flags = CMP_WS_WINDOW_RESIZABLE;
    CMP_TEST_EXPECT(ws.vtable->create_window(NULL, &win_config, &window),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->create_window(ws.ctx, NULL, &window),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    win_config.utf8_title = NULL;
    CMP_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, &window),
                    CMP_ERR_INVALID_ARGUMENT);
    win_config.utf8_title = "Main";
    win_config.width = 0;
    CMP_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, &window),
                    CMP_ERR_RANGE);
    win_config.width = 640;
    win_config.height = 0;
    CMP_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, &window),
                    CMP_ERR_RANGE);
    win_config.height = 480;
    CMP_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));

    invalid_handle.id = 0;
    invalid_handle.generation = 0;
    CMP_TEST_EXPECT(ws.vtable->shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->destroy_window(NULL, window),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->show_window(NULL, window),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->hide_window(NULL, window),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->set_window_title(NULL, window, "Title"),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->set_window_size(NULL, window, 1, 1),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->get_window_size(NULL, window, &width, &height),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->set_window_dpi_scale(NULL, window, 1.0f),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->get_window_dpi_scale(NULL, window, &scale),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->set_clipboard_text(NULL, "abc"),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->get_clipboard_text(
                        NULL, clip_buffer, sizeof(clip_buffer), &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->poll_event(NULL, &event, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->get_time_ms(NULL, &time_ms),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->show_window(ws.ctx, invalid_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->show_window(ws.ctx, window));
    CMP_TEST_OK(ws.vtable->hide_window(ws.ctx, window));

    CMP_TEST_EXPECT(ws.vtable->set_window_title(ws.ctx, window, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->set_window_title(ws.ctx, window, "Title"));

    CMP_TEST_EXPECT(ws.vtable->set_window_size(ws.ctx, window, 0, 1),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(ws.vtable->set_window_size(ws.ctx, window, 1, 0),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(ws.vtable->set_window_size(ws.ctx, window, 800, 600));
    CMP_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window, NULL, &height),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window, &width, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->get_window_size(ws.ctx, window, &width, &height));
    CMP_TEST_ASSERT(width == 800);
    CMP_TEST_ASSERT(height == 600);

    CMP_TEST_EXPECT(ws.vtable->set_window_dpi_scale(ws.ctx, window, 0.0f),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(ws.vtable->set_window_dpi_scale(ws.ctx, window, 2.0f));
    CMP_TEST_EXPECT(ws.vtable->get_window_dpi_scale(ws.ctx, window, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->get_window_dpi_scale(ws.ctx, window, &scale));
    CMP_TEST_ASSERT(scale == 2.0f);

    clip_buffer[0] = 'x';
    CMP_TEST_EXPECT(ws.vtable->get_clipboard_text(ws.ctx, NULL, 0, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->get_clipboard_text(ws.ctx, clip_buffer, 0, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->set_clipboard_text(ws.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->get_clipboard_text(ws.ctx, clip_buffer,
                                              sizeof(clip_buffer), &clip_len));
    CMP_TEST_ASSERT(clip_len == 0);

    CMP_TEST_EXPECT(ws.vtable->set_clipboard_text(ws.ctx, "123456789"),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "abc"));
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "abcdef"));
    CMP_TEST_EXPECT(
        ws.vtable->get_clipboard_text(ws.ctx, clip_buffer, 4, &clip_len),
        CMP_ERR_RANGE);
    CMP_TEST_OK(ws.vtable->get_clipboard_text(ws.ctx, clip_buffer,
                                              sizeof(clip_buffer), &clip_len));
    CMP_TEST_ASSERT(clip_len == 6);
    CMP_TEST_ASSERT(strcmp(clip_buffer, "abcdef") == 0);
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "a"));

    CMP_TEST_EXPECT(ws.vtable->poll_event(ws.ctx, NULL, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(ws.vtable->poll_event(ws.ctx, &event, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->poll_event(ws.ctx, &event, &has_event));
    CMP_TEST_ASSERT(has_event == CMP_FALSE);

    CMP_TEST_EXPECT(ws.vtable->pump_events(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->pump_events(ws.ctx));
    CMP_TEST_EXPECT(ws.vtable->get_time_ms(ws.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));
    CMP_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));

    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = 10.0f;
    rect.height = 10.0f;
    rect_small.x = 0.0f;
    rect_small.y = 0.0f;
    rect_small.width = -1.0f;
    rect_small.height = 1.0f;
    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0f;

    CMP_TEST_EXPECT(gfx.vtable->begin_frame(NULL, window, 1, 1, 1.0f),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, window, 0, 1, 1.0f),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, window, 1, 0, 1.0f),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, window, 1, 1, 0.0f),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.vtable->begin_frame(gfx.ctx, invalid_handle, 1, 1, 1.0f),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.vtable->begin_frame(gfx.ctx, window, 1, 1, 1.0f));

    CMP_TEST_EXPECT(gfx.vtable->end_frame(NULL, window),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->end_frame(gfx.ctx, invalid_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.vtable->end_frame(gfx.ctx, window));

    CMP_TEST_EXPECT(gfx.vtable->clear(NULL, color), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.vtable->clear(gfx.ctx, color));

    CMP_TEST_EXPECT(gfx.vtable->draw_rect(NULL, &rect, color, 0.0f),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, NULL, color, 0.0f),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, &rect_small, color, 0.0f),
                    CMP_ERR_RANGE);
    rect_small.width = 1.0f;
    rect_small.height = -1.0f;
    CMP_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, &rect_small, color, 0.0f),
                    CMP_ERR_RANGE);
    rect_small.width = -1.0f;
    rect_small.height = 1.0f;
    CMP_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, &rect, color, -1.0f),
                    CMP_ERR_RANGE);
    CMP_TEST_OK(gfx.vtable->draw_rect(gfx.ctx, &rect, color, 0.0f));

    CMP_TEST_EXPECT(
        gfx.vtable->draw_line(NULL, 0.0f, 0.0f, 1.0f, 1.0f, color, 1.0f),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_line(gfx.ctx, 0.0f, 0.0f, 1.0f, 1.0f, color, -1.0f),
        CMP_ERR_RANGE);
    CMP_TEST_OK(
        gfx.vtable->draw_line(gfx.ctx, 0.0f, 0.0f, 1.0f, 1.0f, color, 1.0f));

    memset(&path, 0, sizeof(path));
    CMP_TEST_EXPECT(gfx.vtable->draw_path(NULL, &path, color),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->draw_path(gfx.ctx, NULL, color),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->draw_path(gfx.ctx, &path, color),
                    CMP_ERR_STATE);
    CMP_TEST_OK(cmp_path_init(&path, NULL, 0));
    CMP_TEST_OK(cmp_path_move_to(&path, 0.0f, 0.0f));
    CMP_TEST_OK(gfx.vtable->draw_path(gfx.ctx, &path, color));
    path.count = path.capacity + 1u;
    CMP_TEST_EXPECT(gfx.vtable->draw_path(gfx.ctx, &path, color),
                    CMP_ERR_STATE);
    path.count = 1u;
    CMP_TEST_OK(gfx.vtable->draw_path(gfx.ctx, &path, color));
    CMP_TEST_OK(cmp_path_shutdown(&path));

    CMP_TEST_EXPECT(gfx.vtable->push_clip(NULL, &rect),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->push_clip(gfx.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->push_clip(gfx.ctx, &rect_small), CMP_ERR_RANGE);
    rect_small.width = 1.0f;
    rect_small.height = -1.0f;
    CMP_TEST_EXPECT(gfx.vtable->push_clip(gfx.ctx, &rect_small), CMP_ERR_RANGE);
    rect_small.width = -1.0f;
    rect_small.height = 1.0f;
    CMP_TEST_OK(gfx.vtable->push_clip(gfx.ctx, &rect));

    CMP_TEST_EXPECT(gfx.vtable->pop_clip(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.vtable->pop_clip(gfx.ctx));

    memset(&transform, 0, sizeof(transform));
    CMP_TEST_EXPECT(gfx.vtable->set_transform(NULL, &transform),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->set_transform(gfx.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.vtable->set_transform(gfx.ctx, &transform));

    CMP_TEST_EXPECT(gfx.vtable->create_texture(NULL, 1, 1, CMP_TEX_FORMAT_RGBA8,
                                               NULL, 0, &texture),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->create_texture(
                        gfx.ctx, 0, 1, CMP_TEX_FORMAT_RGBA8, NULL, 0, &texture),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(gfx.vtable->create_texture(
                        gfx.ctx, 1, 0, CMP_TEX_FORMAT_RGBA8, NULL, 0, &texture),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.vtable->create_texture(gfx.ctx, 1, 1, 99, NULL, 0, &texture),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->create_texture(
                        gfx.ctx, 1, 1, CMP_TEX_FORMAT_RGBA8, NULL, 4, &texture),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->create_texture(
                        gfx.ctx, 1, 1, CMP_TEX_FORMAT_RGBA8, NULL, 0, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 1, 1, CMP_TEX_FORMAT_BGRA8,
                                           NULL, 0, &texture));
    CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));
    CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 1, 1, CMP_TEX_FORMAT_A8,
                                           NULL, 0, &texture));
    CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));
    CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 1, 1, CMP_TEX_FORMAT_RGBA8,
                                           NULL, 0, &texture));

    CMP_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, texture, 1, 1, 1.0f),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(
        gfx.vtable->update_texture(NULL, texture, 0, 0, 1, 1, NULL, 0),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->update_texture(gfx.ctx, invalid_handle, 0, 0, 1,
                                               1, NULL, 0),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, window, 0, 0, 1, 1, NULL, 0),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, -1, 0, 1, 1, NULL, 0),
        CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 0, 1, NULL, 0),
        CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 0, NULL, 0),
        CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, NULL, 4),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, NULL, 0));

    CMP_TEST_EXPECT(gfx.vtable->destroy_texture(NULL, texture),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, invalid_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, window),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_EXPECT(gfx.vtable->draw_texture(NULL, texture, &rect, &rect, 1.0f),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, invalid_handle, &rect, &rect, 1.0f),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, window, &rect, &rect, 1.0f),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, NULL, &rect, 1.0f),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, NULL, 1.0f),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, -1.0f),
        CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, 2.0f),
        CMP_ERR_RANGE);
    CMP_TEST_OK(gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, 1.0f));

    CMP_TEST_EXPECT(
        gfx.text_vtable->create_font(NULL, "Sans", 12, 400, CMP_FALSE, &font),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, NULL, 12, 400, CMP_FALSE, &font),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 0, 400, CMP_FALSE, &font),
        CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 50, CMP_FALSE, &font),
        CMP_ERR_RANGE);
    CMP_TEST_EXPECT(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 950,
                                                 CMP_FALSE, &font),
                    CMP_ERR_RANGE);
    CMP_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400, CMP_FALSE, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400,
                                             CMP_FALSE, &font));

    CMP_TEST_EXPECT(gfx.text_vtable->measure_text(NULL, font, "hi", 2, &scale,
                                                  &scale, &scale),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, invalid_handle, "hi",
                                                  2, &scale, &scale, &scale),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, font, NULL, 2,
                                                  &scale, &scale, &scale),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, NULL,
                                                  &scale, &scale),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2,
                                                  &scale, NULL, &scale),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2,
                                                  &scale, &scale, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &scale,
                                              &scale, &scale));

    CMP_TEST_EXPECT(
        gfx.text_vtable->draw_text(NULL, font, "hi", 2, 0.0f, 0.0f, color),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->draw_text(gfx.ctx, invalid_handle, "hi", 2,
                                               0.0f, 0.0f, color),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        gfx.text_vtable->draw_text(gfx.ctx, font, NULL, 2, 0.0f, 0.0f, color),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color));

    CMP_TEST_EXPECT(gfx.text_vtable->destroy_font(NULL, font),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->destroy_font(gfx.ctx, invalid_handle),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.text_vtable->destroy_font(gfx.ctx, texture),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));

    CMP_TEST_EXPECT(env.vtable->get_io(NULL, &io), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_io(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_io(env.ctx, &io));

    CMP_TEST_EXPECT(env.vtable->get_sensors(NULL, &sensors),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_sensors(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_sensors(env.ctx, &sensors));

    CMP_TEST_EXPECT(env.vtable->get_camera(NULL, &camera),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_camera(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_camera(env.ctx, &camera));

    CMP_TEST_EXPECT(env.vtable->get_image(NULL, &image),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_image(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_image(env.ctx, &image));

    CMP_TEST_EXPECT(env.vtable->get_video(NULL, &video),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_video(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_video(env.ctx, &video));

    CMP_TEST_EXPECT(env.vtable->get_audio(NULL, &audio),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_audio(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_audio(env.ctx, &audio));

    CMP_TEST_EXPECT(env.vtable->get_network(NULL, &network),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_network(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_network(env.ctx, &network));

    CMP_TEST_EXPECT(env.vtable->get_tasks(NULL, &tasks),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_tasks(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_tasks(env.ctx, &tasks));

    CMP_TEST_EXPECT(env.vtable->get_time_ms(NULL, &time_ms),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(env.vtable->get_time_ms(env.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(env.vtable->get_time_ms(env.ctx, &time_ms));

    CMP_TEST_EXPECT(io.vtable->read_file(NULL, "path", clip_buffer,
                                         sizeof(clip_buffer), &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->read_file(io.ctx, NULL, clip_buffer,
                                         sizeof(clip_buffer), &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->read_file(io.ctx, "path", NULL, 1, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->read_file(io.ctx, "path", clip_buffer,
                                         sizeof(clip_buffer), &clip_len),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(io.vtable->read_file_alloc(NULL, "path", &default_alloc,
                                               &data, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, NULL, &default_alloc,
                                               &data, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &default_alloc,
                                               &data, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &default_alloc,
                                               NULL, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        io.vtable->read_file_alloc(io.ctx, "path", &default_alloc, &data, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    partial_alloc = default_alloc;
    partial_alloc.alloc = NULL;
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &partial_alloc,
                                               &data, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    partial_alloc = default_alloc;
    partial_alloc.realloc = NULL;
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &partial_alloc,
                                               &data, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    partial_alloc = default_alloc;
    partial_alloc.free = NULL;
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &partial_alloc,
                                               &data, &clip_len),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &default_alloc,
                                               &data, &clip_len),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(
        io.vtable->write_file(NULL, "path", clip_buffer, 1, CMP_TRUE),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        io.vtable->write_file(io.ctx, NULL, clip_buffer, 1, CMP_TRUE),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->write_file(io.ctx, "path", NULL, 1, CMP_TRUE),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        io.vtable->write_file(io.ctx, "path", clip_buffer, 1, CMP_TRUE),
        CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(io.vtable->file_exists(NULL, "path", &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->file_exists(io.ctx, NULL, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->file_exists(io.ctx, "path", &has_event),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(io.vtable->delete_file(NULL, "path"),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->delete_file(io.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->delete_file(io.ctx, "path"),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(io.vtable->stat_file(NULL, "path", &file_info),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->stat_file(io.ctx, NULL, &file_info),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(io.vtable->stat_file(io.ctx, "path", &file_info),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(sensors.vtable->is_available(NULL, 0, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(sensors.vtable->is_available(sensors.ctx, 0, &has_event));
    CMP_TEST_EXPECT(sensors.vtable->start(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(sensors.vtable->start(sensors.ctx, 0), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(sensors.vtable->stop(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(sensors.vtable->stop(sensors.ctx, 0), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(sensors.vtable->read(NULL, 0, &reading, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(sensors.vtable->read(sensors.ctx, 0, NULL, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(sensors.vtable->read(sensors.ctx, 0, &reading, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(sensors.vtable->read(sensors.ctx, 0, &reading, &has_event));

    CMP_TEST_EXPECT(camera.vtable->open(NULL, 0), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(camera.vtable->open(camera.ctx, 0), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(camera.vtable->close(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(camera.vtable->close(camera.ctx), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(camera.vtable->start(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(camera.vtable->start(camera.ctx), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(camera.vtable->stop(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(camera.vtable->stop(camera.ctx), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(camera.vtable->read_frame(NULL, &frame, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(camera.vtable->read_frame(camera.ctx, NULL, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(camera.vtable->read_frame(camera.ctx, &frame, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(camera.vtable->read_frame(camera.ctx, &frame, &has_event));

    memset(&image_req, 0, sizeof(image_req));
    CMP_TEST_EXPECT(
        image.vtable->decode(NULL, &image_req, &default_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        image.vtable->decode(image.ctx, NULL, &default_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        image.vtable->decode(image.ctx, &image_req, NULL, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        image.vtable->decode(image.ctx, &image_req, &default_alloc, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    CMP_TEST_EXPECT(
        image.vtable->decode(image.ctx, &image_req, &bad_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    CMP_TEST_EXPECT(
        image.vtable->decode(image.ctx, &image_req, &bad_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    CMP_TEST_EXPECT(
        image.vtable->decode(image.ctx, &image_req, &bad_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(image.vtable->decode(image.ctx, &image_req, &default_alloc,
                                         &image_data),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(image.vtable->free_image(NULL, &default_alloc, &image_data),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(image.vtable->free_image(image.ctx, NULL, &image_data),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(image.vtable->free_image(image.ctx, &default_alloc, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    CMP_TEST_EXPECT(
        image.vtable->free_image(image.ctx, &bad_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    CMP_TEST_EXPECT(
        image.vtable->free_image(image.ctx, &bad_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    CMP_TEST_EXPECT(
        image.vtable->free_image(image.ctx, &bad_alloc, &image_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        image.vtable->free_image(image.ctx, &default_alloc, &image_data),
        CMP_ERR_UNSUPPORTED);

    memset(&video_req, 0, sizeof(video_req));
    CMP_TEST_EXPECT(video.vtable->open(NULL, &video_req),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(video.vtable->open(video.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(video.vtable->open(video.ctx, &video_req),
                    CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(video.vtable->close(NULL), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(video.vtable->close(video.ctx), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(video.vtable->read_frame(NULL, &video_frame, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(video.vtable->read_frame(video.ctx, NULL, &has_event),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(video.vtable->read_frame(video.ctx, &video_frame, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(video.vtable->read_frame(video.ctx, &video_frame, &has_event));

    memset(&audio_req, 0, sizeof(audio_req));
    CMP_TEST_EXPECT(
        audio.vtable->decode(NULL, &audio_req, &default_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        audio.vtable->decode(audio.ctx, NULL, &default_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        audio.vtable->decode(audio.ctx, &audio_req, NULL, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        audio.vtable->decode(audio.ctx, &audio_req, &default_alloc, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    CMP_TEST_EXPECT(
        audio.vtable->decode(audio.ctx, &audio_req, &bad_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    CMP_TEST_EXPECT(
        audio.vtable->decode(audio.ctx, &audio_req, &bad_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    CMP_TEST_EXPECT(
        audio.vtable->decode(audio.ctx, &audio_req, &bad_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(audio.vtable->decode(audio.ctx, &audio_req, &default_alloc,
                                         &audio_data),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(audio.vtable->free_audio(NULL, &default_alloc, &audio_data),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(audio.vtable->free_audio(audio.ctx, NULL, &audio_data),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(audio.vtable->free_audio(audio.ctx, &default_alloc, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    CMP_TEST_EXPECT(
        audio.vtable->free_audio(audio.ctx, &bad_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    CMP_TEST_EXPECT(
        audio.vtable->free_audio(audio.ctx, &bad_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    CMP_TEST_EXPECT(
        audio.vtable->free_audio(audio.ctx, &bad_alloc, &audio_data),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        audio.vtable->free_audio(audio.ctx, &default_alloc, &audio_data),
        CMP_ERR_UNSUPPORTED);

    memset(&net_req, 0, sizeof(net_req));
    CMP_TEST_EXPECT(
        network.vtable->request(NULL, &net_req, &default_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        network.vtable->request(network.ctx, NULL, &default_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        network.vtable->request(network.ctx, &net_req, &default_alloc, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    CMP_TEST_EXPECT(network.vtable->request(network.ctx, &net_req,
                                            &default_alloc, &net_resp),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    CMP_TEST_EXPECT(
        network.vtable->request(network.ctx, &net_req, &bad_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    CMP_TEST_EXPECT(
        network.vtable->request(network.ctx, &net_req, &bad_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    CMP_TEST_EXPECT(
        network.vtable->request(network.ctx, &net_req, &bad_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(network.vtable->request(network.ctx, &net_req,
                                            &default_alloc, &net_resp),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(
        network.vtable->free_response(NULL, &default_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(network.vtable->free_response(network.ctx, NULL, &net_resp),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &default_alloc, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    CMP_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &default_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
    bad_alloc = default_alloc;
    bad_alloc.alloc = NULL;
    CMP_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &bad_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.realloc = NULL;
    CMP_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &bad_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    bad_alloc = default_alloc;
    bad_alloc.free = NULL;
    CMP_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &bad_alloc, &net_resp),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &default_alloc, &net_resp),
        CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(
        tasks.vtable->thread_create(NULL, test_task_ok, &task_state, &window2),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        tasks.vtable->thread_create(tasks.ctx, test_task_ok, &task_state, NULL),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->thread_create(tasks.ctx, test_task_ok,
                                                &task_state, &window2),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(tasks.vtable->thread_join(NULL, window2),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, window2),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(tasks.vtable->mutex_create(NULL, &window2),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_create(tasks.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_create(tasks.ctx, &window2),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(tasks.vtable->mutex_destroy(NULL, window2),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_destroy(tasks.ctx, window2),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(tasks.vtable->mutex_lock(NULL, window2),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_lock(tasks.ctx, window2),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(tasks.vtable->mutex_unlock(NULL, window2),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->mutex_unlock(tasks.ctx, window2),
                    CMP_ERR_UNSUPPORTED);

    CMP_TEST_EXPECT(tasks.vtable->sleep_ms(NULL, 1), CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(tasks.vtable->sleep_ms(tasks.ctx, 1));

    task_state.calls = 0;
    task_state.last_value = 0;
    CMP_TEST_EXPECT(tasks.vtable->task_post(NULL, test_task_ok, &task_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, NULL, &task_state),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(tasks.vtable->task_post(tasks.ctx, test_task_ok, &task_state));
    CMP_TEST_EXPECT(
        tasks.vtable->task_post(tasks.ctx, test_task_fail, &task_state),
        CMP_ERR_UNKNOWN);

    CMP_TEST_EXPECT(
        tasks.vtable->task_post_delayed(NULL, test_task_ok, &task_state, 1),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(
        tasks.vtable->task_post_delayed(tasks.ctx, NULL, &task_state, 1),
        CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(tasks.vtable->task_post_delayed(tasks.ctx, test_task_ok,
                                                &task_state, 1));
    CMP_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, test_task_fail,
                                                    &task_state, 1),
                    CMP_ERR_UNKNOWN);

    CMP_TEST_OK(cmp_object_test_set_handle_is_valid_fail(CMP_TRUE));
    CMP_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window, &width, &height),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_object_test_set_handle_is_valid_fail(CMP_FALSE));

    win_config.width = 320;
    win_config.height = 240;
    win_config.utf8_title = "Second";
    win_config.flags = 0;
    CMP_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window2));
    CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window2));
    CMP_TEST_EXPECT(
        ws.vtable->get_window_size(ws.ctx, window2, &width, &height),
        CMP_ERR_NOT_FOUND);

    CMP_TEST_EXPECT(ws.vtable->show_window(ws.ctx, texture),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, window),
                    CMP_ERR_INVALID_ARGUMENT);

    CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));
    CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));

    CMP_TEST_OK(ws.vtable->shutdown(ws.ctx));
    CMP_TEST_OK(cmp_null_backend_destroy(backend));

    rc = cmp_log_shutdown();
    CMP_TEST_EXPECT(rc, CMP_OK);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPIO io;
    CMPWSConfig ws_config;
    CMPWSWindowConfig win_config;
    CMPHandle window;
    CMPHandle texture;
    CMPHandle font;
    CMPScalar width;
    CMPScalar height;
    CMPScalar baseline;
    CMPColor color;
    char buffer[32];
    cmp_usize length;
    cmp_usize read;
    CMPAllocator default_alloc;
    void *data;
    cmp_usize data_size;

    CMP_TEST_OK(cmp_null_backend_test_maybe_fail_null());
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.enable_logging = CMP_FALSE;
    config.handle_capacity = 16;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));
    CMP_TEST_OK(cmp_null_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(cmp_null_backend_get_env(backend, &env));
    CMP_TEST_OK(env.vtable->get_io(env.ctx, &io));

    ws_config.utf8_app_name = "App";
    ws_config.utf8_app_id = "app.id";
    ws_config.reserved = 0;
    CMP_TEST_NULL_FORCE_FAIL(ws.vtable->init(ws.ctx, &ws_config));
    CMP_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));

    win_config.width = 120;
    win_config.height = 80;
    win_config.utf8_title = "Win";
    win_config.flags = 0;

    CMP_TEST_NULL_FORCE_FAIL(
        ws.vtable->create_window(ws.ctx, &win_config, &window));
    CMP_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));
    CMP_TEST_NULL_FORCE_FAIL(ws.vtable->destroy_window(ws.ctx, window));
    CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));

    CMP_TEST_NULL_FORCE_FAIL(gfx.vtable->create_texture(
        gfx.ctx, 4, 4, CMP_TEX_FORMAT_RGBA8, NULL, 0, &texture));
    CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 4, 4, CMP_TEX_FORMAT_RGBA8,
                                           NULL, 0, &texture));
    CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));

    CMP_TEST_NULL_FORCE_FAIL(gfx.text_vtable->create_font(
        gfx.ctx, "Sans", 12, 400, CMP_FALSE, &font));
    CMP_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400,
                                             CMP_FALSE, &font));
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &width, &height,
                                      &baseline),
        1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &width, &height,
                                      &baseline),
        2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &width, &height,
                                      &baseline),
        3u);
    CMP_TEST_OK(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &width,
                                              &height, &baseline));
    color.r = 1.0f;
    color.g = 1.0f;
    color.b = 1.0f;
    color.a = 1.0f;
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color),
        1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color),
        2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color),
        3u);
    CMP_TEST_OK(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color));
    CMP_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));

    CMP_TEST_NULL_FORCE_FAIL(ws.vtable->set_clipboard_text(ws.ctx, "clip"));
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "clip"));
    CMP_TEST_NULL_FORCE_FAIL(
        ws.vtable->get_clipboard_text(ws.ctx, buffer, sizeof(buffer), &length));
    CMP_TEST_OK(
        ws.vtable->get_clipboard_text(ws.ctx, buffer, sizeof(buffer), &length));

    CMP_TEST_NULL_FORCE_FAIL(
        io.vtable->read_file(io.ctx, "path", buffer, sizeof(buffer), &read));
    CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
    data = NULL;
    data_size = 0;
    CMP_TEST_NULL_FORCE_FAIL(io.vtable->read_file_alloc(
        io.ctx, "path", &default_alloc, &data, &data_size));

    CMP_TEST_NULL_FORCE_FAIL(ws.vtable->shutdown(ws.ctx));
    CMP_TEST_OK(ws.vtable->shutdown(ws.ctx));
    CMP_TEST_OK(cmp_null_backend_destroy(backend));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPWS ws;
    CMPGfx gfx;
    CMPEnv env;
    CMPIO io;
    CMPSensors sensors;
    CMPCamera camera;
    CMPImage image;
    CMPVideo video;
    CMPAudio audio;
    CMPNetwork network;
    CMPTasks tasks;
    CMPWSConfig ws_config;
    CMPWSWindowConfig win_config;
    CMPHandle window;
    CMPHandle texture;
    CMPHandle font;
    CMPHandle task_handle;
    CMPRect rect;
    CMPMat3 transform;
    CMPInputEvent event;
    CMPBool has_event;
    CMPScalar scale;
    cmp_i32 width;
    cmp_i32 height;
    CMPScalar text_width;
    CMPScalar text_height;
    CMPScalar baseline;
    cmp_u32 time_ms;
    cmp_usize length;
    char buffer[32];
    CMPAllocator default_alloc;
    CMPPath path;
    CMPCameraFrame camera_frame;
    CMPBool has_frame;
    CMPVideoFrame video_frame;
    CMPVideoOpenRequest video_request;
    CMPSensorReading sensor_read;
    CMPBool has_reading;
    CMPImageDecodeRequest image_request;
    CMPImageData image_data;
    CMPAudioDecodeRequest audio_request;
    CMPAudioData audio_data;
    CMPNetworkRequest network_request;
    CMPNetworkResponse network_response;
    CMPFileInfo file_info;
    CMPBool exists;
    CMPColor color;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.enable_logging = CMP_FALSE;
    config.handle_capacity = 16;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));
    CMP_TEST_OK(cmp_null_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(cmp_null_backend_get_env(backend, &env));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_io(env.ctx, &io));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_sensors(env.ctx, &sensors));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_camera(env.ctx, &camera));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_image(env.ctx, &image));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_video(env.ctx, &video));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_audio(env.ctx, &audio));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_network(env.ctx, &network));
    CMP_TEST_NULL_FORCE_FAIL(env.vtable->get_tasks(env.ctx, &tasks));
    CMP_TEST_OK(env.vtable->get_io(env.ctx, &io));
    CMP_TEST_OK(env.vtable->get_sensors(env.ctx, &sensors));
    CMP_TEST_OK(env.vtable->get_camera(env.ctx, &camera));
    CMP_TEST_OK(env.vtable->get_image(env.ctx, &image));
    CMP_TEST_OK(env.vtable->get_video(env.ctx, &video));
    CMP_TEST_OK(env.vtable->get_audio(env.ctx, &audio));
    CMP_TEST_OK(env.vtable->get_network(env.ctx, &network));
    CMP_TEST_OK(env.vtable->get_tasks(env.ctx, &tasks));

    ws_config.utf8_app_name = "SweepApp";
    ws_config.utf8_app_id = "com.example.sweep";
    ws_config.reserved = 0;
    CMP_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));

    win_config.width = 120;
    win_config.height = 80;
    win_config.utf8_title = "SweepWin";
    win_config.flags = 0;

    CMP_TEST_NULL_FAIL_SWEEP(
        ws.vtable->create_window(ws.ctx, &win_config, &window),
        CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window)));

    CMP_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));
    CMP_TEST_NULL_FORCE_FAIL_AT(ws.vtable->show_window(ws.ctx, window), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(ws.vtable->show_window(ws.ctx, window), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(ws.vtable->hide_window(ws.ctx, window), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(ws.vtable->hide_window(ws.ctx, window), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_window_title(ws.ctx, window, "Title"), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_window_title(ws.ctx, window, "Title"), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_window_size(ws.ctx, window, 10, 10), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_window_size(ws.ctx, window, 10, 10), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->get_window_size(ws.ctx, window, &width, &height), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->get_window_size(ws.ctx, window, &width, &height), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_window_dpi_scale(ws.ctx, window, 1.0f), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_window_dpi_scale(ws.ctx, window, 1.0f), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->get_window_dpi_scale(ws.ctx, window, &scale), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->get_window_dpi_scale(ws.ctx, window, &scale), 2u);
    CMP_TEST_OK(ws.vtable->show_window(ws.ctx, window));
    CMP_TEST_OK(ws.vtable->hide_window(ws.ctx, window));
    CMP_TEST_OK(ws.vtable->set_window_title(ws.ctx, window, "Title"));
    CMP_TEST_OK(ws.vtable->set_window_size(ws.ctx, window, 10, 10));
    CMP_TEST_OK(ws.vtable->get_window_size(ws.ctx, window, &width, &height));
    CMP_TEST_OK(ws.vtable->set_window_dpi_scale(ws.ctx, window, 1.0f));
    CMP_TEST_OK(ws.vtable->get_window_dpi_scale(ws.ctx, window, &scale));
    CMP_TEST_NULL_FORCE_FAIL(ws.vtable->poll_event(ws.ctx, &event, &has_event));
    CMP_TEST_NULL_FORCE_FAIL(ws.vtable->get_time_ms(ws.ctx, &time_ms));

    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_clipboard_text(ws.ctx, "longtext"), 2u);
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "longtext"));
    CMP_TEST_NULL_FORCE_FAIL_AT(
        ws.vtable->set_clipboard_text(ws.ctx, "evenlongertext"), 2u);
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "evenlongertext"));
    CMP_TEST_NULL_FORCE_FAIL(
        ws.vtable->get_clipboard_text(ws.ctx, buffer, sizeof(buffer), &length));

    CMP_TEST_NULL_FAIL_SWEEP(
        gfx.vtable->create_texture(gfx.ctx, 4, 4, CMP_TEX_FORMAT_RGBA8, NULL, 0,
                                   &texture),
        CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture)));

    CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 4, 4, CMP_TEX_FORMAT_RGBA8,
                                           NULL, 0, &texture));
    rect.x = 0.0f;
    rect.y = 0.0f;
    rect.width = 1.0f;
    rect.height = 1.0f;
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, NULL, 0), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, NULL, 0), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, 1.0f), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, 1.0f), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.vtable->destroy_texture(gfx.ctx, texture),
                                1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.vtable->destroy_texture(gfx.ctx, texture),
                                2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.vtable->destroy_texture(gfx.ctx, texture),
                                3u);
    CMP_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));

    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.vtable->begin_frame(gfx.ctx, window, 10, 10, 1.0f), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.vtable->begin_frame(gfx.ctx, window, 10, 10, 1.0f), 2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.vtable->end_frame(gfx.ctx, window), 1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.vtable->end_frame(gfx.ctx, window), 2u);

    color.r = 0.0f;
    color.g = 0.0f;
    color.b = 0.0f;
    color.a = 1.0f;
    CMP_TEST_NULL_FORCE_FAIL(
        gfx.vtable->draw_rect(gfx.ctx, &rect, color, 0.0f));
    CMP_TEST_NULL_FORCE_FAIL(
        gfx.vtable->draw_line(gfx.ctx, 0.0f, 0.0f, 1.0f, 1.0f, color, 1.0f));
    CMP_TEST_NULL_FORCE_FAIL(gfx.vtable->push_clip(gfx.ctx, &rect));
    CMP_TEST_NULL_FORCE_FAIL(gfx.vtable->pop_clip(gfx.ctx));
    memset(&transform, 0, sizeof(transform));
    CMP_TEST_NULL_FORCE_FAIL(gfx.vtable->set_transform(gfx.ctx, &transform));

    memset(&path, 0, sizeof(path));
    CMP_TEST_OK(cmp_path_init(&path, NULL, 1));
    CMP_TEST_OK(cmp_path_move_to(&path, 0.0f, 0.0f));
    CMP_TEST_NULL_FORCE_FAIL(gfx.vtable->draw_path(gfx.ctx, &path, color));
    CMP_TEST_OK(cmp_path_shutdown(&path));

    CMP_TEST_NULL_FAIL_SWEEP(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400, CMP_FALSE,
                                     &font),
        CMP_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font)));

    CMP_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400,
                                             CMP_FALSE, &font));
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &text_width,
                                      &text_height, &baseline),
        1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &text_width,
                                      &text_height, &baseline),
        2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color),
        1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color),
        2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.text_vtable->destroy_font(gfx.ctx, font),
                                1u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.text_vtable->destroy_font(gfx.ctx, font),
                                2u);
    CMP_TEST_NULL_FORCE_FAIL_AT(gfx.text_vtable->destroy_font(gfx.ctx, font),
                                3u);
    CMP_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));

    CMP_TEST_NULL_FORCE_FAIL(
        io.vtable->read_file(io.ctx, "path", buffer, sizeof(buffer), &length));
    CMP_TEST_NULL_FORCE_FAIL(
        io.vtable->write_file(io.ctx, "path", buffer, 1, CMP_TRUE));
    CMP_TEST_NULL_FORCE_FAIL(io.vtable->file_exists(io.ctx, "path", &exists));
    CMP_TEST_NULL_FORCE_FAIL(io.vtable->delete_file(io.ctx, "path"));
    CMP_TEST_NULL_FORCE_FAIL(io.vtable->stat_file(io.ctx, "path", &file_info));

    CMP_TEST_NULL_FORCE_FAIL(
        sensors.vtable->is_available(sensors.ctx, 0u, &exists));
    CMP_TEST_NULL_FORCE_FAIL(sensors.vtable->start(sensors.ctx, 0u));
    CMP_TEST_NULL_FORCE_FAIL(sensors.vtable->stop(sensors.ctx, 0u));
    CMP_TEST_NULL_FORCE_FAIL(
        sensors.vtable->read(sensors.ctx, 0u, &sensor_read, &has_reading));

    CMP_TEST_NULL_FORCE_FAIL(camera.vtable->open(camera.ctx, 0u));
    CMP_TEST_NULL_FORCE_FAIL(camera.vtable->close(camera.ctx));
    CMP_TEST_NULL_FORCE_FAIL(camera.vtable->start(camera.ctx));
    CMP_TEST_NULL_FORCE_FAIL(camera.vtable->stop(camera.ctx));
    CMP_TEST_NULL_FORCE_FAIL(
        camera.vtable->read_frame(camera.ctx, &camera_frame, &has_frame));

    memset(&video_frame, 0, sizeof(video_frame));
    memset(&camera_frame, 0, sizeof(camera_frame));
    memset(&video_request, 0, sizeof(video_request));
    CMP_TEST_NULL_FORCE_FAIL(video.vtable->open(video.ctx, &video_request));
    CMP_TEST_NULL_FORCE_FAIL(video.vtable->close(video.ctx));
    CMP_TEST_NULL_FORCE_FAIL(
        video.vtable->read_frame(video.ctx, &video_frame, &has_frame));

    CMP_TEST_OK(cmp_get_default_allocator(&default_alloc));
    memset(&image_request, 0, sizeof(image_request));
    memset(&image_data, 0, sizeof(image_data));
    CMP_TEST_NULL_FORCE_FAIL(image.vtable->decode(image.ctx, &image_request,
                                                  &default_alloc, &image_data));
    CMP_TEST_NULL_FORCE_FAIL(
        image.vtable->free_image(image.ctx, &default_alloc, &image_data));

    memset(&audio_request, 0, sizeof(audio_request));
    memset(&audio_data, 0, sizeof(audio_data));
    CMP_TEST_NULL_FORCE_FAIL(audio.vtable->decode(audio.ctx, &audio_request,
                                                  &default_alloc, &audio_data));
    CMP_TEST_NULL_FORCE_FAIL(
        audio.vtable->free_audio(audio.ctx, &default_alloc, &audio_data));

    memset(&network_request, 0, sizeof(network_request));
    memset(&network_response, 0, sizeof(network_response));
    CMP_TEST_NULL_FORCE_FAIL(network.vtable->request(
        network.ctx, &network_request, &default_alloc, &network_response));
    CMP_TEST_NULL_FORCE_FAIL(network.vtable->free_response(
        network.ctx, &default_alloc, &network_response));

    CMP_TEST_NULL_FORCE_FAIL(tasks.vtable->thread_create(
        tasks.ctx, test_task_ok, NULL, &task_handle));
    CMP_TEST_NULL_FORCE_FAIL(tasks.vtable->thread_join(tasks.ctx, task_handle));
    CMP_TEST_NULL_FORCE_FAIL(
        tasks.vtable->mutex_create(tasks.ctx, &task_handle));
    CMP_TEST_NULL_FORCE_FAIL(
        tasks.vtable->mutex_destroy(tasks.ctx, task_handle));
    CMP_TEST_NULL_FORCE_FAIL(tasks.vtable->mutex_lock(tasks.ctx, task_handle));
    CMP_TEST_NULL_FORCE_FAIL(
        tasks.vtable->mutex_unlock(tasks.ctx, task_handle));
    CMP_TEST_NULL_FORCE_FAIL(tasks.vtable->sleep_ms(tasks.ctx, 1u));
    CMP_TEST_NULL_FORCE_FAIL(
        tasks.vtable->task_post(tasks.ctx, test_task_ok, NULL));
    CMP_TEST_NULL_FORCE_FAIL(
        tasks.vtable->task_post_delayed(tasks.ctx, test_task_ok, NULL, 1u));

    CMP_TEST_NULL_FORCE_FAIL_AT(ws.vtable->destroy_window(ws.ctx, window), 3u);
    CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));
    CMP_TEST_OK(ws.vtable->shutdown(ws.ctx));
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_BUSY);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPWS ws;
    CMPWSWindowConfig win_config;
    CMPHandle window;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.enable_logging = CMP_FALSE;
    config.handle_capacity = 4;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));

    win_config.width = 100;
    win_config.height = 100;
    win_config.utf8_title = "Temp";
    win_config.flags = 0;
    CMP_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_BUSY);
    CMP_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));
    CMP_TEST_OK(cmp_null_backend_destroy(backend));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPEnv env;
    CMPTasks tasks;
    TestTaskState task_state;

    memset(&task_state, 0, sizeof(task_state));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.enable_logging = CMP_FALSE;
    config.inline_tasks = CMP_FALSE;
    config.handle_capacity = 4;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_env(backend, &env));
    CMP_TEST_OK(env.vtable->get_tasks(env.ctx, &tasks));
    CMP_TEST_EXPECT(tasks.vtable->sleep_ms(tasks.ctx, 1u), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(
        tasks.vtable->task_post(tasks.ctx, test_task_ok, &task_state),
        CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, test_task_ok,
                                                    &task_state, 1u),
                    CMP_ERR_UNSUPPORTED);
    CMP_TEST_OK(cmp_null_backend_destroy(backend));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;
    CMPWS ws;
    CMPGfx gfx;
    CMPHandle window;
    CMPHandle texture;
    CMPHandle font;
    CMPWSWindowConfig win_config;

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));
    win_config.width = 10;
    win_config.height = 10;
    win_config.utf8_title = "FreeFail";
    win_config.flags = 0;
    CMP_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));
    CMP_TEST_EXPECT(ws.vtable->destroy_window(ws.ctx, window), CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_null_backend_destroy(backend));

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 1, 1, CMP_TEX_FORMAT_RGBA8,
                                           NULL, 0, &texture));
    CMP_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, texture),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_null_backend_destroy(backend));

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_gfx(backend, &gfx));
    CMP_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400,
                                             CMP_FALSE, &font));
    CMP_TEST_EXPECT(gfx.text_vtable->destroy_font(gfx.ctx, font),
                    CMP_ERR_UNKNOWN);
    CMP_TEST_OK(cmp_null_backend_destroy(backend));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_UNKNOWN);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPWS ws;
    CMPEnv env;
    CMPTasks tasks;
    TestTaskState task_state;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.enable_logging = CMP_FALSE;
    config.handle_capacity = 4;
    config.inline_tasks = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));
    CMP_TEST_OK(cmp_null_backend_get_env(backend, &env));
    CMP_TEST_OK(env.vtable->get_tasks(env.ctx, &tasks));
    CMP_TEST_OK(ws.vtable->pump_events(ws.ctx));
    task_state.calls = 0;
    task_state.last_value = 0;
    CMP_TEST_EXPECT(tasks.vtable->sleep_ms(tasks.ctx, 1), CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(
        tasks.vtable->task_post(tasks.ctx, test_task_ok, &task_state),
        CMP_ERR_UNSUPPORTED);
    CMP_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, test_task_ok,
                                                    &task_state, 1),
                    CMP_ERR_UNSUPPORTED);
    CMP_TEST_OK(cmp_null_backend_destroy(backend));
  }

  {
    CMPObjectHeader obj;
    cmp_u32 type_id;

    CMP_TEST_EXPECT(cmp_null_backend_test_set_initialized(NULL, CMP_TRUE),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_test_object_retain(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_test_object_release(NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_test_object_get_type_id(NULL, &type_id),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_EXPECT(cmp_null_backend_test_object_get_type_id(&obj, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    CMP_TEST_OK(cmp_object_header_init(&obj, 42u, 0u, &g_test_object_vtable));
    CMP_TEST_OK(cmp_null_backend_test_object_retain(&obj));
    CMP_TEST_OK(cmp_null_backend_test_object_get_type_id(&obj, &type_id));
    CMP_TEST_ASSERT(type_id == 42u);
    CMP_TEST_OK(cmp_null_backend_test_object_release(&obj));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    CMPEnv env;
    CMPCamera camera;
    CMPCameraConfig cam_config;

    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.enable_logging = CMP_FALSE;
    config.handle_capacity = 4;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_env(backend, &env));
    CMP_TEST_OK(env.vtable->get_camera(env.ctx, &camera));
    CMP_TEST_EXPECT(camera.vtable->open_with_config(camera.ctx, NULL),
                    CMP_ERR_INVALID_ARGUMENT);
    cam_config.camera_id = 1u;
    cam_config.facing = CMP_CAMERA_FACING_UNSPECIFIED;
    cam_config.width = 0u;
    cam_config.height = 0u;
    cam_config.format = CMP_CAMERA_FORMAT_ANY;
    CMP_TEST_EXPECT(camera.vtable->open_with_config(camera.ctx, &cam_config),
                    CMP_ERR_UNSUPPORTED);
    CMP_TEST_OK(cmp_null_backend_destroy(backend));
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;
    int rc;

    rc = cmp_log_shutdown();
    CMP_TEST_ASSERT(rc == CMP_OK || rc == CMP_ERR_STATE);

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_alloc_on = 2;
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_TRUE;
    backend = NULL;
    CMP_TEST_EXPECT(cmp_null_backend_create(&config, &backend),
                    CMP_ERR_OUT_OF_MEMORY);

    CMP_TEST_EXPECT(cmp_log_shutdown(), CMP_ERR_STATE);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;
    CMPWS ws;

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "clip"));
    alloc_state.fail_free_on = 2;
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_UNKNOWN);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;
    CMPWS ws;

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = CMP_FALSE;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    CMP_TEST_OK(cmp_null_backend_get_ws(backend, &ws));
    CMP_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "clip"));
    alloc_state.fail_free_on = 3;
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_UNKNOWN);
  }

  {
    CMPNullBackendConfig config;
    CMPNullBackend *backend;
    TestAllocator alloc_state;
    CMPAllocator alloc;

    CMP_TEST_OK(test_allocator_reset(&alloc_state));
    CMP_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    CMP_TEST_OK(cmp_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    CMP_TEST_OK(cmp_null_backend_create(&config, &backend));
    alloc_state.fail_free_ptr = backend;
    CMP_TEST_EXPECT(cmp_null_backend_destroy(backend), CMP_ERR_UNKNOWN);
  }

  return 0;
}
