/* clang-format off */
#include "ui_preferences.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#else
#include <unistd.h>
#endif
/* clang-format on */

extern int g_malloc_fail_countdown;

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

static int test_preferences_lifecycle(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_preferences *prefs = NULL;
  ui_error_t err;

  err = ui_thread_pool_create(2, &pool);
  if (err != UI_ERROR_NONE)
    return 1;

  err = ui_execution_context_create(&ctx);
  if (err != UI_ERROR_NONE) {
    ui_thread_pool_destroy(pool);
    return 1;
  }

  err = ui_preferences_create(pool, ctx, &prefs);
  if (err != UI_ERROR_NONE) {
    (void)ui_execution_context_destroy(ctx);
    ui_thread_pool_destroy(pool);
    return 1;
  }

  ui_preferences_destroy(prefs);
  (void)ui_execution_context_destroy(ctx);
  ui_thread_pool_destroy(pool);

  return 0;
}

static int test_preferences_set_get(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_preferences *prefs = NULL;
  ui_error_t err;
  char *val = NULL;

  ui_thread_pool_create(2, &pool);
  ui_execution_context_create(&ctx);
  ui_preferences_create(pool, ctx, &prefs);

  err = ui_preferences_set_string(prefs, "test_key", "test_value");
#if defined(__EMSCRIPTEN__)
  if (err != UI_ERROR_NONE)
    return 1;
#else
  if (err != UI_ERROR_UNSUPPORTED)
    return 1;
#endif

  err = ui_preferences_get_string(prefs, "test_key", &val);
#if defined(__EMSCRIPTEN__)
  if (err != UI_ERROR_NONE)
    return 1;
  if (strcmp(val, "test_value") != 0)
    return 1;
  free(val); /* Note: JS _malloced buffer currently needs free() not FREE
                here due to testing context */
#else
  if (err != UI_ERROR_UNSUPPORTED)
    return 1;
#endif

  ui_preferences_destroy(prefs);
  (void)ui_execution_context_destroy(ctx);
  ui_thread_pool_destroy(pool);

  return 0;
}

static int test_preferences_save_binary(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_preferences *prefs = NULL;
  struct ui_promise *promise = NULL;
  ui_error_t err;
  const char *data = "binary_data";

  ui_thread_pool_create(2, &pool);
  ui_execution_context_create(&ctx);
  ui_preferences_create(pool, ctx, &prefs);

  err = ui_preferences_save_binary_async(prefs, "bin_key", data, strlen(data),
                                         &promise);
  if (err != UI_ERROR_NONE)
    return 1;

#if defined(__EMSCRIPTEN__)
  /* Resolves eventually via JS */
#else
  /* Rejected natively currently */
#endif

  (void)ui_promise_destroy(promise);
  ui_preferences_destroy(prefs);
  (void)ui_execution_context_destroy(ctx);
  ui_thread_pool_destroy(pool);

  return 0;
}

static int test_preferences_nulls(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_preferences *prefs = NULL;
  struct ui_promise *promise = NULL;
  char *val = NULL;

  ui_thread_pool_create(2, &pool);
  ui_execution_context_create(&ctx);
  ui_preferences_create(pool, ctx, &prefs);

  if (ui_preferences_create(NULL, ctx, &prefs) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_create(pool, NULL, &prefs) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_create(pool, ctx, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_preferences_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_preferences_set_string(NULL, "k", "v") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_set_string(prefs, NULL, "v") != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_set_string(prefs, "k", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_preferences_get_string(NULL, "k", &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_get_string(prefs, NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_get_string(prefs, "k", NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_preferences_save_binary_async(NULL, "k", "v", 1, &promise) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_save_binary_async(prefs, NULL, "v", 1, &promise) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_save_binary_async(prefs, "k", NULL, 1, &promise) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_preferences_save_binary_async(prefs, "k", "v", 1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_preferences_destroy(prefs);
  (void)ui_execution_context_destroy(ctx);
  ui_thread_pool_destroy(pool);
  return 0;
}

static int test_preferences_oom(void) {
  struct ui_thread_pool *pool = NULL;
  struct ui_execution_context *ctx = NULL;
  struct ui_preferences *prefs = NULL;
  struct ui_promise *promise = NULL;

  ui_thread_pool_create(2, &pool);
  ui_execution_context_create(&ctx);

  g_malloc_fail_countdown = 0;
  if (ui_preferences_create(pool, ctx, &prefs) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_preferences_create(pool, ctx, &prefs);

  g_malloc_fail_countdown = 0;
  if (ui_preferences_save_binary_async(prefs, "k", "v", 1, &promise) !=
      UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_preferences_destroy(prefs);
  (void)ui_execution_context_destroy(ctx);
  ui_thread_pool_destroy(pool);
  return 0;
}

int main(void) {
  int failed = 0;
  failed += run_test("test_preferences_lifecycle", test_preferences_lifecycle);
  failed += run_test("test_preferences_set_get", test_preferences_set_get);
  failed +=
      run_test("test_preferences_save_binary", test_preferences_save_binary);
  failed += run_test("test_preferences_nulls", test_preferences_nulls);
  failed += run_test("test_preferences_oom", test_preferences_oom);
  return failed == 0 ? 0 : 1;
}
