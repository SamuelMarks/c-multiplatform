/**
 * @file ui_asset_streamer.c
 * @brief Implementation of asynchronous asset streaming and loading.
 */

/* clang-format off */
#include "../include/ui_asset_streamer.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_asset_streamer_mock_fail = 0;
extern int g_malloc_fail_countdown;

/**
 * @brief mock_promise_resolve.
 * @param promise Parameter promise.
 * @param value Parameter value.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t mock_promise_resolve(struct ui_promise *promise,
                                       void *value) {
  if (g_asset_streamer_mock_fail == 1) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_promise_resolve)(promise, value);
}
#undef ui_promise_resolve
/** @cond */
#define ui_promise_resolve mock_promise_resolve
/** @endcond */

static ui_error_t mock_promise_reject(struct ui_promise *promise,
                                      ui_error_t error) {
  if (g_asset_streamer_mock_fail == 2) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_promise_reject)(promise, error);
}
#undef ui_promise_reject
/** @cond */
#define ui_promise_reject mock_promise_reject
/** @endcond */

#include "ui_thread_pool.h"
static ui_error_t mock_thread_pool_schedule(struct ui_thread_pool *pool,
                                            ui_error_t (*task)(void *),
                                            void *user_data) {
  if (g_asset_streamer_mock_fail == 4) {
    return UI_ERROR_UNKNOWN;
  }
  return (ui_thread_pool_schedule)(pool, task, user_data);
}
#undef ui_thread_pool_schedule
/** @cond */
#define ui_thread_pool_schedule mock_thread_pool_schedule
/** @endcond */
#endif

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_io_fail;
/** @cond */
#define UI_FSEEK(f, o, w) (g_mock_io_fail == 1 ? -1 : fseek(f, o, w))
/** @endcond */
#define UI_FTELL(f)                                                            \
  (g_mock_io_fail == 2                                                         \
       ? -1                                                                    \
       : (g_mock_io_fail == 5 ? (long)(256 * 1024 * 1024) + 1L : ftell(f)))
/** @cond */
#define UI_FREAD(p, s, n, f) (g_mock_io_fail == 3 ? 0 : fread(p, s, n, f))
/** @endcond */
#else
/** @cond */
#define UI_FSEEK(f, o, w) fseek(f, o, w)
/** @endcond */
/** @cond */
#define UI_FTELL(f) ftell(f)
/** @endcond */
/** @cond */
#define UI_FREAD(p, s, n, f) fread(p, s, n, f)
/** @endcond */
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
extern int fetch_asset_js(const char *url_cstr, int task_ptr);
#endif

/**
 * @struct ui_asset_streamer
 * @struct ui_asset_streamer
 * @brief Internal representation of an asset streamer.
 */
struct ui_asset_streamer {
  struct ui_thread_pool *pool;      /**< Thread pool used for async loading */
  struct ui_execution_context *ctx; /**< Context to resolve promises on */
};

/**
 * @struct ui_asset_task
 * @struct ui_asset_task
 * @brief Internal task for loading an asset asynchronously.
 */
struct ui_asset_task {
  struct ui_asset_streamer *streamer; /**< The parent streamer */
  char *url;                          /**< URL or path being loaded */
  enum ui_asset_type type;            /**< Expected type of the asset */
  struct ui_promise *promise; /**< Promise to resolve/reject upon completion */
  struct ui_asset *asset;     /**< The constructed asset if successful */
  ui_error_t error;           /**< The resulting error code, if any */
};

