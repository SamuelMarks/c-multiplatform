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
    localStorage.setItem(UTF8ToString(key), UTF8ToString(value));
  } catch (e) {
    console.error("localStorage setItem failed", e);
  }
})

EM_JS(char *, get_local_storage_js, (const char *key), {
  try {
    const val = localStorage.getItem(UTF8ToString(key));
    if (val == = null)
      return 0;
    const lengthBytes = lengthBytesUTF8(val) + 1;
    const stringOnWasmHeap = _malloc(lengthBytes);
    if (stringOnWasmHeap) {
      stringToUTF8(val, stringOnWasmHeap, lengthBytes);
    }
    return stringOnWasmHeap;
  } catch (e) {
    console.error("localStorage getItem failed", e);
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

#if defined(__EMSCRIPTEN__)
EM_JS(int, idb_save_js,
      (const char *key_cstr, const uint8_t *data, size_t length,
       int promise_id),
      {
        const key = UTF8ToString(key_cstr);
        const u8 = new Uint8Array(HEAPU8.buffer, data, length)
                       .slice(); /* copy to avoid heap mutation issues */
        if (!window._ui_idb) {
          const req = indexedDB.open("UIFrameworkDB", 1);
          req.onupgradeneeded = (e) = > {
            const db = e.target.result;
            if (!db.objectStoreNames.contains("assets")) {
              db.createObjectStore("assets");
            }
          };
          req.onsuccess = (e) = > {
            window._ui_idb = e.target.result;
            _do_save();
          };
          req.onerror = (e) = > {
            console.error("IDB Open Failed", e);
            _ui_web_bridge_promise_reject_js(promise_id, 2); /* io error */
          };
        } else {
          _do_save();
        }

        function _do_save() {
          const tx = window._ui_idb.transaction("assets", "readwrite");
          const store = tx.objectStore("assets");
          const req = store.put(u8, key);
          req.onsuccess = () = > {
            _ui_web_bridge_promise_resolve_js(promise_id, 0);
          };
          req.onerror = (e) = > {
            console.error("IDB Put Failed", e);
            _ui_web_bridge_promise_reject_js(promise_id, 2);
          };
        }
        return 0;
      })
#endif

/**
 * @brief Asynchronously saves a binary blob (e.g. via IndexedDB or background
 * file write).
 * @param[in,out] prefs The preferences context.
 * @param[in] key The preference key.
 * @param[in] data The binary data to save.
 * @param[in] length The length of the binary data in bytes.
 * @param[out] out_promise Pointer to store the promise tracking the save
 * operation.
 * @return UI_ERROR_NONE on success.
 */
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

#if defined(__EMSCRIPTEN__)
  {
    int js_promise_id;
    rc = ui_web_bridge_promise_to_js(promise, &js_promise_id);
    if (rc == UI_ERROR_NONE) {
      idb_save_js(key, (const uint8_t *)data, length, js_promise_id);
    }
  }
#else
  /* Native implementation stub */
  {
    ui_error_t rc_cleanup = ui_promise_reject(promise, UI_ERROR_UNSUPPORTED);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
#endif

  *out_promise = promise;
  (void)length;
  return UI_ERROR_NONE;
}
