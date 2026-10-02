/**
 * @file cupertino_dynamic_island.c
 * @brief Dynamic Island morphing container implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_dynamic_island.h"
#include "ui_internal_mem.h"
#include <math.h>
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_dynamic_island_mock_set_state_fail = 0;
#endif

static ui_error_t internal_set_state(struct cupertino_dynamic_island *island,
                                     enum cupertino_island_state state,
                                     int animated) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_dynamic_island_mock_set_state_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif
  return cupertino_dynamic_island_set_state(island, state, animated);
}

static void get_state_metrics(enum cupertino_island_state state,
                              float screen_width, float *out_w, float *out_h,
                              float *out_r) {
  if (state == CUPERTINO_ISLAND_MINIMAL) {
    *out_w = CUPERTINO_ISLAND_MINIMAL_SIZE;
    *out_h = CUPERTINO_ISLAND_MINIMAL_SIZE;
    *out_r = CUPERTINO_ISLAND_MINIMAL_SIZE * 0.5f;
  } else if (state == CUPERTINO_ISLAND_EXPANDED) {
    *out_w = (screen_width > 400.0f) ? CUPERTINO_ISLAND_EXPANDED_WIDTH
                                     : (screen_width - 24.0f);
    *out_h = CUPERTINO_ISLAND_EXPANDED_HEIGHT;
    *out_r = 42.0f;
  } else {
    *out_w = CUPERTINO_ISLAND_COMPACT_WIDTH;
    *out_h = CUPERTINO_ISLAND_COMPACT_HEIGHT;
    *out_r = CUPERTINO_ISLAND_COMPACT_HEIGHT * 0.5f;
  }
}

ui_error_t cupertino_dynamic_island_create(
    struct ui_engine *engine,
    const struct cupertino_dynamic_island_descriptor *desc,
    struct cupertino_dynamic_island **out_island) {
  struct cupertino_dynamic_island *island;
  float w, h, r;

  if (!engine || !desc || !out_island) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->initial_state < 0 || (int)desc->initial_state > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  if ((int)desc->initial_template < 0 || (int)desc->initial_template > 4) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  island = (struct cupertino_dynamic_island *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_dynamic_island));
  if (!island) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(island, 0, sizeof(*island));
  island->state = desc->initial_state;
  island->template_type = desc->initial_template;
  island->screen_width =
      (desc->screen_width > 0.0f) ? desc->screen_width : 393.0f;
  island->background_color = UI_COLOR_ARGB(255, 0, 0, 0);

  get_state_metrics(island->state, island->screen_width, &w, &h, &r);
  island->current_width = w;
  island->current_height = h;
  island->current_corner_radius = r;
  island->target_width = w;
  island->target_height = h;
  island->target_corner_radius = r;
  island->is_animating = 0;
  island->morph_progress = 1.0f;

  *out_island = island;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dynamic_island_destroy(struct cupertino_dynamic_island *island) {
  if (!island) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(island);
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dynamic_island_set_state(struct cupertino_dynamic_island *island,
                                   enum cupertino_island_state state,
                                   int animated) {
  float w, h, r;

  if (!island || (int)state < 0 || (int)state > 2) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  island->state = state;
  get_state_metrics(state, island->screen_width, &w, &h, &r);
  island->target_width = w;
  island->target_height = h;
  island->target_corner_radius = r;

  if (!animated) {
    island->current_width = w;
    island->current_height = h;
    island->current_corner_radius = r;
    island->morph_progress = 1.0f;
    island->is_animating = 0;
  } else {
    island->is_animating = 1;
    island->morph_progress = 0.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_dynamic_island_get_state(
    const struct cupertino_dynamic_island *island,
    enum cupertino_island_state *out_state) {
  if (!island || !out_state) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state = island->state;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_dynamic_island_set_template(
    struct cupertino_dynamic_island *island,
    enum cupertino_island_template template_type) {
  if (!island || (int)template_type < 0 || (int)template_type > 4) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  island->template_type = template_type;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_dynamic_island_get_template(
    const struct cupertino_dynamic_island *island,
    enum cupertino_island_template *out_template) {
  if (!island || !out_template) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_template = island->template_type;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dynamic_island_tick(struct cupertino_dynamic_island *island,
                              float delta_ms) {
  float factor;

  if (!island || delta_ms < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (!island->is_animating) {
    return UI_ERROR_NONE;
  }

  factor = 1.0f - (float)exp(-delta_ms / 60.0f);
  island->current_width +=
      (island->target_width - island->current_width) * factor;
  island->current_height +=
      (island->target_height - island->current_height) * factor;
  island->current_corner_radius +=
      (island->target_corner_radius - island->current_corner_radius) * factor;

  if (delta_ms >= 400.0f) {
    island->current_width = island->target_width;
    island->current_height = island->target_height;
    island->current_corner_radius = island->target_corner_radius;
    island->is_animating = 0;
    island->morph_progress = 1.0f;
  } else if ((float)fabs(island->target_width - island->current_width) < 1.0f &&
             (float)fabs(island->target_height - island->current_height) <
                 1.0f) {
    island->current_width = island->target_width;
    island->current_height = island->target_height;
    island->current_corner_radius = island->target_corner_radius;
    island->is_animating = 0;
    island->morph_progress = 1.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_dynamic_island_get_bounds(
    const struct cupertino_dynamic_island *island, float *out_x, float *out_y,
    float *out_w, float *out_h, float *out_radius) {
  if (!island || !out_x || !out_y || !out_w || !out_h || !out_radius) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = island->current_width;
  *out_h = island->current_height;
  *out_x = (island->screen_width - island->current_width) * 0.5f;
  *out_y = CUPERTINO_ISLAND_TOP_MARGIN;
  *out_radius = island->current_corner_radius;
  return UI_ERROR_NONE;
}

ui_error_t
cupertino_dynamic_island_handle_touch(struct cupertino_dynamic_island *island,
                                      int is_long_press,
                                      int *out_state_changed) {
  ui_error_t rc;

  if (!island || !out_state_changed) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_state_changed = 0;

  if (is_long_press < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (island->state == CUPERTINO_ISLAND_EXPANDED) {
    /* Collapse on tap */
    rc = internal_set_state(island, CUPERTINO_ISLAND_COMPACT, 1);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    *out_state_changed = 1;
  } else {
    /* Expand on tap or long-press */
    rc = internal_set_state(island, CUPERTINO_ISLAND_EXPANDED, 1);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    *out_state_changed = 1;
  }

  return UI_ERROR_NONE;
}
