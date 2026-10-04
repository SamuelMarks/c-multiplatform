/**
 * @file md2_fab.c
 * @brief Implementation of Material Design 2 FAB component.
 */

/* clang-format off */
#include "material2/md2_fab.h"
#include "ui_internal_mem.h"
#include "ui_component.h"
#include "ui_button_base.h"
#include <string.h>
/* clang-format on */

struct md2_fab {
  struct ui_component *component;
  struct ui_fab_base *base;
  enum md2_fab_size size;
};

ui_error_t md2_fab_create(struct ui_engine *engine, enum md2_fab_size size,
                          const char *icon, const char *label,
                          struct md2_fab **out_fab) {
  struct md2_fab *fab;
  struct ui_button_base *main_button;
  ui_error_t rc;

  if (!engine || !out_fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  fab = (struct md2_fab *)C_MULTIPLATFORM_MALLOC(sizeof(struct md2_fab));
  if (!fab) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memset(fab, 0, sizeof(*fab));

  fab->size = size;

  rc = ui_component_create(&fab->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  rc = ui_fab_base_create(&fab->base);
  if (rc != UI_ERROR_NONE) {
    ui_component_destroy(fab->component);
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  /* Create and configure main button */
  rc = ui_button_base_create(&main_button);
  if (rc != UI_ERROR_NONE) {
    ui_fab_base_destroy(fab->base);
    ui_component_destroy(fab->component);
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  /* Since icon logic implies setting text on the underlying button in some form
     (or a child node), we'll use set_text for label if extended. If not
     extended, we can just treat the icon as a string for now. A more thorough
     integration would use an icon component or text_node. */
  if (size == MD2_FAB_SIZE_EXTENDED && label) {
    rc = ui_button_base_set_text(main_button, label);
  } else if (icon) {
    rc = ui_button_base_set_text(main_button, icon);
  }

  if (rc != UI_ERROR_NONE) {
    ui_button_base_destroy(main_button);
    ui_fab_base_destroy(fab->base);
    ui_component_destroy(fab->component);
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  rc = ui_fab_base_set_main_button(fab->base, main_button);
  if (rc != UI_ERROR_NONE) {
    ui_button_base_destroy(main_button);
    ui_fab_base_destroy(fab->base);
    ui_component_destroy(fab->component);
    C_MULTIPLATFORM_FREE(fab);
    return rc;
  }

  *out_fab = fab;
  return UI_ERROR_NONE;
}

ui_error_t md2_fab_destroy(struct md2_fab *fab) {
  ui_error_t rc = UI_ERROR_NONE;
  ui_error_t temp_rc;

  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (fab->base) {
    temp_rc = ui_fab_base_destroy(fab->base);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  if (fab->component) {
    temp_rc = ui_component_destroy(fab->component);
    if (temp_rc != UI_ERROR_NONE)
      rc = temp_rc;
  }

  C_MULTIPLATFORM_FREE(fab);
  return rc;
}

ui_error_t md2_fab_set_on_click(struct md2_fab *fab,
                                ui_button_on_click_t on_click,
                                void *user_data) {
  struct ui_button_base *main_button;
  ui_error_t rc;

  if (!fab || !fab->base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_fab_base_get_main_button(fab->base, &main_button);
  if (rc != UI_ERROR_NONE || !main_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_button_base_set_on_click(main_button, on_click, user_data);
}

ui_error_t md2_fab_get_base(struct md2_fab *fab,
                            struct ui_fab_base **out_base) {
  if (!fab || !out_base) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_base = fab->base;
  return UI_ERROR_NONE;
}
