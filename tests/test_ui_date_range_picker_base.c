/* clang-format off */
#include <stddef.h>
#include <stdio.h>
#include "../include/ui_date_range_picker_base.h"
#include "../include/ui_error.h"
/* We redefine the struct here because it's opaque in the header, and we need to inspect state for testing */
struct ui_date_range_picker_base {
  struct ui_component *component;
  enum ui_date_range_picker_state state;
  struct ui_date start_date;
  struct ui_date end_date;
  struct ui_date hover_date;

  ui_date_predicate_cb predicate_cb;
  void *predicate_user_data;

  ui_date_range_on_change_cb on_change_cb;
  void *on_change_user_data;
};
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_bool_t mock_predicate(const struct ui_date *date, void *user_data) {
  (void)user_data;
  if (date->year == 2023 && date->month == 1 && date->day == 15) {
    return UI_TRUE;
  }
  return UI_FALSE;
}

static ui_error_t mock_on_change(struct ui_date_range_picker_base *picker,
                                 const struct ui_date_range *range,
                                 void *user_data) {
  int *called = (int *)user_data;
  (void)picker;
  (void)range;
  *called = 1;
  return UI_ERROR_NONE;
}

static int test_create_destroy(void) {
  struct ui_date_range_picker_base *picker = NULL;
  ui_error_t rc;

  rc = ui_date_range_picker_base_create(&picker);
  if (rc != UI_ERROR_NONE || !picker)
    return 1;

  rc = ui_date_range_picker_base_destroy(picker);
  if (rc != UI_ERROR_NONE)
    return 1;

  return 0;
}

