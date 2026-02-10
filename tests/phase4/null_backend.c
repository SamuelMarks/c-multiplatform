#include "test_utils.h"

#include "m3/m3_backend_null.h"
#include "m3/m3_log.h"
#include "m3/m3_object.h"

#include <stdlib.h>
#include <string.h>

typedef struct TestAllocator {
  m3_usize alloc_calls;
  m3_usize realloc_calls;
  m3_usize free_calls;
  m3_usize fail_alloc_on;
  m3_usize fail_realloc_on;
  m3_usize fail_free_on;
  void *fail_free_ptr;
} TestAllocator;

static int test_allocator_reset(TestAllocator *alloc) {
  if (alloc == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  alloc->alloc_calls = 0;
  alloc->realloc_calls = 0;
  alloc->free_calls = 0;
  alloc->fail_alloc_on = 0;
  alloc->fail_realloc_on = 0;
  alloc->fail_free_on = 0;
  alloc->fail_free_ptr = NULL;
  return M3_OK;
}

static int test_alloc(void *ctx, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->alloc_calls += 1;
  if (alloc->fail_alloc_on != 0 && alloc->alloc_calls == alloc->fail_alloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = malloc(size);
  if (*out_ptr == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }
  return M3_OK;
}

static int test_realloc(void *ctx, void *ptr, m3_usize size, void **out_ptr) {
  TestAllocator *alloc;
  void *mem;

  if (ctx == NULL || out_ptr == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  if (size == 0) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->realloc_calls += 1;
  if (alloc->fail_realloc_on != 0 &&
      alloc->realloc_calls == alloc->fail_realloc_on) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  mem = realloc(ptr, size);
  if (mem == NULL) {
    return M3_ERR_OUT_OF_MEMORY;
  }

  *out_ptr = mem;
  return M3_OK;
}

static int test_free(void *ctx, void *ptr) {
  TestAllocator *alloc;

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  alloc = (TestAllocator *)ctx;
  alloc->free_calls += 1;
  if (alloc->fail_free_ptr != NULL && alloc->fail_free_ptr == ptr) {
    return M3_ERR_UNKNOWN;
  }
  if (alloc->fail_free_on != 0 && alloc->free_calls == alloc->fail_free_on) {
    return M3_ERR_UNKNOWN;
  }

  free(ptr);
  return M3_OK;
}

static int test_allocator_make(TestAllocator *state, M3Allocator *out_alloc) {
  if (state == NULL || out_alloc == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  out_alloc->ctx = state;
  out_alloc->alloc = test_alloc;
  out_alloc->realloc = test_realloc;
  out_alloc->free = test_free;
  return M3_OK;
}

typedef struct TestLogSink {
  int rc;
} TestLogSink;

static int test_log_sink(void *ctx, M3LogLevel level, const char *tag,
                         const char *message, m3_usize length) {
  TestLogSink *sink;

  M3_UNUSED(level);
  M3_UNUSED(tag);
  M3_UNUSED(message);
  M3_UNUSED(length);

  if (ctx == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
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
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestTaskState *)user;
  state->calls += 1;
  state->last_value = 1;
  return M3_OK;
}

static int test_task_fail(void *user) {
  TestTaskState *state;

  if (user == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }

  state = (TestTaskState *)user;
  state->calls += 1;
  state->last_value = -1;
  return M3_ERR_UNKNOWN;
}

static int test_obj_destroy(void *obj) {
  M3_UNUSED(obj);
  return M3_OK;
}

static int test_obj_retain(void *obj) {
  M3_UNUSED(obj);
  return M3_OK;
}

static int test_obj_release(void *obj) {
  M3_UNUSED(obj);
  return M3_OK;
}

static int test_obj_get_type_id(void *obj, m3_u32 *out_type_id) {
  if (obj == NULL || out_type_id == NULL) {
    return M3_ERR_INVALID_ARGUMENT;
  }
  *out_type_id = 42u;
  return M3_OK;
}

static const M3ObjectVTable g_test_object_vtable = {
    test_obj_retain, test_obj_release, test_obj_destroy, test_obj_get_type_id};

int main(void) {
  {
    int rc;

    rc = m3_log_shutdown();
    M3_TEST_ASSERT(rc == M3_OK || rc == M3_ERR_STATE);
  }

  {
    M3NullBackendConfig config;

    M3_TEST_EXPECT(m3_null_backend_config_init(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_null_backend_config_init(&config));
    M3_TEST_ASSERT(config.handle_capacity != 0);
    M3_TEST_ASSERT(config.clipboard_limit != 0);
    M3_TEST_ASSERT(config.enable_logging == M3_TRUE);
    M3_TEST_ASSERT(config.inline_tasks == M3_TRUE);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    M3Allocator bad_alloc;
    TestAllocator alloc_state;
    M3Allocator alloc;

    M3_TEST_OK(m3_null_backend_config_init(&config));
    backend = NULL;
    M3_TEST_EXPECT(m3_null_backend_create(&config, NULL),
                   M3_ERR_INVALID_ARGUMENT);

    config.handle_capacity = 0;
    M3_TEST_EXPECT(m3_null_backend_create(&config, &backend),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_ASSERT(backend == NULL);

    M3_TEST_OK(m3_null_backend_config_init(&config));
    memset(&bad_alloc, 0, sizeof(bad_alloc));
    config.allocator = &bad_alloc;
    M3_TEST_EXPECT(m3_null_backend_create(&config, &backend),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_TRUE));
    M3_TEST_EXPECT(m3_null_backend_create(NULL, &backend), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_core_test_set_default_allocator_fail(M3_FALSE));

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_alloc_on = 1;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_EXPECT(m3_null_backend_create(&config, &backend),
                   M3_ERR_OUT_OF_MEMORY);

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_alloc_on = 2;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_EXPECT(m3_null_backend_create(&config, &backend),
                   M3_ERR_OUT_OF_MEMORY);

    M3_TEST_OK(
        m3_log_test_set_mutex_failures(M3_TRUE, M3_FALSE, M3_FALSE, M3_FALSE));
    M3_TEST_EXPECT(m3_null_backend_create(NULL, &backend), M3_ERR_UNKNOWN);
    M3_TEST_OK(
        m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    int rc;

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.handle_capacity = 8;
    config.enable_logging = M3_TRUE;

    rc = m3_log_shutdown();
    M3_TEST_ASSERT(rc == M3_OK || rc == M3_ERR_STATE);

    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_destroy(backend));

    rc = m3_log_shutdown();
    M3_TEST_EXPECT(rc, M3_ERR_STATE);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    int rc;

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.handle_capacity = 4;
    config.enable_logging = M3_TRUE;

    rc = m3_log_shutdown();
    M3_TEST_ASSERT(rc == M3_OK || rc == M3_ERR_STATE);

    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(
        m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_TRUE));
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_UNKNOWN);
    M3_TEST_OK(
        m3_log_test_set_mutex_failures(M3_FALSE, M3_FALSE, M3_FALSE, M3_FALSE));
    M3_TEST_OK(m3_log_shutdown());
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    config.inline_tasks = M3_TRUE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_UNKNOWN);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestLogSink sink;
    M3LogSink log_sink;
    M3WS ws;
    M3Gfx gfx;
    M3Env env;
    M3IO io;
    M3Sensors sensors;
    M3Camera camera;
    M3Network network;
    M3Tasks tasks;
    M3Allocator default_alloc;
    M3WSConfig ws_config;
    M3WSWindowConfig win_config;
    M3Handle window;
    M3Handle window2;
    M3Handle texture;
    M3Handle font;
    M3Handle invalid_handle;
    M3Rect rect;
    M3Rect rect_small;
    M3Mat3 transform;
    M3Color color;
    char clip_buffer[16];
    m3_usize clip_len;
    m3_i32 width;
    m3_i32 height;
    M3Scalar scale;
    M3Bool has_event;
    M3InputEvent event;
    m3_u32 time_ms;
    M3FileInfo file_info;
    M3SensorReading reading;
    M3CameraFrame frame;
    M3NetworkRequest net_req;
    M3NetworkResponse net_resp;
    TestTaskState task_state;
    void *data;
    int rc;

    M3_TEST_OK(m3_log_init(NULL));
    sink.rc = M3_OK;
    log_sink.ctx = &sink;
    log_sink.write = test_log_sink;
    M3_TEST_OK(m3_log_set_sink(&log_sink));

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.handle_capacity = 16;
    config.clipboard_limit = 8;
    config.enable_logging = M3_TRUE;
    config.inline_tasks = M3_TRUE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));

    M3_TEST_EXPECT(m3_null_backend_destroy(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_get_ws(NULL, &ws), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_get_ws(backend, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_null_backend_test_set_initialized(backend, M3_FALSE));
    M3_TEST_EXPECT(m3_null_backend_get_ws(backend, &ws), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_null_backend_get_gfx(backend, &gfx), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_null_backend_get_env(backend, &env), M3_ERR_STATE);
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_STATE);
    M3_TEST_OK(m3_null_backend_test_set_initialized(backend, M3_TRUE));
    M3_TEST_OK(m3_null_backend_get_ws(backend, &ws));

    M3_TEST_EXPECT(m3_null_backend_get_gfx(NULL, &gfx),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_get_gfx(backend, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_null_backend_get_gfx(backend, &gfx));

    M3_TEST_EXPECT(m3_null_backend_get_env(NULL, &env),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_get_env(backend, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_null_backend_get_env(backend, &env));

    sink.rc = M3_ERR_IO;
    M3_TEST_EXPECT(ws.vtable->pump_events(ws.ctx), M3_ERR_IO);
    sink.rc = M3_OK;

    ws_config.utf8_app_name = "TestApp";
    ws_config.utf8_app_id = "com.example.test";
    ws_config.reserved = 0;
    M3_TEST_EXPECT(ws.vtable->init(NULL, &ws_config), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->init(ws.ctx, NULL), M3_ERR_INVALID_ARGUMENT);
    ws_config.utf8_app_name = NULL;
    M3_TEST_EXPECT(ws.vtable->init(ws.ctx, &ws_config),
                   M3_ERR_INVALID_ARGUMENT);
    ws_config.utf8_app_name = "TestApp";
    ws_config.utf8_app_id = NULL;
    M3_TEST_EXPECT(ws.vtable->init(ws.ctx, &ws_config),
                   M3_ERR_INVALID_ARGUMENT);
    ws_config.utf8_app_id = "com.example.test";
    M3_TEST_OK(ws.vtable->init(ws.ctx, &ws_config));

    win_config.width = 640;
    win_config.height = 480;
    win_config.utf8_title = "Main";
    win_config.flags = M3_WS_WINDOW_RESIZABLE;
    M3_TEST_EXPECT(ws.vtable->create_window(NULL, &win_config, &window),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->create_window(ws.ctx, NULL, &window),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    win_config.utf8_title = NULL;
    M3_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, &window),
                   M3_ERR_INVALID_ARGUMENT);
    win_config.utf8_title = "Main";
    win_config.width = 0;
    M3_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, &window),
                   M3_ERR_RANGE);
    win_config.width = 640;
    win_config.height = 0;
    M3_TEST_EXPECT(ws.vtable->create_window(ws.ctx, &win_config, &window),
                   M3_ERR_RANGE);
    win_config.height = 480;
    M3_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));

    invalid_handle.id = 0;
    invalid_handle.generation = 0;
    M3_TEST_EXPECT(ws.vtable->shutdown(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->destroy_window(NULL, window),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->show_window(NULL, window),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->hide_window(NULL, window),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->set_window_title(NULL, window, "Title"),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->set_window_size(NULL, window, 1, 1),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->get_window_size(NULL, window, &width, &height),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->set_window_dpi_scale(NULL, window, 1.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->get_window_dpi_scale(NULL, window, &scale),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->set_clipboard_text(NULL, "abc"),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->get_clipboard_text(
                       NULL, clip_buffer, sizeof(clip_buffer), &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->poll_event(NULL, &event, &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->get_time_ms(NULL, &time_ms),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->show_window(ws.ctx, invalid_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->show_window(ws.ctx, window));
    M3_TEST_OK(ws.vtable->hide_window(ws.ctx, window));

    M3_TEST_EXPECT(ws.vtable->set_window_title(ws.ctx, window, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->set_window_title(ws.ctx, window, "Title"));

    M3_TEST_EXPECT(ws.vtable->set_window_size(ws.ctx, window, 0, 1),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(ws.vtable->set_window_size(ws.ctx, window, 1, 0),
                   M3_ERR_RANGE);
    M3_TEST_OK(ws.vtable->set_window_size(ws.ctx, window, 800, 600));
    M3_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window, NULL, &height),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window, &width, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->get_window_size(ws.ctx, window, &width, &height));
    M3_TEST_ASSERT(width == 800);
    M3_TEST_ASSERT(height == 600);

    M3_TEST_EXPECT(ws.vtable->set_window_dpi_scale(ws.ctx, window, 0.0f),
                   M3_ERR_RANGE);
    M3_TEST_OK(ws.vtable->set_window_dpi_scale(ws.ctx, window, 2.0f));
    M3_TEST_EXPECT(ws.vtable->get_window_dpi_scale(ws.ctx, window, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->get_window_dpi_scale(ws.ctx, window, &scale));
    M3_TEST_ASSERT(scale == 2.0f);

    clip_buffer[0] = 'x';
    M3_TEST_EXPECT(ws.vtable->get_clipboard_text(ws.ctx, NULL, 0, &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->get_clipboard_text(ws.ctx, clip_buffer, 0, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->set_clipboard_text(ws.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->get_clipboard_text(ws.ctx, clip_buffer,
                                             sizeof(clip_buffer), &clip_len));
    M3_TEST_ASSERT(clip_len == 0);

    M3_TEST_EXPECT(ws.vtable->set_clipboard_text(ws.ctx, "123456789"),
                   M3_ERR_RANGE);
    M3_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "abc"));
    M3_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "abcdef"));
    M3_TEST_EXPECT(
        ws.vtable->get_clipboard_text(ws.ctx, clip_buffer, 4, &clip_len),
        M3_ERR_RANGE);
    M3_TEST_OK(ws.vtable->get_clipboard_text(ws.ctx, clip_buffer,
                                             sizeof(clip_buffer), &clip_len));
    M3_TEST_ASSERT(clip_len == 6);
    M3_TEST_ASSERT(strcmp(clip_buffer, "abcdef") == 0);

    M3_TEST_EXPECT(ws.vtable->poll_event(ws.ctx, NULL, &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(ws.vtable->poll_event(ws.ctx, &event, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->poll_event(ws.ctx, &event, &has_event));
    M3_TEST_ASSERT(has_event == M3_FALSE);

    M3_TEST_EXPECT(ws.vtable->pump_events(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->pump_events(ws.ctx));
    M3_TEST_EXPECT(ws.vtable->get_time_ms(ws.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));
    M3_TEST_OK(ws.vtable->get_time_ms(ws.ctx, &time_ms));

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

    M3_TEST_EXPECT(gfx.vtable->begin_frame(NULL, window, 1, 1, 1.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, window, 0, 1, 1.0f),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, window, 1, 0, 1.0f),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, window, 1, 1, 0.0f),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, invalid_handle, 1, 1, 1.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.vtable->begin_frame(gfx.ctx, window, 1, 1, 1.0f));

    M3_TEST_EXPECT(gfx.vtable->end_frame(NULL, window),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->end_frame(gfx.ctx, invalid_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.vtable->end_frame(gfx.ctx, window));

    M3_TEST_EXPECT(gfx.vtable->clear(NULL, color), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.vtable->clear(gfx.ctx, color));

    M3_TEST_EXPECT(gfx.vtable->draw_rect(NULL, &rect, color, 0.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, NULL, color, 0.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, &rect_small, color, 0.0f),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(gfx.vtable->draw_rect(gfx.ctx, &rect, color, -1.0f),
                   M3_ERR_RANGE);
    M3_TEST_OK(gfx.vtable->draw_rect(gfx.ctx, &rect, color, 0.0f));

    M3_TEST_EXPECT(
        gfx.vtable->draw_line(NULL, 0.0f, 0.0f, 1.0f, 1.0f, color, 1.0f),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->draw_line(gfx.ctx, 0.0f, 0.0f, 1.0f, 1.0f, color, -1.0f),
        M3_ERR_RANGE);
    M3_TEST_OK(
        gfx.vtable->draw_line(gfx.ctx, 0.0f, 0.0f, 1.0f, 1.0f, color, 1.0f));

    M3_TEST_EXPECT(gfx.vtable->push_clip(NULL, &rect), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->push_clip(gfx.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->push_clip(gfx.ctx, &rect_small), M3_ERR_RANGE);
    M3_TEST_OK(gfx.vtable->push_clip(gfx.ctx, &rect));

    M3_TEST_EXPECT(gfx.vtable->pop_clip(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.vtable->pop_clip(gfx.ctx));

    memset(&transform, 0, sizeof(transform));
    M3_TEST_EXPECT(gfx.vtable->set_transform(NULL, &transform),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->set_transform(gfx.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.vtable->set_transform(gfx.ctx, &transform));

    M3_TEST_EXPECT(gfx.vtable->create_texture(NULL, 1, 1, M3_TEX_FORMAT_RGBA8,
                                              NULL, 0, &texture),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->create_texture(
                       gfx.ctx, 0, 1, M3_TEX_FORMAT_RGBA8, NULL, 0, &texture),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(gfx.vtable->create_texture(
                       gfx.ctx, 1, 0, M3_TEX_FORMAT_RGBA8, NULL, 0, &texture),
                   M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.vtable->create_texture(gfx.ctx, 1, 1, 99, NULL, 0, &texture),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->create_texture(
                       gfx.ctx, 1, 1, M3_TEX_FORMAT_RGBA8, NULL, 4, &texture),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->create_texture(
                       gfx.ctx, 1, 1, M3_TEX_FORMAT_RGBA8, NULL, 0, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 1, 1, M3_TEX_FORMAT_RGBA8,
                                          NULL, 0, &texture));

    M3_TEST_EXPECT(gfx.vtable->begin_frame(gfx.ctx, texture, 1, 1, 1.0f),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(
        gfx.vtable->update_texture(NULL, texture, 0, 0, 1, 1, NULL, 0),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->update_texture(gfx.ctx, invalid_handle, 0, 0, 1,
                                              1, NULL, 0),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, window, 0, 0, 1, 1, NULL, 0),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, -1, 0, 1, 1, NULL, 0),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 0, 1, NULL, 0),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 0, NULL, 0),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, NULL, 4),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(
        gfx.vtable->update_texture(gfx.ctx, texture, 0, 0, 1, 1, NULL, 0));

    M3_TEST_EXPECT(gfx.vtable->destroy_texture(NULL, texture),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, invalid_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, window),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_EXPECT(gfx.vtable->draw_texture(NULL, texture, &rect, &rect, 1.0f),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, invalid_handle, &rect, &rect, 1.0f),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, window, &rect, &rect, 1.0f),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, NULL, &rect, 1.0f),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, NULL, 1.0f),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, -1.0f),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, 2.0f),
        M3_ERR_RANGE);
    M3_TEST_OK(gfx.vtable->draw_texture(gfx.ctx, texture, &rect, &rect, 1.0f));

    M3_TEST_EXPECT(
        gfx.text_vtable->create_font(NULL, "Sans", 12, 400, M3_FALSE, &font),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, NULL, 12, 400, M3_FALSE, &font),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 0, 400, M3_FALSE, &font),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 50, M3_FALSE, &font),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 950, M3_FALSE, &font),
        M3_ERR_RANGE);
    M3_TEST_EXPECT(
        gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400, M3_FALSE, NULL),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400, M3_FALSE,
                                            &font));

    M3_TEST_EXPECT(gfx.text_vtable->measure_text(NULL, font, "hi", 2, &scale,
                                                 &scale, &scale),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, invalid_handle, "hi",
                                                 2, &scale, &scale, &scale),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, font, NULL, 2, &scale,
                                                 &scale, &scale),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, NULL,
                                                 &scale, &scale),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.text_vtable->measure_text(gfx.ctx, font, "hi", 2, &scale,
                                             &scale, &scale));

    M3_TEST_EXPECT(
        gfx.text_vtable->draw_text(NULL, font, "hi", 2, 0.0f, 0.0f, color),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.text_vtable->draw_text(gfx.ctx, invalid_handle, "hi", 2,
                                              0.0f, 0.0f, color),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        gfx.text_vtable->draw_text(gfx.ctx, font, NULL, 2, 0.0f, 0.0f, color),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(
        gfx.text_vtable->draw_text(gfx.ctx, font, "hi", 2, 0.0f, 0.0f, color));

    M3_TEST_EXPECT(gfx.text_vtable->destroy_font(NULL, font),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.text_vtable->destroy_font(gfx.ctx, invalid_handle),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.text_vtable->destroy_font(gfx.ctx, texture),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(gfx.text_vtable->destroy_font(gfx.ctx, font));

    M3_TEST_EXPECT(env.vtable->get_io(NULL, &io), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(env.vtable->get_io(env.ctx, NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(env.vtable->get_io(env.ctx, &io));

    M3_TEST_EXPECT(env.vtable->get_sensors(NULL, &sensors),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(env.vtable->get_sensors(env.ctx, &sensors));

    M3_TEST_EXPECT(env.vtable->get_camera(NULL, &camera),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(env.vtable->get_camera(env.ctx, &camera));

    M3_TEST_EXPECT(env.vtable->get_network(NULL, &network),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(env.vtable->get_network(env.ctx, &network));

    M3_TEST_EXPECT(env.vtable->get_tasks(NULL, &tasks),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(env.vtable->get_tasks(env.ctx, &tasks));

    M3_TEST_EXPECT(env.vtable->get_time_ms(NULL, &time_ms),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(env.vtable->get_time_ms(env.ctx, &time_ms));

    M3_TEST_EXPECT(io.vtable->read_file(NULL, "path", clip_buffer,
                                        sizeof(clip_buffer), &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->read_file(io.ctx, NULL, clip_buffer,
                                        sizeof(clip_buffer), &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->read_file(io.ctx, "path", NULL, 1, &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->read_file(io.ctx, "path", clip_buffer,
                                        sizeof(clip_buffer), &clip_len),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(io.vtable->read_file_alloc(NULL, "path", &default_alloc,
                                              &data, &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, NULL, &default_alloc,
                                              &data, &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    M3_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &default_alloc,
                                              &data, &clip_len),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_get_default_allocator(&default_alloc));
    M3_TEST_EXPECT(io.vtable->read_file_alloc(io.ctx, "path", &default_alloc,
                                              &data, &clip_len),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(io.vtable->write_file(NULL, "path", clip_buffer, 1, M3_TRUE),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->write_file(io.ctx, NULL, clip_buffer, 1, M3_TRUE),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->write_file(io.ctx, "path", NULL, 1, M3_TRUE),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        io.vtable->write_file(io.ctx, "path", clip_buffer, 1, M3_TRUE),
        M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(io.vtable->file_exists(NULL, "path", &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->file_exists(io.ctx, NULL, &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->file_exists(io.ctx, "path", &has_event),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(io.vtable->delete_file(NULL, "path"),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->delete_file(io.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->delete_file(io.ctx, "path"), M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(io.vtable->stat_file(NULL, "path", &file_info),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->stat_file(io.ctx, NULL, &file_info),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(io.vtable->stat_file(io.ctx, "path", &file_info),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(sensors.vtable->is_available(NULL, 0, &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(sensors.vtable->is_available(sensors.ctx, 0, &has_event));
    M3_TEST_EXPECT(sensors.vtable->start(NULL, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sensors.vtable->start(sensors.ctx, 0), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(sensors.vtable->stop(NULL, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(sensors.vtable->stop(sensors.ctx, 0), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(sensors.vtable->read(NULL, 0, &reading, &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(sensors.vtable->read(sensors.ctx, 0, &reading, &has_event));

    M3_TEST_EXPECT(camera.vtable->open(NULL, 0), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(camera.vtable->open(camera.ctx, 0), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(camera.vtable->close(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(camera.vtable->close(camera.ctx), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(camera.vtable->start(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(camera.vtable->start(camera.ctx), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(camera.vtable->stop(NULL), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(camera.vtable->stop(camera.ctx), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(camera.vtable->read_frame(NULL, &frame, &has_event),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(camera.vtable->read_frame(camera.ctx, &frame, &has_event));

    memset(&net_req, 0, sizeof(net_req));
    M3_TEST_EXPECT(
        network.vtable->request(NULL, &net_req, &default_alloc, &net_resp),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        network.vtable->request(network.ctx, NULL, &default_alloc, &net_resp),
        M3_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    M3_TEST_EXPECT(network.vtable->request(network.ctx, &net_req,
                                           &default_alloc, &net_resp),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_get_default_allocator(&default_alloc));
    M3_TEST_EXPECT(network.vtable->request(network.ctx, &net_req,
                                           &default_alloc, &net_resp),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(
        network.vtable->free_response(NULL, &default_alloc, &net_resp),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(network.vtable->free_response(network.ctx, NULL, &net_resp),
                   M3_ERR_INVALID_ARGUMENT);
    memset(&default_alloc, 0, sizeof(default_alloc));
    M3_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &default_alloc, &net_resp),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_get_default_allocator(&default_alloc));
    M3_TEST_EXPECT(
        network.vtable->free_response(network.ctx, &default_alloc, &net_resp),
        M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(
        tasks.vtable->thread_create(NULL, test_task_ok, &task_state, &window2),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        tasks.vtable->thread_create(tasks.ctx, test_task_ok, &task_state, NULL),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->thread_create(tasks.ctx, test_task_ok,
                                               &task_state, &window2),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(tasks.vtable->thread_join(NULL, window2),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->thread_join(tasks.ctx, window2),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(tasks.vtable->mutex_create(NULL, &window2),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_create(tasks.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_create(tasks.ctx, &window2),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(tasks.vtable->mutex_destroy(NULL, window2),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_destroy(tasks.ctx, window2),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(tasks.vtable->mutex_lock(NULL, window2),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_lock(tasks.ctx, window2),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(tasks.vtable->mutex_unlock(NULL, window2),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->mutex_unlock(tasks.ctx, window2),
                   M3_ERR_UNSUPPORTED);

    M3_TEST_EXPECT(tasks.vtable->sleep_ms(NULL, 1), M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(tasks.vtable->sleep_ms(tasks.ctx, 1));

    task_state.calls = 0;
    task_state.last_value = 0;
    M3_TEST_EXPECT(tasks.vtable->task_post(NULL, test_task_ok, &task_state),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(tasks.vtable->task_post(tasks.ctx, NULL, &task_state),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(tasks.vtable->task_post(tasks.ctx, test_task_ok, &task_state));
    M3_TEST_EXPECT(
        tasks.vtable->task_post(tasks.ctx, test_task_fail, &task_state),
        M3_ERR_UNKNOWN);

    M3_TEST_EXPECT(
        tasks.vtable->task_post_delayed(NULL, test_task_ok, &task_state, 1),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(
        tasks.vtable->task_post_delayed(tasks.ctx, NULL, &task_state, 1),
        M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(tasks.vtable->task_post_delayed(tasks.ctx, test_task_ok,
                                               &task_state, 1));
    M3_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, test_task_fail,
                                                   &task_state, 1),
                   M3_ERR_UNKNOWN);

    M3_TEST_OK(m3_object_test_set_handle_is_valid_fail(M3_TRUE));
    M3_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window, &width, &height),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_object_test_set_handle_is_valid_fail(M3_FALSE));

    win_config.width = 320;
    win_config.height = 240;
    win_config.utf8_title = "Second";
    win_config.flags = 0;
    M3_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window2));
    M3_TEST_OK(ws.vtable->destroy_window(ws.ctx, window2));
    M3_TEST_EXPECT(ws.vtable->get_window_size(ws.ctx, window2, &width, &height),
                   M3_ERR_NOT_FOUND);

    M3_TEST_EXPECT(ws.vtable->show_window(ws.ctx, texture),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, window),
                   M3_ERR_INVALID_ARGUMENT);

    M3_TEST_OK(gfx.vtable->destroy_texture(gfx.ctx, texture));
    M3_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));

    M3_TEST_OK(ws.vtable->shutdown(ws.ctx));
    M3_TEST_OK(m3_null_backend_destroy(backend));

    rc = m3_log_shutdown();
    M3_TEST_EXPECT(rc, M3_OK);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    M3WS ws;
    M3WSWindowConfig win_config;
    M3Handle window;

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.enable_logging = M3_FALSE;
    config.handle_capacity = 4;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_ws(backend, &ws));

    win_config.width = 100;
    win_config.height = 100;
    win_config.utf8_title = "Temp";
    win_config.flags = 0;
    M3_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_BUSY);
    M3_TEST_OK(ws.vtable->destroy_window(ws.ctx, window));
    M3_TEST_OK(m3_null_backend_destroy(backend));
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;
    M3WS ws;
    M3Gfx gfx;
    M3Handle window;
    M3Handle texture;
    M3Handle font;
    M3WSWindowConfig win_config;

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_ws(backend, &ws));
    win_config.width = 10;
    win_config.height = 10;
    win_config.utf8_title = "FreeFail";
    win_config.flags = 0;
    M3_TEST_OK(ws.vtable->create_window(ws.ctx, &win_config, &window));
    M3_TEST_EXPECT(ws.vtable->destroy_window(ws.ctx, window), M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_null_backend_destroy(backend));

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_gfx(backend, &gfx));
    M3_TEST_OK(gfx.vtable->create_texture(gfx.ctx, 1, 1, M3_TEX_FORMAT_RGBA8,
                                          NULL, 0, &texture));
    M3_TEST_EXPECT(gfx.vtable->destroy_texture(gfx.ctx, texture),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_null_backend_destroy(backend));

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_gfx(backend, &gfx));
    M3_TEST_OK(gfx.text_vtable->create_font(gfx.ctx, "Sans", 12, 400, M3_FALSE,
                                            &font));
    M3_TEST_EXPECT(gfx.text_vtable->destroy_font(gfx.ctx, font),
                   M3_ERR_UNKNOWN);
    M3_TEST_OK(m3_null_backend_destroy(backend));
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_free_on = 1;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_UNKNOWN);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    M3WS ws;
    M3Env env;
    M3Tasks tasks;
    TestTaskState task_state;

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.enable_logging = M3_FALSE;
    config.handle_capacity = 4;
    config.inline_tasks = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_ws(backend, &ws));
    M3_TEST_OK(m3_null_backend_get_env(backend, &env));
    M3_TEST_OK(env.vtable->get_tasks(env.ctx, &tasks));
    M3_TEST_OK(ws.vtable->pump_events(ws.ctx));
    task_state.calls = 0;
    task_state.last_value = 0;
    M3_TEST_EXPECT(tasks.vtable->sleep_ms(tasks.ctx, 1), M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(
        tasks.vtable->task_post(tasks.ctx, test_task_ok, &task_state),
        M3_ERR_UNSUPPORTED);
    M3_TEST_EXPECT(tasks.vtable->task_post_delayed(tasks.ctx, test_task_ok,
                                                   &task_state, 1),
                   M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_null_backend_destroy(backend));
  }

  {
    M3ObjectHeader obj;
    m3_u32 type_id;

    M3_TEST_EXPECT(m3_null_backend_test_set_initialized(NULL, M3_TRUE),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_test_object_retain(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_test_object_release(NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_test_object_get_type_id(NULL, &type_id),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_EXPECT(m3_null_backend_test_object_get_type_id(&obj, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    M3_TEST_OK(m3_object_header_init(&obj, 42u, 0u, &g_test_object_vtable));
    M3_TEST_OK(m3_null_backend_test_object_retain(&obj));
    M3_TEST_OK(m3_null_backend_test_object_get_type_id(&obj, &type_id));
    M3_TEST_ASSERT(type_id == 42u);
    M3_TEST_OK(m3_null_backend_test_object_release(&obj));
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    M3Env env;
    M3Camera camera;
    M3CameraConfig cam_config;

    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.enable_logging = M3_FALSE;
    config.handle_capacity = 4;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_env(backend, &env));
    M3_TEST_OK(env.vtable->get_camera(env.ctx, &camera));
    M3_TEST_EXPECT(camera.vtable->open_with_config(camera.ctx, NULL),
                   M3_ERR_INVALID_ARGUMENT);
    cam_config.camera_id = 1u;
    cam_config.facing = M3_CAMERA_FACING_UNSPECIFIED;
    cam_config.width = 0u;
    cam_config.height = 0u;
    cam_config.format = M3_CAMERA_FORMAT_ANY;
    M3_TEST_EXPECT(camera.vtable->open_with_config(camera.ctx, &cam_config),
                   M3_ERR_UNSUPPORTED);
    M3_TEST_OK(m3_null_backend_destroy(backend));
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;
    int rc;

    rc = m3_log_shutdown();
    M3_TEST_ASSERT(rc == M3_OK || rc == M3_ERR_STATE);

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    alloc_state.fail_alloc_on = 2;
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_TRUE;
    backend = NULL;
    M3_TEST_EXPECT(m3_null_backend_create(&config, &backend),
                   M3_ERR_OUT_OF_MEMORY);

    M3_TEST_EXPECT(m3_log_shutdown(), M3_ERR_STATE);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;
    M3WS ws;

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_ws(backend, &ws));
    M3_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "clip"));
    alloc_state.fail_free_on = 2;
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_UNKNOWN);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;
    M3WS ws;

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    config.enable_logging = M3_FALSE;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    M3_TEST_OK(m3_null_backend_get_ws(backend, &ws));
    M3_TEST_OK(ws.vtable->set_clipboard_text(ws.ctx, "clip"));
    alloc_state.fail_free_on = 3;
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_UNKNOWN);
  }

  {
    M3NullBackendConfig config;
    M3NullBackend *backend;
    TestAllocator alloc_state;
    M3Allocator alloc;

    M3_TEST_OK(test_allocator_reset(&alloc_state));
    M3_TEST_OK(test_allocator_make(&alloc_state, &alloc));
    M3_TEST_OK(m3_null_backend_config_init(&config));
    config.allocator = &alloc;
    config.handle_capacity = 4;
    M3_TEST_OK(m3_null_backend_create(&config, &backend));
    alloc_state.fail_free_ptr = backend;
    M3_TEST_EXPECT(m3_null_backend_destroy(backend), M3_ERR_UNKNOWN);
  }

  return 0;
}
