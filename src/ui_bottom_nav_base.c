/**
 * @file ui_bottom_nav_base.c
 * @brief Implementation of the bottom navigation base component.
 */

#include "ui_bottom_nav_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_bottom_nav_base
 * @struct ui_bottom_nav_base
 * @brief Internal representation of a bottom navigation bar base.
 */
struct ui_bottom_nav_base {
  struct ui_component *component; /**< Core UI component */
  struct ui_signal
      *active_index_signal; /**< Optional signal to bind active index */
};

/**
 * @struct ui_bottom_nav_item_base
 * @struct ui_bottom_nav_item_base
 * @brief Internal representation of a bottom navigation item.
 */
struct ui_bottom_nav_item_base {
  struct ui_component *component; /**< Core UI component for the item */
  int active;                     /**< Active state flag */
};

ui_error_t ui_bottom_nav_base_create(struct ui_bottom_nav_base **out_nav) {
  struct ui_bottom_nav_base *nav;
  ui_error_t rc;

  if (!out_nav) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  nav = (struct ui_bottom_nav_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_bottom_nav_base));
  if (!nav) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_component_create(&nav->component);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(nav);
    return rc;
  }

  /* ARIA role="navigation" typically applied here */

  *out_nav = nav;
  return UI_ERROR_NONE;
}

#ifdef UI_TEST_MOCK_ALLOC
int g_bottom_nav_mock_fail = 0;
static ui_error_t mock_component_destroy(struct ui_component *c) {
  if (g_bottom_nav_mock_fail == 20)
    return UI_ERROR_UNKNOWN;
  return ui_component_destroy(c);
}
#define ui_component_destroy mock_component_destroy
#endif

ui_error_t ui_bottom_nav_base_destroy(struct ui_bottom_nav_base *nav) {
  if (!nav) {
    return UI_ERROR_NONE;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(nav->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(nav);
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_base_get_component(struct ui_bottom_nav_base *nav,
                                 struct ui_component **out_component) {
  if (!nav || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = nav->component;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_base_append_item(struct ui_bottom_nav_base *nav,
                               struct ui_bottom_nav_item_base *item) {
  if (!nav || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* DOM node appending logic */
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_item_base_create(struct ui_bottom_nav_item_base **out_item) {
  struct ui_bottom_nav_item_base *item;
  ui_error_t rc;

  if (!out_item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  item = (struct ui_bottom_nav_item_base *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_bottom_nav_item_base));
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

ui_error_t
ui_bottom_nav_item_base_destroy(struct ui_bottom_nav_item_base *item) {
  if (!item) {
    return UI_ERROR_NONE;
  }
  {
    ui_error_t rc_cleanup = ui_component_destroy(item->component);
    if (rc_cleanup != UI_ERROR_NONE) {
      (void)rc_cleanup; /* Avoid override */
    }
  }
  C_MULTIPLATFORM_FREE(item);
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_item_base_get_component(struct ui_bottom_nav_item_base *item,
                                      struct ui_component **out_component) {
  if (!item || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = item->component;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_item_base_set_active(struct ui_bottom_nav_item_base *item,
                                   int active) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  item->active = active;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_item_base_get_active(struct ui_bottom_nav_item_base *item,
                                   int *out_active) {
  if (!item || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_active = item->active;
  return UI_ERROR_NONE;
}

ui_error_t
ui_bottom_nav_base_bind_active_index(struct ui_bottom_nav_base *widget,
                                     struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
