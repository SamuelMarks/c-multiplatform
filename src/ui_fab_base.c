/**
 * @file ui_fab_base.c
 * @brief Implementation of the Floating Action Button (FAB) base widget.
 */
/* clang-format off */
#include "ui_fab_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

/** @brief Duration of FAB expansion in milliseconds. */
#define EXPANSION_DURATION_MS 200.0f

/**
 * @struct ui_fab_base
 * @brief Represents the internal state of a FAB widget.
 */
struct ui_fab_base {
  struct ui_button_base *main_button; /**< The main action button of the FAB. */
  struct ui_button_base *
      *action_buttons; /**< Array of secondary action buttons. */
  size_t action_count; /**< Current number of action buttons. */
  size_t
      action_capacity; /**< Allocated capacity of the action buttons array. */

  enum ui_fab_state state; /**< Current visual and semantic state of the FAB. */
  float expansion_progress; /**< Expansion progress from 0.0 (collapsed) to 1.0
                               (expanded). */

  struct ui_ripple_config
      ripple_cfg; /**< Configuration for the ripple effect. */
  struct ui_ripple_state
      ripple_state; /**< Current runtime state of the ripple effect. */
  struct ui_signal
      *disabled_signal; /**< Signal indicating if the FAB is disabled. */
  struct ui_signal *text_signal; /**< Signal providing text for the FAB. */
};

