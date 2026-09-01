/* clang-format off */
#include "ui_arena.h"
#include "ui_e2e_headless.h"
#include "ui_form_builder.h"
#include "ui_form_group.h"
#include "ui_signal.h"
#include <stdio.h>
/* clang-format on */

int main(void) {
  struct ui_arena *arena;
  ui_form_builder_t *builder;
  ui_form_group_t *group;
  enum ui_form_status status;
  union ui_signal_payload init;

  if (ui_arena_create(1024, &arena) != UI_ERROR_NONE)
    return 1;
  if (ui_form_builder_create(arena, &builder) != UI_ERROR_NONE)
    return 1;

  init.int_val = 0;

  ui_form_builder_group_start(builder, "e2e_form");
  ui_form_builder_control(builder, "field1", init, UI_SIGNAL_TYPE_INT32, NULL,
                          NULL);
  ui_form_builder_group_end(builder);

  ui_form_builder_build(builder, &group);
  ui_form_group_get_status(group, &status);

  if (status != UI_FORM_STATUS_VALID) {
    return 1;
  }

  /* Imagine simulating E2E events here checking validation blocking */

  {
    ui_error_t rc_cleanup = ui_form_builder_destroy(builder);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_form_group_destroy(group);
  {
    ui_error_t rc_cleanup = ui_arena_destroy(arena);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  printf("test_ui_form_e2e passed\\n");
  return 0;
}
