/**
 * @file test_ui_runtime_fuzz.c
 * @brief Parser error path and fuzzing stress tests verifying zero-leak
 * guarantees.
 */

/* clang-format off */
#include "ui_runtime_schema.h"
#include "ui_runtime_builder.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

static const char *malformed_payloads[] = {
    "",
    "   ",
    "{",
    "}",
    "{\"routes\":",
    "{\"routes\": [",
    "{\"routes\": [{\"path\": \"/\"}]",
    "{\"routes\": null}",
    "{\"routes\": [1, 2, 3]}",
    "{\"routes\": [{\"path\": 123, \"root\": {}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": 456}}]}",
    "{\"app_id\": 789, \"routes\": []}",
    "{\"unknown_key\": \"fail\", \"routes\": []}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"row\", \"extra\": "
    "1}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"row\", \"props\": "
    "{\"unknown_type\": []}}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"row\", "
    "\"bindings\": {\"unknown_bind\": \"path\"}}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"row\", "
    "\"validators\": [{\"type\": \"bad_val\"}]}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"row\", \"events\": "
    "{\"bad_evt\": {}}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"row\", \"events\": "
    "{\"on_click\": {\"steps\": [{\"action\": \"invalid\"}]}}}}]}",
    "{\"routes\": [{\"path\": \"/\", \"root\": {\"type\": \"\"}}]}",
    "{ \"routes\": [ { \"path\": \"/\", \"root\": { \"type\": \"row\", "
    "\"children\": [ { \"type\": \"bad_node\", \"invalid_prop\": true } ] } } "
    "] }",
    NULL};

int main(void) {
  size_t i;
  struct ui_arena *arena = NULL;
  struct ui_component_registry *registry = NULL;
  ui_error_t rc;

  rc = ui_component_registry_get_default(&registry);
  assert(rc == UI_ERROR_NONE);

  /* Stress test parser with malformed and invalid inputs */
  for (i = 0; malformed_payloads[i] != NULL; i++) {
    struct ui_runtime_app_manifest *manifest = NULL;
    struct ui_runtime_node *node = NULL;

    rc = ui_arena_create(32768, &arena);
    assert(rc == UI_ERROR_NONE);

    /* Validate must catch all malformed or schema-violating payloads */
    rc = ui_runtime_schema_validate(malformed_payloads[i]);
    assert(rc != UI_ERROR_NONE);

    /* Parse must cleanly reject without leaking memory */
    rc = ui_runtime_schema_parse(arena, malformed_payloads[i], &manifest);
    assert(rc != UI_ERROR_NONE);
    assert(manifest == NULL);

    /* Parse node must also reject */
    rc = ui_runtime_schema_parse_node(arena, malformed_payloads[i], &node);
    assert(rc != UI_ERROR_NONE);
    assert(node == NULL);

    rc = ui_arena_destroy(arena);
    assert(rc == UI_ERROR_NONE);
  }

  /* Test unregistered component instantiation failure */
  {
    struct ui_runtime_node bad_node;
    struct ui_dom_node *out_dom = NULL;

    memset(&bad_node, 0, sizeof(bad_node));
    bad_node.type = "nonexistent_widget_type_xyz";

    rc = ui_runtime_build_tree(&bad_node, registry, NULL, NULL, NULL, NULL,
                               &out_dom);
    assert(rc == UI_ERROR_NOT_FOUND);
    assert(out_dom == NULL);
  }

  printf("test_ui_runtime_fuzz passed\n");
  return 0;
}
