#include "ui_nav_rail_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

struct ui_nav_rail_base {
  struct ui_component *component;
  struct ui_signal *active_index_signal;
};

/** \brief ui_nav_rail_item_base */
struct ui_nav_rail_item_base {
  struct ui_component *component;
  int active;
};

ui_error_t ui_nav_rail_base_create(struct ui_nav_rail_base **out_rail) {
  struct ui_nav_rail_base *rail;
  ui_error_t rc;

  if (!out_rail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rail = (struct ui_nav_rail_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_nav_rail_base));
  if (!rail) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&rail->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(rail);
    return rc;
  }

  /* ARIA role="navigation" typically applied here to the wrapper */

  *out_rail = rail;
  return UI_ERROR_NONE;
}

ui_error_t ui_nav_rail_base_destroy(struct ui_nav_rail_base *rail) {
  if (!rail) {
    return UI_ERROR_NONE;
  }
  if (rail->component) {
    (void)ui_component_destroy(rail->component);
  }
  C_MULTIPLATFORM_FREE(rail);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_nav_rail_base_get_component(struct ui_nav_rail_base *rail,
                                          struct ui_component **out_component) {
  if (!rail || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = rail->component;
  return UI_ERROR_NONE;
}

ui_error_t ui_nav_rail_base_append_item(struct ui_nav_rail_base *rail,
                                        struct ui_nav_rail_item_base *item) {
  if (!rail || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* DOM node appending logic */
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_nav_rail_item_base_create(struct ui_nav_rail_item_base **out_item) {
  struct ui_nav_rail_item_base *item;
  ui_error_t rc;

  if (!out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct ui_nav_rail_item_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_nav_rail_item_base));
  if (!item) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&item->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(item);
    return rc;
  }

  item->active = 0;
  *out_item = item;
  return UI_ERROR_NONE;
}

ui_error_t ui_nav_rail_item_base_destroy(struct ui_nav_rail_item_base *item) {
  if (!item) {
    return UI_ERROR_NONE;
  }
  if (item->component) {
    (void)ui_component_destroy(item->component);
  }
  C_MULTIPLATFORM_FREE(item);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t
ui_nav_rail_item_base_get_component(struct ui_nav_rail_item_base *item,
                                    struct ui_component **out_component) {
  if (!item || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = item->component;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_nav_rail_item_base_set_active(struct ui_nav_rail_item_base *item,
                                            int active) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  item->active = active;
  /* Aria state / Visual state updates would happen here */
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_nav_rail_item_base_get_active(struct ui_nav_rail_item_base *item,
                                            int *out_active) {
  if (!item || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_active = item->active;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_nav_rail_base_bind_active_index(struct ui_nav_rail_base *widget,
                                              struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
