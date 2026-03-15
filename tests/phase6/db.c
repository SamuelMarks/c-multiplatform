/* clang-format off */
#include "cmpc/cmp_db.h"
#include "cmpc/cmp_event.h"
#include "cmpc/cmp_tasks.h"
#include "cmpc/cmp_core.h"

#include <stdio.h>
#include <string.h>

#include "test_utils.h"
/* clang-format on */

static int test_cmp_db_init(void) {
  CMPDb db;
  CMPAllocator allocator;
  cmp_get_default_allocator(&allocator);

  CMP_TEST_EXPECT(cmp_db_init(NULL, &allocator, ":memory:"),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_db_init(&db, NULL, ":memory:"), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_db_init(&db, &allocator, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_db_init(&db, &allocator, ":memory:"));
  CMP_TEST_EXPECT(db.handle != NULL, 1);

  CMP_TEST_EXPECT(cmp_db_shutdown(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_OK(cmp_db_shutdown(&db));

  return 0;
}

static int test_cmp_db_execute(void) {
  CMPDb db;
  CMPAllocator allocator;
  cmp_get_default_allocator(&allocator);

  CMP_TEST_OK(cmp_db_init(&db, &allocator, ":memory:"));

  CMP_TEST_EXPECT(cmp_db_execute(NULL, "SELECT 1"), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_db_execute(&db, NULL), CMP_ERR_INVALID_ARGUMENT);

  CMP_TEST_OK(cmp_db_execute(
      &db, "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT);"));
  CMP_TEST_OK(
      cmp_db_execute(&db, "INSERT INTO users (name) VALUES ('Alice');"));

  /* Syntax error test */
  CMP_TEST_EXPECT(
      cmp_db_execute(&db, "CREATE TABLE users (id INTEGER PRIMARY KEY);"),
      CMP_ERR_UNKNOWN);

  CMP_TEST_OK(cmp_db_shutdown(&db));

  return 0;
}

static int g_async_status = -1;
static int g_async_ticks = 0;

static void CMP_CALL test_cmp_db_async_cb(void *ctx, int status, void *user) {
  (void)ctx;
  (void)user;
  g_async_status = status;
}

static int CMP_CALL test_db_poll_ui(void *user, CMPBool *out_continue) {
  CMPEventLoop *loop = (CMPEventLoop *)user;
  g_async_ticks++;
  if (g_async_status != -1 || g_async_ticks > 100) {
    cmp_event_loop_stop(loop);
    *out_continue = CMP_FALSE;
  } else {
    *out_continue = CMP_TRUE;
  }
  return CMP_OK;
}

static int test_cmp_db_execute_async(void) {
  CMPDb db;
  CMPAllocator allocator;
  CMPTasks tasks;
  CMPEventLoop loop;
  CMPTasksDefaultConfig tasks_config;
  CMPAsyncEventLoopConfig loop_config;

  cmp_get_default_allocator(&allocator);

  CMP_TEST_OK(cmp_db_init(&db, &allocator, ":memory:"));

  CMP_TEST_OK(cmp_tasks_default_config_init(&tasks_config));
  tasks_config.allocator = &allocator;
  tasks_config.worker_count = 1;
  CMP_TEST_OK(cmp_tasks_default_create(&tasks_config, &tasks));

  memset(&loop_config, 0, sizeof(loop_config));
  loop_config.on_poll_ui = test_db_poll_ui;
  loop_config.ctx = &loop;

  CMP_TEST_OK(cmp_event_loop_async_init(&allocator, &loop_config, &loop));

  CMP_TEST_EXPECT(cmp_db_execute_async(NULL, &tasks, &loop, "SELECT 1",
                                       test_cmp_db_async_cb, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_db_execute_async(&db, NULL, &loop, "SELECT 1",
                                       test_cmp_db_async_cb, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_db_execute_async(&db, &tasks, &loop, NULL,
                                       test_cmp_db_async_cb, NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  g_async_status = -1;
  g_async_ticks = 0;
  CMP_TEST_OK(cmp_db_execute_async(&db, &tasks, &loop,
                                   "CREATE TABLE async_table (id INTEGER);",
                                   test_cmp_db_async_cb, NULL));

  /* Run the loop until the callback fires and stops it */
  CMP_TEST_OK(cmp_event_loop_run(&loop));

  CMP_TEST_EXPECT(g_async_status, CMP_OK);

  CMP_TEST_OK(cmp_tasks_default_destroy(&tasks));
  CMP_TEST_OK(cmp_event_loop_async_destroy(&allocator, &loop));

  CMP_TEST_OK(cmp_db_shutdown(&db));

  return 0;
}

int main(void) {
  if (test_cmp_db_init() != 0)
    return 1;
  if (test_cmp_db_execute() != 0)
    return 1;
  if (test_cmp_db_execute_async() != 0)
    return 1;

  printf("All db tests passed!\n");
  return 0;
}