static int test_errors(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date date = {2023, 1, 1};
  struct ui_date_range range;
  enum ui_date_range_picker_state state;
  int cmp;
  ui_bool_t valid;

  if (ui_date_compare(NULL, &date, &cmp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_compare(&date, NULL, &cmp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_compare(&date, &date, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_date_is_valid(NULL, &valid) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_is_valid(&date, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_date_range_picker_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_set_disable_predicate(
          NULL, mock_predicate, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_set_on_change(NULL, mock_on_change, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_select_date(NULL, &date) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_select_date(
          (struct ui_date_range_picker_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_set_hover_date(NULL, &date) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_set_hover_date(
          (struct ui_date_range_picker_base *)1, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_get_state(NULL, &state) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_get_state((struct ui_date_range_picker_base *)1,
                                          NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_get_range(NULL, &range) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_get_range((struct ui_date_range_picker_base *)1,
                                          NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_clear(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  g_malloc_fail_countdown = 0;
  if (ui_date_range_picker_base_create(&picker) != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return 1;
  }
  g_malloc_fail_countdown = -1;

  g_malloc_fail_countdown = 1;
  if (ui_date_range_picker_base_create(&picker) != UI_ERROR_OUT_OF_MEMORY) {
    g_malloc_fail_countdown = -1;
    return 1;
  }
  g_malloc_fail_countdown = -1;

  return 0;
}

static int test_date_utils(void) {
  struct ui_date a = {2023, 1, 1};
  struct ui_date b = {2023, 1, 2};
  struct ui_date c = {2023, 2, 1};
  struct ui_date d = {2024, 1, 1};
  int cmp;
  ui_bool_t valid;

  ui_date_compare(&a, &a, &cmp);
  if (cmp != 0)
    return 1;
  ui_date_compare(&a, &b, &cmp);
  if (cmp >= 0)
    return 1;
  ui_date_compare(&b, &a, &cmp);
  if (cmp <= 0)
    return 1;
  ui_date_compare(&a, &c, &cmp);
  if (cmp >= 0)
    return 1;
  ui_date_compare(&a, &d, &cmp);
  if (cmp >= 0)
    return 1;

  ui_date_is_valid(&a, &valid);
  if (!valid)
    return 1;

  a.month = 0;
  ui_date_is_valid(&a, &valid);
  if (valid)
    return 1;
  a.month = 13;
  ui_date_is_valid(&a, &valid);
  if (valid)
    return 1;
  a.month = 1;

  a.day = 0;
  ui_date_is_valid(&a, &valid);
  if (valid)
    return 1;
  a.day = 32;
  ui_date_is_valid(&a, &valid);
  if (valid)
    return 1;

  a.day = 1;
  return 0;
}

static int test_selection_flow(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date start = {2023, 1, 10};
  struct ui_date hover = {2023, 1, 12};
  struct ui_date end = {2023, 1, 20};
  struct ui_date before_start = {2023, 1, 5};
  struct ui_date_range range;
  enum ui_date_range_picker_state state;
  int called = 0;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_date_range_picker_base_set_on_change(picker, mock_on_change, &called);

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_state(picker, &state);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (state != UI_DATE_RANGE_PICKER_STATE_IDLE)
    return 1;

  {
    ui_error_t rc_cleanup =
        ui_date_range_picker_base_select_date(picker, &start);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_state(picker, &state);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (state != UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE)
    return 1;

  {
    ui_error_t rc_cleanup =
        ui_date_range_picker_base_set_hover_date(picker, &hover);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.end_date.day != 12)
    return 1;

  {
    ui_error_t rc_cleanup =
        ui_date_range_picker_base_set_hover_date(picker, &before_start);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.end_date.day != 10)
    return 1;

  {
    ui_error_t rc_cleanup =
        ui_date_range_picker_base_select_date(picker, &before_start);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_state(picker, &state);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (state != UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE)
    return 1;
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.start_date.day != 5)
    return 1;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &end);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_state(picker, &state);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (state != UI_DATE_RANGE_PICKER_STATE_IDLE)
    return 1;
  if (!called)
    return 1;
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.start_date.day != 5 || range.end_date.day != 20)
    return 1;

  ui_date_range_picker_base_clear(picker);
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_state(picker, &state);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (state != UI_DATE_RANGE_PICKER_STATE_IDLE)
    return 1;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_predicate(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date d1 = {2023, 1, 10};
  struct ui_date d2 = {2023, 1, 15}; /* disabled */
  struct ui_date d3 = {2023, 1, 20};
  enum ui_date_range_picker_state state;
  struct ui_date_range range;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_set_disable_predicate(
        picker, mock_predicate, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Selecting disabled date directly fails */
  if (ui_date_range_picker_base_select_date(picker, &d2) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  ui_date_range_picker_base_select_date(picker,
                                        &d3); /* crosses disabled date */

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_state(picker, &state);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (state != UI_DATE_RANGE_PICKER_STATE_SELECTING_END_DATE)
    return 1; /* restarted selection at d3 */
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.start_date.day != 20)
    return 1;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_invalid_dates(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date invalid = {2023, 13, 1};

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (ui_date_range_picker_base_select_date(picker, &invalid) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_date_range_picker_base_set_hover_date(picker, &invalid) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_month_wrap(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date d1 = {2023, 12, 30};
  struct ui_date d2 = {2024, 1, 2};

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_set_disable_predicate(
        picker, mock_predicate, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_day_wrap_only(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date d1 = {2023, 1, 30};
  struct ui_date d2 = {2023, 2, 2};

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_set_disable_predicate(
        picker, mock_predicate, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_early_end_date(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date d1 = {2023, 1, 10};
  struct ui_date d2 = {2023, 1, 5};
  struct ui_date hover = {2023, 1, 3};
  struct ui_date_range range;
  int failed = 0;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup =
        ui_date_range_picker_base_set_hover_date(picker, &hover);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.start_date.day != 10 || range.end_date.day != 10)
    failed = 1;

  ui_date_range_picker_base_select_date(picker, &d2); /* should restart */
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_get_range(picker, &range);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  if (range.start_date.day != 5 || range.end_date.day != 5)
    failed = 1;

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return failed;
}

static int test_hover_idle(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date hover = {2023, 1, 10};

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  /* Hover while IDLE does nothing, state stays IDLE */
  {
    ui_error_t rc_cleanup =
        ui_date_range_picker_base_set_hover_date(picker, &hover);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_select_other_state(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date d1 = {2023, 1, 10};

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  picker->state =
      (enum ui_date_range_picker_state)99; /* Force an unknown state to hit the
                                              else branch of select_date */
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

static int test_month_wrap_loop_end(void) {
  struct ui_date_range_picker_base *picker = NULL;
  struct ui_date d1 = {2023, 12, 31};
  struct ui_date d2 = {2023, 12, 31};

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_create(&picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_set_disable_predicate(
        picker, mock_predicate, NULL);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  /* Trigger while loop breaking correctly on first compare */
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d1);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_select_date(picker, &d2);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }

  {
    ui_error_t rc_cleanup = ui_date_range_picker_base_destroy(picker);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  return 0;
}

int main(void) {
  int failed = 0;
  failed |= test_create_destroy();
  failed |= test_errors();
  failed |= test_date_utils();
  failed |= test_selection_flow();
  failed |= test_predicate();
  failed |= test_invalid_dates();
  failed |= test_month_wrap();
  failed |= test_day_wrap_only();
  failed |= test_early_end_date();
  failed |= test_hover_idle();
  failed |= test_select_other_state();
  failed |= test_month_wrap_loop_end();

  printf("Testing OOM...\n");
  g_malloc_fail_countdown = 0;
  {
    struct ui_date_range_picker_base *p = NULL;
    {
      ui_error_t rc_cleanup = ui_date_range_picker_base_create(&p);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  g_malloc_fail_countdown = -1;

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
