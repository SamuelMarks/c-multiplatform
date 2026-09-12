/**
 * @file test_ui_runtime_builder.c
 * @brief Unit tests for ui_runtime_builder.
 */

/* clang-format off */
#include "ui_runtime_builder.h"
#include "ui_runtime_schema.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include "ui_form_builder.h"
#include "ui_form_group.h"
#include "ui_form_control.h"
#include "ui_signal.h"
#include "ui_dom_node.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

struct mock_comp_instance {
  char last_prop_key[64];
  char last_prop_val[64];
  ui_error_t (*attached_cb)(void *user_data);
  void *attached_ud;
};

static int s_mock_factory_fail = 0;
static int s_mock_set_prop_fail = 0;
static int s_mock_get_cva_fail = 0;
static int s_mock_cva_bind_fail = 0;
static int s_mock_attach_event_fail = 0;
static int s_mock_append_child_called = 0;
static int s_mock_append_child_fail = 0;
static struct mock_comp_instance *s_last_mock_inst = NULL;

static ui_error_t
mock_cva_reg_fail(void *instance,
                  ui_error_t (*callback)(union ui_signal_payload, void *),
                  void *ud) {
  (void)instance;
  (void)callback;
  (void)ud;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t mock_factory_child_with_parent(void **out_instance,
                                                 struct ui_dom_node **out_dom) {
  struct mock_comp_instance *inst;
  struct ui_dom_node *dom = NULL;
  ui_error_t rc;

  inst = (struct mock_comp_instance *)malloc(sizeof(struct mock_comp_instance));
  if (!inst) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(inst, 0, sizeof(*inst));
  s_last_mock_inst = inst;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dom);
  if (rc != UI_ERROR_NONE) {
    free(inst);
    return rc;
  }
  dom->parent = (struct ui_dom_node *)0x1234;
  *out_instance = inst;
  *out_dom = dom;
  return UI_ERROR_NONE;
}

static ui_error_t mock_factory_null_dom(void **out_instance,
                                        struct ui_dom_node **out_dom) {
  struct mock_comp_instance *inst =
      (struct mock_comp_instance *)malloc(sizeof(struct mock_comp_instance));
  if (!inst) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(inst, 0, sizeof(*inst));
  s_last_mock_inst = inst;
  *out_instance = inst;
  *out_dom = NULL;
  return UI_ERROR_NONE;
}

static ui_error_t mock_factory(void **out_instance,
                               struct ui_dom_node **out_dom) {
  struct mock_comp_instance *inst;
  struct ui_dom_node *dom = NULL;
  ui_error_t rc;

  if (s_mock_factory_fail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  inst = (struct mock_comp_instance *)malloc(sizeof(struct mock_comp_instance));
  if (!inst) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(inst, 0, sizeof(*inst));
  s_last_mock_inst = inst;

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &dom);
  if (rc != UI_ERROR_NONE) {
    free(inst);
    return rc;
  }
  *out_instance = inst;
  *out_dom = dom;
  return UI_ERROR_NONE;
}

