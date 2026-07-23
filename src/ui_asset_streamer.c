/* clang-format off */
#include "../include/ui_asset_streamer.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int g_mock_io_fail;
#define UI_FSEEK(f, o, w) (g_mock_io_fail == 1 ? -1 : fseek(f, o, w))
#define UI_FTELL(f) (g_mock_io_fail == 2 ? -1 : ftell(f))
#define UI_FREAD(p, s, n, f) (g_mock_io_fail == 3 ? 0 : fread(p, s, n, f))
#else
#define UI_FSEEK(f, o, w) fseek(f, o, w)
#define UI_FTELL(f) ftell(f)
#define UI_FREAD(p, s, n, f) fread(p, s, n, f)
#endif

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
extern int fetch_asset_js(const char *url_cstr, int task_ptr);
#endif

struct ui_asset_streamer {
  struct ui_thread_pool *pool;
  struct ui_execution_context *ctx;
};

struct ui_asset_task {
  struct ui_asset_streamer *streamer;
  char *url;
  enum ui_asset_type type;
  struct ui_promise *promise;
  struct ui_asset *asset;
  enum ui_error error;
};

/**
 * @brief Creates a new asset streamer.
 *
 * @param pool The thread pool to use for background loading.
 * @param ctx The execution context to use for resolving promises.
 * @param out_streamer Pointer to receive the new streamer handle.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error
ui_asset_streamer_create(struct ui_thread_pool *pool,
                         struct ui_execution_context *ctx,
                         struct ui_asset_streamer **out_streamer) {
  struct ui_asset_streamer *streamer = NULL;

  if (!pool || !ctx || !out_streamer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  streamer =
      (struct ui_asset_streamer *)UI_MALLOC(sizeof(struct ui_asset_streamer));
  if (!streamer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  streamer->pool = pool;
  streamer->ctx = ctx;
  *out_streamer = streamer;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys an asset streamer and frees its resources.
 *
 * @param streamer The streamer to destroy.
 * @return UI_ERROR_NONE on success, UI_ERROR_INVALID_ARGUMENT if streamer is
 * NULL.
 */
enum ui_error ui_asset_streamer_destroy(struct ui_asset_streamer *streamer) {
  if (!streamer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  UI_FREE(streamer);
  return UI_ERROR_NONE;
}

/**
 * @brief Frees a loaded asset.
 *
 * @param asset The asset to free.
 */
void ui_asset_destroy(struct ui_asset *asset) {
  if (asset) {
    UI_FREE(asset->url);
    UI_FREE(asset->data);
    UI_FREE(asset);
  }
}

static enum ui_error asset_task_complete(void *user_data) {
  struct ui_asset_task *task = (struct ui_asset_task *)user_data;
  if (task->error == UI_ERROR_NONE) {
    ui_promise_resolve(task->promise, task->asset);
  } else {
    ui_promise_reject(task->promise, task->error);
  }

  UI_FREE(task->url);
  UI_FREE(task);
  return UI_ERROR_NONE;
}

static enum ui_error asset_task_execute(void *user_data) {
  struct ui_asset_task *task = (struct ui_asset_task *)user_data;
  FILE *f = NULL;
  long size;
  size_t read_bytes;

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
#endif
#ifdef UI_TEST_MOCK_ALLOC
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

  task->asset = (struct ui_asset *)UI_MALLOC(sizeof(struct ui_asset));
  if (!task->asset) {
    task->error = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
  memset(task->asset, 0, sizeof(struct ui_asset));

  task->asset->type = task->type;
  task->asset->size = (ui_uint32)size;
  task->asset->data =
      UI_MALLOC((size_t)size + 1); /* +1 for null terminator safety */
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
    size_t url_len;
    url_len = strlen(task->url);
    task->asset->url = (char *)UI_MALLOC(url_len + 1);
    if (!task->asset->url) {
      task->error = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }
#if defined(_MSC_VER)
    strcpy_s(task->asset->url, url_len + 1, task->url);
#else
    strcpy(task->asset->url, task->url);
#endif
  }

cleanup:
  if (f) {
    fclose(f);
  }

  if (task->error != UI_ERROR_NONE && task->asset) {
    if (task->asset->data) {
      UI_FREE(task->asset->data);
    }
    UI_FREE(task->asset);
    task->asset = NULL;
  }
  {
    enum ui_error sched_rc = ui_execution_context_schedule(
        task->streamer->ctx, asset_task_complete, task);
    if (sched_rc != UI_ERROR_NONE) {
      if (task->asset) {
        if (task->asset->url)
          UI_FREE(task->asset->url);
        if (task->asset->data)
          UI_FREE(task->asset->data);
        UI_FREE(task->asset);
      }
      if (task->url) {
        UI_FREE(task->url);
      }
      UI_FREE(task);
    }
    return sched_rc;
  }
}

/**
 * @brief Requests an asset to be loaded asynchronously.
 *
 * @param streamer The streamer.
 * @param url The URL or file path of the asset.
 * @param type The expected type of the asset.
 * @param out_promise Pointer to receive a promise that resolves with (struct
 * ui_asset*).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
enum ui_error ui_asset_streamer_request(struct ui_asset_streamer *streamer,
                                        const char *url,
                                        enum ui_asset_type type,
                                        struct ui_promise **out_promise) {
  enum ui_error rc = UI_ERROR_NONE;
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

  task = (struct ui_asset_task *)UI_MALLOC(sizeof(struct ui_asset_task));
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
  task->url = (char *)UI_MALLOC(url_len + 1);
  if (!task->url) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
#if defined(_MSC_VER)
  strcpy_s(task->url, url_len + 1, url);
#else
  strcpy(task->url, url);
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
      ui_promise_destroy(promise);
    }
    if (task) {
      if (task->url) {
        UI_FREE(task->url);
      }
      UI_FREE(task);
    }
  }
  return rc;
}
