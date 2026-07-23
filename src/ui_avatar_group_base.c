#include "ui_avatar_group_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_avatar_group_base {
  struct ui_component *component;
  unsigned int total_avatars;
  unsigned int max_avatars;
  struct ui_computed *data_signal;
};

/** \brief ui_error */
enum ui_error
ui_avatar_group_base_create(struct ui_avatar_group_base **out_group) {
  struct ui_avatar_group_base *group;
  enum ui_error rc;

  if (!out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  group = (struct ui_avatar_group_base *)UI_MALLOC(
      sizeof(struct ui_avatar_group_base));
  if (!group) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&group->component);
  if (rc != UI_ERROR_NONE) {
    UI_FREE(group);
    return rc;
  }

  group->total_avatars = 0;
  group->max_avatars = 0; /* 0 means no limit */

  /* Set ARIA role="group" typically */

  *out_group = group;
  return UI_ERROR_NONE;
}

void ui_avatar_group_base_destroy(struct ui_avatar_group_base *group) {
  if (!group) {
    return;
  }
  ui_component_destroy(group->component);
  UI_FREE(group);
}

/** \brief ui_error */
enum ui_error
ui_avatar_group_base_get_component(struct ui_avatar_group_base *group,
                                   struct ui_component **out_component) {
  if (!group || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = group->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

/** \brief ui_error */
enum ui_error
ui_avatar_group_base_set_max_avatars(struct ui_avatar_group_base *group,
                                     unsigned int max_avatars) {
  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  group->max_avatars = max_avatars;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
ui_avatar_group_base_get_max_avatars(struct ui_avatar_group_base *group,
                                     unsigned int *out_max_avatars) {
  if (!group || !out_max_avatars) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_max_avatars = group->max_avatars;
  return UI_ERROR_NONE;
}

/** \brief ui_avatar_group_base_get_truncation_remainder */
enum ui_error ui_avatar_group_base_get_truncation_remainder(
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

/** \brief ui_error */
enum ui_error
ui_avatar_group_base_bind_data(struct ui_avatar_group_base *widget,
                               struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
