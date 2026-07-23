/* clang-format off */
#include "ui_fab_base.h"
#include "ui_internal_mem.h"
#include <stdlib.h>
/* clang-format on */

#define EXPANSION_DURATION_MS 200.0f

struct ui_fab_base {
  struct ui_button_base *main_button;
  struct ui_button_base **action_buttons;
  size_t action_count;
  size_t action_capacity;

  enum ui_fab_state state;
  float expansion_progress; /* 0.0 to 1.0 */

  struct ui_ripple_config ripple_cfg;
  struct ui_ripple_state ripple_state;
  struct ui_signal *disabled_signal;
  struct ui_signal *text_signal;
};

enum ui_error ui_fab_base_create(struct ui_fab_base **out_fab) {
  struct ui_fab_base *fab;
  enum ui_error rc = UI_ERROR_NONE;

  if (!out_fab) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  fab = (struct ui_fab_base *)UI_MALLOC(sizeof(struct ui_fab_base));
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

  ui_ripple_config_init(&fab->ripple_cfg);
  fab->ripple_state.active = 0;

  *out_fab = fab;

cleanup:
  return rc;
}

void ui_fab_base_destroy(struct ui_fab_base *fab) {
  if (!fab) {
    return;
  }

  if (fab->main_button) {
    ui_button_base_destroy(fab->main_button);
  }

  if (fab->action_buttons) {
    size_t i;
    for (i = 0; i < fab->action_count; i++) {
      ui_button_base_destroy(fab->action_buttons[i]);
    }
    UI_FREE(fab->action_buttons);
  }

  UI_FREE(fab);
}

enum ui_error ui_fab_base_set_main_button(struct ui_fab_base *fab,
                                          struct ui_button_base *button) {
  if (!fab || !button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (fab->main_button) {
    ui_button_base_destroy(fab->main_button);
  }

  fab->main_button = button;
  return UI_ERROR_NONE;
}

enum ui_error ui_fab_base_get_main_button(const struct ui_fab_base *fab,
                                          struct ui_button_base **out_button) {
  if (!fab || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_button = fab->main_button;
  return UI_ERROR_NONE;
}

enum ui_error ui_fab_base_add_action(struct ui_fab_base *fab,
                                     struct ui_button_base *action_button) {
  enum ui_error rc = UI_ERROR_NONE;
  size_t new_cap;
  struct ui_button_base **new_actions;
  size_t i;

  if (!fab || !action_button) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  if (fab->action_count == fab->action_capacity) {
    new_cap = fab->action_capacity == 0 ? 2 : fab->action_capacity * 2;
    new_actions = (struct ui_button_base **)UI_MALLOC(
        new_cap * sizeof(struct ui_button_base *));
    if (!new_actions) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    if (fab->action_buttons) {
      for (i = 0; i < fab->action_count; i++) {
        new_actions[i] = fab->action_buttons[i];
      }
      UI_FREE(fab->action_buttons);
    }

    fab->action_buttons = new_actions;
    fab->action_capacity = new_cap;
  }

  fab->action_buttons[fab->action_count++] = action_button;

cleanup:
  return rc;
}

enum ui_error ui_fab_base_get_action_count(const struct ui_fab_base *fab,
                                           size_t *out_count) {
  if (!fab || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = fab->action_count;
  return UI_ERROR_NONE;
}

enum ui_error ui_fab_base_get_action(const struct ui_fab_base *fab,
                                     size_t index,
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

enum ui_error ui_fab_base_toggle(struct ui_fab_base *fab) {
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

enum ui_error ui_fab_base_get_state(const struct ui_fab_base *fab,
                                    enum ui_fab_state *out_state) {
  if (!fab || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = fab->state;
  return UI_ERROR_NONE;
}

enum ui_error ui_fab_base_tick(struct ui_fab_base *fab, float dt_ms) {
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

enum ui_error ui_fab_base_get_expansion_progress(const struct ui_fab_base *fab,
                                                 float *out_progress) {
  if (!fab || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_progress = fab->expansion_progress;
  return UI_ERROR_NONE;
}

/** \brief ui_error */
enum ui_error
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

enum ui_error ui_fab_base_start_ripple(struct ui_fab_base *fab, float origin_x,
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

enum ui_error ui_fab_base_bind_disabled(struct ui_fab_base *widget,
                                        struct ui_signal *disabled_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->disabled_signal = disabled_signal;
  return UI_ERROR_NONE;
}

enum ui_error ui_fab_base_bind_text(struct ui_fab_base *widget,
                                    struct ui_signal *text_signal) {
  if (!widget) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  widget->text_signal = text_signal;
  return UI_ERROR_NONE;
}
