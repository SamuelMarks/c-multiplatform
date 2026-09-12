/**
 * @file test_ui_app_state_registry.c
 * @brief Unit tests for ui_app_state_registry.
 */

/* clang-format off */
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
  struct ui_app_state_registry *registry = NULL;
  struct ui_signal *sig = NULL;
  struct ui_signal *sig2 = NULL;
  struct ui_signal *ret_sig = NULL;
  union ui_signal_payload init_val;
  union ui_signal_payload new_val;
  union ui_signal_payload get_val;
  ui_error_t rc;

  new_val.int_val = 0;
  get_val.int_val = 0;

  rc = ui_arena_create(65536, &arena);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks */
  rc = ui_app_state_registry_create(NULL, NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  rc = ui_app_state_registry_destroy(NULL);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_register_signal(NULL, "key", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Create registry with arena */
  rc = ui_app_state_registry_create(arena, &registry);
  assert(rc == UI_ERROR_NONE);
  assert(registry != NULL);

  /* Create signal */
  init_val.int_val = 42;
  rc = ui_signal_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig);
  assert(rc == UI_ERROR_NONE);

  rc = ui_signal_create(arena, init_val, UI_SIGNAL_TYPE_INT32, NULL, NULL,
                        UI_SIGNAL_MODE_SINGLE_THREADED, &sig2);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks on register_signal */
  rc = ui_app_state_registry_register_signal(registry, NULL, sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_state_registry_register_signal(registry, "key", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Register signal */
  rc = ui_app_state_registry_register_signal(registry, "cart_count", sig);
  assert(rc == UI_ERROR_NONE);

  /* Re-register signal with same key (overwrite) */
  rc = ui_app_state_registry_register_signal(registry, "cart_count", sig2);
  assert(rc == UI_ERROR_NONE);
  rc = ui_app_state_registry_register_signal(registry, "cart_count", sig);
  assert(rc == UI_ERROR_NONE);

  /* NULL checks on get_signal */
  rc = ui_app_state_registry_get_signal(NULL, "cart_count", &ret_sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_state_registry_get_signal(registry, NULL, &ret_sig);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_state_registry_get_signal(registry, "cart_count", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Lookup signal */
  rc = ui_app_state_registry_get_signal(registry, "cart_count", &ret_sig);
  assert(rc == UI_ERROR_NONE);
  assert(ret_sig == sig);

  /* NULL checks on get_value */
  rc = ui_app_state_registry_get_value(NULL, "cart_count", &get_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_state_registry_get_value(registry, NULL, &get_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_state_registry_get_value(registry, "cart_count", NULL);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Get value of nonexistent key */
  rc = ui_app_state_registry_get_value(registry, "nonexistent", &get_val);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Get value */
  rc = ui_app_state_registry_get_value(registry, "cart_count", &get_val);
  assert(rc == UI_ERROR_NONE);
  assert(get_val.int_val == 42);

  /* NULL checks on set_value */
  rc = ui_app_state_registry_set_value(NULL, "cart_count", new_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);
  rc = ui_app_state_registry_set_value(registry, NULL, new_val);
  assert(rc == UI_ERROR_INVALID_ARGUMENT);

  /* Set value of existing signal */
  new_val.int_val = 100;
  rc = ui_app_state_registry_set_value(registry, "cart_count", new_val);
  assert(rc == UI_ERROR_NONE);

  rc = ui_app_state_registry_get_value(registry, "cart_count", &get_val);
  assert(rc == UI_ERROR_NONE);
  assert(get_val.int_val == 100);

  /* Set value of non-existing signal (creates new signal and registers it) */
  new_val.int_val = 777;
  rc =
      ui_app_state_registry_set_value(registry, "auto_created_signal", new_val);
  assert(rc == UI_ERROR_NONE);
  rc = ui_app_state_registry_get_value(registry, "auto_created_signal",
                                       &get_val);
  assert(rc == UI_ERROR_NONE);
  assert(get_val.int_val == 777);

  /* Nonexistent key */
  rc = ui_app_state_registry_get_signal(registry, "nonexistent", &ret_sig);
  assert(rc == UI_ERROR_NOT_FOUND);

  /* Destroy registry allocated on arena */
  rc = ui_app_state_registry_destroy(registry);
  assert(rc == UI_ERROR_NONE);

  /* Arena allocation failure tests */
  {
    struct ui_arena *tiny_arena = NULL;
    struct ui_app_state_registry *tiny_reg = NULL;
    rc = ui_arena_create(64, &tiny_arena);
    assert(rc == UI_ERROR_NONE);
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_create(tiny_arena, &tiny_reg);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
    ui_arena_destroy(tiny_arena);
  }

  {
    struct ui_arena *small_arena = NULL;
    struct ui_app_state_registry *small_reg = NULL;
    rc = ui_arena_create(16, &small_arena);
    assert(rc == UI_ERROR_NONE);
    rc = ui_app_state_registry_create(small_arena, &small_reg);
    assert(rc == UI_ERROR_NONE);
    /* Small arena block is full; next allocation requires new block which fails
     * on OOM */
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_register_signal(small_reg, "testkey", sig);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
    ui_arena_destroy(small_arena);
  }

  {
    struct ui_arena *small_arena2 = NULL;
    struct ui_app_state_registry *small_reg2 = NULL;
    rc = ui_arena_create(48, &small_arena2);
    assert(rc == UI_ERROR_NONE);
    rc = ui_app_state_registry_create(small_arena2, &small_reg2);
    assert(rc == UI_ERROR_NONE);
    /* Small arena block fits registry and entry, but not long key; block 2
     * fails on OOM */
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_register_signal(
        small_reg2,
        "a_very_long_key_name_that_exceeds_remaining_arena_capacity_by_far_and_"
        "wide",
        sig);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;
    ui_arena_destroy(small_arena2);
  }

  /* Heap registry creation & destruction & set_value on heap */
  {
    struct ui_app_state_registry *heap_reg = NULL;
    rc = ui_app_state_registry_create(NULL, &heap_reg);
    assert(rc == UI_ERROR_NONE);
    rc = ui_app_state_registry_register_signal(heap_reg, "count", sig);
    assert(rc == UI_ERROR_NONE);
    /* auto-create on heap reg */
    new_val.int_val = 888;
    rc = ui_app_state_registry_set_value(heap_reg, "heap_auto", new_val);
    assert(rc == UI_ERROR_NONE);
    rc = ui_app_state_registry_destroy(heap_reg);
    assert(rc == UI_ERROR_NONE);
  }

  /* Global singleton checks */
  {
    struct ui_app_state_registry *global_reg = NULL;
    struct ui_app_state_registry *global_reg2 = NULL;

    /* NULL out_registry */
    rc = ui_app_state_registry_get_global(NULL);
    assert(rc == UI_ERROR_INVALID_ARGUMENT);

    /* First get_global initializes */
    rc = ui_app_state_registry_get_global(&global_reg);
    assert(rc == UI_ERROR_NONE);
    assert(global_reg != NULL);

    /* Second get_global returns cached singleton */
    rc = ui_app_state_registry_get_global(&global_reg2);
    assert(rc == UI_ERROR_NONE);
    assert(global_reg == global_reg2);

    /* Shutdown global */
    rc = ui_app_state_registry_shutdown_global();
    assert(rc == UI_ERROR_NONE);

    /* Second shutdown when already NULL */
    rc = ui_app_state_registry_shutdown_global();
    assert(rc == UI_ERROR_NONE);
  }

  /* OOM tests */
  {
    struct ui_app_state_registry *oom_reg = NULL;

    /* Fail create */
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_create(NULL, &oom_reg);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;

    /* Fail register_signal entry malloc */
    rc = ui_app_state_registry_create(NULL, &oom_reg);
    assert(rc == UI_ERROR_NONE);
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_register_signal(oom_reg, "k1", sig);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;

    /* Fail register_signal key malloc */
    g_malloc_fail_countdown = 1;
    rc = ui_app_state_registry_register_signal(oom_reg, "k2", sig);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;

    /* Fail get_global create */
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_get_global(&oom_reg);
    assert(rc == UI_ERROR_OUT_OF_MEMORY);
    g_malloc_fail_countdown = -1;

    /* Fail set_value signal creation */
    g_malloc_fail_countdown = 0;
    rc = ui_app_state_registry_set_value(oom_reg, "fail_sig", new_val);
    assert(rc != UI_ERROR_NONE);
    g_malloc_fail_countdown = -1;

    ui_app_state_registry_destroy(oom_reg);
  }

  rc = ui_arena_destroy(arena);
  assert(rc == UI_ERROR_NONE);

  printf("test_ui_app_state_registry passed\n");
  return 0;
}