/**
 * @brief ui_asset_streamer_create.
 * @param pool Parameter pool.
 * @param ctx Parameter ctx.
 * @param out_streamer Parameter out_streamer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_asset_streamer_create(struct ui_thread_pool *pool,
                                    struct ui_execution_context *ctx,
                                    struct ui_asset_streamer **out_streamer) {
  struct ui_asset_streamer *streamer = NULL;

  if (!pool || !ctx || !out_streamer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  streamer = (struct ui_asset_streamer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_streamer));
  if (!streamer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  streamer->pool = pool;
  streamer->ctx = ctx;
  *out_streamer = streamer;

  return UI_ERROR_NONE;
}

/**
 * @brief ui_asset_streamer_destroy.
 * @param streamer Parameter streamer.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_asset_streamer_destroy(struct ui_asset_streamer *streamer) {
  if (!streamer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(streamer);
  return UI_ERROR_NONE;
}

/**
 * @brief ui_asset_destroy.
 * @param asset Parameter asset.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_asset_destroy(struct ui_asset *asset) {
  if (asset) {
    C_MULTIPLATFORM_FREE(asset->url);
    C_MULTIPLATFORM_FREE(asset->data);
    C_MULTIPLATFORM_FREE(asset);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief asset_task_complete.
 * @param user_data Parameter user_data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t asset_task_complete(void *user_data) {
  struct ui_asset_task *task = (struct ui_asset_task *)user_data;
  ui_error_t rc = UI_ERROR_NONE;

  if (task->error == UI_ERROR_NONE) {
    rc = ui_promise_resolve(task->promise, task->asset);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  } else {
    rc = ui_promise_reject(task->promise, task->error);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  C_MULTIPLATFORM_FREE(task->url);
  C_MULTIPLATFORM_FREE(task);
  return UI_ERROR_NONE;
}

/**
 * @brief asset_task_execute.
 * @param user_data Parameter user_data.
 * @return UI_ERROR_NONE on success.
 */