static ui_error_t mock_set_prop(void *instance, const char *key,
                                const char *val) {
  struct mock_comp_instance *inst = (struct mock_comp_instance *)instance;
  if (s_mock_set_prop_fail) {
    return UI_ERROR_UNKNOWN;
  }
  if (inst && key && val) {
    strncpy(inst->last_prop_key, key, sizeof(inst->last_prop_key) - 1);
    inst->last_prop_key[sizeof(inst->last_prop_key) - 1] = '\0';
    strncpy(inst->last_prop_val, val, sizeof(inst->last_prop_val) - 1);
    inst->last_prop_val[sizeof(inst->last_prop_val) - 1] = '\0';
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_get_cva(void *instance,
                               struct ui_control_value_accessor *out_cva) {
  (void)instance;
  if (s_mock_get_cva_fail) {
    return UI_ERROR_UNKNOWN;
  }
  if (out_cva) {
    memset(out_cva, 0, sizeof(*out_cva));
    if (s_mock_cva_bind_fail) {
      out_cva->register_on_change = mock_cva_reg_fail;
    }
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_attach_event(void *instance, const char *event_name,
                                    ui_error_t (*callback)(void *user_data),
                                    void *user_data) {
  struct mock_comp_instance *inst = (struct mock_comp_instance *)instance;
  (void)event_name;
  if (s_mock_attach_event_fail) {
    return UI_ERROR_UNKNOWN;
  }
  if (inst) {
    inst->attached_cb = callback;
    inst->attached_ud = user_data;
  }
  return UI_ERROR_NONE;
}

static ui_error_t mock_append_child(void *instance, struct ui_dom_node *child) {
  (void)instance;
  (void)child;
  s_mock_append_child_called++;
  if (s_mock_append_child_fail) {
    return UI_ERROR_UNKNOWN;
  }
  return UI_ERROR_NONE;
}

static void run_builder_edge_and_mock_tests(void) {
  struct ui_arena *arena = NULL;
  struct ui_component_registry *custom_reg = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_component_vtable vt;
  struct ui_component_vtable vt_no_append;
  struct ui_runtime_node node;
  struct ui_runtime_node child_node;
  struct ui_runtime_prop prop;
  struct ui_runtime_binding b_cva;
  struct ui_runtime_binding b_text;
  struct ui_runtime_binding b_dis;
  struct ui_runtime_binding b_vis;
  struct ui_runtime_validator_def v_req, v_pat, v_pat_null, v_min, v_max;
  struct ui_runtime_workflow wf;
  struct ui_form_builder *fb = NULL;
  ui_form_group_t *fg = NULL;
  struct ui_signal *sig_str = NULL;
  struct ui_signal *sig_int = NULL;
  struct ui_signal *sig_true = NULL;
  struct ui_signal *sig_false = NULL;
  union ui_signal_payload p;
  struct ui_dom_node *dom = NULL;
  struct ui_dom_node *test_dom = NULL;
  int w = 0, h = 0;
  ui_error_t rc;
  extern int g_malloc_fail_countdown;

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_create(arena, &ctx);
  assert(rc == UI_ERROR_NONE);
  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  /* Setup form group and control for CVA testing */
  rc = ui_form_builder_create(arena, &fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_start(fb, "profile");
  assert(rc == UI_ERROR_NONE);
  p.ptr_val = (void *)"val";
  rc = ui_form_builder_control(fb, "field", p, UI_SIGNAL_TYPE_POINTER, NULL,
                               NULL);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_end(fb);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_build(fb, &fg);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_form_group(ctx, "profile", fg);
  assert(rc == UI_ERROR_NONE);

  /* Setup signals for text, disabled, visibility testing */
  p.ptr_val = (void *)"hello_string";
  rc = ui_signal_create(arena, p, UI_SIGNAL_TYPE_POINTER, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_str);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "sig_str", sig_str);
  assert(rc == UI_ERROR_NONE);

  p.int_val = 12345;
  p.ptr_val = NULL;
  rc = ui_signal_create(arena, p, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_int);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "sig_int", sig_int);
  assert(rc == UI_ERROR_NONE);

  p.bool_val = UI_TRUE;
  rc = ui_signal_create(arena, p, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_true);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "sig_true", sig_true);
  assert(rc == UI_ERROR_NONE);

  p.bool_val = UI_FALSE;
  rc = ui_signal_create(arena, p, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_false);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "sig_false", sig_false);
  assert(rc == UI_ERROR_NONE);

  /* Register mock widget in custom registry */
  rc = ui_component_registry_create(&custom_reg);
  assert(rc == UI_ERROR_NONE);

  memset(&vt, 0, sizeof(vt));
  vt.factory = mock_factory;
  vt.set_prop = mock_set_prop;
  vt.get_cva = mock_get_cva;
  vt.attach_event = mock_attach_event;
  vt.append_child = mock_append_child;
  rc = ui_component_registry_register(custom_reg, "mock-widget", &vt);
  assert(rc == UI_ERROR_NONE);

  memset(&vt_no_append, 0, sizeof(vt_no_append));
  vt_no_append.factory = mock_factory;
  vt_no_append.set_prop = mock_set_prop;
  rc = ui_component_registry_register(custom_reg, "mock-widget-no-append",
                                      &vt_no_append);
  assert(rc == UI_ERROR_NONE);

  {
    struct ui_component_vtable vt_child_parent;
    memset(&vt_child_parent, 0, sizeof(vt_child_parent));
    vt_child_parent.factory = mock_factory_child_with_parent;
    rc = ui_component_registry_register(custom_reg, "child-with-parent",
                                        &vt_child_parent);
    assert(rc == UI_ERROR_NONE);
  }

  /* Base node setup */
  memset(&node, 0, sizeof(node));
  node.type = "mock-widget";
  node.id = "my_mock";

  /* 1. NULL parameter checks */
  rc =
      ui_runtime_build_tree(NULL, custom_reg, ctx, app_state, NULL, NULL, &dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* 2. Passing registry == NULL (uses default registry) */
  rc = ui_runtime_build_tree(&node, NULL, ctx, app_state, NULL, NULL, &dom);
  /* "mock-widget" is not in default registry, returns NOT_FOUND */
  assert(rc == UI_ERROR_NOT_FOUND);

  /* 3. Unknown type */
  node.type = "non_existent_widget_type";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NOT_FOUND);
  node.type = "mock-widget";

  /* 4. Factory failure */
  s_mock_factory_fail = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  s_mock_factory_fail = 0;

  /* 5. Node with id == NULL */
  node.id = NULL;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.id = "my_mock";

  /* 6. Static property: success & failure */
  memset(&prop, 0, sizeof(prop));
  prop.key = "custom_key";
  prop.val = "custom_val";
  node.props = &prop;

  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  s_mock_set_prop_fail = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_set_prop_fail = 0;
  node.props = NULL;

  /* 7. Dynamic binding CVA: success, get_cva fail, resolve fail, and validators
   */
  memset(&b_cva, 0, sizeof(b_cva));
  b_cva.type = UI_RUNTIME_BINDING_CVA;
  b_cva.source_path = "profile.field";
  node.bindings = &b_cva;

  /* Setup all 4 validator types */
  memset(&v_req, 0, sizeof(v_req));
  memset(&v_pat, 0, sizeof(v_pat));
  memset(&v_pat_null, 0, sizeof(v_pat_null));
  memset(&v_min, 0, sizeof(v_min));
  memset(&v_max, 0, sizeof(v_max));

  v_req.type = UI_RUNTIME_VALIDATOR_REQUIRED;
  v_req.next = &v_pat;
  v_pat.type = UI_RUNTIME_VALIDATOR_PATTERN;
  v_pat.param = "^[0-9]+$";
  v_pat.next = &v_pat_null;
  v_pat_null.type = UI_RUNTIME_VALIDATOR_PATTERN;
  v_pat_null.param = NULL;
  v_pat_null.next = &v_min;
  v_min.type = UI_RUNTIME_VALIDATOR_MIN_LENGTH;
  v_min.int_param = 2;
  v_min.next = &v_max;
  v_max.type = UI_RUNTIME_VALIDATOR_MAX_LENGTH;
  v_max.int_param = 100;
  node.validators = &v_req;

  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.validators = NULL;

  /* get_cva fail */
  s_mock_get_cva_fail = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_get_cva_fail = 0;

  /* resolve_form_control fail */
  b_cva.source_path = "nonexistent.field";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc != UI_ERROR_NONE);
  node.bindings = NULL;

  /* 8. Text binding: string, int, missing */
  memset(&b_text, 0, sizeof(b_text));
  b_text.type = UI_RUNTIME_BINDING_TEXT;

  b_text.source_path = "sig_str";
  node.bindings = &b_text;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  b_text.source_path = "sig_int";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  b_text.source_path = "missing_signal";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc != UI_ERROR_NONE);
  node.bindings = NULL;

  /* 9. Disabled binding: true, false, missing */
  memset(&b_dis, 0, sizeof(b_dis));
  b_dis.type = UI_RUNTIME_BINDING_DISABLED;

  b_dis.source_path = "sig_true";
  node.bindings = &b_dis;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  b_dis.source_path = "sig_false";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  b_dis.source_path = "missing_signal";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc != UI_ERROR_NONE);
  node.bindings = NULL;

  /* 10. Visibility binding: true, false, missing */
  memset(&b_vis, 0, sizeof(b_vis));
  b_vis.type = UI_RUNTIME_BINDING_VISIBILITY;

  b_vis.source_path = "sig_true";
  node.bindings = &b_vis;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  b_vis.source_path = "sig_false";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  b_vis.source_path = "missing_signal";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc != UI_ERROR_NONE);
  node.bindings = NULL;

  /* 11. Workflow events: success, attach_event fail, OOM */
  memset(&wf, 0, sizeof(wf));
  wf.event_name = "on_click";
  node.workflows = &wf;

  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  assert(s_last_mock_inst != NULL);
  assert(s_last_mock_inst->attached_cb != NULL);

  /* Trigger bridge callback */
  rc = s_last_mock_inst->attached_cb(s_last_mock_inst->attached_ud);
  assert(rc == UI_ERROR_NONE);
  rc = s_last_mock_inst->attached_cb(NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  {
    void *dummy_bridge = NULL;
    void *ptr_bridge = &dummy_bridge;
    rc = s_last_mock_inst->attached_cb(ptr_bridge);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);
  }
  ui_dom_node_destroy(dom);

  /* attach_event failure */
  s_mock_attach_event_fail = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_attach_event_fail = 0;

  /* OOM allocating bridge */
  node.id = NULL;
  g_malloc_fail_countdown = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  node.id = "my_mock";
  node.workflows = NULL;

  /* Additional branch coverage tests */

  /* Component with no factory */
  {
    struct ui_component_vtable vt_no_fac;
    memset(&vt_no_fac, 0, sizeof(vt_no_fac));
    rc = ui_component_registry_register(custom_reg, "no-factory", &vt_no_fac);
    assert(rc == UI_ERROR_NONE);
    node.type = "no-factory";
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_NOT_FOUND);
    node.type = "mock-widget";
  }

  /* Component with null dom_root from factory */
  {
    struct ui_component_vtable vt_null_dom;
    memset(&vt_null_dom, 0, sizeof(vt_null_dom));
    vt_null_dom.factory = mock_factory_null_dom;
    rc = ui_component_registry_register(custom_reg, "null-dom-widget",
                                        &vt_null_dom);
    assert(rc == UI_ERROR_NONE);
    node.type = "null-dom-widget";
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_NONE);
    assert(dom == NULL);
    node.type = "mock-widget";
  }

  /* id attribute setting OOM */
  g_malloc_fail_countdown = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc != UI_ERROR_NONE);
  g_malloc_fail_countdown = -1;

  /* vtable with set_prop == NULL */
  {
    struct ui_component_vtable vt_no_prop;
    memset(&vt_no_prop, 0, sizeof(vt_no_prop));
    vt_no_prop.factory = mock_factory;
    rc = ui_component_registry_register(custom_reg, "no-prop-widget",
                                        &vt_no_prop);
    assert(rc == UI_ERROR_NONE);
    node.type = "no-prop-widget";
    node.props = &prop;
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_NONE);
    ui_dom_node_destroy(dom);
    node.props = NULL;
    node.type = "mock-widget";
  }

  /* CVA with get_cva == NULL */
  {
    node.type = "no-prop-widget";
    node.bindings = &b_cva;
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_NONE);
    ui_dom_node_destroy(dom);
    node.type = "mock-widget";
    node.bindings = NULL;
  }

  /* CVA with ctx == NULL */
  node.bindings = &b_cva;
  rc = ui_runtime_build_tree(&node, custom_reg, NULL, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.bindings = NULL;

  /* CVA bind failure */
  b_cva.source_path = "profile.field";
  s_mock_cva_bind_fail = 1;
  node.bindings = &b_cva;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_cva_bind_fail = 0;
  node.bindings = NULL;

  /* Validator failures when ctrl->arena is NULL */
  {
    ui_form_control_t *ctrl_ptr = NULL;
    struct ui_arena *saved_arena;
    rc = ui_dynamic_context_resolve_form_control(ctx, "profile.field",
                                                 &ctrl_ptr);
    assert(rc == UI_ERROR_NONE);

    saved_arena = *(struct ui_arena **)ctrl_ptr;

    node.bindings = &b_cva;

    node.validators = &v_req;
    v_req.next = NULL;
    *(struct ui_arena **)ctrl_ptr = NULL;
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    *(struct ui_arena **)ctrl_ptr = saved_arena;

    node.validators = &v_pat;
    v_pat.next = NULL;
    *(struct ui_arena **)ctrl_ptr = NULL;
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    *(struct ui_arena **)ctrl_ptr = saved_arena;

    node.validators = &v_min;
    v_min.next = NULL;
    *(struct ui_arena **)ctrl_ptr = NULL;
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    *(struct ui_arena **)ctrl_ptr = saved_arena;

    node.validators = &v_max;
    v_max.next = NULL;
    *(struct ui_arena **)ctrl_ptr = NULL;
    rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                               &dom);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    *(struct ui_arena **)ctrl_ptr = saved_arena;

    node.validators = NULL;
    node.bindings = NULL;
  }

  /* Text binding with ctx == NULL */
  node.bindings = &b_text;
  b_text.source_path = "sig_str";
  rc = ui_runtime_build_tree(&node, custom_reg, NULL, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  /* Text binding with set_prop == NULL */
  node.type = "no-prop-widget";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.type = "mock-widget";

  /* Text binding set_prop failure (both string and int) */
  s_mock_set_prop_fail = 1;
  b_text.source_path = "sig_str";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  b_text.source_path = "sig_int";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_set_prop_fail = 0;
  node.bindings = NULL;

  /* Disabled binding with ctx == NULL */
  node.bindings = &b_dis;
  b_dis.source_path = "sig_true";
  rc = ui_runtime_build_tree(&node, custom_reg, NULL, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  /* Disabled binding with set_prop == NULL */
  node.type = "no-prop-widget";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.type = "mock-widget";

  /* Disabled binding set_prop failure */
  s_mock_set_prop_fail = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_set_prop_fail = 0;
  node.bindings = NULL;

  /* Visibility binding with ctx == NULL */
  node.bindings = &b_vis;
  b_vis.source_path = "sig_false";
  rc = ui_runtime_build_tree(&node, custom_reg, NULL, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  /* Visibility binding with dom_root == NULL */
  node.type = "null-dom-widget";
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  assert(dom == NULL);
  node.type = "mock-widget";

  /* Visibility binding set_attribute failure */
  node.id = NULL;
  g_malloc_fail_countdown = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;
  node.id = "my_mock";
  node.bindings = NULL;

  /* Workflow with attach_event == NULL */
  node.type = "no-prop-widget";
  node.workflows = &wf;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.type = "mock-widget";
  node.workflows = NULL;

  /* Children: child_dom == NULL */
  memset(&child_node, 0, sizeof(child_node));
  child_node.type = "null-dom-widget";
  node.first_child = &child_node;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);
  node.first_child = NULL;

  /* Children: vtable->append_child success & failure */
  memset(&child_node, 0, sizeof(child_node));
  child_node.type = "mock-widget";
  node.type = "mock-widget";
  node.first_child = &child_node;
  s_mock_append_child_called = 0;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  assert(s_mock_append_child_called > 0);
  ui_dom_node_destroy(dom);

  s_mock_append_child_fail = 1;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_UNKNOWN);
  s_mock_append_child_fail = 0;
  node.first_child = NULL;

  /* Children: parent dom_root == NULL without append_child */
  node.type = "null-dom-widget";
  child_node.type = "mock-widget";
  node.first_child = &child_node;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  assert(dom == NULL);
  node.type = "mock-widget";
  node.first_child = NULL;

  /* Children: parent dom_root != NULL without append_child (calls
   * ui_dom_node_append_child) */
  node.type = "mock-widget-no-append";
  child_node.type = "mock-widget";
  node.first_child = &child_node;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NONE);
  ui_dom_node_destroy(dom);

  /* Children: ui_dom_node_append_child failure when child already has parent */
  memset(&child_node, 0, sizeof(child_node));
  child_node.type = "child-with-parent";
  node.first_child = &child_node;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  node.first_child = NULL;
  node.type = "mock-widget";

  /* Children: child node build error */
  child_node.type = "non_existent_child_type";
  node.first_child = &child_node;
  rc = ui_runtime_build_tree(&node, custom_reg, ctx, app_state, NULL, NULL,
                             &dom);
  assert(rc == UI_ERROR_NOT_FOUND);
  node.first_child = NULL;

  /* 13. Preview viewport edge cases */
  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &test_dom);
  assert(rc == UI_ERROR_NONE);

  /* Viewport dimension setting OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, 100, 100);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 1;
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, 100, 100);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = 3;
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, 100, 100);
  assert(rc == UI_ERROR_OUT_OF_MEMORY);
  g_malloc_fail_countdown = -1;

  rc = ui_runtime_preview_viewport_set_dimensions(NULL, 100, 100);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, 0, 100);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, -1, 100);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, 100, 0);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_preview_viewport_set_dimensions(test_dom, 100, -1);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_runtime_preview_viewport_get_dimensions(NULL, &w, &h);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_preview_viewport_get_dimensions(test_dom, NULL, &h);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_runtime_preview_viewport_get_dimensions(test_dom, &w, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Missing attributes */
  rc = ui_runtime_preview_viewport_get_dimensions(test_dom, &w, &h);
  assert(rc == UI_ERROR_NOT_FOUND);

  rc = ui_dom_node_set_attribute(test_dom, "data-viewport-width", "500");
  assert(rc == UI_ERROR_NONE);
  rc = ui_runtime_preview_viewport_get_dimensions(test_dom, &w, &h);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Non-numeric values */
  rc = ui_dom_node_set_attribute(test_dom, "data-viewport-height", "invalid");
  assert(rc == UI_ERROR_NONE);
  rc = ui_runtime_preview_viewport_get_dimensions(test_dom, &w, &h);
  assert(rc != UI_ERROR_NONE);

  rc = ui_dom_node_set_attribute(test_dom, "data-viewport-width", "invalid");
  assert(rc == UI_ERROR_NONE);
  rc = ui_runtime_preview_viewport_get_dimensions(test_dom, &w, &h);
  assert(rc != UI_ERROR_NONE);

  ui_dom_node_destroy(test_dom);

  ui_dynamic_context_destroy(ctx);
  ui_app_state_registry_destroy(app_state);
  ui_component_registry_destroy(custom_reg);
  ui_arena_destroy(arena);
}

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_component_registry *registry = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_dom_node *root_dom = NULL;
  struct ui_form_builder *fbuilder = NULL;
  ui_form_group_t *fgroup = NULL;
  ui_form_control_t *fctrl = NULL;
  struct ui_signal *sig_count = NULL;
  struct ui_signal *sig_dis = NULL;
  union ui_signal_payload payload;
  int vw = 0;
  int vh = 0;
  ui_error_t rc;

  const char *node_json =
      "{\n"
      "  \"id\": \"card_container\",\n"
      "  \"type\": \"ui_card_base\",\n"
      "  \"props\": {\n"
      "    \"title\": \"User Profile\"\n"
      "  },\n"
      "  \"children\": [\n"
      "    {\n"
      "      \"id\": \"input_email\",\n"
      "      \"type\": \"ui_input_base\",\n"
      "      \"props\": {\n"
      "        \"placeholder\": \"email@domain.com\"\n"
      "      },\n"
      "      \"bindings\": {\n"
      "        \"bind_cva\": \"user.email\"\n"
      "      },\n"
      "      \"validators\": [\n"
      "        {\"type\": \"required\"}\n"
      "      ]\n"
      "    },\n"
      "    {\n"
      "      \"id\": \"btn_action\",\n"
      "      \"type\": \"ui_button_base\",\n"
      "      \"props\": {\n"
      "        \"text\": \"Save\"\n"
      "      },\n"
      "      \"bindings\": {\n"
      "        \"bind_disabled\": \"is_saving\"\n"
      "      },\n"
      "      \"events\": {\n"
      "        \"on_click\": {\n"
      "          \"steps\": [\n"
      "            {\"stepId\": \"s1\", \"action\": \"mutate_state\", "
      "\"target\": \"counter\", \"value\": \"increment\"}\n"
      "          ]\n"
      "        }\n"
      "      }\n"
      "    }\n"
      "  ]\n"
      "}";

  struct ui_runtime_node *ast_root = NULL;

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_component_registry_get_default(&registry);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_create(arena, &ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  /* Setup form group "user" with control "email" */
  rc = ui_form_builder_create(arena, &fbuilder);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_start(fbuilder, "user");
  assert(rc == UI_ERROR_NONE);

  payload.ptr_val = (void *)"initial@test.com";
  rc = ui_form_builder_control(fbuilder, "email", payload,
                               UI_SIGNAL_TYPE_POINTER, NULL, NULL);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_group_end(fbuilder);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_builder_build(fbuilder, &fgroup);
  assert(rc == UI_ERROR_NONE);
  rc = ui_form_group_get_control(fgroup, "email", &fctrl);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_register_form_group(ctx, "user", fgroup);
  assert(rc == UI_ERROR_NONE);

  /* Setup signals */
  payload.int_val = 0;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_count);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "counter", sig_count);
  assert(rc == UI_ERROR_NONE);

  payload.bool_val = UI_FALSE;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_BOOL, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig_dis);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "is_saving", sig_dis);
  assert(rc == UI_ERROR_NONE);

  /* Parse AST from json */
  rc = ui_runtime_schema_parse_node(arena, node_json, &ast_root);
  assert(rc == UI_ERROR_NONE);
  assert(ast_root != NULL);

  /* Build DOM tree */
  rc = ui_runtime_build_tree(ast_root, registry, ctx, app_state, NULL, NULL,
                             &root_dom);
  assert(rc == UI_ERROR_NONE);
  assert(root_dom != NULL);
  assert(root_dom->first_child != NULL);

  /* Test preview viewport dimensions API */
  rc = ui_runtime_preview_viewport_set_dimensions(root_dom, 390, 844);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_preview_viewport_get_dimensions(root_dom, &vw, &vh);
  assert(rc == UI_ERROR_NONE);
  assert(vw == 390);
  assert(vh == 844);

  /* Desktop viewport */
  rc = ui_runtime_preview_viewport_set_dimensions(root_dom, 1920, 1080);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_preview_viewport_get_dimensions(root_dom, &vw, &vh);
  assert(rc == UI_ERROR_NONE);
  assert(vw == 1920);
  assert(vh == 1080);

  /* Test validators directly */
  {
    ui_validator_fn vfn = NULL;
    ui_bool_t is_valid = UI_FALSE;
    union ui_signal_payload val;
    int min_limit = 5;
    int max_limit = 10;

    /* Required validator */
    rc = ui_validators_required(&vfn);
    assert(rc == UI_ERROR_NONE && vfn != NULL);
    val.ptr_val = (void *)"hello";
    rc = vfn(NULL, val, NULL, &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_TRUE);
    val.ptr_val = (void *)"";
    rc = vfn(NULL, val, NULL, &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_FALSE);

    /* Pattern validator */
    rc = ui_validators_pattern(&vfn);
    assert(rc == UI_ERROR_NONE && vfn != NULL);
    val.ptr_val = (void *)"test@example.com";
    rc = vfn(NULL, val, (void *)"@", &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_TRUE);
    val.ptr_val = (void *)"invalid_email";
    rc = vfn(NULL, val, (void *)"@", &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_FALSE);

    /* Min length validator */
    rc = ui_validators_min_length(&vfn);
    assert(rc == UI_ERROR_NONE && vfn != NULL);
    val.ptr_val = (void *)"123456";
    rc = vfn(NULL, val, (void *)&min_limit, &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_TRUE);
    val.ptr_val = (void *)"123";
    rc = vfn(NULL, val, (void *)&min_limit, &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_FALSE);

    /* Max length validator */
    rc = ui_validators_max_length(&vfn);
    assert(rc == UI_ERROR_NONE && vfn != NULL);
    val.ptr_val = (void *)"12345";
    rc = vfn(NULL, val, (void *)&max_limit, &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_TRUE);
    val.ptr_val = (void *)"123456789012345";
    rc = vfn(NULL, val, (void *)&max_limit, &is_valid);
    assert(rc == UI_ERROR_NONE && is_valid == UI_FALSE);
  }

  /* Cleanup */
  rc = ui_dom_node_destroy(root_dom);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_destroy(app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  run_builder_edge_and_mock_tests();

  printf("test_ui_runtime_builder passed\n");
  return 0;
}
