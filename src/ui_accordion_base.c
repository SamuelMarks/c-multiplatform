/* clang-format off */
#include "ui_accordion_base.h"
#include "ui_internal_mem.h"
#include <stddef.h>
/* clang-format on */

struct ui_accordion_base {
  struct ui_disclosure_base **disclosures;
  size_t count;
  size_t capacity;
  struct ui_disclosure_base *active_disclosure;
  ui_accordion_on_change_t on_change;
  void *user_data;
  struct ui_computed *data_signal;
};

static enum ui_error
on_child_disclosure_toggle(struct ui_disclosure_base *disclosure,
                           int is_expanded, void *user_data) {
  struct ui_accordion_base *accordion = (struct ui_accordion_base *)user_data;
  size_t i;
  enum ui_error rc = UI_ERROR_NONE;

  if (!is_expanded) {
    if (accordion->active_disclosure == disclosure) {
      accordion->active_disclosure = NULL;
      if (accordion->on_change) {
        rc = accordion->on_change(accordion, NULL, accordion->user_data);
      }
    }
    return rc;
  }

  /* We are expanding this item. Let's record it so that when we collapse
     others, they don't trigger cascade events back to us. */
  accordion->active_disclosure = disclosure;

  /* Collapse all other items. */
  for (i = 0; i < accordion->count; ++i) {
    int check_expanded = 0;
    if (accordion->disclosures[i] != disclosure) {
      ui_disclosure_base_is_expanded(accordion->disclosures[i],
                                     &check_expanded);
      if (check_expanded) {
        ui_disclosure_base_set_expanded(accordion->disclosures[i], 0);
      }
    }
  }

  if (accordion->on_change) {
    rc = accordion->on_change(accordion, disclosure, accordion->user_data);
  }
  return rc;
}

/**
 * @brief Creates a new accordion base manager.
 *
 * @param out_accordion Pointer to receive the allocated accordion.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_create(struct ui_accordion_base **out_accordion) {
  struct ui_accordion_base *accordion;

  if (!out_accordion) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  accordion =
      (struct ui_accordion_base *)UI_MALLOC(sizeof(struct ui_accordion_base));
  if (!accordion) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  accordion->disclosures = NULL;
  accordion->count = 0;
  accordion->capacity = 0;
  accordion->active_disclosure = NULL;
  accordion->on_change = NULL;
  accordion->user_data = NULL;

  *out_accordion = accordion;
  return UI_ERROR_NONE;
}

/**
 * @brief Destroys an accordion base manager.
 * Note: This does not destroy the individual ui_disclosure_base components.
 *
 * @param accordion The accordion to destroy.
 */
void ui_accordion_base_destroy(struct ui_accordion_base *accordion) {
  size_t i;
  if (!accordion)
    return;

  /* Unhook callbacks to prevent dangling pointers */
  for (i = 0; i < accordion->count; ++i) {
    ui_disclosure_base_set_on_toggle(accordion->disclosures[i], NULL, NULL);
  }

  UI_FREE(accordion->disclosures);
  UI_FREE(accordion);
}

/**
 * @brief Adds a disclosure to the accordion group.
 *
 * @param accordion The accordion manager.
 * @param disclosure The disclosure component to add.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_add_disclosure(struct ui_accordion_base *accordion,
                                 struct ui_disclosure_base *disclosure) {
  size_t i;

  if (!accordion || !disclosure)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < accordion->count; ++i) {
    if (accordion->disclosures[i] == disclosure)
      return UI_ERROR_NONE; /* Already added */
  }

  if (accordion->count >= accordion->capacity) {
    size_t new_cap = accordion->capacity == 0 ? 4 : accordion->capacity * 2;
    struct ui_disclosure_base **new_arr =
        (struct ui_disclosure_base **)UI_REALLOC(
            accordion->disclosures,
            new_cap * sizeof(struct ui_disclosure_base *));
    if (!new_arr)
      return UI_ERROR_OUT_OF_MEMORY;
    accordion->disclosures = new_arr;
    accordion->capacity = new_cap;
  }

  accordion->disclosures[accordion->count++] = disclosure;
  ui_disclosure_base_set_on_toggle(disclosure, on_child_disclosure_toggle,
                                   accordion);

  {
    int check_expanded = 0;
    ui_disclosure_base_is_expanded(disclosure, &check_expanded);
    if (check_expanded) {
      (void)on_child_disclosure_toggle(disclosure, 1, accordion);
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Removes a disclosure from the accordion group.
 *
 * @param accordion The accordion manager.
 * @param disclosure The disclosure component to remove.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_remove_disclosure(struct ui_accordion_base *accordion,
                                    struct ui_disclosure_base *disclosure) {
  size_t i;
  int found = -1;

  if (!accordion || !disclosure)
    return UI_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < accordion->count; ++i) {
    if (accordion->disclosures[i] == disclosure) {
      found = (int)i;
      break;
    }
  }

  if (found == -1)
    return UI_ERROR_NOT_FOUND;

  ui_disclosure_base_set_on_toggle(disclosure, NULL, NULL);

  if (accordion->active_disclosure == disclosure) {
    accordion->active_disclosure = NULL;
  }

  for (i = (size_t)found; i < accordion->count - 1; ++i) {
    accordion->disclosures[i] = accordion->disclosures[i + 1];
  }
  accordion->count--;

  return UI_ERROR_NONE;
}

/**
 * @brief Sets the currently active (expanded) disclosure in the group.
 * Unexpands all other disclosures in this group.
 *
 * @param accordion The accordion manager.
 * @param disclosure The disclosure to set as active. If NULL, collapses all.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_set_active(struct ui_accordion_base *accordion,
                             struct ui_disclosure_base *disclosure) {
  size_t i;
  int valid_disclosure = 0;

  if (!accordion)
    return UI_ERROR_INVALID_ARGUMENT;

  if (disclosure) {
    for (i = 0; i < accordion->count; ++i) {
      if (accordion->disclosures[i] == disclosure) {
        valid_disclosure = 1;
        break;
      }
    }
    if (!valid_disclosure)
      return UI_ERROR_NOT_FOUND;

    ui_disclosure_base_set_expanded(disclosure, 1);
  } else {
    for (i = 0; i < accordion->count; ++i) {
      ui_disclosure_base_set_expanded(accordion->disclosures[i], 0);
    }
  }

  return UI_ERROR_NONE;
}
/**
 * @brief Retrieves the currently active (expanded) disclosure in the group.
 *
 * @param accordion The accordion manager.
 * @return The active disclosure, or NULL if none are expanded.
 */
enum ui_error
ui_accordion_base_get_active(const struct ui_accordion_base *accordion,
                             struct ui_disclosure_base **out_active) {
  if (!accordion || !out_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  *out_active = accordion->active_disclosure;
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the callback for when the active disclosure changes.
 *
 * @param accordion The accordion manager.
 * @param on_change The callback to invoke.
 * @param user_data Opaque user data.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_accordion_base_set_on_change(struct ui_accordion_base *accordion,
                                ui_accordion_on_change_t on_change,
                                void *user_data) {
  if (!accordion)
    return UI_ERROR_INVALID_ARGUMENT;
  accordion->on_change = on_change;
  accordion->user_data = user_data;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the data property.
 *
 * @param widget The widget.
 * @param signal The signal to bind to.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_accordion_base_bind_data(struct ui_accordion_base *widget,
                                          struct ui_computed *signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->data_signal = signal;
  return UI_ERROR_NONE;
}
