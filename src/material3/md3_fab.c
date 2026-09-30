/**
 * @file md3_fab.c
 * @brief Material 3 Floating Action Button (FAB) component implementation.
 */

/* clang-format off */
#include "material3/md3_fab.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

ui_error_t md3_fab_create(struct ui_engine *engine, enum md3_fab_size size,
                          enum md3_fab_variant variant, const char *icon,
                          const char *label, struct md3_fab **out_fab) {
  struct md3_fab *fab;
  struct ui_button_base *main_btn;
  ui_error_t rc;

  if (!engine || !out_fab || (unsigned)size >= (unsigned)MD3_FAB_SIZE_COUNT ||
      (unsigned)variant >= (unsigned)MD3_FAB_VARIANT_COUNT) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  fab = (struct md3_fab *)C_MULTIPLATFORM_MALLOC(sizeof(struct md3_fab));
  if (!fab) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(fab, 0, sizeof(struct md3_fab));
  fab->size = size;
  fab->variant = variant;
  fab->elevation = 3; /* Level 3 resting elevation default */

  rc = ui_fab_base_create(&fab->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  rc = ui_button_base_create(&main_btn);
  if (rc != UI_ERROR_NONE) {
    ui_fab_base_destroy(fab->base);
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  if (icon) {
#if defined(_MSC_VER)
    strncpy_s(fab->icon_name, sizeof(fab->icon_name), icon,
              sizeof(fab->icon_name) - 1);
#else
    strncpy(fab->icon_name, icon, sizeof(fab->icon_name) - 1);
    fab->icon_name[sizeof(fab->icon_name) - 1] = '\0';
#endif
  }

  if (label) {
#if defined(_MSC_VER)
    strncpy_s(fab->label, sizeof(fab->label), label, sizeof(fab->label) - 1);
#else
    strncpy(fab->label, label, sizeof(fab->label) - 1);
    fab->label[sizeof(fab->label) - 1] = '\0';
#endif
    rc = ui_button_base_set_text(main_btn, label);
    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(main_btn);
      ui_fab_base_destroy(fab->base);
      C_MULTIPLATFORM_FREE(fab);
      return rc;
    }
  } else if (icon) {
    rc = ui_button_base_set_text(main_btn, icon);
    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(main_btn);
      ui_fab_base_destroy(fab->base);
      C_MULTIPLATFORM_FREE(fab);
      return rc;
    }
  }

  rc = ui_fab_base_set_main_button(fab->base, main_btn);
  *out_fab = fab;
  return rc;
}

ui_error_t md3_fab_destroy(struct md3_fab *fab) {
  ui_error_t rc;

  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_fab_base_destroy(fab->base);
  C_MULTIPLATFORM_FREE(fab);
  return rc;
}

ui_error_t md3_fab_set_elevation(struct md3_fab *fab, unsigned int level) {
  if (!fab || level > 5) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  fab->elevation = level;
  return UI_ERROR_NONE;
}

ui_error_t md3_fab_add_speed_dial_action(struct md3_fab *fab, const char *icon,
                                         const char *label,
                                         ui_button_on_click_t on_click,
                                         void *user_data) {
  struct ui_button_base *action_btn;
  ui_error_t rc;

  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_button_base_create(&action_btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  if (label) {
    rc = ui_button_base_set_text(action_btn, label);
    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(action_btn);
      return rc;
    }
  } else if (icon) {
    rc = ui_button_base_set_text(action_btn, icon);
    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(action_btn);
      return rc;
    }
  }

  if (on_click) {
    rc = ui_button_base_set_on_click(action_btn, on_click, user_data);
  }

  rc = ui_fab_base_add_action(fab->base, action_btn);
  if (rc != UI_ERROR_NONE) {
    ui_button_base_destroy(action_btn);
    return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_fab_toggle(struct md3_fab *fab) {
  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_fab_base_toggle(fab->base);
}

ui_error_t md3_fab_tick(struct md3_fab *fab, float dt_ms) {
  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_fab_base_tick(fab->base, dt_ms);
}

ui_error_t md3_fab_get_speed_dial_state(const struct md3_fab *fab,
                                        enum ui_fab_state *out_state) {
  if (!fab || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_fab_base_get_state(fab->base, out_state);
}

ui_error_t md3_fab_set_on_click(struct md3_fab *fab,
                                ui_button_on_click_t on_click,
                                void *user_data) {
  struct ui_button_base *main_btn;
  ui_error_t rc;

  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_fab_base_get_main_button(fab->base, &main_btn);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return ui_button_base_set_on_click(main_btn, on_click, user_data);
}

ui_error_t md3_fab_get_base(struct md3_fab *fab,
                            struct ui_fab_base **out_base) {
  if (!fab || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = fab->base;
  return UI_ERROR_NONE;
}
