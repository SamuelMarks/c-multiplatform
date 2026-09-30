/**
 * @file ds_material3.c
 * @brief Material 3 design system screenshot driver.
 */

/* clang-format off */
#include "design_system_renderer.h"
#include "material3/md3_color.h"
#include "../screenshot_material3/components_render.h"
#include <stdio.h>
/* clang-format on */

static ui_error_t
material3_render_all_driver(const struct design_system_options *opts) {
  struct md3_color_scheme scheme;
  ui_color_t seed = UI_COLOR_ARGB(255, 103, 80, 164);
  struct render_options m3_opts;
  ui_error_t rc;

  m3_opts.output_dir = opts->output_dir;
  m3_opts.is_dark = opts->is_dark;

  rc = md3_color_scheme_create(seed, opts->is_dark, MD3_PALETTE_MODE_TONAL_SPOT,
                               &scheme);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  render_all_buttons(&scheme, &m3_opts);
  render_all_pickers(&scheme, &m3_opts);
  render_all_selection_controls(&scheme, &m3_opts);
  render_all_containment_and_cards(&scheme, &m3_opts);
  render_all_navigation(&scheme, &m3_opts);
  render_all_overlays_and_dialogs(&scheme, &m3_opts);
  render_all_data_and_hierarchy(&scheme, &m3_opts);
  render_all_media_and_workspace(&scheme, &m3_opts);
  render_all_progress_and_shapes(&scheme, &m3_opts);
  render_catalog_overview(&scheme, &m3_opts);

  return UI_ERROR_NONE;
}

const struct design_system_driver g_driver_material3 = {
    "material3", "Material 3 & Expressive",
    "Google Material 3 design system with dynamic tonal spot palettes and "
    "expressive shapes",
    material3_render_all_driver};
