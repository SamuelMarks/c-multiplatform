/* clang-format off */
#include "cmpc/cmp_fs.h"
#include "cmpc/cmp_core.h"
#include "cmpc/cmp_tasks.h"
#include "test_utils.h"

#include <stdlib.h>
/* clang-format on */

typedef struct FSTestState {
  int called;
  int status;
  cmp_usize size;
} FSTestState;

static void CMP_CALL fs_callback(void *task_ctx, const char *path, void *data,
                                 cmp_usize size, int status, void *user) {
  FSTestState *state = (FSTestState *)user;
  CMP_UNUSED(task_ctx);
  CMP_UNUSED(path);
  CMP_UNUSED(data);
  state->called = 1;
  state->status = status;
  state->size = size;
  if (data != NULL) {
    CMPAllocator alloc;
    cmp_get_default_allocator(&alloc);
    alloc.free(alloc.ctx, data);
  }
}

static int test_fs_poll_ui(void *ctx, CMPBool *out_continue) {
  FSTestState *state = (FSTestState *)ctx;
  if (state->called) {
    *out_continue = 0;
  }
  return CMP_OK;
}

static int CMP_CALL mock_read_file_alloc(void *io, const char *utf8_path,
                                         const CMPAllocator *allocator,
                                         void **out_data, cmp_usize *out_size) {
  FILE *f;
  cmp_usize len;
  void *data;

  CMP_UNUSED(io);

  f = fopen(utf8_path, "rb");
  if (f == NULL) {
    return CMP_ERR_NOT_FOUND;
  }

  fseek(f, 0, SEEK_END);
  len = (cmp_usize)ftell(f);
  fseek(f, 0, SEEK_SET);

  if (allocator->alloc(allocator->ctx, len, &data) != CMP_OK) {
    fclose(f);
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (len > 0) {
    fread(data, 1, len, f);
  }
  fclose(f);

  *out_data = data;
  *out_size = len;
  return CMP_OK;
}

static CMPIOVTable g_mock_io_vtable = {
    NULL, mock_read_file_alloc, NULL, NULL, NULL, NULL};

static CMPIO g_mock_io = {NULL, &g_mock_io_vtable};

static int CMP_CALL mock_get_io(void *env, CMPIO *out_io) {
  CMP_UNUSED(env);
  *out_io = g_mock_io;
  return CMP_OK;
}

static CMPEnvVTable g_mock_env_vtable = {mock_get_io, NULL, NULL, NULL,
                                         NULL,        NULL, NULL, NULL};

int test_fs(void) {
  CMPAllocator alloc;
  CMPEventLoop loop;
  CMPAsyncEventLoopConfig loop_config;
  CMPTasks tasks;
  CMPTasksDefaultConfig tasks_config;
  FSTestState state;
  CMPEnv env;

  env.ctx = NULL;
  env.vtable = &g_mock_env_vtable;

  CMP_TEST_OK(cmp_get_default_allocator(&alloc));
  CMP_TEST_OK(cmp_tasks_default_config_init(&tasks_config));
  tasks_config.allocator = &alloc;
  tasks_config.worker_count = 2;
  CMP_TEST_OK(cmp_tasks_default_create(&tasks_config, &tasks));

  state.called = 0;
  state.status = -1;
  state.size = 0;

  loop_config.ctx = &state;
  loop_config.on_poll_ui = test_fs_poll_ui;
  CMP_TEST_OK(cmp_event_loop_async_init(&alloc, &loop_config, &loop));

  /* Write a dummy file to read */
  {
    FILE *f = fopen("test_fs_dummy.txt", "wb");
    if (f != NULL) {
      fwrite("hello", 1, 5, f);
      fclose(f);
    }
  }

  /* Just a dummy env, cmp_fs_read_file_async doesn't strictly use env
   * internally */
  /* If it did, we would init it. */

  CMP_TEST_OK(cmp_fs_read_async(&env, &tasks, &loop, &alloc,
                                "test_fs_dummy.txt", fs_callback, &state));

  CMP_TEST_OK(cmp_event_loop_run(&loop));

  CMP_TEST_ASSERT(state.called == 1);
  CMP_TEST_ASSERT(state.status == CMP_OK);
  CMP_TEST_ASSERT(state.size == 5);

  /* Test non-existent file */
  state.called = 0;
  CMP_TEST_OK(cmp_fs_read_async(&env, &tasks, &loop, &alloc,
                                "test_fs_does_not_exist.txt", fs_callback,
                                &state));

  CMP_TEST_OK(cmp_event_loop_run(&loop));
  CMP_TEST_ASSERT(state.called == 1);
  CMP_TEST_ASSERT(state.status != CMP_OK);

  CMP_TEST_OK(cmp_event_loop_async_destroy(&alloc, &loop));
  CMP_TEST_OK(cmp_tasks_default_destroy(&tasks));

  remove("test_fs_dummy.txt");

  return 0;
}

int main(void) {
  if (test_fs() != 0)
    return 1;
  return 0;
}