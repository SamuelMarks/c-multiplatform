/**
 * @file test_ui_ffi_runtime.c
 * @brief Unit tests for ui_ffi_runtime Foreign Function Interface bindings.
 */

/* clang-format off */
#include "ui_ffi_runtime.h"
#include "ui_app_state_registry.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t mock_ffi_aot(struct ui_dom_node *parent,
                               struct ui_dynamic_context *ctx,
                               struct ui_app_state_registry *app_state,
                               struct ui_dom_node **out_root) {
  struct ui_dom_node *node = NULL;
  ui_error_t rc;
  (void)ctx;
  (void)app_state;

  if (!out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &node);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  if (parent) {
    rc = ui_dom_node_append_child(parent, node);
    if (rc != UI_ERROR_NONE) {
      ui_dom_node_destroy(node);
      return rc;
    }
  }
  *out_root = node;
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_dom_node *host = NULL;
  struct ui_dom_node *root = NULL;
  struct ui_dom_node *snippet_root = NULL;
  int int_val = 0;
  const char *str_val = NULL;
  ui_error_t rc;

  const char *app_json = "{\n"
                         "  \"app_id\": \"ffi_app\",\n"
                         "  \"routes\": [\n"
                         "    {\n"
                         "      \"path\": \"/\",\n"
                         "      \"root\": {\n"
                         "        \"type\": \"container\"\n"
                         "      }\n"
                         "    }\n"
                         "  ]\n"
                         "}";

  const char *snippet_json = "{\n"
                             "  \"type\": \"ui_button_base\",\n"
                             "  \"props\": {\"text\": \"FFI Button\"}\n"
                             "}";

  /* Invalid argument checks */
  rc = ui_ffi_app_load_runtime(NULL, host, &root);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_ffi_app_load_runtime(app_json, host, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_ffi_app_load_aot(NULL, host, &root);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_ffi_app_load_aot(mock_ffi_aot, host, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_ffi_app_state_set_int(NULL, 10);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_ffi_app_state_get_int(NULL, &int_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_ffi_app_state_get_int("key", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_ffi_app_state_set_string(NULL, "val");
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_ffi_app_state_set_string("key", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_ffi_app_state_get_string(NULL, &str_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_ffi_app_state_get_string("key", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* App state FFI tests */
  rc = ui_ffi_app_state_set_int("session_id", 4242);
  assert(rc == UI_ERROR_NONE);

  rc = ui_ffi_app_state_get_int("session_id", &int_val);
  assert(rc == UI_ERROR_NONE);
  assert(int_val == 4242);

  rc = ui_ffi_app_state_get_int("nonexistent_int", &int_val);
  assert(rc != UI_ERROR_NONE);

  rc = ui_ffi_app_state_set_string("user_role", "admin");
  assert(rc == UI_ERROR_NONE);

  rc = ui_ffi_app_state_get_string("user_role", &str_val);
  assert(rc == UI_ERROR_NONE);
  assert(str_val != NULL);
  assert(strcmp(str_val, "admin") == 0);

  rc = ui_ffi_app_state_get_string("nonexistent_str", &str_val);
  assert(rc != UI_ERROR_NONE);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &host);
  assert(rc == UI_ERROR_NONE);

  /* FFI runtime load */
  rc = ui_ffi_app_load_runtime(app_json, host, &root);
  assert(rc == UI_ERROR_NONE);
  assert(root != NULL);

  /* FFI AoT load */
  rc = ui_ffi_app_load_aot(mock_ffi_aot, host, &root);
  assert(rc == UI_ERROR_NONE);
  assert(root != NULL);

  /* FFI snippet load */
  rc = ui_ffi_app_load_snippet(snippet_json, host, &snippet_root);
  assert(rc == UI_ERROR_NONE);
  assert(snippet_root != NULL);

  /* Cleanup host */
  rc = ui_dom_node_destroy(host);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_shutdown_global();
  assert(rc == UI_ERROR_NONE);

  /* OOM tests for get_global failure */
  g_malloc_fail_countdown = 0;
  rc = ui_ffi_app_state_set_int("k", 1);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = ui_ffi_app_state_get_int("k", &int_val);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = ui_ffi_app_state_set_string("k", "v");
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = ui_ffi_app_state_get_string("k", &str_val);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_app_state_registry_shutdown_global();
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_ffi_runtime passed\n");
  return 0;
}
