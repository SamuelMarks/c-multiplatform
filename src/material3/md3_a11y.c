/**
 * @file md3_a11y.c
 * @brief Implementation of Material 3 Accessibility (a11y) safeguards, touch
 * targets, high-contrast mode, focus rings, roving tabindex, announcer, and
 * ARIA synchronization.
 */

/* clang-format off */
#include "material3/md3_a11y.h"
#include "ui_component.h"
#include "ui_focus_manager.h"
#include "ui_focus_trap.h"
#include "ui_internal_mem.h"
#include "ui_live_announcer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static int g_md3_high_contrast_enabled = 0;

/**
 * @struct md3_roving_tabindex
 * @brief Internal state for Material 3 roving tabindex management.
 */
struct md3_roving_tabindex {
  /** @brief Total number of focusable items. */
  size_t item_count;
  /** @brief Currently focused item index. */
  size_t focused_index;
};

/**
 * @struct md3_announcer
 * @brief Internal state for Material 3 live announcer.
 */
struct md3_announcer {
  /** @brief Pointer to base UI live announcer instance. */
  struct ui_live_announcer *base;
};

ui_error_t md3_a11y_enforce_touch_target(float width, float height,
                                         float *out_pad_x, float *out_pad_y) {
  if (!out_pad_x || !out_pad_y || width < 0.0f || height < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (width < 48.0f) {
    *out_pad_x = (48.0f - width) / 2.0f;
  } else {
    *out_pad_x = 0.0f;
  }

  if (height < 48.0f) {
    *out_pad_y = (48.0f - height) / 2.0f;
  } else {
    *out_pad_y = 0.0f;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_a11y_validate_touch_target(float width, float height,
                                          int *out_is_valid) {
  if (!out_is_valid || width < 0.0f || height < 0.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (width >= 48.0f && height >= 48.0f) {
    *out_is_valid = 1;
  } else {
    *out_is_valid = 0;
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_a11y_is_high_contrast_enabled(struct ui_engine *engine,
                                             int *out_enabled) {
  if (!engine || !out_enabled) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_enabled = g_md3_high_contrast_enabled;
  return UI_ERROR_NONE;
}

ui_error_t md3_a11y_set_high_contrast_enabled(struct ui_engine *engine,
                                              int enabled) {
  if (!engine) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  g_md3_high_contrast_enabled = enabled ? 1 : 0;
  return UI_ERROR_NONE;
}

ui_error_t md3_a11y_get_forced_colors_border(int is_high_contrast,
                                             float *out_border_width,
                                             ui_color_t *out_border_color) {
  if (!out_border_width || !out_border_color) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (is_high_contrast) {
    *out_border_width = 2.0f;
    *out_border_color = UI_COLOR_ARGB(255, 255, 255, 255);
  } else {
    *out_border_width = 0.0f;
    *out_border_color = UI_COLOR_ARGB(0, 0, 0, 0);
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_a11y_get_focus_ring(ui_color_t bg_color,
                                   struct ui_focus_ring *out_ring) {
  float r, g, b, lum;

  if (!out_ring) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  r = (float)UI_COLOR_RED(bg_color) / 255.0f;
  g = (float)UI_COLOR_GREEN(bg_color) / 255.0f;
  b = (float)UI_COLOR_BLUE(bg_color) / 255.0f;
  lum = 0.2126f * r + 0.7152f * g + 0.0722f * b;

  out_ring->offset = 2.0f;
  out_ring->width = 3.0f;
  out_ring->inner_width = 1.5f;

  if (lum > 0.5f) {
    /* Light background: outer ring is dark, inner ring is light */
    out_ring->color = UI_COLOR_ARGB(255, 0, 0, 0);
    out_ring->inner_color = UI_COLOR_ARGB(255, 255, 255, 255);
  } else {
    /* Dark background: outer ring is light, inner ring is dark */
    out_ring->color = UI_COLOR_ARGB(255, 255, 255, 255);
    out_ring->inner_color = UI_COLOR_ARGB(255, 0, 0, 0);
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_roving_tabindex_create(size_t item_count,
                                      struct md3_roving_tabindex **out_roving) {
  struct md3_roving_tabindex *roving;

  if (!out_roving || item_count == 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  roving = (struct md3_roving_tabindex *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_roving_tabindex));
  if (!roving) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  roving->item_count = item_count;
  roving->focused_index = 0;

  *out_roving = roving;
  return UI_ERROR_NONE;
}

ui_error_t md3_roving_tabindex_destroy(struct md3_roving_tabindex *roving) {
  if (!roving) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(roving);
  return UI_ERROR_NONE;
}

ui_error_t md3_roving_tabindex_handle_key(struct md3_roving_tabindex *roving,
                                          enum ui_key_code key,
                                          size_t *out_focused_index) {
  if (!roving || !out_focused_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  switch (key) {
  case UI_KEY_DOWN:
  case UI_KEY_RIGHT:
    roving->focused_index = (roving->focused_index + 1) % roving->item_count;
    break;

  case UI_KEY_UP:
  case UI_KEY_LEFT:
    if (roving->focused_index == 0) {
      roving->focused_index = roving->item_count - 1;
    } else {
      roving->focused_index--;
    }
    break;

  case UI_KEY_HOME:
    roving->focused_index = 0;
    break;

  case UI_KEY_END:
    roving->focused_index = roving->item_count - 1;
    break;

  default:
    break;
  }

  *out_focused_index = roving->focused_index;
  return UI_ERROR_NONE;
}

ui_error_t
md3_roving_tabindex_get_focused(const struct md3_roving_tabindex *roving,
                                size_t *out_focused_index) {
  if (!roving || !out_focused_index) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_focused_index = roving->focused_index;
  return UI_ERROR_NONE;
}

ui_error_t md3_roving_tabindex_set_focused(struct md3_roving_tabindex *roving,
                                           size_t index) {
  if (!roving || index >= roving->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  roving->focused_index = index;
  return UI_ERROR_NONE;
}

ui_error_t md3_announcer_create(struct md3_announcer **out_announcer) {
  struct md3_announcer *announcer;
  ui_error_t rc;

  if (!out_announcer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  announcer = (struct md3_announcer *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct md3_announcer));
  if (!announcer) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  rc = ui_live_announcer_create(&announcer->base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(announcer);
    return rc;
  }

  *out_announcer = announcer;
  return UI_ERROR_NONE;
}

ui_error_t md3_announcer_destroy(struct md3_announcer *announcer) {
  ui_error_t rc;

  if (!announcer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_live_announcer_destroy(announcer->base);
  C_MULTIPLATFORM_FREE(announcer);
  return rc;
}

ui_error_t md3_announcer_announce(struct md3_announcer *announcer,
                                  const char *message,
                                  enum ui_live_politeness politeness) {
  if (!announcer || !message) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_live_announce(announcer->base, message, politeness);
}

ui_error_t md3_announcer_clear(struct md3_announcer *announcer) {
  if (!announcer) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return ui_live_announcer_clear(announcer->base);
}

ui_error_t md3_a11y_sync_aria(struct ui_component *comp, const char *role,
                              int is_checked, int is_expanded, int is_selected,
                              int is_disabled, double value_now,
                              double value_min, double value_max,
                              const char *has_popup) {
  char num_buf[32];
  ui_error_t rc;

  if (!comp) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (role != NULL) {
    rc = ui_component_set_property(comp, "role", role);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (is_checked >= 0) {
    const char *chk_str;
    if (is_checked == 1) {
      chk_str = "true";
    } else if (is_checked == 0) {
      chk_str = "false";
    } else {
      chk_str = "mixed";
    }
    rc = ui_component_set_property(comp, "aria-checked", chk_str);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (is_expanded >= 0) {
    rc = ui_component_set_property(comp, "aria-expanded",
                                   is_expanded ? "true" : "false");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (is_selected >= 0) {
    rc = ui_component_set_property(comp, "aria-selected",
                                   is_selected ? "true" : "false");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (is_disabled >= 0) {
    rc = ui_component_set_property(comp, "aria-disabled",
                                   is_disabled ? "true" : "false");
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (value_now >= 0.0) {
#if defined(_MSC_VER)
    sprintf_s(num_buf, sizeof(num_buf), "%.2f", value_now);
#else
    snprintf(num_buf, sizeof(num_buf), "%.2f", value_now);
#endif
    rc = ui_component_set_property(comp, "aria-valuenow", num_buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (value_min >= 0.0) {
#if defined(_MSC_VER)
    sprintf_s(num_buf, sizeof(num_buf), "%.2f", value_min);
#else
    snprintf(num_buf, sizeof(num_buf), "%.2f", value_min);
#endif
    rc = ui_component_set_property(comp, "aria-valuemin", num_buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (value_max >= 0.0) {
#if defined(_MSC_VER)
    sprintf_s(num_buf, sizeof(num_buf), "%.2f", value_max);
#else
    snprintf(num_buf, sizeof(num_buf), "%.2f", value_max);
#endif
    rc = ui_component_set_property(comp, "aria-valuemax", num_buf);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  if (has_popup != NULL) {
    rc = ui_component_set_property(comp, "aria-haspopup", has_popup);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t md3_a11y_trap_focus(struct ui_focus_trap **out_trap,
                               struct ui_focus_manager *manager,
                               struct ui_dom_node *root) {
  struct ui_focus_trap *trap;
  ui_error_t rc;

  if (!out_trap || !manager || !root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_focus_trap_create(&trap);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_focus_trap_activate(trap, manager, root);
  *out_trap = trap;
  return rc;
}

ui_error_t md3_a11y_release_focus(struct ui_focus_trap *trap,
                                  struct ui_focus_manager *manager) {
  ui_error_t rc;

  if (!trap || !manager) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  rc = ui_focus_trap_deactivate(trap, manager);
  ui_focus_trap_destroy(trap);
  return rc;
}