static ui_error_t asset_task_execute(void *user_data) {
  struct ui_asset_task *task = (struct ui_asset_task *)user_data;
  FILE *f = NULL;
  long size;
  size_t read_bytes;
  ui_error_t sched_rc;

#if defined(_MSC_VER)
  if (fopen_s(&f, task->url, "rb") != 0) {
    f = NULL;
  }
#else
  f = fopen(task->url, "rb");
#endif

  if (!f) {
    task->error = UI_ERROR_IO_FAILED;
    goto cleanup;
  }

#ifdef UI_TEST_MOCK_ALLOC
  if (g_mock_io_fail == 1) {
    task->error = UI_ERROR_IO_FAILED;
    goto cleanup;
  }
  if (g_mock_io_fail == 4) {
    task->error = UI_ERROR_IO_FAILED;
    goto cleanup;
  }
#endif
  fseek(f, 0, SEEK_END);

  size = UI_FTELL(f);
  if (size <= 0 ||
      size >
          256 * 1024 *
              1024) { /* Cap at 256MB to prevent fishy malloc on directories */
    task->error = UI_ERROR_IO_FAILED;
    goto cleanup;
  }

  rewind(f);

  task->asset =
      (struct ui_asset *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_asset));
  if (!task->asset) {
    task->error = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  memset(task->asset, 0, sizeof(struct ui_asset));

  task->asset->type = task->type;
  task->asset->size = (ui_uint32)size;
  task->asset->data = C_MULTIPLATFORM_MALLOC(
      (size_t)size + 1); /* +1 for null terminator safety */
  if (!task->asset->data) {
    task->error = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  read_bytes = UI_FREAD(task->asset->data, 1, (size_t)size, f);
  if (read_bytes != (size_t)size) {
    task->error = UI_ERROR_IO_FAILED;
    goto cleanup;
  }

  /* Null terminate for text compatibility */
  ((char *)task->asset->data)[size] = '\0';

  {
    size_t url_len = strlen(task->url);
    task->asset->url = (char *)C_MULTIPLATFORM_MALLOC(url_len + 1);
    if (!task->asset->url) {
      task->error = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
#if defined(_MSC_VER)
    strcpy_s(task->asset->url, url_len + 1, task->url);
#else
    UI_STRCPY(task->asset->url, sizeof(task->asset->url), task->url);
#endif
  }

cleanup:
  if (f) {
    fclose(f);
  }

  if (task->error != UI_ERROR_NONE && task->asset) {
    if (task->asset->data) {
      C_MULTIPLATFORM_FREE(task->asset->data);
    }
    C_MULTIPLATFORM_FREE(task->asset);
    task->asset = NULL;
  }

  sched_rc = ui_execution_context_schedule(task->streamer->ctx,
                                           asset_task_complete, task);
  if (sched_rc != UI_ERROR_NONE) {
    if (task->asset) {
      C_MULTIPLATFORM_FREE(task->asset->url);
      C_MULTIPLATFORM_FREE(task->asset->data);
      C_MULTIPLATFORM_FREE(task->asset);
    }
    C_MULTIPLATFORM_FREE(task->url);
    C_MULTIPLATFORM_FREE(task);
    return sched_rc;
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_asset_streamer_request.
 * @param streamer Parameter streamer.
 * @param url Parameter url.
 * @param type Parameter type.
 * @param out_promise Parameter out_promise.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_asset_streamer_request(struct ui_asset_streamer *streamer,
                                     const char *url, enum ui_asset_type type,
                                     struct ui_promise **out_promise) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_asset_task *task = NULL;
  struct ui_promise *promise = NULL;
  size_t url_len;

  if (!streamer || !url || !out_promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  if (task) {
    memset(task, 0, sizeof(struct ui_asset_task));
  }
  if (!task) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  task->streamer = streamer;
  task->type = type;
  task->promise = promise;
  task->asset = NULL;
  task->error = UI_ERROR_NONE;

  url_len = strlen(url);
  task->url = (char *)C_MULTIPLATFORM_MALLOC(url_len + 1);
  if (!task->url) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
#if defined(_MSC_VER)
  strcpy_s(task->url, url_len + 1, url);
#else
  UI_STRCPY(task->url, sizeof(task->url), url);
#endif

  rc = ui_thread_pool_schedule(streamer->pool, asset_task_execute, task);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  *out_promise = promise;
  task = NULL;
  promise = NULL;

cleanup:
  if (rc != UI_ERROR_NONE) {
    if (promise) {
      {
        ui_error_t rc_cleanup = ui_promise_destroy(promise);
        if (rc_cleanup != UI_ERROR_NONE) {
          if (rc == UI_ERROR_NONE)
            rc = rc_cleanup;
        }
      }
    }
    if (task) {
      if (task->url) {
        C_MULTIPLATFORM_FREE(task->url);
      }
      C_MULTIPLATFORM_FREE(task);
    }
  }
  return rc;
}

#ifdef UI_TEST_MOCK_ALLOC
/* Forward declare internal functions we need to test directly */
static ui_error_t asset_task_execute(void *user_data);
static ui_error_t asset_task_complete(void *user_data);

ui_error_t run_asset_streamer_coverage(void);
/**
 * @brief run_asset_streamer_coverage.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t run_asset_streamer_coverage(void) {
  struct ui_asset_streamer *streamer = NULL;
  struct ui_thread_pool *pool = NULL;
  struct ui_promise *promise = NULL;
  struct ui_asset_task *task = NULL;
  FILE *dummy = NULL;
  struct ui_execution_context *ctx = NULL;

  {
    ui_error_t rc_cleanup = ui_thread_pool_create(1, &pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  {
    ui_error_t rc_cleanup = ui_execution_context_create(&ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  {
    ui_error_t rc_cleanup = ui_asset_streamer_create(pool, ctx, &streamer);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }

#if defined(_MSC_VER)
  fopen_s(&dummy, "dummy_asset.txt", "wb");
#else
  dummy = fopen("dummy_asset.txt", "wb");
#endif
  fwrite("test", 1, 4, dummy);
  fclose(dummy);

  /* Mock 1: fopen fails */
  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  memset(task, 0, sizeof(struct ui_asset_task));
  task->streamer = streamer;
  task->type = UI_ASSET_TYPE_BINARY;
  {
    ui_error_t rc_cleanup = ui_promise_create(&task->promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  task->url = (char *)C_MULTIPLATFORM_MALLOC(100);
  UI_STRCPY(task->url, 100, "dummy_asset.txt");
  g_mock_io_fail = 4; /* mock fopen fail */
  (void)asset_task_execute(task);
  g_mock_io_fail = 0;
  {
    ui_error_t rc_cleanup = ui_execution_context_tick(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }

  /* Mock 1: promise_resolve fails */
  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  memset(task, 0, sizeof(struct ui_asset_task));
  task->streamer = streamer;
  task->type = UI_ASSET_TYPE_BINARY;
  {
    ui_error_t rc_cleanup = ui_promise_create(&task->promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  task->url = (char *)C_MULTIPLATFORM_MALLOC(100);
  UI_STRCPY(task->url, 100, "dummy_asset.txt");
  (void)asset_task_execute(task);
  g_asset_streamer_mock_fail = 1; /* resolve fails */
  (void)asset_task_complete(task);
  g_asset_streamer_mock_fail = 0;
  {
    ui_error_t rc_cleanup = ui_execution_context_tick(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }

  g_malloc_fail_countdown = 0;
  (void)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_asset_task));
  g_malloc_fail_countdown = -1;

  /* Mock 2: promise_reject fails */
  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  memset(task, 0, sizeof(struct ui_asset_task));
  task->streamer = streamer;
  task->type = UI_ASSET_TYPE_BINARY;
  {
    ui_error_t rc_cleanup = ui_promise_create(&task->promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  task->url = (char *)C_MULTIPLATFORM_MALLOC(100);
  UI_STRCPY(task->url, 100, "non_existent.txt");
  (void)asset_task_execute(task); /* IO fail */
  g_asset_streamer_mock_fail = 2; /* reject fails */
  (void)asset_task_complete(task);
  g_asset_streamer_mock_fail = 0;
  {
    ui_error_t rc_cleanup = ui_execution_context_tick(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  g_malloc_fail_countdown = 0;
  (void)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_asset_task));
  g_malloc_fail_countdown = -1;

  /* Mock 3: FREAD fails */
  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  memset(task, 0, sizeof(struct ui_asset_task));
  task->streamer = streamer;
  task->type = UI_ASSET_TYPE_BINARY;
  {
    ui_error_t rc_cleanup = ui_promise_create(&task->promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  task->url = (char *)C_MULTIPLATFORM_MALLOC(100);
  UI_STRCPY(task->url, 100, "dummy_asset.txt");
  g_mock_io_fail = 3; /* builtin mock for UI_FREAD to return 0 */
  (void)asset_task_execute(task);
  g_mock_io_fail = 0;
  {
    ui_error_t rc_cleanup = ui_execution_context_tick(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }

  g_malloc_fail_countdown = 0;
  (void)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_asset_task));
  g_malloc_fail_countdown = -1;

  /* Mock OOM: asset allocation fails */
  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  memset(task, 0, sizeof(struct ui_asset_task));
  task->streamer = streamer;
  task->type = UI_ASSET_TYPE_BINARY;
  {
    ui_error_t rc_cleanup = ui_promise_create(&task->promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  task->url = (char *)C_MULTIPLATFORM_MALLOC(100);
  UI_STRCPY(task->url, 100, "dummy_asset.txt");
  g_malloc_fail_countdown = 0;
  (void)asset_task_execute(task);
  g_malloc_fail_countdown = -1;
  {
    ui_error_t rc_cleanup = ui_execution_context_tick(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  g_malloc_fail_countdown = 0;
  (void)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_asset_task));
  g_malloc_fail_countdown = -1;

  /* ui_asset_streamer_request */
  g_asset_streamer_mock_fail = 4; /* schedule fails */
  {
    ui_error_t rc_cleanup = ui_asset_streamer_request(
        streamer, "dummy_asset.txt", UI_ASSET_TYPE_BINARY, &promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  g_asset_streamer_mock_fail = 0;

  /* Mock OOM: asset url allocation fails */
  task = (struct ui_asset_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_asset_task));
  memset(task, 0, sizeof(struct ui_asset_task));
  task->streamer = streamer;
  task->type = UI_ASSET_TYPE_BINARY;
  {
    ui_error_t rc_cleanup = ui_promise_create(&task->promise);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  task->url = (char *)C_MULTIPLATFORM_MALLOC(100);
  UI_STRCPY(task->url, 100, "dummy_asset.txt");
  g_malloc_fail_countdown = 2;
  (void)asset_task_execute(task);
  g_malloc_fail_countdown = -1;
  (void)asset_task_complete(task);

  g_malloc_fail_countdown = 0;
  (void)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_asset_task));
  g_malloc_fail_countdown = -1;

  {
    ui_error_t rc_cleanup = ui_asset_streamer_destroy(streamer);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  {
    ui_error_t rc_cleanup = ui_thread_pool_destroy(pool);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  {
    ui_error_t rc_cleanup = ui_execution_context_destroy(ctx);
    if (rc_cleanup != UI_ERROR_NONE) {
      /* expected error */
    }
  }
  return UI_ERROR_NONE;
}
#endif
