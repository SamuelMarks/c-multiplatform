/* clang-format off */
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include "ui_form_builder.h"
#include "ui_form_group.h"
#include "ui_form_control.h"
#include "ui_signal.h"
#include "ui_computed.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* clang-format on */

extern int g_malloc_fail_countdown;
extern int g_mock_strcpy_fail;

int main(void) {
  struct ui_arena *arena = NULL;
  struct ui_arena *small_arena = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_dynamic_context *heap_ctx = NULL;
  struct ui_dynamic_context *small_ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_signal *sig = NULL;
  struct ui_signal *ret_sig = NULL;
  struct ui_signal *app_sig = NULL;
  struct ui_form_builder *builder = NULL;
  ui_form_group_t *user_group = NULL;
  ui_form_control_t *email_ctrl = NULL;
  ui_form_control_t *resolved_ctrl = NULL;
  struct ui_computed *dummy_comp = (struct ui_computed *)0x5678;
  struct ui_computed *ret_comp = NULL;
  union ui_signal_payload payload;
  char long_path[256];
  ui_error_t rc;

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  /* 1. NULL checks */
  rc = ui_dynamic_context_create(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_register_signal(NULL, "sig", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_get_signal(NULL, "sig", &ret_sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_register_form_group(NULL, "grp", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_get_form_group(NULL, "grp", &user_group);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_register_computed(NULL, "comp", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_get_computed(NULL, "comp", &ret_comp);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_resolve_form_control(NULL, "path", &resolved_ctrl);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_resolve_signal(NULL, NULL, "path", &ret_sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* 2. Create context with arena and without arena */
  rc = ui_dynamic_context_create(arena, &ctx);
  assert(rc == UI_ERROR_NONE);
  assert(ctx != NULL);

  /* Additional NULL checks with valid ctx */
  rc = ui_dynamic_context_register_signal(ctx, NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_register_signal(ctx, "sig", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_get_signal(ctx, NULL, &ret_sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_get_signal(ctx, "sig", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_register_form_group(ctx, NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_register_form_group(ctx, "grp", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_get_form_group(ctx, NULL, &user_group);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_get_form_group(ctx, "grp", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_register_computed(ctx, NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_register_computed(ctx, "comp", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_get_computed(ctx, NULL, &ret_comp);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_get_computed(ctx, "comp", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_resolve_form_control(ctx, NULL, &resolved_ctrl);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_resolve_form_control(ctx, "path", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_dynamic_context_resolve_signal(ctx, NULL, NULL, &ret_sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_dynamic_context_resolve_signal(ctx, NULL, "path", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* 3. Signal Registration & Lookup */
  payload.int_val = 123;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_register_signal(ctx, "counter", sig);
  assert(rc == UI_ERROR_NONE);

  /* Non-head update: counter2 is head, counter is second */
  rc = ui_dynamic_context_register_signal(ctx, "counter2", sig);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_signal(ctx, "counter", sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_get_signal(ctx, "counter", &ret_sig);
  assert(rc == UI_ERROR_NONE);
  assert(ret_sig == sig);

  rc = ui_dynamic_context_get_signal(ctx, "not_found", &ret_sig);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* 4. Form Group Registration & Lookup */
  rc = ui_form_builder_create(arena, &builder);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_builder_group_start(builder, "user");
  assert(rc == UI_ERROR_NONE);

  payload.ptr_val = (void *)"test@example.com";
  rc = ui_form_builder_control(builder, "email", payload,
                               UI_SIGNAL_TYPE_POINTER, NULL, NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_builder_group_end(builder);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_builder_build(builder, &user_group);
  assert(rc == UI_ERROR_NONE);

  rc = ui_form_group_get_control(user_group, "email", &email_ctrl);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_register_form_group(ctx, "user", user_group);
  assert(rc == UI_ERROR_NONE);

  /* Non-head update: user2 is head, user is second */
  rc = ui_dynamic_context_register_form_group(ctx, "user2", user_group);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_form_group(ctx, "user", user_group);
  assert(rc == UI_ERROR_NONE);

  {
    ui_form_group_t *found_grp = NULL;
    rc = ui_dynamic_context_get_form_group(ctx, "user", &found_grp);
    assert(rc == UI_ERROR_NONE);
    assert(found_grp == user_group);

    rc = ui_dynamic_context_get_form_group(ctx, "missing_grp", &found_grp);
    assert(rc == UI_ERROR_NOT_FOUND);
  }

  /* 5. Form Control Resolution */
  /* Resolve with dot */
  rc = ui_dynamic_context_resolve_form_control(ctx, "user.email",
                                               &resolved_ctrl);
  assert(rc == UI_ERROR_NONE);
  assert(resolved_ctrl == email_ctrl);

  /* Path too long */
  memset(long_path, 'a', 200);
  long_path[150] = '.';
  long_path[151] = 'x';
  long_path[152] = '\0';
  rc = ui_dynamic_context_resolve_form_control(ctx, long_path, &resolved_ctrl);
  assert(rc == UI_ERROR_OUT_OF_BOUNDS);

  /* Missing group with dot */
  rc = ui_dynamic_context_resolve_form_control(ctx, "missing.email",
                                               &resolved_ctrl);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Missing control in valid group with dot */
  rc = ui_dynamic_context_resolve_form_control(ctx, "user.password",
                                               &resolved_ctrl);
  assert(rc != UI_ERROR_NONE);

  /* Resolve without dot */
  rc = ui_dynamic_context_resolve_form_control(ctx, "email", &resolved_ctrl);
  assert(rc == UI_ERROR_NONE);
  assert(resolved_ctrl == email_ctrl);

  /* Missing control without dot */
  rc = ui_dynamic_context_resolve_form_control(ctx, "missing_field",
                                               &resolved_ctrl);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Resolve without dot when no groups exist */
  {
    struct ui_dynamic_context *empty_ctx = NULL;
    rc = ui_dynamic_context_create(arena, &empty_ctx);
    assert(rc == UI_ERROR_NONE);
    rc = ui_dynamic_context_resolve_form_control(empty_ctx, "email",
                                                 &resolved_ctrl);
    assert(rc == UI_ERROR_NOT_FOUND);
  }

  /* 6. Computed Registration & Lookup */
  rc = ui_dynamic_context_register_computed(ctx, "comp_val", dummy_comp);
  assert(rc == UI_ERROR_NONE);

  /* Non-head update: comp_val2 is head, comp_val is second */
  rc = ui_dynamic_context_register_computed(ctx, "comp_val2", dummy_comp);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_computed(ctx, "comp_val", dummy_comp);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_get_computed(ctx, "comp_val", &ret_comp);
  assert(rc == UI_ERROR_NONE);
  assert(ret_comp == dummy_comp);

  rc = ui_dynamic_context_get_computed(ctx, "missing_comp", &ret_comp);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* 7. Signal Resolution with app. and local routing */
  rc = ui_app_state_registry_create(arena, &app_state);
  assert(rc == UI_ERROR_NONE);

  payload.int_val = 999;
  rc = ui_signal_create(arena, payload, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &app_sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_register_signal(app_state, "cart_total", app_sig);
  assert(rc == UI_ERROR_NONE);

  /* Resolve app.cart_total with app_state */
  rc = ui_dynamic_context_resolve_signal(ctx, app_state, "app.cart_total",
                                         &ret_sig);
  assert(rc == UI_ERROR_NONE);
  assert(ret_sig == app_sig);

  /* Resolve app.missing with app_state */
  rc = ui_dynamic_context_resolve_signal(ctx, app_state, "app.missing",
                                         &ret_sig);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Resolve app. routing with NULL app_state (uses global state) */
  {
    struct ui_app_state_registry *global_reg = NULL;
    rc = ui_app_state_registry_get_global(&global_reg);
    assert(rc == UI_ERROR_NONE);
    rc = ui_app_state_registry_register_signal(global_reg, "global_cart",
                                               app_sig);
    assert(rc == UI_ERROR_NONE);

    rc = ui_dynamic_context_resolve_signal(ctx, NULL, "app.global_cart",
                                           &ret_sig);
    assert(rc == UI_ERROR_NONE);
    assert(ret_sig == app_sig);

    /* Test failure when global app state cannot be created */
    rc = ui_app_state_registry_shutdown_global();
    assert(rc == UI_ERROR_NONE);
    g_malloc_fail_countdown = 0;
    rc = ui_dynamic_context_resolve_signal(ctx, NULL, "app.global_fail",
                                           &ret_sig);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);
  }

  /* Resolve local counter */
  rc = ui_dynamic_context_resolve_signal(ctx, app_state, "counter", &ret_sig);
  assert(rc == UI_ERROR_NONE);
  assert(ret_sig == sig);

  /* Resolve local missing */
  rc = ui_dynamic_context_resolve_signal(ctx, app_state, "missing_counter",
                                         &ret_sig);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* 8. Heap-allocated context with all entity types and destroy */
  rc = ui_dynamic_context_create(NULL, &heap_ctx);
  assert(rc == UI_ERROR_NONE);
  assert(heap_ctx != NULL);

  rc = ui_dynamic_context_register_signal(heap_ctx, "sig1", sig);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_form_group(heap_ctx, "grp1", user_group);
  assert(rc == UI_ERROR_NONE);
  rc = ui_dynamic_context_register_computed(heap_ctx, "cmp1", dummy_comp);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(heap_ctx);
  assert(rc == UI_ERROR_NONE);

  /* 9. Heap-allocated context OOM & error paths */
  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_create(NULL, &heap_ctx);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  rc = ui_dynamic_context_create(NULL, &heap_ctx);
  assert(rc == UI_ERROR_NONE);

  /* OOM on signal entry */
  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_register_signal(heap_ctx, "sig_oom", sig);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* OOM on signal strdup */
  g_malloc_fail_countdown = 1;
  rc = ui_dynamic_context_register_signal(heap_ctx, "sig_oom", sig);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* UI_STRCPY failure on signal */
  g_mock_strcpy_fail = 1;
  rc = ui_dynamic_context_register_signal(heap_ctx, "sig_strcpy", sig);
  g_mock_strcpy_fail = 0;
  assert(rc == UI_ERROR_UNKNOWN);

  /* OOM on form group entry */
  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_register_form_group(heap_ctx, "grp_oom", user_group);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* OOM on form group strdup */
  g_malloc_fail_countdown = 1;
  rc = ui_dynamic_context_register_form_group(heap_ctx, "grp_oom", user_group);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* UI_STRCPY failure on form group */
  g_mock_strcpy_fail = 1;
  rc = ui_dynamic_context_register_form_group(heap_ctx, "grp_strcpy",
                                              user_group);
  g_mock_strcpy_fail = 0;
  assert(rc == UI_ERROR_UNKNOWN);

  /* OOM on computed entry */
  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_register_computed(heap_ctx, "cmp_oom", dummy_comp);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* OOM on computed strdup */
  g_malloc_fail_countdown = 1;
  rc = ui_dynamic_context_register_computed(heap_ctx, "cmp_oom", dummy_comp);
  g_malloc_fail_countdown = -1;
  assert(rc == UI_ERROR_OUT_OF_MEMORY);

  /* UI_STRCPY failure on computed */
  g_mock_strcpy_fail = 1;
  rc = ui_dynamic_context_register_computed(heap_ctx, "cmp_strcpy", dummy_comp);
  g_mock_strcpy_fail = 0;
  assert(rc == UI_ERROR_UNKNOWN);

  rc = ui_dynamic_context_destroy(heap_ctx);
  assert(rc == UI_ERROR_NONE);

  /* 10. Arena-exhaustion paths */
  rc = ui_arena_create(16, &small_arena);
  assert(rc == UI_ERROR_NONE);

  rc = ui_dynamic_context_create(small_arena, &small_ctx);
  assert(rc == UI_ERROR_NONE);

  /* Next alloc in small arena requests new block: countdown=0 causes OOM */
  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_register_signal(small_ctx, "overflow_signal_name",
                                          sig);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_register_form_group(small_ctx, "overflow_group_name",
                                              user_group);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_register_computed(small_ctx, "overflow_computed_name",
                                            dummy_comp);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);

  rc = ui_dynamic_context_destroy(small_ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(small_arena);
  assert(rc == UI_ERROR_NONE);

  /* Context create with already exhausted arena */
  rc = ui_arena_create(16, &small_arena);
  assert(rc == UI_ERROR_NONE);
  g_malloc_fail_countdown = 0;
  rc = ui_dynamic_context_create(small_arena, &small_ctx);
  g_malloc_fail_countdown = -1;
  assert(rc != UI_ERROR_NONE);
  rc = ui_arena_destroy(small_arena);
  assert(rc == UI_ERROR_NONE);

  /* Test string alloc failure in ctx_strdup on arena */
  {
    char long_name[300];
    struct ui_arena *med_arena = NULL;
    struct ui_dynamic_context *med_ctx = NULL;
    memset(long_name, 'x', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\0';

    rc = ui_arena_create(64, &med_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_dynamic_context_create(med_arena, &med_ctx);
    assert(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 0;
    rc = ui_dynamic_context_register_signal(med_ctx, long_name, sig);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);
    rc = ui_arena_destroy(med_arena);
    assert(rc == UI_ERROR_NONE);

    rc = ui_arena_create(64, &med_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_dynamic_context_create(med_arena, &med_ctx);
    assert(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 0;
    rc = ui_dynamic_context_register_form_group(med_ctx, long_name, user_group);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);
    rc = ui_arena_destroy(med_arena);
    assert(rc == UI_ERROR_NONE);

    rc = ui_arena_create(64, &med_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_dynamic_context_create(med_arena, &med_ctx);
    assert(rc == UI_ERROR_NONE);

    g_malloc_fail_countdown = 0;
    rc = ui_dynamic_context_register_computed(med_ctx, long_name, dummy_comp);
    g_malloc_fail_countdown = -1;
    assert(rc != UI_ERROR_NONE);
    rc = ui_arena_destroy(med_arena);
    assert(rc == UI_ERROR_NONE);

    /* UI_STRCPY failure on arena context */
    g_mock_strcpy_fail = 1;
    rc = ui_dynamic_context_register_signal(ctx, "fail_arena_strcpy", sig);
    g_mock_strcpy_fail = 0;
    assert(rc == UI_ERROR_UNKNOWN);
  }

  /* Teardown */
  rc = ui_dynamic_context_destroy(ctx);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_destroy(app_state);
  assert(rc == UI_ERROR_NONE);

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_dynamic_context passed\n");
  return 0;
}
