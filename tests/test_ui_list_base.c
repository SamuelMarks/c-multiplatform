/* clang-format off */
#include "ui_list_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_list_base *list = NULL;
  struct ui_list_item_base *item = NULL;
  struct ui_component *comp;
  enum ui_list_orientation orientation;
  enum ui_error rc;

  printf("Testing ui_list_base_create...\n");
  if (ui_list_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_list_base_create(&list);
  if (rc != UI_ERROR_NONE || !list) {
    printf("Failed to create list base.\n");
    return 1;
  }

  printf("Testing list orientation...\n");
  rc = ui_list_base_get_orientation(list, &orientation);
  if (rc != UI_ERROR_NONE || orientation != UI_LIST_ORIENTATION_VERTICAL)
    return 1;
  rc = ui_list_base_set_orientation(list, UI_LIST_ORIENTATION_HORIZONTAL);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_list_base_get_orientation(list, &orientation);
  if (rc != UI_ERROR_NONE || orientation != UI_LIST_ORIENTATION_HORIZONTAL)
    return 1;

  printf("Testing ui_list_base_get_component...\n");
  rc = ui_list_base_get_component(list, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  printf("Testing ui_list_item_base_create...\n");
  if (ui_list_item_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_list_item_base_create(&item);
  if (rc != UI_ERROR_NONE || !item) {
    printf("Failed to create list item base.\n");
    return 1;
  }

  printf("Testing ui_list_item_base_get_component...\n");
  rc = ui_list_item_base_get_component(item, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  printf("Testing list append_item...\n");
  rc = ui_list_base_append_item(list, item);
  if (rc != UI_ERROR_NONE)
    return 1;

  printf("Testing invalid arguments...\n");
  if (ui_list_base_bind_data(NULL, (struct ui_computed *)1) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_bind_data(list, (struct ui_computed *)1) != UI_ERROR_NONE)
    return 1;
  if (ui_list_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_get_component(list, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_set_orientation(NULL, UI_LIST_ORIENTATION_VERTICAL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_get_orientation(NULL, &orientation) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_get_orientation(list, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_append_item(NULL, item) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_base_append_item(list, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_item_base_get_component(NULL, &comp) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_list_item_base_get_component(item, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_list_item_base_destroy(item);
  ui_list_base_destroy(list);

  ui_list_item_base_destroy(NULL);
  ui_list_base_destroy(NULL);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_list_base *list = NULL;
  struct ui_list_item_base *item = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM conditions for list...\n");
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_list_base_create(&list);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    if (rc == UI_ERROR_NONE) {
      ui_list_base_destroy(list);
      break;
    }
  }

  printf("Testing OOM conditions for list item...\n");
  for (i = 0; i < 3; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_list_item_base_create(&item);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    if (rc == UI_ERROR_NONE) {
      ui_list_item_base_destroy(item);
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  printf("--- ui_list_base Tests ---\n");

  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_list_base tests passed.\n");
  return 0;
}
