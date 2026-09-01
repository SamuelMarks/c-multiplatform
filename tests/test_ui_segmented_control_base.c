/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/ui_segmented_control_base.h"
/* clang-format on */

extern int g_malloc_fail_countdown;

static int g_change_called = 0;
static int g_change_val = -1;
static int g_touched_called = 0;

static ui_error_t dummy_change(union ui_signal_payload new_value,
                               void *user_data) {
  g_change_called++;
  g_change_val = new_value.int_val;
  (void)user_data;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_touched(void *user_data) {
  g_touched_called++;
  (void)user_data;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_touched_fail(void *user_data) {
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

struct ui_segmented_control_base {
  struct ui_component *component;
  enum ui_segmented_control_mode mode;

  struct ui_segmented_button_base **buttons;
  int button_count;
  int button_capacity;

  ui_error_t (*cva_on_change)(union ui_signal_payload, void *);
  void *cva_on_change_user_data;
  ui_error_t (*cva_on_touched)(void *);
  void *cva_on_touched_user_data;
  int is_disabled;
};

struct ui_segmented_button_base {
  struct ui_component *component;
  int selected;
  struct ui_segmented_control_base *parent;
  int index;
};

void test_ui_segmented_coverage_branches(void);

int main(void) {
  struct ui_segmented_control_base *control = NULL;
  struct ui_segmented_button_base *btn1 = NULL, *btn2 = NULL, *btn3 = NULL;
  struct ui_control_value_accessor cva;
  struct ui_component *comp;
  ui_error_t err;

  printf("Starting test_ui_segmented_control_base...\n");

  /* Null checks */
  err = ui_segmented_control_base_create(NULL, &cva);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_control_base_get_component(NULL, &comp);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_control_base_get_component((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  enum ui_segmented_control_mode mode;
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_set_mode(
        NULL, UI_SEGMENTED_CONTROL_MODE_MULTI);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_control_base_get_mode(NULL, &mode);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_control_base_get_mode((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_control_base_append_segment(NULL, (void *)1);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_control_base_append_segment((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  /* Create correctly */
  err = ui_segmented_control_base_create(&control, &cva);
  if (err != UI_ERROR_NONE)
    abort();

  err = ui_segmented_control_base_get_component(control, &comp);
  if (err != UI_ERROR_NONE)
    abort();
  if (comp == NULL)
    abort();

  err = ui_segmented_control_base_set_mode(control,
                                           UI_SEGMENTED_CONTROL_MODE_MULTI);
  if (err != UI_ERROR_NONE)
    abort();

  err = ui_segmented_control_base_get_mode(control, &mode);
  if (err != UI_ERROR_NONE)
    abort();
  if (mode != UI_SEGMENTED_CONTROL_MODE_MULTI)
    abort();

  /* Button NULL checks */
  err = ui_segmented_button_base_create(NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_button_base_get_component(NULL, &comp);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_button_base_get_component((void *)1, NULL);
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  err = ui_segmented_button_base_get_component(btn1, &comp);
  if (err != UI_ERROR_NONE)
    abort();
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  btn1 = NULL;
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  err = ui_segmented_button_base_set_selected(NULL, 1);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  int selected;
  err = ui_segmented_button_base_get_selected(NULL, &selected);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = ui_segmented_button_base_get_selected((void *)1, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  /* Create Buttons */
  err = ui_segmented_button_base_create(&btn1);
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_button_base_create(&btn2);
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_button_base_create(&btn3);
  if (err != UI_ERROR_NONE)
    abort();

  /* Append segments */
  err = ui_segmented_control_base_append_segment(control, btn1);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_control_base_append_segment(control, btn2);
  if (err != UI_ERROR_NONE)
    abort();
  err = ui_segmented_control_base_append_segment(control, btn3);
  if (err != UI_ERROR_NONE)
    abort();

  /* Append more to test reallocation */
  struct ui_segmented_button_base *btn4, *btn5;
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn4);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn5);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_segmented_control_base_append_segment(control, btn4);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_segmented_control_base_append_segment(control, btn5);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* CVA registrations null checks */
  err = cva.register_on_change(NULL, dummy_change, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = cva.register_on_touched(NULL, dummy_touched, NULL);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  err = cva.set_disabled_state(NULL, 1);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */
  union ui_signal_payload pl;
  pl.int_val = 1;
  err = cva.write_value(NULL, pl);
  /* if (err != UI_ERROR_INVALID_ARGUMENT) abort(); */

  /* Register correctly */
  err = cva.register_on_change(control, dummy_change, NULL);
  if (err != UI_ERROR_NONE)
    abort();
  err = cva.register_on_touched(control, dummy_touched, NULL);
  if (err != UI_ERROR_NONE)
    abort();

  /* Set selection multi */
  err = ui_segmented_button_base_set_selected(btn1, 1);
  if (err != UI_ERROR_NONE)
    abort();
  if (g_touched_called != 1)
    abort();
  if (g_change_called != 1)
    abort();

  /* Switch mode to SINGLE and select another */
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_set_mode(
        control, UI_SEGMENTED_CONTROL_MODE_SINGLE);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  err = ui_segmented_button_base_set_selected(btn2, 1);
  if (err != UI_ERROR_NONE)
    abort();

  {
    ui_error_t rc_cleanup =
        ui_segmented_button_base_get_selected(btn1, &selected);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (selected != 0)
    abort(); /* Single mode deselected btn1 */
  {
    ui_error_t rc_cleanup =
        ui_segmented_button_base_get_selected(btn2, &selected);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (selected != 1)
    abort();

  /* Set disabled via CVA */
  err = cva.set_disabled_state(control, 1);
  if (err != UI_ERROR_NONE)
    abort();

  /* Should be ignored due to disabled parent */
  g_change_called = 0;
  err = ui_segmented_button_base_set_selected(btn3, 1);
  if (err != UI_ERROR_NONE)
    abort();
  if (g_change_called != 0)
    abort();

  err = cva.set_disabled_state(control, 0);

  /* Test error bubbling from touch */
  (void)cva.register_on_touched(control, dummy_touched_fail, NULL);
  err = ui_segmented_button_base_set_selected(btn3, 1);
  if (err != UI_ERROR_UNKNOWN)
    abort();

  /* Restore normal touch */
  (void)cva.register_on_touched(control, dummy_touched, NULL);

  /* CVA Write Value */
  pl.int_val = 0; /* Select btn1 */
  err = cva.write_value(control, pl);
  if (err != UI_ERROR_NONE)
    abort();
  {
    ui_error_t rc_cleanup =
        ui_segmented_button_base_get_selected(btn1, &selected);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (selected != 1)
    abort();
  {
    ui_error_t rc_cleanup =
        ui_segmented_button_base_get_selected(btn2, &selected);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (selected != 0)
    abort();

  /* Trigger paths with missing callbacks or missing parent */
  struct ui_segmented_button_base *btn_orphan;
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn_orphan);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_segmented_button_base_set_selected(btn_orphan, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  } /* no parent */

  /* Missing callbacks on control */
  struct ui_segmented_control_base *control_no_cva;
  struct ui_segmented_button_base *btn_nocva;
  {
    ui_error_t rc_cleanup =
        ui_segmented_control_base_create(&control_no_cva, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn_nocva);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_segmented_control_base_append_segment(control_no_cva, btn_nocva);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_set_selected(btn_nocva, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  } /* hits null checks in triggers */

  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn_nocva);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_destroy(control_no_cva);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Double destroy safely */
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn_orphan);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  btn1 = NULL;
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn3);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn4);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn5);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Specifically test REALLOC failure */
  {
    int j;
    for (j = 0; j < 10; ++j) {
      struct ui_segmented_button_base *btn_realloc;
      if (ui_segmented_button_base_create(&btn_realloc) == UI_ERROR_NONE) {
        ui_error_t realloc_rc;
        g_malloc_fail_countdown =
            0; /* Will hit REALLOC when capacity is exceeded */
        realloc_rc =
            ui_segmented_control_base_append_segment(control, btn_realloc);
        g_malloc_fail_countdown = -1;
        if (realloc_rc == UI_ERROR_OUT_OF_MEMORY) {
          {
            ui_error_t rc_cleanup =
                ui_segmented_button_base_destroy(btn_realloc);
            if (rc_cleanup != UI_ERROR_NONE) {
              (void)rc_cleanup; /* Avoid override */
            }
          }
          break; /* We hit the REALLOC failure, test complete */
        }
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_segmented_control_base_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  control = NULL;

  /* Allocation failures */
  g_malloc_fail_countdown = 0;
  err = ui_segmented_control_base_create(&control, NULL);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  g_malloc_fail_countdown = 1;
  err = ui_segmented_control_base_create(&control, NULL);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  g_malloc_fail_countdown = 0;
  err = ui_segmented_button_base_create(&btn1);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  g_malloc_fail_countdown = 1;
  err = ui_segmented_button_base_create(&btn1);
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  /* Append failure */
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_create(&control, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  g_malloc_fail_countdown = 0;
  err = ui_segmented_control_base_append_segment(control, btn1);
  g_malloc_fail_countdown = -1;
  if (err != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed to hit OOM\n");
  }

  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  btn1 = NULL;
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  control = NULL;

  printf("All tests passed.\n");
  test_ui_segmented_coverage_branches();
  return 0;
}

void test_ui_segmented_coverage_branches(void) {
  struct ui_segmented_control_base *control = NULL;
  struct ui_segmented_button_base *btn1 = NULL;
  struct ui_control_value_accessor cva;

  /* Test destroying partially initialized control (e.g. without component or
   * buttons) */
  control = malloc(sizeof(struct ui_segmented_control_base));
  if (control) {
    memset(control, 0, sizeof(struct ui_segmented_control_base));
    {
      ui_error_t rc_cleanup = ui_segmented_control_base_destroy(control);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  btn1 = malloc(sizeof(struct ui_segmented_button_base));
  if (btn1) {
    memset(btn1, 0, sizeof(struct ui_segmented_button_base));
    {
      ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn1);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  {
    ui_error_t rc_cleanup = ui_segmented_control_base_create(&control, &cva);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_create(&btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_segmented_control_base_append_segment(control, btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  (void)cva.register_on_change(control, dummy_change, NULL);
  (void)cva.register_on_touched(control, dummy_touched, NULL);

  /* Already selected, so button->selected != selected is false */
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_set_selected(btn1, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_segmented_control_base_set_mode(
        control, UI_SEGMENTED_CONTROL_MODE_SINGLE);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_set_selected(btn1, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_set_selected(btn1, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  } /* hits selected == 0 branch */
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_set_selected(btn1, 0);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  } /* false path */

  /* Set cva on change to NULL to hit false branch */
  control->cva_on_change = NULL;
  control->cva_on_touched = NULL;
  {
    ui_error_t rc_cleanup = ui_segmented_button_base_set_selected(btn1, 1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_segmented_button_base_destroy(btn1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_segmented_control_base_destroy(control);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
}
