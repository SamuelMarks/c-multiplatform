/**
 * @file cupertino_control_center.c
 * @brief iOS 18+ Control Center Modular System Controls implementation.
 */

/* clang-format off */
#include "cupertino/cupertino_control_center.h"
#include "cupertino/cupertino_haptics.h"
#include "ui_internal_mem.h"
#include <string.h>
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
int g_cupertino_control_center_mock_button_create_fail = 0;
int g_cupertino_control_center_mock_slider_create_fail = 0;
int g_cupertino_control_center_mock_button_destroy_fail = 0;
int g_cupertino_control_center_mock_slider_destroy_fail = 0;
int g_cupertino_control_center_mock_haptic_fail = 0;
int g_cupertino_control_center_mock_geometry_fail = 0;
int g_cupertino_control_center_mock_appearance_fail = 0;

static ui_error_t mock_button_base_create(struct ui_button_base **out_base) {
  if (g_cupertino_control_center_mock_button_create_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_create(out_base);
}
#undef ui_button_base_create
/** @cond */
#define ui_button_base_create mock_button_base_create
/** @endcond */

static ui_error_t
mock_slider_base_create(struct ui_slider_base **out_base,
                        struct ui_control_value_accessor *out_cva) {
  if (g_cupertino_control_center_mock_slider_create_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slider_base_create(out_base, out_cva);
}
#undef ui_slider_base_create
/** @cond */
#define ui_slider_base_create mock_slider_base_create
/** @endcond */

static ui_error_t mock_button_base_destroy(struct ui_button_base *button) {
  if (g_cupertino_control_center_mock_button_destroy_fail) {
    ui_button_base_destroy(button);
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_button_base_destroy(button);
}
#undef ui_button_base_destroy
/** @cond */
#define ui_button_base_destroy mock_button_base_destroy
/** @endcond */

static ui_error_t mock_slider_base_destroy(struct ui_slider_base *slider) {
  if (g_cupertino_control_center_mock_slider_destroy_fail) {
    ui_slider_base_destroy(slider);
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return ui_slider_base_destroy(slider);
}
#undef ui_slider_base_destroy
/** @cond */
#define ui_slider_base_destroy mock_slider_base_destroy
/** @endcond */

static ui_error_t
mock_cupertino_haptic_impact(enum cupertino_haptic_impact_style style,
                             float intensity) {
  if (g_cupertino_control_center_mock_haptic_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  return cupertino_haptic_impact(style, intensity);
}
#undef cupertino_haptic_impact
/** @cond */
#define cupertino_haptic_impact mock_cupertino_haptic_impact
/** @endcond */
#endif

#define CUPERTINO_CC_1X1_SIZE 60.0f
#define CUPERTINO_CC_2X1_WIDTH 128.0f
#define CUPERTINO_CC_2X1_HEIGHT 60.0f
#define CUPERTINO_CC_2X2_SIZE 128.0f
#define CUPERTINO_CC_EXPANDED_SLIDER_WIDTH 60.0f
#define CUPERTINO_CC_EXPANDED_SLIDER_HEIGHT 180.0f

static ui_error_t cupertino_control_center_update_geometry(
    struct cupertino_control_center_module *module) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_control_center_mock_geometry_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if (module->is_expanded && module->has_slider) {
    module->width = CUPERTINO_CC_EXPANDED_SLIDER_WIDTH;
    module->height = CUPERTINO_CC_EXPANDED_SLIDER_HEIGHT;
    module->corner_radius = 28.0f;
    return UI_ERROR_NONE;
  }

  switch (module->layout) {
  case CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_CIRCULAR:
    module->width = CUPERTINO_CC_1X1_SIZE;
    module->height = CUPERTINO_CC_1X1_SIZE;
    module->corner_radius = 30.0f;
    break;
  case CUPERTINO_CONTROL_CENTER_LAYOUT_1X1_SQUIRCLE:
    module->width = CUPERTINO_CC_1X1_SIZE;
    module->height = CUPERTINO_CC_1X1_SIZE;
    module->corner_radius = 18.0f;
    break;
  case CUPERTINO_CONTROL_CENTER_LAYOUT_2X1_PILL:
    module->width = CUPERTINO_CC_2X1_WIDTH;
    module->height = CUPERTINO_CC_2X1_HEIGHT;
    module->corner_radius = 18.0f;
    break;
  case CUPERTINO_CONTROL_CENTER_LAYOUT_2X2_MACRO:
  default:
    module->width = CUPERTINO_CC_2X2_SIZE;
    module->height = CUPERTINO_CC_2X2_SIZE;
    module->corner_radius = 20.0f;
    break;
  }

  return UI_ERROR_NONE;
}

static ui_error_t cupertino_control_center_update_appearance(
    struct cupertino_control_center_module *module) {
#ifdef UI_TEST_MOCK_ALLOC
  if (g_cupertino_control_center_mock_appearance_fail) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
#endif

  if (module->is_active) {
    module->background_color = module->active_tint;
  } else {
    /* Translucent dark squircle surface in iOS Control Center */
    module->background_color = UI_COLOR_ARGB(140, 44, 44, 46);
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_create(
    struct ui_engine *engine,
    const struct cupertino_control_center_module_descriptor *desc,
    struct cupertino_control_center_module **out_module) {
  struct cupertino_control_center_module *mod;
  ui_error_t rc;

  if (!engine || !desc || !out_module) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  mod = (struct cupertino_control_center_module *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_control_center_module));
  if (!mod) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(mod, 0, sizeof(*mod));
  mod->layout = desc->layout;
  mod->is_active = desc->is_active ? 1 : 0;
  mod->has_slider = desc->has_slider ? 1 : 0;
  mod->slider_value =
      (desc->initial_slider_value >= 0.0f && desc->initial_slider_value <= 1.0f)
          ? desc->initial_slider_value
          : 0.0f;

  if (desc->active_tint_color != 0) {
    mod->active_tint = desc->active_tint_color;
  } else {
    mod->active_tint = UI_COLOR_ARGB(255, 0, 122, 255); /* SystemBlue */
  }

  if (desc->title) {
#if defined(_MSC_VER)
    strncpy_s(mod->title, sizeof(mod->title), desc->title, _TRUNCATE);
#else
    strncpy(mod->title, desc->title, sizeof(mod->title) - 1);
    mod->title[sizeof(mod->title) - 1] = '\0';
#endif
  }

  if (desc->subtitle) {
#if defined(_MSC_VER)
    strncpy_s(mod->subtitle, sizeof(mod->subtitle), desc->subtitle, _TRUNCATE);
#else
    strncpy(mod->subtitle, desc->subtitle, sizeof(mod->subtitle) - 1);
    mod->subtitle[sizeof(mod->subtitle) - 1] = '\0';
#endif
  }

  if (desc->symbol_name) {
#if defined(_MSC_VER)
    strncpy_s(mod->symbol_name, sizeof(mod->symbol_name), desc->symbol_name,
              _TRUNCATE);
#else
    strncpy(mod->symbol_name, desc->symbol_name, sizeof(mod->symbol_name) - 1);
    mod->symbol_name[sizeof(mod->symbol_name) - 1] = '\0';
#endif
  }

  rc = cupertino_control_center_update_geometry(mod);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(mod);
    return rc;
  }

  rc = cupertino_control_center_update_appearance(mod);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(mod);
    return rc;
  }

  rc = ui_button_base_create(&mod->button_base);
  if (rc != UI_ERROR_NONE) {
    C_MULTIPLATFORM_FREE(mod);
    return rc;
  }

  if (mod->has_slider) {
    rc = ui_slider_base_create(&mod->slider_base, NULL);
    if (rc != UI_ERROR_NONE) {
      ui_button_base_destroy(mod->button_base);
      C_MULTIPLATFORM_FREE(mod);
      return rc;
    }
  }

  *out_module = mod;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_destroy(
    struct cupertino_control_center_module *module) {
  ui_error_t rc;

  if (!module) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (module->button_base) {
    rc = ui_button_base_destroy(module->button_base);
    if (rc != UI_ERROR_NONE) {
      /* Continue cleanup */
    }
    module->button_base = NULL;
  }

  if (module->slider_base) {
    rc = ui_slider_base_destroy(module->slider_base);
    if (rc != UI_ERROR_NONE) {
      /* Continue cleanup */
    }
    module->slider_base = NULL;
  }

  C_MULTIPLATFORM_FREE(module);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_set_active(
    struct cupertino_control_center_module *module, int is_active) {
  int new_active;
  ui_error_t rc;

  if (!module) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  new_active = is_active ? 1 : 0;
  if (module->is_active != new_active) {
    module->is_active = new_active;
    rc = cupertino_haptic_impact(CUPERTINO_HAPTIC_IMPACT_LIGHT, 1.0f);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
    rc = cupertino_control_center_update_appearance(module);
    if (rc != UI_ERROR_NONE) {
      return rc;
    }
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_is_active(
    const struct cupertino_control_center_module *module, int *out_is_active) {
  if (!module || !out_is_active) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_active = module->is_active;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_toggle(
    struct cupertino_control_center_module *module) {
  if (!module) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  return cupertino_control_center_module_set_active(module, !module->is_active);
}

ui_error_t cupertino_control_center_module_set_expanded(
    struct cupertino_control_center_module *module, int is_expanded) {
  if (!module) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  module->is_expanded = is_expanded ? 1 : 0;
  return cupertino_control_center_update_geometry(module);
}

ui_error_t cupertino_control_center_module_is_expanded(
    const struct cupertino_control_center_module *module,
    int *out_is_expanded) {
  if (!module || !out_is_expanded) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_is_expanded = module->is_expanded;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_set_slider_value(
    struct cupertino_control_center_module *module, float value) {
  if (!module || value < 0.0f || value > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  module->slider_value = value;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_get_slider_value(
    const struct cupertino_control_center_module *module, float *out_value) {
  if (!module || !out_value) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_value = module->slider_value;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_set_symbol_morph(
    struct cupertino_control_center_module *module, const char *symbol_from,
    const char *symbol_to, float progress) {
  if (!module || !symbol_from || !symbol_to || progress < 0.0f ||
      progress > 1.0f) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  strncpy_s(module->symbol_name, sizeof(module->symbol_name), symbol_from,
            _TRUNCATE);
  strncpy_s(module->symbol_morph_target, sizeof(module->symbol_morph_target),
            symbol_to, _TRUNCATE);
#else
  strncpy(module->symbol_name, symbol_from, sizeof(module->symbol_name) - 1);
  module->symbol_name[sizeof(module->symbol_name) - 1] = '\0';
  strncpy(module->symbol_morph_target, symbol_to,
          sizeof(module->symbol_morph_target) - 1);
  module->symbol_morph_target[sizeof(module->symbol_morph_target) - 1] = '\0';
#endif
  module->symbol_morph_progress = progress;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_get_symbol_morph(
    const struct cupertino_control_center_module *module, const char **out_from,
    const char **out_to, float *out_progress) {
  if (!module || !out_from || !out_to || !out_progress) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_from = module->symbol_name;
  *out_to = module->symbol_morph_target;
  *out_progress = module->symbol_morph_progress;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_get_bounds(
    const struct cupertino_control_center_module *module, float *out_w,
    float *out_h, float *out_radius) {
  if (!module || !out_w || !out_h || !out_radius) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_w = module->width;
  *out_h = module->height;
  *out_radius = module->corner_radius;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_get_button_base(
    const struct cupertino_control_center_module *module,
    struct ui_button_base **out_button) {
  if (!module || !out_button) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_button = module->button_base;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_module_get_slider_base(
    const struct cupertino_control_center_module *module,
    struct ui_slider_base **out_slider) {
  if (!module || !out_slider) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_slider = module->slider_base;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_grid_create(
    struct ui_engine *engine, struct cupertino_control_center_grid **out_grid) {
  struct cupertino_control_center_grid *grid;

  if (!engine || !out_grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  grid = (struct cupertino_control_center_grid *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct cupertino_control_center_grid));
  if (!grid) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  memset(grid, 0, sizeof(*grid));
  grid->padding = 16.0f;
  grid->spacing = 14.0f;
  grid->item_count = 0;

  *out_grid = grid;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_grid_destroy(
    struct cupertino_control_center_grid *grid) {
  if (!grid) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  C_MULTIPLATFORM_FREE(grid);
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_grid_add_module(
    struct cupertino_control_center_grid *grid,
    struct cupertino_control_center_module *module, int col, int row) {
  size_t idx;

  if (!grid || !module || col < 0 || row < 0) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (grid->item_count >= CUPERTINO_CONTROL_CENTER_MAX_GRID_MODULES) {
    return UI_ERROR_OUT_OF_MEMORY;
  }

  idx = grid->item_count;
  grid->items[idx].module = module;
  grid->items[idx].col = col;
  grid->items[idx].row = row;
  grid->item_count++;

  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_grid_get_module_count(
    const struct cupertino_control_center_grid *grid, size_t *out_count) {
  if (!grid || !out_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_count = grid->item_count;
  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_grid_get_module_at(
    const struct cupertino_control_center_grid *grid, size_t index,
    struct cupertino_control_center_module **out_module, int *out_col,
    int *out_row) {
  if (!grid || !out_module || index >= grid->item_count) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_module = grid->items[index].module;
  if (out_col) {
    *out_col = grid->items[index].col;
  }
  if (out_row) {
    *out_row = grid->items[index].row;
  }

  return UI_ERROR_NONE;
}

ui_error_t cupertino_control_center_grid_get_bounds(
    const struct cupertino_control_center_grid *grid, float *out_w,
    float *out_h) {
  size_t i;
  int max_col = -1;
  int max_row = -1;
  float total_w;
  float total_h;

  if (!grid || !out_w || !out_h) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  if (grid->item_count == 0) {
    *out_w = grid->padding * 2.0f;
    *out_h = grid->padding * 2.0f;
    return UI_ERROR_NONE;
  }

  for (i = 0; i < grid->item_count; i++) {
    int col_span = 1;
    int row_span = 1;

    if (grid->items[i].module) {
      if (grid->items[i].module->layout ==
          CUPERTINO_CONTROL_CENTER_LAYOUT_2X1_PILL) {
        col_span = 2;
      } else if (grid->items[i].module->layout ==
                 CUPERTINO_CONTROL_CENTER_LAYOUT_2X2_MACRO) {
        col_span = 2;
        row_span = 2;
      }
    }

    if (grid->items[i].col + col_span > max_col) {
      max_col = grid->items[i].col + col_span;
    }
    if (grid->items[i].row + row_span > max_row) {
      max_row = grid->items[i].row + row_span;
    }
  }

  total_w = (grid->padding * 2.0f) + ((float)max_col * CUPERTINO_CC_1X1_SIZE);
  if (max_col > 1) {
    total_w += (float)(max_col - 1) * grid->spacing;
  }
  total_h = (grid->padding * 2.0f) + ((float)max_row * CUPERTINO_CC_1X1_SIZE);
  if (max_row > 1) {
    total_h += (float)(max_row - 1) * grid->spacing;
  }

  *out_w = total_w;
  *out_h = total_h;
  return UI_ERROR_NONE;
}
