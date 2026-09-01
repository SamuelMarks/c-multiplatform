/* clang-format off */
#include "ui_asset_streamer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(_MSC_VER)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#else
#include <unistd.h>
/* clang-format on */
#endif

extern int g_malloc_fail_countdown;

static ui_error_t sleep_ms(int ms) {
#if defined(_MSC_VER)
  Sleep(ms);
#else
  usleep(ms * 1000);
#endif
  return UI_ERROR_NONE;
}

static int run_test(const char *name, int (*test_fn)(void)) {
  int rc;
  printf("Running %s... ", name);
  rc = test_fn();
  if (rc == 0) {
    printf("PASS\n");
    return 0;
  } else {
    printf("FAIL\n");
    return 1;
  }
}

static ui_error_t on_resolve(void *result, void *user_data, void **out_result) {
  struct ui_asset *asset = (struct ui_asset *)result;
  int *completed = (int *)user_data;
  *completed = 1;
  ui_asset_destroy(asset);
  return UI_ERROR_NONE;
}

static ui_error_t on_reject(ui_error_t err, void *user_data,
                            void **out_result) {
  int *completed = (int *)user_data;
  *completed = -1; /* -1 means rejected */
  (void)err;
  return UI_ERROR_NONE;
}

static int test_successful_load(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_asset_streamer *streamer = NULL;
  struct ui_promise *promise = NULL;
  int completed = 0;
  FILE *f = NULL;
  ui_error_t err;

  /* Create dummy file */
#if defined(_MSC_VER)
  fopen_s(&f, "test_asset.txt", "wb");
#else
  f = fopen("test_asset.txt", "wb");
#endif
  if (!f)
    return 1;
  fwrite("hello", 1, 5, f);
  fclose(f);

  err = ui_thread_pool_create(2, &pool);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_execution_context_create(&ctx);
  if (err != UI_ERROR_NONE) {
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_asset_streamer_create(pool, ctx, &streamer);
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_asset_streamer_request(streamer, "test_asset.txt",
                                  UI_ASSET_TYPE_TEXT, &promise);
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_asset_streamer_destroy(streamer);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_promise_then(promise, on_resolve, on_reject, &completed, NULL);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Wait for background thread to load and push to ctx */
  while (completed == 0) {
    ui_execution_context_tick(ctx);
    sleep_ms(10);
  }

  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_asset_streamer_destroy(streamer);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_thread_pool_destroy(pool);

  remove("test_asset.txt");

  return completed == 1 ? 0 : 1;
}

static int test_failed_load(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_asset_streamer *streamer = NULL;
  struct ui_promise *promise = NULL;
  int completed = 0;
  ui_error_t err;

  err = ui_thread_pool_create(2, &pool);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_execution_context_create(&ctx);
  if (err != UI_ERROR_NONE) {
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_asset_streamer_create(pool, ctx, &streamer);
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_asset_streamer_request(streamer, "non_existent_file_12345.bin",
                                  UI_ASSET_TYPE_BINARY, &promise);
  if (err != UI_ERROR_NONE) {
    {
      ui_error_t rc_cleanup = ui_asset_streamer_destroy(streamer);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    {
      ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_promise_then(promise, on_resolve, on_reject, &completed, NULL);
  if (err != UI_ERROR_NONE)
    return 1;

  /* Wait for background thread to fail and push to ctx */
  while (completed == 0) {
    ui_execution_context_tick(ctx);
    sleep_ms(10);
  }

  {
    ui_error_t rc_cleanup = ui_promise_destroy(promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_asset_streamer_destroy(streamer);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_thread_pool_destroy(pool);

  return completed == -1 ? 0 : 1;
}

static int test_edge_cases(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_asset_streamer *streamer = NULL;
  struct ui_promise *promise = NULL;
  int i;
  ui_error_t err;
  FILE *f;

  /* NULL arguments */
  {
    ui_error_t rc_cleanup = ui_asset_streamer_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_asset_destroy(NULL);

  assert(ui_asset_streamer_create(NULL, NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);

  ui_thread_pool_create(1, &pool);
  ui_execution_context_create(&ctx);

  assert(ui_asset_streamer_create(pool, NULL, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_asset_streamer_create(pool, ctx, NULL) ==
         UI_ERROR_INVALID_ARGUMENT);
  assert(ui_asset_streamer_create(NULL, ctx, &streamer) ==
         UI_ERROR_INVALID_ARGUMENT);

  err = ui_asset_streamer_create(pool, ctx, &streamer);
  assert(err == UI_ERROR_NONE);

  assert(ui_asset_streamer_request(NULL, "test", UI_ASSET_TYPE_TEXT,
                                   &promise) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_asset_streamer_request(streamer, NULL, UI_ASSET_TYPE_TEXT,
                                   &promise) == UI_ERROR_INVALID_ARGUMENT);
  assert(ui_asset_streamer_request(streamer, "test", UI_ASSET_TYPE_TEXT,
                                   NULL) == UI_ERROR_INVALID_ARGUMENT);

  /* OOM loop for create */
  for (i = 0; i < 5; i++) {
    struct ui_asset_streamer *test_streamer = NULL;
    g_malloc_fail_countdown = i;
    err = ui_asset_streamer_create(pool, ctx, &test_streamer);
    if (err == UI_ERROR_NONE) {
      {
        ui_error_t rc_cleanup = ui_asset_streamer_destroy(test_streamer);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      break;
    } else {
      assert(err == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;

  /* Create dummy file */
#if defined(_MSC_VER)
  fopen_s(&f, "test_asset_edge.txt", "wb");
#else
  f = fopen("test_asset_edge.txt", "wb");
#endif
  if (f) {
    fwrite("hello", 1, 5, f);
    fclose(f);
  }

  /* OOM loop for request (and task loading internally) */
  for (i = 0; i < 20; i++) {
    struct ui_promise *p = NULL;
    int completed = 0;

    g_malloc_fail_countdown = i;
    err = ui_asset_streamer_request(streamer, "test_asset_edge.txt",
                                    UI_ASSET_TYPE_TEXT, &p);

    if (err == UI_ERROR_NONE) {
      /* Wait for background thread to hit OOM (or pass) */
      err = ui_promise_then(p, on_resolve, on_reject, &completed, NULL);
      if (err == UI_ERROR_NONE) {
        while (completed == 0) {
          ui_execution_context_tick(ctx);
          sleep_ms(10);
        }
      } else {
        /* Even if promise_then fails, the background task is running.
           We must wait for it to avoid leaking the task and overwriting
           ctx->task. We can just tick a few times to let it finish. */
        int ticks = 0;
        while (ticks < 10) {
          ui_execution_context_tick(ctx);
          sleep_ms(10);
          ticks++;
        }
      }
      {
        ui_error_t rc_cleanup = ui_promise_destroy(p);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      if (completed == 1) {
        break; /* OOM loop finished */
      }
    } else {
      assert(err == UI_ERROR_OUT_OF_MEMORY);
    }
  }
  g_malloc_fail_countdown = -1;

  /* Mock IO failure tests */
  extern int g_mock_io_fail;
  for (i = 1; i <= 5; i++) {
    struct ui_promise *p = NULL;
    int completed = 0;

    g_mock_io_fail = i;
    err = ui_asset_streamer_request(streamer, "test_asset_edge.txt",
                                    UI_ASSET_TYPE_TEXT, &p);

    if (err == UI_ERROR_NONE) {
      err = ui_promise_then(p, on_resolve, on_reject, &completed, NULL);
      if (err == UI_ERROR_NONE) {
        while (completed == 0) {
          ui_execution_context_tick(ctx);
          sleep_ms(10);
        }
      }
      {
        ui_error_t rc_cleanup = ui_promise_destroy(p);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      assert(completed == -1); /* Must reject */
    }
  }
  g_mock_io_fail = 0;

  remove("test_asset_edge.txt");

  /* Try to load a directory to trigger read errors (Linux/Unix behavior) */
  {
    struct ui_promise *p = NULL;
    int completed = 0;
    err = ui_asset_streamer_request(streamer, ".", UI_ASSET_TYPE_BINARY, &p);
    if (err == UI_ERROR_NONE) {
      ui_promise_then(p, on_resolve, on_reject, &completed, NULL);
      while (completed == 0) {
        ui_execution_context_tick(ctx);
        sleep_ms(10);
      }
      {
        ui_error_t rc_cleanup = ui_promise_destroy(p);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
  }

  /* Trigger schedule failure by corrupting the streamer's ctx */
  {
    struct ui_asset_streamer_hack {
      void *pool;
      void *ctx;
    };
    struct ui_asset_streamer_hack *hack =
        (struct ui_asset_streamer_hack *)streamer;
    void *old_ctx = hack->ctx;
    struct ui_promise *p = NULL;

    hack->ctx =
        NULL; /* This will cause ui_execution_context_schedule to fail */

#if defined(_MSC_VER)
    fopen_s(&f, "test_asset_sched_fail.txt", "wb");
#else
    f = fopen("test_asset_sched_fail.txt", "wb");
#endif
    if (f) {
      fwrite("hello", 1, 5, f);
      fclose(f);
    }

    err = ui_asset_streamer_request(streamer, "test_asset_sched_fail.txt",
                                    UI_ASSET_TYPE_TEXT, &p);
    if (err == UI_ERROR_NONE) {
      sleep_ms(50);
      {
        ui_error_t rc_cleanup = ui_promise_destroy(p);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }

    /* Do it again but with IO failure to test branches when asset is NULL */
    g_mock_io_fail = 1;
    err = ui_asset_streamer_request(streamer, "test_asset_sched_fail.txt",
                                    UI_ASSET_TYPE_TEXT, &p);
    if (err == UI_ERROR_NONE) {
      sleep_ms(50);
      {
        ui_error_t rc_cleanup = ui_promise_destroy(p);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    g_mock_io_fail = 0;

    remove("test_asset_sched_fail.txt");
    hack->ctx = old_ctx;
  }

  {
    ui_error_t rc_cleanup = ui_asset_streamer_destroy(streamer);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_thread_pool_destroy(pool);

#ifdef UI_TEST_MOCK_ALLOC
  extern ui_error_t run_asset_streamer_coverage(void);
  run_asset_streamer_coverage();
#endif
  return 0;
}

int main(void) {
  int failed = 0;
  failed += run_test("test_successful_load", test_successful_load);
  failed += run_test("test_failed_load", test_failed_load);
  failed += run_test("test_edge_cases", test_edge_cases);
  return failed == 0 ? 0 : 1;
}
