/**
 * @file ui_nav_rail_base.c
 * @brief Implementation of the UI navigation rail base component.
 * @details Represents a vertically aligned navigation component.
 */
#include "ui_nav_rail_base.h"

/* clang-format off */
#include "ui_aria.h"
#include "ui_component.h"
#include <stdlib.h>
#include "ui_internal_mem.h"
/* clang-format on */

/**
 * @struct ui_nav_rail_base
 * @brief Container state for a navigation rail widget.
 */
struct ui_nav_rail_base {
  struct ui_component *component; /**< Underlying DOM component */
  struct ui_signal
      *active_index_signal; /**< Signal bound to the active index */
};

/**
 * @struct ui_nav_rail_item_base
 * @brief State for a single navigation rail item.
 */
struct ui_nav_rail_item_base {
  struct ui_component *component; /**< Underlying DOM component */
  int active;                     /**< True if this item is currently active */
};

/**
 * @brief Creates a new navigation rail base widget.
 * @param[out] out_rail Pointer to store the created rail.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Destroys a navigation rail base widget.
 * @param[in,out] rail The rail widget to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_destroy(struct ui_nav_rail_base *rail) {
  if (!rail) {
    return UI_ERROR_NONE;
  }
  if (rail->component) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(rail->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  C_MULTIPLATFORM_FREE(rail);
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the underlying DOM component of the navigation rail.
 * @param[in,out] rail The rail widget.
 * @param[out] out_component Pointer to store the DOM component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_get_component(struct ui_nav_rail_base *rail,
                                          struct ui_component **out_component) {
  if (!rail || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = rail->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Appends an item to the navigation rail.
 * @param[in,out] rail The rail widget.
 * @param[in,out] item The rail item to append.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_append_item(struct ui_nav_rail_base *rail,
                                        struct ui_nav_rail_item_base *item) {
  if (!rail || !item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  /* DOM node appending logic */
  return UI_ERROR_NONE;
}

/**
 * @brief Creates a new navigation rail item.
 * @param[out] out_item Pointer to store the created item.
 * @return UI_ERROR_NONE on success.
 */
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

/**
 * @brief Destroys a navigation rail item.
 * @param[in,out] item The rail item to destroy.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_item_base_destroy(struct ui_nav_rail_item_base *item) {
  if (!item) {
    return UI_ERROR_NONE;
  }
  if (item->component) {
    {
      ui_error_t rc_cleanup = ui_component_destroy(item->component);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  C_MULTIPLATFORM_FREE(item);
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the underlying DOM component of the navigation rail item.
 * @param[in,out] item The rail item.
 * @param[out] out_component Pointer to store the DOM component.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
ui_nav_rail_item_base_get_component(struct ui_nav_rail_item_base *item,
                                    struct ui_component **out_component) {
  if (!item || !out_component) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_component = item->component;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the active (selected) state of a navigation rail item.
 * @param[in,out] item The rail item.
 * @param[in] active Non-zero to activate, 0 to deactivate.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_item_base_set_active(struct ui_nav_rail_item_base *item,
                                            int active) {
  if (!item) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  item->active = active;
  /* Aria state / Visual state updates would happen here */
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the active state of a navigation rail item.
 * @param[in] item The rail item.
 * @param[out] out_active Pointer to store the active state.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_item_base_get_active(struct ui_nav_rail_item_base *item,
                                            int *out_active) {
  if (!item || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_active = item->active;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the active index of the navigation rail to a reactive signal.
 * @param[in,out] widget The rail widget.
 * @param[in,out] signal The active index signal.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_nav_rail_base_bind_active_index(struct ui_nav_rail_base *widget,
                                              struct ui_signal *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->active_index_signal = signal;
  return UI_ERROR_NONE;
}
