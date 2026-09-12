/**
 * @file main.c
 * @brief Form Builder & AoT Ejection lifecycle demonstration.
 */

/* clang-format off */
#include "ui_runtime_schema.h"
#include "ui_runtime_builder.h"
#include "ui_runtime_eject.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_form_builder.h"
#include "ui_form_group.h"
#include "ui_form_control.h"
#include "ui_dom_node.h"
#include "ui_arena.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_component_registry *registry = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_dom_node *runtime_dom = NULL;
  struct ui_runtime_node *ast_root = NULL;
  struct ui_form_builder *fb = NULL;
  ui_form_group_t *user_group = NULL;
  union ui_signal_payload payload;
  char ejected_c[16384];
  char ejected_h[4096];
  ui_error_t rc;

  const char *form_schema_json =
      "{\n"
      "  \"id\": \"survey_form\",\n"
      "  \"type\": \"ui_card_base\",\n"
      "  \"props\": {\n"
      "    \"title\": \"Customer Feedback Survey\"\n"
      "  },\n"
      "  \"children\": [\n"
      "    {\n"
      "      \"id\": \"row_input\",\n"
      "      \"type\": \"row\",\n"
      "      \"children\": [\n"
      "        {\n"
      "          \"id\": \"input_email\",\n"
      "          \"type\": \"ui_input_base\",\n"
      "          \"props\": {\n"
      "            \"placeholder\": \"your.email@example.com\"\n"
      "          },\n"
      "          \"bindings\": {\n"
      "            \"bind_cva\": \"user.email\"\n"
      "          },\n"
      "          \"validators\": [\n"
      "            {\"type\": \"required\"}\n"
      "          ]\n"
      "        },\n"
      "        {\n"
      "          \"id\": \"btn_submit\",\n"
      "          \"type\": \"ui_button_base\",\n"
      "          \"props\": {\n"
      "            \"text\": \"Submit Feedback\"\n"
      "          }\n"
      "        }\n"
      "      ]\n"
      "    }\n"
      "  ]\n"
      "}";

  printf("=== Form Builder & AoT Ejection Demo ===\n");

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_get_default(&registry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_create(arena, &ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  /* Set up form group "user" with control "email" */
  rc = ui_form_builder_create(arena, &fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_start(fb, "user");
  assert(rc == UI_ERROR_NONE);

  payload.ptr_val = (void *)"visitor@site.com";
  rc = ui_form_builder_control(fb, "email", payload, UI_SIGNAL_TYPE_POINTER,
                               NULL, NULL);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_end(fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_build(fb, &user_group);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_form_group(ctx, "user", user_group);
  assert(rc == UI_ERROR_NONE);

  /* Step 1: Parse the visually designed schema */
  printf("1. Parsing UI Schema JSON...\n");
  rc = ui_runtime_schema_parse_node(arena, form_schema_json, &ast_root);
  assert(rc == UI_ERROR_NONE);
  assert(ast_root != NULL);

  /* Step 2: Render schema live using ui_runtime_build */
  printf("2. Building live DOM tree via Runtime Interpreter...\n");
  rc = ui_runtime_build_tree(ast_root, registry, ctx, app_state, NULL, NULL,
                             &runtime_dom);
  assert(rc == UI_ERROR_NONE);
  assert(runtime_dom != NULL);
  printf("   -> Runtime DOM tree successfully constructed!\n");

  /* Step 3: Trigger AoT Ejection */
  printf("3. Ejecting UI Schema to C89 AoT Source Code...\n");
  rc = ui_runtime_eject_tree_to_c_buffer(ast_root, "generated_feedback_form",
                                         ejected_c, sizeof(ejected_c),
                                         ejected_h, sizeof(ejected_h));
  assert(rc == UI_ERROR_NONE);
  printf("   -> Emitted C89 Code Size: %d bytes (header: %d bytes)\n",
         (int)strlen(ejected_c), (int)strlen(ejected_h));

  /* Step 4: Write out generated files */
  rc = ui_runtime_eject_node_to_c(ast_root, "generated_feedback_form",
                                  "generated_feedback_form.c",
                                  "generated_feedback_form.h");
  assert(rc == UI_ERROR_NONE);
  printf("   -> Wrote generated_feedback_form.c and .h\n");

  /* Clean up generated files */
  remove("generated_feedback_form.c");
  remove("generated_feedback_form.h");

  rc = ui_dom_node_destroy(runtime_dom);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_destroy(app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("=== Form Builder lifecycle completed successfully ===\n");
  return 0;
}
