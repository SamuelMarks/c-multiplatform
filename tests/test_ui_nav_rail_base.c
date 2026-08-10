/* clang-format off */
#include "ui_nav_rail_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

struct ui_nav_rail_base {
  struct ui_component *component;
  struct ui_signal *active_index_signal;
};

struct ui_nav_rail_item_base {
  struct ui_component *component;
  int active;
};

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_nav_rail_base *rail = NULL;
  struct ui_nav_rail_item_base *item = NULL;
  struct ui_component *comp;
  int active;
  ui_error_t rc;

  printf("Testing ui_nav_rail_base_create...\n");
  if (ui_nav_rail_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_nav_rail_base_create(&rail);
  if (rc != UI_ERROR_NONE || !rail)
    return 1;

  printf("Testing ui_nav_rail_base_get_component...\n");
  rc = ui_nav_rail_base_get_component(rail, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  printf("Testing ui_nav_rail_item_base_create...\n");
  if (ui_nav_rail_item_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_nav_rail_item_base_create(&item);
  if (rc != UI_ERROR_NONE || !item) {
    printf("Failed to create nav rail item, rc = %d\n", rc);
    return 1;
  }

  printf("Testing ui_nav_rail_item_base_get_component...\n");
  rc = ui_nav_rail_item_base_get_component(item, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  printf("Testing item active state...\n");
  rc = ui_nav_rail_item_base_get_active(item, &active);
  if (rc != UI_ERROR_NONE || active != 0)
    return 1;
  rc = ui_nav_rail_item_base_set_active(item, 1);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_nav_rail_item_base_get_active(item, &active);
  if (rc != UI_ERROR_NONE || active != 1)
    return 1;

  printf("Testing append_item...\n");
  rc = ui_nav_rail_base_append_item(rail, item);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing invalid arguments...\n");
  if (ui_nav_rail_base_bind_active_index(NULL, (struct ui_signal *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_base_bind_active_index(rail, (struct ui_signal *)1) !=
      UI_ERROR_NONE)
    return 1;
  if (ui_nav_rail_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_base_get_component(rail, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_base_append_item(NULL, item) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_base_append_item(rail, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_item_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_item_base_get_component(item, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_item_base_set_active(NULL, 1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_item_base_get_active(NULL, &active) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_nav_rail_item_base_get_active(item, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_nav_rail_item_base_destroy(item);
  (void)ui_nav_rail_base_destroy(rail);
  ui_nav_rail_item_base_destroy(NULL);
  (void)ui_nav_rail_base_destroy(NULL);

  {
    struct ui_nav_rail_base *empty_rail;
    struct ui_nav_rail_item_base *empty_item;
    ui_nav_rail_base_create(&empty_rail);
    ui_component_destroy(empty_rail->component);
    empty_rail->component = NULL;
    ui_nav_rail_base_destroy(empty_rail);

    ui_nav_rail_item_base_create(&empty_item);
    ui_component_destroy(empty_item->component);
    empty_item->component = NULL;
    ui_nav_rail_item_base_destroy(empty_item);
  }

  return 0;
}

static int run_oom_tests(void) {
  struct ui_nav_rail_base *rail = NULL;
  struct ui_nav_rail_item_base *item = NULL;
  ui_error_t rc;
  int i;

  printf("Testing OOM conditions for rail...\n");
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_nav_rail_base_create(&rail);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    if (rc == UI_ERROR_NONE) {
      (void)ui_nav_rail_base_destroy(rail);
      break;
    }
  }

  printf("Testing OOM conditions for item...\n");
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_nav_rail_item_base_create(&item);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    if (rc == UI_ERROR_NONE) {
      ui_nav_rail_item_base_destroy(item);
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  printf("--- ui_nav_rail_base Tests ---\n");

  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_nav_rail_base tests passed.\n");
  return 0;
}
