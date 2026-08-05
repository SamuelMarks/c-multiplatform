/* clang-format off */
#include "ui_grid_list_base.h"
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_grid_list_base *gl = NULL;
  ui_error_t rc;
  int cols;
  size_t count;
  int rows;
  const struct ui_grid_list_item *item;

  printf("Testing ui_grid_list_base_create...\n");
  if (ui_grid_list_base_create(NULL, 2) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_create(&gl, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  rc = ui_grid_list_base_create(&gl, 2);
  if (rc != UI_ERROR_NONE || !gl) {
    printf("Failed to create grid list.\n");
    return 1;
  }

  printf("Testing columns getter/setter...\n");
  if (ui_grid_list_base_set_columns(NULL, 3) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_set_columns(gl, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_set_columns(gl, 3) != UI_ERROR_NONE)
    return 1;

  if (ui_grid_list_base_get_columns(NULL, &cols) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_get_columns(gl, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_grid_list_base_get_columns(gl, &cols);
  if (cols != 3)
    return 1;

  printf("Testing item additions...\n");
  if (ui_grid_list_base_add_item(NULL, 1, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_add_item(gl, 0, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_add_item(gl, 1, 0) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Add 5 items to test capacity expansion */
  ui_grid_list_base_add_item(gl, 1, 2); /* Item 0: cspan 2, rspan 1 */
  ui_grid_list_base_add_item(gl, 2, 1); /* Item 1: cspan 1, rspan 2 */
  ui_grid_list_base_add_item(gl, 1, 1); /* Item 2: cspan 1, rspan 1 */
  ui_grid_list_base_add_item(gl, 1, 1); /* Item 3: cspan 1, rspan 1 */
  ui_grid_list_base_add_item(
      gl, 1, 4); /* Item 4: cspan 4, rspan 1 (will clamp to 3) */

  if (ui_grid_list_base_get_item_count(NULL, &count) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_get_item_count(gl, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_grid_list_base_get_item_count(gl, &count);
  if (count != 5)
    return 1;

  if (ui_grid_list_base_get_item(NULL, 0, &item) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_get_item(gl, 0, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_get_item(gl, 5, &item) != UI_ERROR_OUT_OF_BOUNDS)
    return 1;

  ui_grid_list_base_get_item(gl, 0, &item);
  if (item->rowspan != 1 || item->colspan != 2)
    return 1;

  printf("Testing layout calculate...\n");
  if (ui_grid_list_base_calculate_rows(NULL, &rows) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_grid_list_base_calculate_rows(gl, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  /* Layout visualization for 3 columns:
     Item 0 (c2, r1): [0, 0] [0, 1]
     Item 1 (c1, r2): [0, 2], [1, 2]
     Item 2 (c1, r1): [1, 0]
     Item 3 (c1, r1): [1, 1]
     Item 4 (c3, r1): [2, 0] [2, 1] [2, 2]
     Total rows should be 3.
  */
  rc = ui_grid_list_base_calculate_rows(gl, &rows);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (rows != 3) {
    printf("Calculated %d rows, expected 3.\n", rows);
    return 1;
  }

  ui_grid_list_base_destroy(gl);
  ui_grid_list_base_create(&gl, 2);
  ui_grid_list_base_destroy(gl);
  ui_grid_list_base_bind_data(NULL, (struct ui_computed *)1);
  ui_grid_list_base_create(&gl, 2);
  ui_grid_list_base_bind_data(gl, (struct ui_computed *)1);
  ui_grid_list_base_destroy(gl);
  ui_grid_list_base_destroy(NULL);
  return 0;
}

static int run_oom_tests(void) {
  struct ui_grid_list_base *gl = NULL;
  ui_error_t rc;
  int rows;

  printf("Testing OOM on create...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_grid_list_base_create(&gl, 3);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  rc = ui_grid_list_base_create(&gl, 3);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing OOM on add_item...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_grid_list_base_add_item(gl, 1, 1);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    ui_grid_list_base_destroy(gl);
    ui_grid_list_base_create(&gl, 2);
    ui_grid_list_base_destroy(gl);
    ui_grid_list_base_bind_data(NULL, (struct ui_computed *)1);
    ui_grid_list_base_create(&gl, 2);
    ui_grid_list_base_bind_data(gl, (struct ui_computed *)1);
    ui_grid_list_base_destroy(gl);
    return 1;
  }

  /* Add successfully */
  ui_grid_list_base_add_item(gl, 1, 1);

  /* Trigger realloc by adding items (capacity starts at 4) */
  ui_grid_list_base_add_item(gl, 1, 1);
  ui_grid_list_base_add_item(gl, 1, 1);
  ui_grid_list_base_add_item(gl, 1, 1);

  g_malloc_fail_countdown = 0;
  rc = ui_grid_list_base_add_item(
      gl, 1, 1); /* Should fail on 5th add (capacity 4 -> 8) */
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    ui_grid_list_base_destroy(gl);
    ui_grid_list_base_create(&gl, 2);
    ui_grid_list_base_destroy(gl);
    ui_grid_list_base_bind_data(NULL, (struct ui_computed *)1);
    ui_grid_list_base_create(&gl, 2);
    ui_grid_list_base_bind_data(gl, (struct ui_computed *)1);
    ui_grid_list_base_destroy(gl);
    return 1;
  }

  printf("Testing OOM on calculate_rows...\n");
  g_malloc_fail_countdown = 0;
  rc = ui_grid_list_base_calculate_rows(gl, &rows);
  g_malloc_fail_countdown = -1;
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    ui_grid_list_base_destroy(gl);
    ui_grid_list_base_create(&gl, 2);
    ui_grid_list_base_destroy(gl);
    ui_grid_list_base_bind_data(NULL, (struct ui_computed *)1);
    ui_grid_list_base_create(&gl, 2);
    ui_grid_list_base_bind_data(gl, (struct ui_computed *)1);
    ui_grid_list_base_destroy(gl);
    return 1;
  }

  ui_grid_list_base_destroy(gl);
  ui_grid_list_base_create(&gl, 2);
  ui_grid_list_base_destroy(gl);
  ui_grid_list_base_bind_data(NULL, (struct ui_computed *)1);
  ui_grid_list_base_create(&gl, 2);
  ui_grid_list_base_bind_data(gl, (struct ui_computed *)1);
  ui_grid_list_base_destroy(gl);
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

  printf("All test_ui_grid_list_base passed.\n");
  return 0;
}
