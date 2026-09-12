/**
 * @file test_ui_workflow_engine.c
 * @brief Unit tests for ui_workflow_engine.
 */

/* clang-format off */
#include "ui_workflow_engine.h"
#include "ui_runtime_router.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include "ui_signal.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_runtime_router *router = NULL;
  struct ui_dom_node *mount_host = NULL;
  struct ui_runtime_node home_node;
  struct ui_workflow_context wf_ctx;
  struct ui_runtime_workflow wf;
  struct ui_runtime_workflow_step step1;
  struct ui_runtime_workflow_step step2;
  struct ui_runtime_workflow_step step3;
  struct ui_runtime_workflow_step step_extra;
  struct ui_signal *cart_sig = NULL;
  struct ui_signal *local_sig = NULL;
  union ui_signal_payload payload;
  ui_error_t rc;

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks */
  rc = ui_workflow_engine_execute(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_create(arena, &ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dom_node_create(UI_DOM_NODE_TYPE_ELEMENT, &mount_host);
  assert(rc == UI_ERROR_NONE);

  rc = ui_runtime_router_create(arena, &router);
  assert(rc == UI_ERROR_NONE);

  memset(&home_node, 0, sizeof(home_node));
  home_node.id = "screen_home";
  home_node.type = "row";
  rc = ui_runtime_router_register_route(router, "/home", &home_node);
  assert(rc == UI_ERROR_NONE);

  memset(&wf_ctx, 0, sizeof(wf_ctx));
  wf_ctx.ctx = ctx;
  wf_ctx.app_state = app_state;
  wf_ctx.router = router;
  wf_ctx.mount_host = mount_host;

  memset(&wf, 0, sizeof(wf));
  rc = ui_workflow_engine_execute(&wf, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  payload.int_val = 10;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &cart_sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_register_signal(app_state, "cart_count", cart_sig);
  assert(rc == UI_ERROR_NONE);

  /* Local context signal */
  payload.int_val = 5;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &local_sig);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "local_var", local_sig);
  assert(rc == UI_ERROR_NONE);

  /* Step 1: increment cart_count */
  memset(&step1, 0, sizeof(step1));
  step1.step_id = "step1";
  step1.action = UI_RUNTIME_ACTION_MUTATE_STATE;
  step1.target = "app.cart_count";
  step1.value = "increment";
  step1.next = &step2;

  /* Step 2: mutate state directly */
  memset(&step2, 0, sizeof(step2));
  step2.step_id = "step2";
  step2.action = UI_RUNTIME_ACTION_MUTATE_STATE;
  step2.target = "app.cart_count";
  step2.value = "50";
  step2.next = &step3;

  /* Step 3: simulated http step */
  memset(&step3, 0, sizeof(step3));
  step3.step_id = "step3";
  step3.action = UI_RUNTIME_ACTION_HTTP;
  step3.success_criteria = "$statusCode == 200";
  step3.next = NULL;

  memset(&wf, 0, sizeof(wf));
  wf.event_name = "on_click";
  wf.first_step = &step1;

  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_get_value(app_state, "cart_count", &payload);
  assert(rc == UI_ERROR_NONE);
  assert(payload.int_val == 50);

  /* Mutate local signal */
  memset(&step_extra, 0, sizeof(step_extra));
  step_extra.action = UI_RUNTIME_ACTION_MUTATE_STATE;
  step_extra.target = "local_var";
  step_extra.value = "99";
  wf.first_step = &step_extra;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* Mutate state with string value (fails integer coerce) */
  step_extra.target = "app.mode";
  step_extra.value = "dark_mode";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* Mutate state with app. target when wf_ctx.app_state is NULL */
  step_extra.target = "app.unsupported";
  step_extra.value = "10";
  wf_ctx.app_state = NULL;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc != UI_ERROR_NONE);
  wf_ctx.app_state = app_state;

  /* Mutate state with new app. signal under OOM on heap app_state */
  {
    struct ui_app_state_registry *heap_state = NULL;
    struct ui_workflow_context heap_wf_ctx;
    rc = ui_app_state_registry_create(NULL, &heap_state);
    assert(rc == UI_ERROR_NONE);
    memset(&heap_wf_ctx, 0, sizeof(heap_wf_ctx));
    heap_wf_ctx.app_state = heap_state;
    step_extra.target = "app.fail_alloc_signal";
    step_extra.value = "999";
    g_malloc_fail_countdown = 0;
    rc = ui_workflow_engine_execute(&wf, &heap_wf_ctx);
    assert(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;
    ui_app_state_registry_destroy(heap_state);
  }

  /* Mutate state with NULL target or value */
  step_extra.target = NULL;
  step_extra.value = "10";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);
  step_extra.target = "app.mode";
  step_extra.value = NULL;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* Increment nonexistent signal */
  step_extra.target = "app.nonexistent_sig";
  step_extra.value = "increment";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc != UI_ERROR_NONE);

  /* Mutate nonexistent local signal */
  step_extra.target = "nonexistent_local_sig";
  step_extra.value = "123";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc != UI_ERROR_NONE);

  /* Navigate action: success */
  step_extra.action = UI_RUNTIME_ACTION_NAVIGATE;
  step_extra.target = "/home";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* Navigate action: failure (unregistered route) */
  step_extra.target = "/nonexistent_route";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc != UI_ERROR_NONE);

  /* Navigate action: NULL router or NULL mount_host or NULL target */
  step_extra.target = "/home";
  wf_ctx.router = NULL;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);
  wf_ctx.router = router;

  wf_ctx.mount_host = NULL;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);
  wf_ctx.mount_host = mount_host;

  step_extra.target = NULL;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* HTTP action: empty criteria */
  step_extra.action = UI_RUNTIME_ACTION_HTTP;
  step_extra.success_criteria = "";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* HTTP action: NULL criteria */
  step_extra.success_criteria = NULL;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* HTTP action: criteria with no digits */
  step_extra.success_criteria = "$status == OK";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* HTTP action: failing criteria */
  step_extra.success_criteria = "$statusCode == 500";
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc != UI_ERROR_NONE);

  /* Unknown action falls through */
  step_extra.action = (enum ui_runtime_workflow_action)999;
  rc = ui_workflow_engine_execute(&wf, &wf_ctx);
  assert(rc == UI_ERROR_NONE);

  /* Cleanup */
  rc = ui_dom_node_destroy(mount_host);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_workflow_engine passed\n");
  return 0;
}
