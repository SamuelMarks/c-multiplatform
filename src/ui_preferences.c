/**
 * @file ui_preferences.c
 * @brief Implementation of persistent UI preferences (local storage, IDB, etc).
 */

/* clang-format off */
#include "../include/ui_preferences.h"
#include "../include/ui_web_bridge.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <string.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif
/* clang-format on */

/**
 * @struct ui_preferences
 * @brief Context for asynchronous storage interactions.
 */
struct ui_preferences {
  struct ui_thread_pool *pool; /**< Thread pool for native background IO. */
  struct ui_execution_context *ctx; /**< Execution context for callbacks. */
};

/**
 * @brief Creates a new preferences context.
 * @param[in,out] pool The thread pool for native background IO.
 * @param[in,out] ctx The execution context for callbacks.
 * @param[out] out_prefs Pointer to store the created preferences context.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_create(struct ui_thread_pool *pool,
                                 struct ui_execution_context *ctx,
                                 struct ui_preferences **out_prefs) {
  struct ui_preferences *prefs;

  if (!pool || !ctx || !out_prefs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  prefs = (struct ui_preferences *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_preferences));
  if (!prefs) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  prefs->pool = pool;
  prefs->ctx = ctx;
  *out_prefs = prefs;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a preferences context.
 * @param[in,out] prefs The context to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_destroy(struct ui_preferences *prefs) {
  if (!prefs) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  C_MULTIPLATFORM_FREE(prefs);
  return UI_ERROR_NONE;
}

#if defined(__EMSCRIPTEN__)
EM_JS(void, set_local_storage_js, (const char *key, const char *value), {
  try {
    const k = UTF8ToString(key), v = UTF8ToString(value);
    if (typeof localStorage != 'undefined') {
      localStorage.setItem(k, v);
    } else {
      if (!globalThis._mock_ls)
        globalThis._mock_ls = new Map();
      globalThis._mock_ls.set(k, v);
    }
  } catch (e) {
  }
})

EM_JS(char *, get_local_storage_js, (const char *key), {
  try {
    const k = UTF8ToString(key);
    let val = null;
    if (typeof localStorage != 'undefined') {
      val = localStorage.getItem(k);
    } else if (globalThis._mock_ls) {
      val = globalThis._mock_ls.get(k) || null;
    }
    if (!val)
      return 0;
    const lengthBytes = lengthBytesUTF8(val) + 1;
    const stringOnWasmHeap = _malloc(lengthBytes);
    if (stringOnWasmHeap) {
      stringToUTF8(val, stringOnWasmHeap, lengthBytes);
    }
    return stringOnWasmHeap;
  } catch (e) {
    return 0;
  }
})
#endif

/**
 * @brief Synchronously sets a string preference (where supported, e.g. web
 * localStorage).
 * @param[in,out] prefs The preferences context.
 * @param[in] key The preference key.
 * @param[in] value The preference string value.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_set_string(struct ui_preferences *prefs,
                                     const char *key, const char *value) {
  if (!prefs || !key || !value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(__EMSCRIPTEN__)
  set_local_storage_js(key, value);
  return UI_ERROR_NONE;
#else
  /* Native implementation - file based or registry based. For now, we will
   * simulate. */
  /* In a real implementation this would use cfs or SQLite */
  return UI_ERROR_UNSUPPORTED;
#endif
}

/**
 * @brief Synchronously gets a string preference (where supported).
 * @param[in,out] prefs The preferences context.
 * @param[in] key The preference key.
 * @param[out] out_value Pointer to store the retrieved string (caller must
 * free).
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_preferences_get_string(struct ui_preferences *prefs,
                                     const char *key, char **out_value) {
  if (!prefs || !key || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(__EMSCRIPTEN__)
  *out_value = get_local_storage_js(key);
  if (*out_value == NULL) {
    return UI_ERROR_NOT_FOUND;
  }
  return UI_ERROR_NONE;
#else
  /* Native implementation */
  *out_value = NULL;
  return UI_ERROR_UNSUPPORTED;
#endif
}

ui_error_t ui_preferences_save_binary_async(struct ui_preferences *prefs,
                                            const char *key, const void *data,
                                            size_t length,
                                            struct ui_promise **out_promise) {
  ui_error_t rc;
  struct ui_promise *promise;

  if (!prefs || !key || !data || !out_promise) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_promise_create(&promise);
  if (rc != UI_ERROR_NONE)
    return rc;

  {
    ui_error_t rc_cleanup = ui_promise_reject(promise, UI_ERROR_UNSUPPORTED);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  *out_promise = promise;
  (void)length;
  return UI_ERROR_NONE;
}
