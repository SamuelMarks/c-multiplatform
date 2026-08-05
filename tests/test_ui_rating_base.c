/* clang-format off */
#include "ui_rating_base.h"
#include <stdio.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t dummy_on_change(union ui_signal_payload new_value,
                                  void *user_data) {
  float *val = (float *)user_data;
  *val = new_value.float_val;
  return UI_ERROR_NONE;
}

static ui_error_t dummy_on_touched(void *user_data) {
  int *touched = (int *)user_data;
  *touched = 1;
  return UI_ERROR_NONE;
}

static int run_normal_tests(void) {
  struct ui_rating_base *rating = NULL;
  ui_error_t rc;
  unsigned int max_rating = 0;
  float value = -1.0f;
  float fraction = -1.0f;
  struct ui_icon_base *icon = NULL;
  struct ui_control_value_accessor cva;
  float on_change_val = 0.0f;
  int on_touched_val = 0;

  printf("Testing ui_rating_base_create...\n");
  if (ui_rating_base_create(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_rating_base_create(&rating, &cva);
  if (rc != UI_ERROR_NONE || !rating)
    return 1;

  /* CVA branch tests */
  {
    union ui_signal_payload payload;
    payload.float_val = 3.0f;
    if (cva.write_value(NULL, payload) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (cva.write_value(rating, payload) != UI_ERROR_NONE)
      return 1;

    if (cva.register_on_change(NULL, dummy_on_change, &on_change_val) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (cva.register_on_change(rating, dummy_on_change, &on_change_val) !=
        UI_ERROR_NONE)
      return 1;

    if (cva.register_on_touched(NULL, dummy_on_touched, &on_touched_val) !=
        UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (cva.register_on_touched(rating, dummy_on_touched, &on_touched_val) !=
        UI_ERROR_NONE)
      return 1;

    if (cva.set_disabled_state(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
      return 1;
    if (cva.set_disabled_state(rating, 1) != UI_ERROR_NONE)
      return 1;

    ui_rating_base_set_value(rating, 4.0f);
    if (on_change_val != 4.0f)
      return 1;
    if (on_touched_val != 1)
      return 1;

    /* Unregister to trigger the `!rating->cva_on_change` logic during set_value
     */
    cva.register_on_change(rating, NULL, NULL);
    cva.register_on_touched(rating, NULL, NULL);

    ui_rating_base_set_value(rating, 5.0f);
  }

  printf("Testing getters and setters...\n");
  if (ui_rating_base_get_max(NULL, &max_rating) != UI_ERROR_INVALID_ARGUMENT) {
    printf("1\n");
    return 1;
  }
  if (ui_rating_base_get_max(rating, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("2\n");
    return 1;
  }

  ui_rating_base_get_max(rating, &max_rating);
  if (max_rating != 5) {
    printf("3\n");
    return 1;
  }

  if (ui_rating_base_set_max(NULL, 10) != UI_ERROR_INVALID_ARGUMENT) {
    printf("4\n");
    return 1;
  }
  if (ui_rating_base_set_max(rating, 0) != UI_ERROR_INVALID_ARGUMENT) {
    printf("5\n");
    return 1;
  }

  rc = ui_rating_base_set_max(rating, 10);
  if (rc != UI_ERROR_NONE) {
    printf("6\n");
    return 1;
  }
  ui_rating_base_get_max(rating, &max_rating);
  if (max_rating != 10) {
    printf("7\n");
    return 1;
  }

  if (ui_rating_base_get_value(NULL, &value) != UI_ERROR_INVALID_ARGUMENT) {
    printf("8\n");
    return 1;
  }
  if (ui_rating_base_get_value(rating, NULL) != UI_ERROR_INVALID_ARGUMENT) {
    printf("9\n");
    return 1;
  }

  ui_rating_base_get_value(rating, &value);
  if (value != 5.0f) {
    printf("10\n");
    return 1;
  } /* We set it to 5.0f earlier */

  if (ui_rating_base_set_value(NULL, 2.5f) != UI_ERROR_INVALID_ARGUMENT) {
    printf("11\n");
    return 1;
  }
  if (ui_rating_base_set_value(rating, -1.0f) != UI_ERROR_INVALID_ARGUMENT) {
    printf("12\n");
    return 1;
  }

  rc = ui_rating_base_set_value(rating, 2.5f);
  if (rc != UI_ERROR_NONE)
    return 1;
  ui_rating_base_get_value(rating, &value);
  if (value != 2.5f)
    return 1;

  /* Clamp to max */
  ui_rating_base_set_value(rating, 20.0f);
  ui_rating_base_get_value(rating, &value);
  if (value != 10.0f)
    return 1;

  /* Lowering max clamps value */
  ui_rating_base_set_max(rating, 5);
  ui_rating_base_get_value(rating, &value);
  if (value != 5.0f)
    return 1;

  printf("Testing item fractions...\n");
  ui_rating_base_set_value(rating, 2.75f);

  if (ui_rating_base_get_item_fraction(NULL, 0, &fraction) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_rating_base_get_item_fraction(rating, 0, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_rating_base_get_item_fraction(rating, 5, &fraction) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc =
      ui_rating_base_get_item_fraction(rating, 0, &fraction); /* fully filled */
  if (rc != UI_ERROR_NONE || fraction != 1.0f)
    return 1;

  rc =
      ui_rating_base_get_item_fraction(rating, 1, &fraction); /* fully filled */
  if (rc != UI_ERROR_NONE || fraction != 1.0f)
    return 1;

  rc = ui_rating_base_get_item_fraction(rating, 2, &fraction); /* 0.75 filled */
  if (rc != UI_ERROR_NONE || fraction != 0.75f)
    return 1;

  rc = ui_rating_base_get_item_fraction(rating, 3, &fraction); /* empty */
  if (rc != UI_ERROR_NONE || fraction != 0.0f)
    return 1;

  printf("Testing icon getters...\n");
  if (ui_rating_base_get_full_icon(NULL, &icon) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_rating_base_get_full_icon(rating, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_rating_base_get_full_icon(rating, &icon);
  if (rc != UI_ERROR_NONE || !icon)
    return 1;

  if (ui_rating_base_get_half_icon(NULL, &icon) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_rating_base_get_half_icon(rating, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_rating_base_get_half_icon(rating, &icon);
  if (rc != UI_ERROR_NONE || !icon)
    return 1;

  if (ui_rating_base_get_empty_icon(NULL, &icon) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_rating_base_get_empty_icon(rating, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_rating_base_get_empty_icon(rating, &icon);
  if (rc != UI_ERROR_NONE || !icon)
    return 1;

  (void)ui_rating_base_destroy(rating);

  {
    /* Test destruction branch when only some icons were allocated due to
     * failure */
    struct ui_rating_base *rating2 = NULL;
    struct ui_rating_base *rating3 = NULL;
    g_malloc_fail_countdown = 2; /* Fails on third icon */
    ui_rating_base_create(&rating2, NULL);
    g_malloc_fail_countdown = -1;

    g_malloc_fail_countdown = 1; /* Fails on second icon */
    ui_rating_base_create(&rating3, NULL);
    g_malloc_fail_countdown = -1;

    (void)ui_rating_base_destroy(NULL);
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_rating_base *rating = NULL;
  ui_error_t rc;

  printf("Testing OOM on create... 0\n");
  g_malloc_fail_countdown = 0;
  rc = ui_rating_base_create(&rating, NULL);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at 0, rc=%d\n", rc);
    return 1;
  }

  printf("Testing OOM on create... 1\n");
  g_malloc_fail_countdown = 1;
  rc = ui_rating_base_create(&rating, NULL);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at 1, rc=%d\n", rc);
    return 1;
  }

  printf("Testing OOM on create... 2\n");
  g_malloc_fail_countdown = 2;
  rc = ui_rating_base_create(&rating, NULL);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at 2, rc=%d\n", rc);
    return 1;
  }

  printf("Testing OOM on create... 3\n");
  g_malloc_fail_countdown = 3;
  rc = ui_rating_base_create(&rating, NULL);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Failed at 3, rc=%d\n", rc);
    return 1;
  }

  return 0;
}

int main(void) {
  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All tests passed.\n");
  return 0;
}
