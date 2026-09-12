/**
 * @file test_ui_app_load.c
 * @brief Unit tests for ui_app_load unified isomorphic loader.
 */

/* clang-format off */
#include "ui_app_load.h"
#include "ui_dom_node.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t mock_aot_mount(struct ui_dom_node *parent_node,
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

  rc = ui_dom_node_set_tag_name(node, "section");
  if (rc != UI_ERROR_NONE) {
    ui_dom_node_destroy(node);
    return rc;
  }

  if (parent_node) {
    rc = ui_dom_node_append_child(parent_node, node);
    if (rc != UI_ERROR_NONE) {
      ui_dom_node_destroy(node);
      return rc;
    }
  }

  *out_root = node;
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_dom_node *mount_host = NULL;
  struct ui_dom_node *root_dom = NULL;
  struct ui_dom_node *snippet_dom = NULL;
  struct ui_app_load_config cfg;
  ui_error_t rc;

  const char *app_schema = "{\n"
                           "  \"app_id\": \"test_app\",\n"
                           "  \"initial_route\": \"/\",\n"
                           "  \"routes\": [\n"
                           "    {\n"
                           "      \"path\": \"/\",\n"
                           "      \"root\": {\n"
                           "        \"id\": \"app_root\",\n"
                           "        \"type\": \"row\"\n"
                           "      }\n"
                           "    }\n"
                           "  ]\n"
                           "}";

  const char *multi_route_schema = "{\n"
                                   "  \"app_id\": \"multi_app\",\n"
                                   "  \"initial_route\": \"/second\",\n"
                                   "  \"routes\": [\n"
                                   "    {\n"
                                   "      \"path\": \"/first\",\n"
                                   "      \"root\": {\n"
                                   "        \"id\": \"r1\",\n"
                                   "        \"type\": \"row\"\n"
                                   "      }\n"
                                   "    },\n"
                                   "    {\n"
                                   "      \"path\": \"/second\",\n"
                                   "      \"root\": {\n"
                                   "        \"id\": \"r2\",\n"
                                   "        \"type\": \"column\"\n"
                                   "      }\n"
                                   "    }\n"
                                   "  ]\n"
                                   "}";

  const char *fallback_route_schema = "{\n"
                                      "  \"app_id\": \"fallback_app\",\n"
                                      "  \"initial_route\": \"/nonexistent\",\n"
                                      "  \"routes\": [\n"
                                      "    {\n"
                                      "      \"path\": \"/first\",\n"
                                      "      \"root\": {\n"
                                      "        \"id\": \"r1\",\n"
                                      "        \"type\": \"row\"\n"
                                      "      }\n"
                                      "    }\n"
                                      "  ]\n"
                                      "}";

  const char *no_root_node_schema = "{\n"
                                    "  \"app_id\": \"no_root_app\",\n"
                                    "  \"routes\": [\n"
                                    "    {\n"
                                    "      \"path\": \"/noroot\"\n"
                                    "    }\n"
                                    "  ]\n"
                                    "}";

  const char *empty_routes_schema = "{\n"
                                    "  \"app_id\": \"empty_routes_app\",\n"
                                    "  \"routes\": []\n"
                                    "}";

  const char *snippet_json = "{\n"
                             "  \"id\": \"ota_banner\",\n"
                             "  \"type\": \"ui_card_base\",\n"
                             "  \"props\": {\n"
                             "    \"title\": \"Special Promotion\"\n"
                             "  }\n"
                             "}";

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &mount_host);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks */
  rc = ui_app_load(NULL, mount_host, &root_dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = UI_APP_LOAD_MODE_RUNTIME;
  cfg.schema_payload = app_schema;
  rc = ui_app_load(&cfg, mount_host, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Invalid mode */
  cfg.mode = (enum ui_app_load_mode)999;
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* AoT mode NULL mount */
  cfg.mode = UI_APP_LOAD_MODE_AOT;
  cfg.aot_mount = NULL;
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Runtime mode NULL schema payload */
  cfg.mode = UI_APP_LOAD_MODE_RUNTIME;
  cfg.schema_payload = NULL;
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Runtime mode invalid JSON schema payload */
  cfg.schema_payload = "{ not valid json";
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc != UI_ERROR_NONE);

  /* Runtime mode empty routes */
  cfg.schema_payload = empty_routes_schema;
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Runtime mode route without root node */
  cfg.schema_payload = no_root_node_schema;
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc != UI_ERROR_NONE);

  /* Runtime mode route with unknown component type */
  cfg.schema_payload = "{\n"
                       "  \"app_id\": \"unknown_type_app\",\n"
                       "  \"routes\": [\n"
                       "    {\n"
                       "      \"path\": \"/\",\n"
                       "      \"root\": {\n"
                       "        \"id\": \"r_unk\",\n"
                       "        \"type\": \"completely_unknown_type_name\"\n"
                       "      }\n"
                       "    }\n"
                       "  ]\n"
                       "}";
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc != UI_ERROR_NONE);

  /* 1. Runtime mode mounting */
  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = UI_APP_LOAD_MODE_RUNTIME;
  cfg.schema_payload = app_schema;

  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc == UI_ERROR_NONE);
  assert(root_dom != NULL);
  assert(mount_host->first_child != NULL);

  /* Mount with NULL mount_host */
  root_dom = NULL;
  rc = ui_app_load(&cfg, NULL, &root_dom);
  assert(rc == UI_ERROR_NONE);
  assert(root_dom != NULL);
  ui_dom_node_destroy(root_dom);
  root_dom = NULL;

  /* Multi route with matching initial route */
  cfg.schema_payload = multi_route_schema;
  rc = ui_app_load(&cfg, NULL, &root_dom);
  assert(rc == UI_ERROR_NONE);
  assert(root_dom != NULL);
  ui_dom_node_destroy(root_dom);
  root_dom = NULL;

  /* Fallback route when initial route does not match */
  cfg.schema_payload = fallback_route_schema;
  rc = ui_app_load(&cfg, NULL, &root_dom);
  assert(rc == UI_ERROR_NONE);
  assert(root_dom != NULL);
  ui_dom_node_destroy(root_dom);
  root_dom = NULL;

  /* 2. AoT mode mounting */
  memset(&cfg, 0, sizeof(cfg));
  cfg.mode = UI_APP_LOAD_MODE_AOT;
  cfg.aot_mount = mock_aot_mount;

  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc == UI_ERROR_NONE);
  assert(root_dom != NULL);

  /* 3. OTA Snippet mounting into AoT DOM */
  /* Invalid arguments */
  rc = ui_app_load_snippet(NULL, root_dom, NULL, NULL, &snippet_dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_load_snippet(snippet_json, NULL, NULL, NULL, &snippet_dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_load_snippet(snippet_json, root_dom, NULL, NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Invalid snippet JSON */
  rc = ui_app_load_snippet("invalid json", root_dom, NULL, NULL, &snippet_dom);
  assert(rc != UI_ERROR_NONE);

  /* Snippet with unknown component type */
  rc = ui_app_load_snippet("{\"type\": \"completely_unknown_widget\"}",
                           root_dom, NULL, NULL, &snippet_dom);
  assert(rc != UI_ERROR_NONE);

  /* Valid snippet */
  rc = ui_app_load_snippet(snippet_json, root_dom, NULL, NULL, &snippet_dom);
  assert(rc == UI_ERROR_NONE);
  assert(snippet_dom != NULL);
  assert(root_dom->first_child != NULL);

  /* OOM tests */
  cfg.mode = UI_APP_LOAD_MODE_RUNTIME;
  cfg.schema_payload = app_schema;
  g_malloc_fail_countdown = 0;
  rc = ui_app_load(&cfg, mount_host, &root_dom);
  assert(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 0;
  rc = ui_app_load_snippet(snippet_json, root_dom, NULL, NULL, &snippet_dom);
  assert(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* Cleanup */
  rc = ui_dom_node_destroy(mount_host);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_app_load passed\n");
  return 0;
}
