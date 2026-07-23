/* clang-format off */
#include "ui_avatar_group_base.h"
#include "ui_avatar_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static int run_normal_tests(void) {
  struct ui_avatar_group_base *group = NULL;
  struct ui_avatar_base *avatar = NULL;
  struct ui_component *comp;
  unsigned int max_avatars;
  unsigned int remainder;
  enum ui_error rc;

  printf("Testing ui_avatar_group_base_create...\n");
  if (ui_avatar_group_base_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  rc = ui_avatar_group_base_create(&group);
  if (rc != UI_ERROR_NONE || !group)
    return 1;

  printf("Testing ui_avatar_group_base_get_component...\n");
  rc = ui_avatar_group_base_get_component(group, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return 1;

  printf("Testing default limits...\n");
  rc = ui_avatar_group_base_get_max_avatars(group, &max_avatars);
  if (rc != UI_ERROR_NONE || max_avatars != 0)
    return 1;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 0)
    return 1;

  printf("Testing set_max_avatars...\n");
  rc = ui_avatar_group_base_set_max_avatars(group, 2);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_avatar_group_base_get_max_avatars(group, &max_avatars);
  if (rc != UI_ERROR_NONE || max_avatars != 2)
    return 1;

  printf("Testing append and truncation logic...\n");
  ui_avatar_base_create(&avatar);

  /* Add 1 (Total: 1, Max: 2 -> Remainder: 0) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 0)
    return 1;

  /* Add 2 (Total: 2, Max: 2 -> Remainder: 0) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 0)
    return 1;

  /* Add 3 (Total: 3, Max: 2 -> Remainder: 1) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 1)
    return 1;

  /* Add 4 (Total: 4, Max: 2 -> Remainder: 2) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 2)
    return 1;

  if (ui_avatar_group_base_bind_data(NULL, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_bind_data(group, (struct ui_computed *)0x123) !=
      UI_ERROR_NONE)
    return 1;

  printf("Testing invalid arguments...\n");
  if (ui_avatar_group_base_get_component(NULL, &comp) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_get_component(group, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_append_avatar(NULL, avatar) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_append_avatar(group, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_set_max_avatars(NULL, 5) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_get_max_avatars(NULL, &max_avatars) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_get_max_avatars(group, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_get_truncation_remainder(NULL, &remainder) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_avatar_group_base_get_truncation_remainder(group, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_avatar_base_destroy(avatar);
  ui_avatar_group_base_destroy(group);
  ui_avatar_group_base_destroy(NULL);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_avatar_group_base *group = NULL;
  enum ui_error rc;
  int i;

  printf("Testing OOM conditions for group...\n");
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_avatar_group_base_create(&group);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return 1;
    }
    if (rc == UI_ERROR_NONE) {
      ui_avatar_group_base_destroy(group);
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  return 0;
}

int main(void) {
  printf("--- ui_avatar_group_base Tests ---\n");

  if (run_normal_tests() != 0) {
    printf("Normal tests failed.\n");
    return 1;
  }

  if (run_oom_tests() != 0) {
    printf("OOM tests failed.\n");
    return 1;
  }

  printf("All ui_avatar_group_base tests passed.\n");
  return 0;
}
