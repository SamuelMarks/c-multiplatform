/* clang-format off */
#include "ui_avatar_group_base.h"
#include "ui_avatar_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

extern int g_malloc_fail_countdown;

static ui_error_t run_normal_tests(void) {
  struct ui_avatar_group_base *group = NULL;
  struct ui_avatar_base *avatar = NULL;
  struct ui_component *comp;
  unsigned int max_avatars;
  unsigned int remainder;
  ui_error_t rc = UI_ERROR_NONE;

  printf("Testing ui_avatar_group_base_create...\n");
  rc = ui_avatar_group_base_create(NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_create(&group);
  if (rc != UI_ERROR_NONE || !group)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing ui_avatar_group_base_get_component...\n");
  rc = ui_avatar_group_base_get_component(group, &comp);
  if (rc != UI_ERROR_NONE || !comp)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing default limits...\n");
  rc = ui_avatar_group_base_get_max_avatars(group, &max_avatars);
  if (rc != UI_ERROR_NONE || max_avatars != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing set_max_avatars...\n");
  rc = ui_avatar_group_base_set_max_avatars(group, 2);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_max_avatars(group, &max_avatars);
  if (rc != UI_ERROR_NONE || max_avatars != 2)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing append and truncation logic...\n");
  rc = ui_avatar_base_create(&avatar);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Add 1 (Total: 1, Max: 2 -> Remainder: 0) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Add 2 (Total: 2, Max: 2 -> Remainder: 0) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 0)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Add 3 (Total: 3, Max: 2 -> Remainder: 1) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 1)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  /* Add 4 (Total: 4, Max: 2 -> Remainder: 2) */
  rc = ui_avatar_group_base_append_avatar(group, avatar);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(group, &remainder);
  if (rc != UI_ERROR_NONE || remainder != 2)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_avatar_group_base_bind_data(NULL, NULL);

  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_bind_data(group, (struct ui_computed *)0x123);
  if (rc != UI_ERROR_NONE)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  printf("Testing invalid arguments...\n");
  rc = ui_avatar_group_base_get_component(NULL, &comp);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_component(group, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_append_avatar(NULL, avatar);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_append_avatar(group, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_set_max_avatars(NULL, 5);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_max_avatars(NULL, &max_avatars);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_max_avatars(group, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(NULL, &remainder);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  rc = ui_avatar_group_base_get_truncation_remainder(group, NULL);
  if (rc != UI_ERROR_INVALID_ARGUMENT)
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;

  rc = ui_avatar_base_destroy(avatar);

  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_avatar_group_base_destroy(group);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_avatar_group_base_destroy(NULL);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_avatar_group_base *group = NULL;
  ui_error_t rc = UI_ERROR_NONE;
  int i;

  printf("Testing OOM conditions for group...\n");
  for (i = 0; i < 10; i++) {
    g_malloc_fail_countdown = i;
    rc = ui_avatar_group_base_create(&group);
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      g_malloc_fail_countdown = -1;
      return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
    }
    if (rc == UI_ERROR_NONE) {
      rc = ui_avatar_group_base_destroy(group);
      if (rc != UI_ERROR_NONE)
        return rc;
      break;
    }
  }

  g_malloc_fail_countdown = -1;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
  return UI_ERROR_NONE;
}

int main(void) {
  ui_error_t rc = UI_ERROR_NONE;
  printf("--- ui_avatar_group_base Tests ---\n");

  rc = run_normal_tests();
  if (rc != UI_ERROR_NONE) {
    printf("Normal tests failed.\n");
    return 1;
  }

  rc = run_oom_tests();
  if (rc != UI_ERROR_NONE) {
    printf("OOM tests failed.\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  printf("All ui_avatar_group_base tests passed.\n");
  return 0;
}
