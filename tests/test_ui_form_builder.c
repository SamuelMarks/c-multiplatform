/* clang-format off */
#include "../include/ui_form_builder.h"
#include "../include/ui_error.h"
#include "../include/ui_arena.h"
#include "../include/ui_form_control.h"
#include "../include/ui_form_node.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static enum ui_error dummy_validate_fail2(struct ui_form_control *control,
                                          union ui_signal_payload value,
                                          void *user_data,
                                          ui_bool_t *out_is_valid) {
  *out_is_valid = 0;
  return UI_ERROR_NONE;
}

static int test_form_builder_oom(void) {
  struct ui_arena *tiny_arena = NULL;
  ui_form_builder_t *dummy_builder = NULL;
  union ui_signal_payload dummy;
  int i, countdown;
  dummy.int_val = 1;

  if (ui_arena_create(1, &tiny_arena) != UI_ERROR_NONE)
    return 1;

  /* Test group_start nested array push fail */
  for (countdown = 0; countdown < 5; countdown++) {
    ui_form_builder_create(tiny_arena, &dummy_builder);
    ui_form_builder_group_start(dummy_builder, "root");
    ui_form_builder_array_start(dummy_builder, "arr");

    g_malloc_fail_countdown = countdown;
    ui_form_builder_group_start(dummy_builder, "nested");
    g_malloc_fail_countdown = -1;

    ui_form_builder_destroy(dummy_builder);
  }

  /* Test group_start nested group add_node fail */
  for (countdown = 0; countdown < 5; countdown++) {
    ui_form_builder_create(tiny_arena, &dummy_builder);
    ui_form_builder_group_start(dummy_builder, "root");
    ui_form_builder_group_start(dummy_builder, "g1");

    g_malloc_fail_countdown = countdown;
    ui_form_builder_group_start(dummy_builder, "nested");
    g_malloc_fail_countdown = -1;

    ui_form_builder_destroy(dummy_builder);
  }

  /* Test array_start fail */
  for (countdown = 0; countdown < 5; countdown++) {
    ui_form_builder_create(tiny_arena, &dummy_builder);
    ui_form_builder_group_start(dummy_builder, "root");

    g_malloc_fail_countdown = countdown;
    ui_form_builder_array_start(dummy_builder, "arr");
    g_malloc_fail_countdown = -1;

    ui_form_builder_destroy(dummy_builder);
  }

  /* Test array_start nested array push fail */
  for (countdown = 0; countdown < 5; countdown++) {
    ui_form_builder_create(tiny_arena, &dummy_builder);
    ui_form_builder_group_start(dummy_builder, "root");
    ui_form_builder_array_start(dummy_builder, "arr1");

    g_malloc_fail_countdown = countdown;
    ui_form_builder_array_start(dummy_builder, "arr2");
    g_malloc_fail_countdown = -1;

    ui_form_builder_destroy(dummy_builder);
  }

  /* Test control_create fail and add_validator fail and array_push fail */
  for (countdown = 0; countdown < 10; countdown++) {
    ui_form_builder_create(tiny_arena, &dummy_builder);
    ui_form_builder_group_start(dummy_builder, "root");
    ui_form_builder_array_start(dummy_builder, "arr1");

    g_malloc_fail_countdown = countdown;
    ui_form_builder_control(dummy_builder, "c1", dummy, UI_SIGNAL_TYPE_INT32,
                            dummy_validate_fail2, NULL);
    g_malloc_fail_countdown = -1;

    ui_form_builder_destroy(dummy_builder);
  }

  /* Build without root */
  {
    ui_form_group_t *root = NULL;
    ui_form_builder_create(tiny_arena, &dummy_builder);
    ui_form_builder_build(dummy_builder, &root);
    ui_form_builder_destroy(dummy_builder);
  }

  ui_arena_destroy(tiny_arena);
  return 0;
}

