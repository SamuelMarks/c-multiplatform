/**
 * @file main.c
 * @brief Form Builder & AoT Ejection lifecycle demonstration.
 */

/* clang-format off */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
/* clang-format on */

/**
 * @brief Main entry point demonstrating form building and ejection.
 * @return 0 on success, non-zero on failure.
 */
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
  int exit_code = 0;
  ui_error_t rc;

  const char *form_schema_json =
      "{\"id\":\"survey_form\",\"type\":\"ui_card_base\","
      "\"props\":{\"title\":\"Customer Feedback Survey\"},"
      "\"children\":[{\"id\":\"row_input\",\"type\":\"row\","
      "\"children\":[{\"id\":\"input_email\",\"type\":\"ui_input_base\","
      "\"props\":{\"placeholder\":\"your.email@example.com\"},"
      "\"bindings\":{\"bind_cva\":\"user.email\"},"
      "\"validators\":[{\"type\":\"required\"}]},"
      "{\"id\":\"btn_submit\",\"type\":\"ui_button_base\","
      "\"props\":{\"text\":\"Submit Feedback\"}}]}]}";

  printf("=== Form Builder & AoT Ejection Demo ===\n");

  rc = ui_arena_create(65536, &arena);
  if (rc != UI_ERROR_NONE) {
    return 1;
  }

  rc = ui_component_registry_get_default(&registry);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  rc = ui_dynamic_context_create(arena, &ctx);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  rc = ui_app_state_registry_create(arena, &app_state);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Set up form group "user" with control "email" */
  rc = ui_form_builder_create(arena, &fb);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  rc = ui_form_builder_group_start(fb, "user");
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  payload.ptr_val = (void *)"visitor@site.com";
  rc = ui_form_builder_control(fb, "email", payload, UI_SIGNAL_TYPE_POINTER,
                               NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  rc = ui_form_builder_group_end(fb);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  rc = ui_form_builder_build(fb, &user_group);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  rc = ui_dynamic_context_register_form_group(ctx, "user", user_group);
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }

  /* Step 1: Parse the visually designed schema */
  printf("1. Parsing UI Schema JSON...\n");
  rc = ui_runtime_schema_parse_node(arena, form_schema_json, &ast_root);
  if (rc != UI_ERROR_NONE || !ast_root) {
    exit_code = 1;
    goto cleanup;
  }

  /* Step 2: Render schema live using ui_runtime_build */
  printf("2. Building live DOM tree via Runtime Interpreter...\n");
  rc = ui_runtime_build_tree(ast_root, registry, ctx, app_state, NULL, NULL,
                             &runtime_dom);
  if (rc != UI_ERROR_NONE || !runtime_dom) {
    exit_code = 1;
    goto cleanup;
  }
  printf("   -> Runtime DOM tree successfully constructed!\n");

  /* Step 3: Trigger AoT Ejection */
  printf("3. Ejecting UI Schema to C89 AoT Source Code...\n");
  rc = ui_runtime_eject_tree_to_c_buffer(ast_root, "generated_feedback_form",
                                         ejected_c, sizeof(ejected_c),
                                         ejected_h, sizeof(ejected_h));
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  printf("   -> Emitted C89 Code Size: %d bytes (header: %d bytes)\n",
         (int)strlen(ejected_c), (int)strlen(ejected_h));

  /* Step 4: Write out generated files */
  rc = ui_runtime_eject_node_to_c(ast_root, "generated_feedback_form",
                                  "generated_feedback_form.c",
                                  "generated_feedback_form.h");
  if (rc != UI_ERROR_NONE) {
    exit_code = 1;
    goto cleanup;
  }
  printf("   -> Wrote generated_feedback_form.c and .h\n");

  /* Clean up generated files */
  remove("generated_feedback_form.c");
  remove("generated_feedback_form.h");

cleanup:
  if (runtime_dom) {
    rc = ui_dom_node_destroy(runtime_dom);
    if (rc != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }

  if (ctx) {
    rc = ui_dynamic_context_destroy(ctx);
    if (rc != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }

  if (app_state) {
    rc = ui_app_state_registry_destroy(app_state);
    if (rc != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }

  if (arena) {
    rc = ui_arena_destroy(arena);
    if (rc != UI_ERROR_NONE && exit_code == 0) {
      exit_code = 1;
    }
  }

  if (exit_code == 0) {
    printf("=== Form Builder lifecycle completed successfully ===\n");
  }
  return exit_code;
}
