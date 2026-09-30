/**
 * @file md3_radio_button.c
 * @brief Material 3 Radio Button component implementation wrapping
 * ui_radio_group_base.
 */

/* clang-format off */
#include "material3/md3_radio_button.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

static ui_error_t md3_radio_cva_write_value(void *component,
                                            union ui_signal_payload value) {
  struct md3_radio_group *grp;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  grp = (struct md3_radio_group *)component;
  return md3_radio_group_set_selected_index(grp, (int)value.int_val);
}

static ui_error_t md3_radio_cva_set_disabled_state(void *component,
                                                   ui_bool_t is_disabled) {
  struct md3_radio_group *grp;

  if (!component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  grp = (struct md3_radio_group *)component;
  return md3_radio_group_set_disabled(grp, is_disabled ? 1 : 0);
}

/**
 * @brief Creates a Material 3 radio group wrapping ui_radio_group_base.
 *
 * @param engine Pointer to ui_engine instance.
 * @param out_group Pointer to receive newly created radio group.
 * @param out_cva Optional pointer to receive CVA interface.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t md3_radio_group_create(struct ui_engine *engine,
                                  struct md3_radio_group **out_group,
                                  struct ui_control_value_accessor **out_cva) {
  struct md3_radio_group *grp;
  ui_error_t rc;

  if (!engine || !out_group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  grp = (struct md3_radio_group *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_radio_group));
  if (!grp) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(grp, 0, sizeof(struct md3_radio_group));
  grp->selected_index = -1;

  rc = ui_radio_group_base_create(&grp->base, NULL);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(grp);
    return rc;
  }

  grp->cva.component = grp;
  grp->cva.write_value = md3_radio_cva_write_value;
  grp->cva.set_disabled_state = md3_radio_cva_set_disabled_state;
  grp->cva.register_on_change = NULL;
  grp->cva.register_on_touched = NULL;

  if (out_cva) {
    *out_cva = &grp->cva;
  }
  *out_group = grp;

  return UI_ERROR_NONE;
}

/**
 * @brief Destroys a Material 3 radio group.
 *
 * @param group Radio group to destroy.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_radio_group_destroy(struct md3_radio_group *group) {
  ui_error_t rc;

  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_radio_group_base_destroy(group->base);
  C_MULTIPLATFORM_FREE(group);
  return rc;
}

/**
 * @brief Sets selected index of the radio group.
 *
 * @param group The radio group.
 * @param index Zero-based selected index (-1 for none).
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_radio_group_set_selected_index(struct md3_radio_group *group,
                                              int index) {
  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  group->selected_index = index;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets selected index of the radio group.
 *
 * @param group The radio group.
 * @param out_index Pointer to receive selected index (-1 if none).
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t
md3_radio_group_get_selected_index(const struct md3_radio_group *group,
                                   int *out_index) {
  if (!group || !out_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_index = group->selected_index;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets disabled state of the radio group.
 *
 * @param group The radio group.
 * @param disabled 1 if disabled, 0 if enabled.
 * @return UI_ERROR_NONE on success, or an error code.
 */
ui_error_t md3_radio_group_set_disabled(struct md3_radio_group *group,
                                        int disabled) {
  if (!group) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  group->is_disabled = disabled ? 1 : 0;
  return UI_ERROR_NONE;
}

/**
 * @brief Retrieves underlying ui_radio_group_base handle.
 *
 * @param group The radio group.
 * @param out_base Pointer to receive ui_radio_group_base handle.
 * @return UI_ERROR_NONE on success, or UI_ERROR_INVALID_ARGUMENT.
 */
ui_error_t md3_radio_group_get_base(struct md3_radio_group *group,
                                    struct ui_radio_group_base **out_base) {
  if (!group || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_base = group->base;
  return UI_ERROR_NONE;
}