static int test_form_builder(void) {
  struct ui_arena *arena = NULL;
  ui_form_builder_t *builder = NULL;
  ui_form_group_t *root = NULL;
  union ui_signal_payload dummy;
  int i;
  dummy.int_val = 1;

  if (ui_arena_create(4096, &arena) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_create(arena, &builder) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_group_start(builder, NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_control(builder, "ctrl1", dummy, UI_SIGNAL_TYPE_INT32,
                              NULL, NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_array_start(builder, "arr") != UI_ERROR_NONE)
    return 1;
  if (ui_form_builder_control(builder, NULL, dummy, UI_SIGNAL_TYPE_INT32, NULL,
                              NULL) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_group_start(builder, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_form_builder_control(builder, "ctrl2", dummy, UI_SIGNAL_TYPE_INT32,
                              NULL, NULL) != UI_ERROR_NONE)
    return 1;
  if (ui_form_builder_group_end(builder) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_array_end(builder) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_group_end(builder) != UI_ERROR_NONE)
    return 1;

  if (ui_form_builder_build(builder, &root) != UI_ERROR_NONE)
    return 1;

  ui_form_builder_destroy(builder);

  /* Null checks */
  ui_form_builder_create(NULL, NULL);
  ui_form_builder_create(arena, NULL);
  ui_form_builder_group_start(NULL, NULL);
  ui_form_builder_group_end(NULL);
  ui_form_builder_array_start(NULL, NULL);
  ui_form_builder_array_end(NULL);
  ui_form_builder_control(NULL, NULL, dummy, 0, NULL, NULL);
  ui_form_builder_build(NULL, NULL);
  ui_form_builder_destroy(NULL);

  /* Error states */
  ui_form_builder_create(arena, &builder);

  ui_form_builder_group_end(builder);                          /* error */
  ui_form_builder_array_end(builder);                          /* error */
  ui_form_builder_array_start(builder, "arr");                 /* error */
  ui_form_builder_control(builder, "c", dummy, 0, NULL, NULL); /* error */

  ui_form_builder_group_start(builder, "root");
  ui_form_builder_group_start(builder, "child");
  ui_form_builder_array_end(builder); /* wrong end */

  /* force depth error */
  for (i = 0; i < 31; i++) {
    ui_form_builder_group_start(builder, "nested");
  }
  ui_form_builder_group_start(builder, "nested");     /* 32nd should fail */
  ui_form_builder_array_start(builder, "nested_arr"); /* should fail */
  ui_form_builder_destroy(builder);

  /* Test Extra builder errors */
  ui_form_builder_create(arena, &builder);
  ui_form_builder_group_start(builder, "root");
  ui_form_builder_array_start(builder, "arr");
  ui_form_builder_group_end(builder); /* error */
  ui_form_builder_array_end(builder);
  ui_form_builder_group_end(builder);

  ui_form_builder_group_start(builder, "child");
  ui_form_builder_build(builder, NULL); /* error invalid arg */
  ui_form_builder_build(
      builder, (ui_form_group_t **)&dummy.ptr_val); /* error depth!=0 */

  ui_form_builder_destroy(builder);

  ui_form_builder_create(arena, &builder);
  ui_form_builder_build(builder,
                        (ui_form_group_t **)&dummy.ptr_val); /* root is null */

  ui_form_builder_group_start(builder, "root");
  ui_form_builder_control(builder, "ctrl", dummy, UI_SIGNAL_TYPE_INT32,
                          dummy_validate_fail2, NULL);
  ui_form_builder_group_end(builder);
  ui_form_builder_build(builder, &root);
  ui_form_builder_destroy(builder);

  {
    ui_form_node_t n;
    ui_form_group_get_node(root, "ctrl", &n);
    ui_form_control_set_value(n.node.control, dummy); /* triggers equality */
    ui_form_control_destroy(n.node.control);
  }

  g_malloc_fail_countdown = 0;
  if (ui_form_builder_create(arena, &builder) != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  ui_arena_destroy(arena);

  return 0;
}

static void force_depth_error(void);
int main(void) {
  force_depth_error();
  if (test_form_builder_oom() || test_form_builder())
    return 1;
  printf("test_ui_form_builder passed\n");
  return 0;
}

static void force_depth_error(void) {
  struct ui_arena *arena = NULL;
  ui_form_builder_t *builder = NULL;
  ui_form_group_t *root = NULL;
  ui_arena_create(4096, &arena);
  ui_form_builder_create(arena, &builder);
  ui_form_builder_group_start(builder, "root");
  ui_form_builder_group_start(builder, "child");
  ui_form_builder_build(builder, &root); /* triggers depth!=0 error */
  ui_form_builder_destroy(builder);
  ui_arena_destroy(arena);
}
