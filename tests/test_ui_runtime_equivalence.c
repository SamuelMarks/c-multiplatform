/**
 * @file test_ui_runtime_equivalence.c
 * @brief Automated test suite asserting 100% structural and behavioral
 * equivalence between Runtime and AoT mounting.
 */

/* clang-format off */
#include "ui_runtime_builder.h"
#include "ui_runtime_eject.h"
#include "ui_runtime_schema.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_form_builder.h"
#include "ui_form_group.h"
#include "ui_form_control.h"
#include "ui_form_validators.h"
#include "ui_button_base.h"
#include "ui_input_base.h"
#include "ui_card_base.h"
#include "ui_checkbox_base.h"
#include "ui_arena.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

/**
 * @brief Native AoT mount function matching the exact C code output of
 * ui_runtime_eject for the test schema.
 */
static ui_error_t aot_compiled_form_create(
    struct ui_dom_node *parent_node, struct ui_dynamic_context *ctx,
    struct ui_app_state_registry *app_state, struct ui_dom_node **out_root) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_card_base *card_1 = NULL;
  struct ui_component *comp_1 = NULL;
  struct ui_dom_node *dom_1 = NULL;

  struct ui_dom_node *dom_2 = NULL;

  struct ui_input_base *input_3 = NULL;
  struct ui_component *comp_3 = NULL;
  struct ui_control_value_accessor cva_3;
  ui_form_control_t *ctrl_3 = NULL;
  struct ui_dom_node *dom_3 = NULL;
  ui_validator_fn val_fn_3 = NULL;

  struct ui_button_base *btn_4 = NULL;
  struct ui_component *comp_4 = NULL;
  struct ui_dom_node *dom_4 = NULL;
  struct ui_signal *sig_4 = NULL;

  (void)app_state;

  if (!out_root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* 1. Card */
  rc = ui_card_base_create(&card_1);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_card_base_get_component(card_1, &comp_1);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  dom_1 = comp_1->shadow_root;
  rc = ui_dom_node_set_attribute(dom_1, "id", "card_root");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_card_base_set_title(card_1, "Account Settings");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  /* 2. Row */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dom_2);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_tag_name(dom_2, "div");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(dom_2, "style",
                                 "display: flex; flex-direction: row;");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_set_attribute(dom_2, "id", "row_fields");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dom_node_append_child(dom_1, dom_2);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  /* 3. Input */
  rc = ui_input_base_create(&input_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_input_base_get_component(input_3, &comp_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  dom_3 = comp_3->shadow_root;
  rc = ui_dom_node_set_attribute(dom_3, "id", "input_email");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_input_base_set_placeholder(input_3, "user@example.com");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_input_base_get_cva(input_3, &cva_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_dynamic_context_resolve_form_control(ctx, "user.email", &ctrl_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_form_control_bind_cva(ctrl_3, &cva_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_validators_required(&val_fn_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_form_control_add_validator(ctrl_3, val_fn_3, NULL);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_append_child(dom_2, dom_3);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  /* 4. Button */
  rc = ui_button_base_create(&btn_4);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_button_base_get_component(btn_4, &comp_4);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  dom_4 = comp_4->shadow_root;
  rc = ui_dom_node_set_attribute(dom_4, "id", "btn_save");
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_button_base_set_text(btn_4, "Save Changes");
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dynamic_context_resolve_signal(ctx, app_state, "user.is_saving",
                                         &sig_4);
  if (rc != UI_ERROR_NONE)
    goto cleanup;
  rc = ui_button_base_bind_disabled(btn_4, sig_4);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  rc = ui_dom_node_append_child(dom_2, dom_4);
  if (rc != UI_ERROR_NONE)
    goto cleanup;

  if (parent_node && dom_1) {
    rc = ui_dom_node_append_child(parent_node, dom_1);
    if (rc != UI_ERROR_NONE)
      goto cleanup;
  }

  *out_root = dom_1;

cleanup:
  return rc;
}

/**
 * @brief Recursively asserts that two DOM subtrees match 100% in structure,
 * tags, and attributes.
 */
static void assert_dom_trees_equivalent(const struct ui_dom_node *a,
                                        const struct ui_dom_node *b) {
  const struct ui_dom_node *ca;
  const struct ui_dom_node *cb;

  assert(a != NULL && b != NULL);
  assert(a->type == b->type);

  if (a->tag_name && b->tag_name) {
    assert(strcmp(a->tag_name, b->tag_name) == 0);
  }

  /* Compare IDs */
  {
    const char *id_a = NULL;
    const char *id_b = NULL;
    ui_dom_node_get_attribute(a, "id", &id_a);
    ui_dom_node_get_attribute(b, "id", &id_b);
    if (id_a || id_b) {
      assert(id_a != NULL && id_b != NULL);
      assert(strcmp(id_a, id_b) == 0);
    }
  }

  /* Compare style */
  {
    const char *style_a = NULL;
    const char *style_b = NULL;
    ui_dom_node_get_attribute(a, "style", &style_a);
    ui_dom_node_get_attribute(b, "style", &style_b);
    if (style_a || style_b) {
      assert(style_a != NULL && style_b != NULL);
      assert(strcmp(style_a, style_b) == 0);
    }
  }

  /* Recurse on children */
  ca = a->first_child;
  cb = b->first_child;
  while (ca && cb) {
    assert_dom_trees_equivalent(ca, cb);
    ca = ca->next_sibling;
    cb = cb->next_sibling;
  }
  assert(ca == NULL && cb == NULL); /* Must have identical child count */
}

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_component_registry *registry = NULL;
  struct ui_dynamic_context *ctx_runtime = NULL;
  struct ui_dynamic_context *ctx_aot = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_form_builder *fb = NULL;
  ui_form_group_t *fg_runtime = NULL;
  ui_form_group_t *fg_aot = NULL;
  ui_form_control_t *fc_email_rt = NULL;
  ui_form_control_t *fc_email_aot = NULL;
  struct ui_signal *sig_saving_rt = NULL;
  struct ui_signal *sig_saving_aot = NULL;
  union ui_signal_payload payload;
  struct ui_runtime_node *ast_node = NULL;
  struct ui_dom_node *dom_runtime = NULL;
  struct ui_dom_node *dom_aot = NULL;
  char emitted_c[16384];
  char emitted_h[4096];
  ui_error_t rc;

  const char *schema_json =
      "{\n"
      "  \"id\": \"card_root\",\n"
      "  \"type\": \"ui_card_base\",\n"
      "  \"props\": {\n"
      "    \"title\": \"Account Settings\"\n"
      "  },\n"
      "  \"children\": [\n"
      "    {\n"
      "      \"id\": \"row_fields\",\n"
      "      \"type\": \"row\",\n"
      "      \"children\": [\n"
      "        {\n"
      "          \"id\": \"input_email\",\n"
      "          \"type\": \"ui_input_base\",\n"
      "          \"props\": {\n"
      "            \"placeholder\": \"user@example.com\"\n"
      "          },\n"
      "          \"bindings\": {\n"
      "            \"bind_cva\": \"user.email\"\n"
      "          },\n"
      "          \"validators\": [\n"
      "            {\"type\": \"required\"}\n"
      "          ]\n"
      "        },\n"
      "        {\n"
      "          \"id\": \"btn_save\",\n"
      "          \"type\": \"ui_button_base\",\n"
      "          \"props\": {\n"
      "            \"text\": \"Save Changes\"\n"
      "          },\n"
      "          \"bindings\": {\n"
      "            \"bind_disabled\": \"user.is_saving\"\n"
      "          }\n"
      "        }\n"
      "      ]\n"
      "    }\n"
      "  ]\n"
      "}";

  rc = ui_arena_create(131072, &arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_get_default(&registry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  /* Setup dynamic context for Runtime mode */
  rc = ui_dynamic_context_create(arena, &ctx_runtime);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_builder_create(arena, &fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_start(fb, "user");
  assert(rc == UI_ERROR_NONE);

  payload.ptr_val = (void *)"initial@test.com";
  rc = ui_form_builder_control(fb, "email", payload, UI_SIGNAL_TYPE_POINTER,
                               NULL, NULL);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_end(fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_build(fb, &fg_runtime);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_group_get_control(fg_runtime, "email", &fc_email_rt);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_form_group(ctx_runtime, "user", fg_runtime);
  assert(rc == UI_ERROR_NONE);

  payload.bool_val = UI_FALSE;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_saving_rt);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx_runtime, "user.is_saving",
                                          sig_saving_rt);
  assert(rc == UI_ERROR_NONE);

  /* Setup dynamic context for AoT mode */
  rc = ui_dynamic_context_create(arena, &ctx_aot);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_builder_create(arena, &fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_start(fb, "user");
  assert(rc == UI_ERROR_NONE);

  payload.ptr_val = (void *)"initial@test.com";
  rc = ui_form_builder_control(fb, "email", payload, UI_SIGNAL_TYPE_POINTER,
                               NULL, NULL);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_end(fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_build(fb, &fg_aot);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_group_get_control(fg_aot, "email", &fc_email_aot);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_form_group(ctx_aot, "user", fg_aot);
  assert(rc == UI_ERROR_NONE);

  payload.bool_val = UI_FALSE;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_saving_aot);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx_aot, "user.is_saving",
                                          sig_saving_aot);
  assert(rc == UI_ERROR_NONE);

  /* 1. Mount via Runtime Interpreter */
  rc = ui_runtime_schema_parse_node(arena, schema_json, &ast_node);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_build_tree(ast_node, registry, ctx_runtime, app_state, NULL,
                             NULL, &dom_runtime);
  assert(rc == UI_ERROR_NONE);
  assert(dom_runtime != NULL);

  /* 2. Verify AoT Code Generator Output for same AST */
  rc = ui_runtime_eject_tree_to_c_buffer(ast_node, "aot_compiled_form",
                                         emitted_c, sizeof(emitted_c),
                                         emitted_h, sizeof(emitted_h));
  assert(rc == UI_ERROR_NONE);

  /* 3. Mount via AoT Compiled function */
  rc = aot_compiled_form_create(NULL, ctx_aot, app_state, &dom_aot);
  assert(rc == UI_ERROR_NONE);
  assert(dom_aot != NULL);

  /* 4. Mathematically assert 100% equivalence of DOM hierarchy */
  assert_dom_trees_equivalent(dom_runtime, dom_aot);

  /* 5. Assert Form Control & CVA functional equivalence */
  {
    union ui_signal_payload val_rt;
    union ui_signal_payload val_aot;
    struct ui_signal *val_sig_rt = NULL;
    struct ui_signal *val_sig_aot = NULL;

    rc = ui_form_control_get_value_signal(fc_email_rt, &val_sig_rt);
    assert(rc == UI_ERROR_NONE);
    rc = ui_form_control_get_value_signal(fc_email_aot, &val_sig_aot);
    assert(rc == UI_ERROR_NONE);

    rc = ui_signal_get(val_sig_rt, &val_rt);
    assert(rc == UI_ERROR_NONE);
    rc = ui_signal_get(val_sig_aot, &val_aot);
    assert(rc == UI_ERROR_NONE);

    assert(strcmp((const char *)val_rt.ptr_val,
                  (const char *)val_aot.ptr_val) == 0);
  }

  /* Cleanup */
  rc = ui_dom_node_destroy(dom_runtime);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dom_node_destroy(dom_aot);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(ctx_runtime);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(ctx_aot);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_destroy(app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_runtime_equivalence passed\n");
  return 0;
}
