/**
 * @file ui_avatar_group_base.c
 * @brief Implementation of the avatar group component.
 */

#include "ui_avatar_group_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_avatar_group_base
 * @struct ui_avatar_group_base
 * @brief Internal representation of an avatar group component.
 */
struct ui_avatar_group_base {
  struct ui_component *component;  /**< Core UI component */
  unsigned int total_avatars;      /**< Total number of avatars added */
  unsigned int max_avatars;        /**< Maximum number of avatars to display */
  struct ui_computed *data_signal; /**< Optional data signal */
};

/*
 * @brief ui_avatar_group_base_create.
 * @param out_group Parameter out_group.
 * @return Return value.
 */
ui_error_t
ui_avatar_group_base_create(struct ui_avatar_group_base **out_group) {
  struct ui_avatar_group_base *group;
  ui_error_t rc;

  if (!out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  group = (struct ui_avatar_group_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_avatar_group_base));
  if (!group) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&group->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(group);
    return rc;
  }

  group->total_avatars = 0;
  group->max_avatars = 0; /* 0 means no limit */

  /* Set ARIA role="group" typically */

  *out_group = group;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_destroy.
 * @param group Parameter group.
 * @return Return value.
 */
ui_error_t ui_avatar_group_base_destroy(struct ui_avatar_group_base *group) {
  if (!group) {
    return UI_ERROR_NONE;
  }
  (void)ui_component_destroy(group->component);
  C_MULTIPLATFORM_FREE(group);
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_get_component.
 * @param group Parameter group.
 * @param out_component Parameter out_component.
 * @return Return value.
 */
ui_error_t
ui_avatar_group_base_get_component(struct ui_avatar_group_base *group,
                                   struct ui_component **out_component) {
  if (!group || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = group->component;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_append_avatar.
 * @param group Parameter group.
 * @param avatar Parameter avatar.
 * @return Return value.
 */
ui_error_t
ui_avatar_group_base_append_avatar(struct ui_avatar_group_base *group,
                                   struct ui_avatar_base *avatar) {
  if (!group || !avatar) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  group->total_avatars++;
  /* In reality, we'd append avatar->component to group->component->shadow_root
   * here, but we only track the count for logic validation in the base layer.
   */
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_set_max_avatars.
 * @param group Parameter group.
 * @param max_avatars Parameter max_avatars.
 * @return Return value.
 */
ui_error_t
ui_avatar_group_base_set_max_avatars(struct ui_avatar_group_base *group,
                                     unsigned int max_avatars) {
  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  group->max_avatars = max_avatars;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_get_max_avatars.
 * @param group Parameter group.
 * @param out_max_avatars Parameter out_max_avatars.
 * @return Return value.
 */
ui_error_t
ui_avatar_group_base_get_max_avatars(struct ui_avatar_group_base *group,
                                     unsigned int *out_max_avatars) {
  if (!group || !out_max_avatars) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_max_avatars = group->max_avatars;
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_get_truncation_remainder.
 * @param group Parameter group.
 * @param out_remainder Parameter out_remainder.
 * @return Return value.
 */
ui_error_t ui_avatar_group_base_get_truncation_remainder(
    struct ui_avatar_group_base *group, unsigned int *out_remainder) {
  if (!group || !out_remainder) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (group->max_avatars == 0 || group->total_avatars <= group->max_avatars) {
    *out_remainder = 0;
  } else {
    *out_remainder = group->total_avatars - group->max_avatars;
  }
  return UI_ERROR_NONE;
}

/*
 * @brief ui_avatar_group_base_bind_data.
 * @param widget Parameter widget.
 * @param signal Parameter signal.
 * @return Return value.
 */
ui_error_t ui_avatar_group_base_bind_data(struct ui_avatar_group_base *widget,
                                          struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