/**
 * @brief Creates a new FAB base widget.
 * @param[out] out_fab Pointer to store the created FAB.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_create(struct ui_fab_base **out_fab) {
  struct ui_fab_base *fab;
  ui_error_t rc = UI_ERROR_NONE;

  if (!out_fab) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  fab =
      (struct ui_fab_base *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_fab_base));
  if (!fab) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  fab->main_button = NULL;
  fab->action_buttons = NULL;
  fab->action_count = 0;
  fab->action_capacity = 0;

  fab->state = UI_FAB_STATE_COLLAPSED;
  fab->expansion_progress = 0.0f;

  rc = ui_ripple_config_init(&fab->ripple_cfg);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(fab);
    goto cleanup;
  }
  fab->ripple_state.active = 0;

  *out_fab = fab;

cleanup:
  return rc;
}

/**
 * @brief Destroys a FAB base widget.
 * @param[in,out] fab The FAB to destroy.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_destroy(struct ui_fab_base *fab) {
  if (!fab) {
    return UI_ERROR_NONE;
  }

  if (fab->main_button) {
    {
      ui_error_t rc_cleanup = ui_button_base_destroy(fab->main_button);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  if (fab->action_buttons) {
    size_t i;
    for (i = 0; i < fab->action_count; i++) {
      {
        ui_error_t rc_cleanup = ui_button_base_destroy(fab->action_buttons[i]);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
    }
    C_MULTIPLATFORM_FREE(fab->action_buttons);
  }

  C_MULTIPLATFORM_FREE(fab);
  return UI_ERROR_NONE;
}

/**
 * @brief Sets the main button for the FAB.
 * @param[in,out] fab The FAB widget.
 * @param[in,out] button The main button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_set_main_button(struct ui_fab_base *fab,
                                       struct ui_button_base *button) {
  if (!fab || !button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (fab->main_button) {
    {
      ui_error_t rc_cleanup = ui_button_base_destroy(fab->main_button);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  fab->main_button = button;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the main button from the FAB.
 * @param[in] fab The FAB widget.
 * @param[out] out_button Pointer to store the main button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_get_main_button(const struct ui_fab_base *fab,
                                       struct ui_button_base **out_button) {
  if (!fab || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_button = fab->main_button;
  return UI_ERROR_NONE;
}

/**
 * @brief Adds an action button to the FAB.
 * @param[in,out] fab The FAB widget.
 * @param[in,out] action_button The action button to add.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_add_action(struct ui_fab_base *fab,
                                  struct ui_button_base *action_button) {
  ui_error_t rc = UI_ERROR_NONE;
  size_t new_cap;
  struct ui_button_base **new_actions;
  size_t i;

  if (!fab || !action_button) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (fab->action_count == fab->action_capacity) {
    new_cap = fab->action_capacity == 0 ? 2 : fab->action_capacity * 2;
    new_actions = (struct ui_button_base **)C_MULTIPLATFORM_MALLOC(
        (size_t)new_cap * sizeof(struct ui_button_base *));
    if (!new_actions) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    if (fab->action_buttons) {
      for (i = 0; i < fab->action_count; i++) {
        new_actions[i] = fab->action_buttons[i];
      }
      C_MULTIPLATFORM_FREE(fab->action_buttons);
    }

    fab->action_buttons = new_actions;
    fab->action_capacity = new_cap;
  }

  fab->action_buttons[fab->action_count++] = action_button;

cleanup:
  return rc;
}

/**
 * @brief Gets the number of actions in the FAB.
 * @param[in] fab The FAB widget.
 * @param[out] out_count Pointer to store the action count.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_get_action_count(const struct ui_fab_base *fab,
                                        size_t *out_count) {
  if (!fab || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = fab->action_count;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets an action button by index.
 * @param[in] fab The FAB widget.
 * @param[in] index The index of the action button.
 * @param[out] out_button Pointer to store the action button.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_get_action(const struct ui_fab_base *fab, size_t index,
                                  struct ui_button_base **out_button) {
  if (!fab || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (index >= fab->action_count) {
    return UI_ERROR_OUT_OF_BOUNDS;
  }

  *out_button = fab->action_buttons[index];
  return UI_ERROR_NONE;
}

/**
 * @brief Toggles the FAB's expanded state.
 * @param[in,out] fab The FAB widget.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_toggle(struct ui_fab_base *fab) {
  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (fab->state == UI_FAB_STATE_COLLAPSED ||
      fab->state == UI_FAB_STATE_COLLAPSING) {
    fab->state = UI_FAB_STATE_EXPANDING;
  } else {
    fab->state = UI_FAB_STATE_COLLAPSING;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Gets the current state of the FAB.
 * @param[in] fab The FAB widget.
 * @param[out] out_state Pointer to store the state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_get_state(const struct ui_fab_base *fab,
                                 enum ui_fab_state *out_state) {
  if (!fab || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = fab->state;
  return UI_ERROR_NONE;
}

/**
 * @brief Ticks the FAB's animation.
 * @param[in,out] fab The FAB widget.
 * @param[in] dt_ms Delta time in milliseconds.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_tick(struct ui_fab_base *fab, float dt_ms) {
  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Handle expansion animation */
  if (fab->state == UI_FAB_STATE_EXPANDING) {
    fab->expansion_progress += dt_ms / EXPANSION_DURATION_MS;
    if (fab->expansion_progress >= 1.0f) {
      fab->expansion_progress = 1.0f;
      fab->state = UI_FAB_STATE_EXPANDED;
    }
  } else if (fab->state == UI_FAB_STATE_COLLAPSING) {
    fab->expansion_progress -= dt_ms / EXPANSION_DURATION_MS;
    if (fab->expansion_progress <= 0.0f) {
      fab->expansion_progress = 0.0f;
      fab->state = UI_FAB_STATE_COLLAPSED;
    }
  }

  /* Handle ripple animation if active */
  if (fab->ripple_state.active) {
    fab->ripple_state.elapsed_ms += dt_ms;
    if (fab->ripple_state.elapsed_ms >= fab->ripple_cfg.duration_ms) {
      fab->ripple_state.active = 0;
      fab->ripple_state.opacity = 0.0f;
    } else {
      float progress =
          fab->ripple_state.elapsed_ms / fab->ripple_cfg.duration_ms;

      /* Simple ease-out for radius, fade-out for opacity */
      fab->ripple_state.radius = fab->ripple_cfg.max_radius *
                                 (1.0f - (1.0f - progress) * (1.0f - progress));
      fab->ripple_state.opacity = 1.0f - progress;
    }
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Gets the FAB's expansion progress.
 * @param[in] fab The FAB widget.
 * @param[out] out_progress Pointer to store the expansion progress (0.0
 * to 1.0).
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_get_expansion_progress(const struct ui_fab_base *fab,
                                              float *out_progress) {
  if (!fab || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = fab->expansion_progress;
  return UI_ERROR_NONE;
}

/**
 * @brief Gets the FAB's current ripple state.
 * @param[in] fab The FAB widget.
 * @param[out] out_ripple_state Pointer to store the ripple state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t
ui_fab_base_get_ripple_state(const struct ui_fab_base *fab,
                             struct ui_ripple_state **out_ripple_state) {
  if (!fab || !out_ripple_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Using casting to drop const to provide the inner pointer, though the
   * returned struct shouldn't be manually modified */
  *out_ripple_state = (struct ui_ripple_state *)&fab->ripple_state;
  return UI_ERROR_NONE;
}

/**
 * @brief Starts a ripple effect on the FAB.
 * @param[in,out] fab The FAB widget.
 * @param[in] origin_x The X origin of the ripple.
 * @param[in] origin_y The Y origin of the ripple.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_start_ripple(struct ui_fab_base *fab, float origin_x,
                                    float origin_y) {
  if (!fab) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (fab->ripple_cfg.center_origin) {
    /* Usually the caller would know the bounds, for a simple base component we
     * assume 0,0 is center or caller computes it */
    fab->ripple_state.center_x = 0.0f;
    fab->ripple_state.center_y = 0.0f;
  } else {
    fab->ripple_state.center_x = origin_x;
    fab->ripple_state.center_y = origin_y;
  }

  fab->ripple_state.radius = 0.0f;
  fab->ripple_state.opacity = 1.0f;
  fab->ripple_state.elapsed_ms = 0.0f;
  fab->ripple_state.active = 1;

  return UI_ERROR_NONE;
}

/**
 * @brief Binds the FAB's disabled state to a signal.
 * @param[in,out] widget The FAB widget.
 * @param[in,out] disabled_signal The signal representing disabled state.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_bind_disabled(struct ui_fab_base *widget,
                                     struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

/**
 * @brief Binds the FAB's text to a signal.
 * @param[in,out] widget The FAB widget.
 * @param[in,out] text_signal The signal representing the text.
 * @return UI_ERROR_NONE on success, or an appropriate error code.
 */
ui_error_t ui_fab_base_bind_text(struct ui_fab_base *widget,
                                 struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
