/**
 * @file ui_ffi_runtime.c
 * @brief Foreign Function Interface implementation for external language
 * bindings.
 */

/* clang-format off */
#include "ui_ffi_runtime.h"
#include "ui_app_state_registry.h"
#include <string.h>
#include <stdlib.h>
/* clang-format on */

/**
 * @brief FFI entry point to mount an application from a serialized JSON schema.
 */
ui_error_t ui_ffi_app_load_runtime(const char *json_schema,
                                   struct ui_dom_node *mount_host,
                                   struct ui_dom_node **out_root) {
  struct ui_app_load_config cfg;

  if (!json_schema || !out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = UI_APP_LOAD_MODE_RUNTIME;
  cfg.schema_payload = json_schema;

  return ui_app_load(&cfg, mount_host, out_root);
}

/**
 * @brief FFI entry point to mount an application using a compiled Ahead-of-Time
 * function pointer.
 */
ui_error_t ui_ffi_app_load_aot(ui_aot_mount_fn aot_mount,
                               struct ui_dom_node *mount_host,
                               struct ui_dom_node **out_root) {
  struct ui_app_load_config cfg;

  if (!aot_mount || !out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = UI_APP_LOAD_MODE_AOT;
  cfg.aot_mount = aot_mount;

  return ui_app_load(&cfg, mount_host, out_root);
}

/**
 * @brief FFI entry point to mount a dynamic runtime schema snippet into an
 * existing DOM node.
 */
ui_error_t ui_ffi_app_load_snippet(const char *snippet_json,
                                   struct ui_dom_node *parent_node,
                                   struct ui_dom_node **out_snippet_root) {
  return ui_app_load_snippet(snippet_json, parent_node, NULL, NULL,
                             out_snippet_root);
}

/**
 * @brief FFI entry point to set an integer value in the global app state.
 */
ui_error_t ui_ffi_app_state_set_int(const char *key, int val) {
  struct ui_app_state_registry *g_state = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  if (!key) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_app_state_registry_get_global(&g_state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  payload.int_val = (ui_int32)val;
  return ui_app_state_registry_set_value(g_state, key, payload);
}

/**
 * @brief FFI entry point to get an integer value from the global app state.
 */
ui_error_t ui_ffi_app_state_get_int(const char *key, int *out_val) {
  struct ui_app_state_registry *g_state = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  if (!key || !out_val) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_app_state_registry_get_global(&g_state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_app_state_registry_get_value(g_state, key, &payload);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  *out_val = (int)payload.int_val;
  return UI_ERROR_NONE;
}

/**
 * @brief FFI entry point to set a string value in the global app state.
 */
ui_error_t ui_ffi_app_state_set_string(const char *key, const char *val) {
  struct ui_app_state_registry *g_state = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  if (!key || !val) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_app_state_registry_get_global(&g_state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  payload.ptr_val = (void *)val;
  return ui_app_state_registry_set_value(g_state, key, payload);
}

/**
 * @brief FFI entry point to get a string value from the global app state.
 */
ui_error_t ui_ffi_app_state_get_string(const char *key, const char **out_val) {
  struct ui_app_state_registry *g_state = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  if (!key || !out_val) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_app_state_registry_get_global(&g_state);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_app_state_registry_get_value(g_state, key, &payload);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  *out_val = (const char *)payload.ptr_val;
  return UI_ERROR_NONE;
}
