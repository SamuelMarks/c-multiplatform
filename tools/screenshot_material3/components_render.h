/**
 * @file components_render.h
 * @brief High-level Material 3 component rendering module for screenshot
 * generation.
 */

#ifndef MD3_COMPONENTS_RENDER_H
#define MD3_COMPONENTS_RENDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "material3/md3_color.h"
#include "ui_error.h"

/**
 * @struct render_options
 * @brief Rendering options for Material 3 screenshot generation.
 */
struct render_options {
  const char *output_dir; /**< Directory where screenshots are saved */
  int is_dark;            /**< 1 for dark theme, 0 for light theme */
  float dpi_scale; /**< Device Pixel Ratio scale factor (e.g. 1.0f or 2.0f) */
};

/**
 * @brief Renders all Material 3 button variants and saves screenshots.
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_buttons(const struct md3_color_scheme *scheme,
                              const struct render_options *opts);

/**
 * @brief Renders Material 3 Datepicker, Date Range Picker, and Timepicker.
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_pickers(const struct md3_color_scheme *scheme,
                              const struct render_options *opts);

/**
 * @brief Renders Material 3 selection controls (Checkbox, Radio, Switch,
 * Slider, etc.).
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_selection_controls(const struct md3_color_scheme *scheme,
                                         const struct render_options *opts);

/**
 * @brief Renders Material 3 containment & cards (Cards, Chips, Badge, Avatar,
 * List, etc.).
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
render_all_containment_and_cards(const struct md3_color_scheme *scheme,
                                 const struct render_options *opts);

/**
 * @brief Renders Material 3 navigation components (Navigation Bar, Rail,
 * Drawer, App Bars, Tabs, etc.).
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_navigation(const struct md3_color_scheme *scheme,
                                 const struct render_options *opts);

/**
 * @brief Renders Material 3 overlays, dialogs, menus, and sheets.
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t
render_all_overlays_and_dialogs(const struct md3_color_scheme *scheme,
                                const struct render_options *opts);

/**
 * @brief Renders Material 3 data & hierarchy components (Table, Paginator,
 * Tree, etc.).
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_data_and_hierarchy(const struct md3_color_scheme *scheme,
                                         const struct render_options *opts);

/**
 * @brief Renders Material 3 media & workspace components (Chat, Timeline,
 * Uploader, Editor, etc.).
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_media_and_workspace(const struct md3_color_scheme *scheme,
                                          const struct render_options *opts);

/**
 * @brief Renders Material 3 progress indicators and expressive shapes.
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_all_progress_and_shapes(const struct md3_color_scheme *scheme,
                                          const struct render_options *opts);

/**
 * @brief Renders a comprehensive Material 3 Catalog hero overview composite
 * image.
 * @param scheme Color scheme.
 * @param opts Render options.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t render_catalog_overview(const struct md3_color_scheme *scheme,
                                   const struct render_options *opts);

#ifdef __cplusplus
}
#endif

#endif /* MD3_COMPONENTS_RENDER_H */
