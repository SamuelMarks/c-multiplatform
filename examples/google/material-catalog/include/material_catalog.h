/* clang-format off */
#ifndef MATERIAL_CATALOG_H
#define MATERIAL_CATALOG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cmp.h>
#include <themes/cmp_material3_color.h>
#include <themes/cmp_material3_navigation.h>
#include <themes/cmp_material3_communication.h>
#include <themes/cmp_material3_components.h>
#include <themes/cmp_material3_containment.h>
#include <themes/cmp_material3_selection_controls.h>
#include <themes/cmp_material3_information.h>
#include <themes/cmp_material3_pickers_menus.h>

/* clang-format on */

/**
 * @file material_catalog.h
 * @brief Material Catalog Application logic for C-Multiplatform.
 */

/**
 * @enum material_catalog_error_t
 * @brief Error codes for Material Catalog operations.
 */
typedef enum material_catalog_error_t {
  MATERIAL_CATALOG_SUCCESS = 0,
  MATERIAL_CATALOG_ERROR_NULL_POINTER = 1,
  MATERIAL_CATALOG_ERROR_INITIALIZATION_FAILED = 2,
  MATERIAL_CATALOG_ERROR_OUT_OF_MEMORY = 3
} material_catalog_error_t;

/**
 * @enum catalog_theme_t
 * @brief Available themes for the Material Catalog.
 */
typedef enum catalog_theme_t {
  CATALOG_THEME_SYSTEM = 0,
  CATALOG_THEME_LIGHT = 1,
  CATALOG_THEME_DARK = 2
} catalog_theme_t;

/**
 * @enum catalog_palette_id_t
 * @brief Base color palettes for the Material Catalog.
 */
typedef enum catalog_palette_id_t {
  CATALOG_PALETTE_BLUE = 0,
  CATALOG_PALETTE_BROWN = 1,
  CATALOG_PALETTE_GREEN = 2,
  CATALOG_PALETTE_INDIGO = 3,
  CATALOG_PALETTE_ORANGE = 4,
  CATALOG_PALETTE_PINK = 5,
  CATALOG_PALETTE_PURPLE = 6,
  CATALOG_PALETTE_RED = 7,
  CATALOG_PALETTE_TEAL = 8,
  CATALOG_PALETTE_YELLOW = 9
} catalog_palette_id_t;

/**
 * @enum theme_font_family_t
 * @brief Material Catalog font family types.
 */
typedef enum theme_font_family_t {
  THEME_FONT_DEFAULT = 0,
  THEME_FONT_SANS_SERIF = 1,
  THEME_FONT_SERIF = 2,
  THEME_FONT_MONOSPACE = 3,
  THEME_FONT_CURSIVE = 4
} theme_font_family_t;

/**
 * @enum theme_typography_scale_t
 * @brief Material Typography scales.
 */
typedef enum theme_typography_scale_t {
  THEME_TYPOGRAPHY_H1 = 0,
  THEME_TYPOGRAPHY_H2 = 1,
  THEME_TYPOGRAPHY_H3 = 2,
  THEME_TYPOGRAPHY_H4 = 3,
  THEME_TYPOGRAPHY_H5 = 4,
  THEME_TYPOGRAPHY_H6 = 5,
  THEME_TYPOGRAPHY_SUBTITLE1 = 6,
  THEME_TYPOGRAPHY_SUBTITLE2 = 7,
  THEME_TYPOGRAPHY_BODY1 = 8,
  THEME_TYPOGRAPHY_BODY2 = 9,
  THEME_TYPOGRAPHY_BUTTON = 10,
  THEME_TYPOGRAPHY_CAPTION = 11,
  THEME_TYPOGRAPHY_OVERLINE = 12
} theme_typography_scale_t;

/**
 * @struct theme_typography_style_t
 * @brief Specific typography mapping for a scale.
 */
typedef struct theme_typography_style_t {
  float font_size;
  float letter_spacing;
  int font_weight;
  theme_font_family_t family;
} theme_typography_style_t;

/**
 * @enum theme_shape_corner_family_t
 * @brief Material Catalog shape corner types.
 */
typedef enum theme_shape_corner_family_t {
  THEME_SHAPE_ROUNDED = 0,
  THEME_SHAPE_CUT = 1
} theme_shape_corner_family_t;

/**
 * @enum theme_shape_size_t
 * @brief Material Catalog shape size constraints.
 */
typedef enum theme_shape_size_t {
  THEME_SHAPE_SMALL = 0,  /* Max 16dp */
  THEME_SHAPE_MEDIUM = 1, /* Max 32dp */
  THEME_SHAPE_LARGE = 2   /* Max 48dp */
} theme_shape_size_t;

/**
 * @struct material_theme_t
 * @brief Persistable theme configuration.
 */
typedef struct material_theme_t {
  catalog_theme_t mode;
  catalog_palette_id_t palette_id;
  theme_font_family_t font_family;
  theme_shape_corner_family_t shape_family;
} material_theme_t;

/**
 * @struct theme_saver_t
 * @brief Handles saving and loading themes to disk.
 */
typedef struct theme_saver_t {
  const char *config_file_path;
  material_theme_t current;
} theme_saver_t;

/**
 * @struct material_component_t
 * @brief Material component metadata.
 */
typedef struct material_component_t {
  int id;
  const char *name;
  const char *description;
  int icon_resource_id;
  int tint_icon_flag;
  const char *guidelines_url;
  const char *docs_url;
  const char *source_url;
} material_component_t;

/**
 * @struct material_example_t
 * @brief Example metadata for a specific material component.
 */
typedef struct material_example_t {
  int component_id;
  const char *name;
  const char *description;
  const char *source_url;
  void (*render)(cmp_ui_node_t *container);
} material_example_t;

/**
 * @enum catalog_screen_id_t
 * @brief Screens available in the navigation stack.
 */
typedef enum catalog_screen_id_t {
  CATALOG_SCREEN_HOME = 0,
  CATALOG_SCREEN_COMPONENT_DETAILS = 1,
  CATALOG_SCREEN_EXAMPLE_VIEWER = 2
} catalog_screen_id_t;

/**
 * @struct catalog_route_t
 * @brief A single route entry in the navigation stack.
 */
typedef struct catalog_route_t {
  catalog_screen_id_t screen_id;
  int component_id;
  int example_index;
} catalog_route_t;

#define CATALOG_MAX_BACK_STACK 16

/**
 * @struct catalog_router_t
 * @brief Manages the navigation stack for the catalog.
 */
typedef struct catalog_router_t {
  catalog_route_t stack[CATALOG_MAX_BACK_STACK];
  int stack_size;
} catalog_router_t;

/**
 * @struct material_catalog_state_t
 * @brief Application state for the Material Catalog.
 */
typedef struct catalog_state_t {
  cmp_window_t *window;
  cmp_ui_node_t *root_node;

  catalog_router_t router;
  catalog_theme_t current_theme;
  float window_width;
  float window_height;

  cmp_rect_t window_insets; /* Safe areas padding */

  cmp_arena_t ui_arena;
  cmp_arena_t state_arena;

  int is_ui_dirty; /* Flag to trigger recomposition */

  cmp_a11y_tree_t *a11y_tree; /* Accessibility tree graph context */
  int is_rtl;                 /* Right-to-Left writing mode flag */
} catalog_state_t;

typedef catalog_state_t material_catalog_state_t;

/**
 * @brief Initializes the Material Catalog application state.
 *
 * @param state Pointer to the state structure to initialize.
 * @return material_catalog_error_t MATERIAL_CATALOG_SUCCESS on success, or an
 * error code.
 */
int material_catalog_init(material_catalog_state_t *state);

/**
 * @brief Creates the main UI tree for the Material Catalog.
 *
 * @param state Pointer to the initialized state structure.
 * @return material_catalog_error_t MATERIAL_CATALOG_SUCCESS on success, or an
 * error code.
 */
int material_catalog_create_ui(material_catalog_state_t *state);

/**
 * @brief Invalidates the UI, causing a recomposition/redraw on the next frame.
 *
 * @param state Pointer to the catalog state struct.
 */
void material_catalog_invalidate_ui(material_catalog_state_t *state);

/**
 * @brief Navigates to a specific screen, pushing it onto the back stack.
 *
 * @param state Pointer to the catalog state struct.
 * @param screen_id The ID of the screen to navigate to.
 * @param component_id The associated component ID (if applicable).
 * @param example_index The associated example index (if applicable).
 */
void material_catalog_navigate_to(material_catalog_state_t *state,
                                  catalog_screen_id_t screen_id,
                                  int component_id, int example_index);

/**
 * @brief Navigates back to the previous screen.
 *
 * @param state Pointer to the catalog state struct.
 */
void material_catalog_navigate_back(material_catalog_state_t *state);

/**
 * @brief Rebuilds the UI tree based on the current state.
 *
 * @param state Pointer to the catalog state struct.
 * @return material_catalog_error_t MATERIAL_CATALOG_SUCCESS on success, or an
 * error code.
 */
int material_catalog_recompose_ui(material_catalog_state_t *state);

/**
 * @brief Runs the main application loop.
 *
 * @param state Pointer to the initialized state structure.
 * @return material_catalog_error_t MATERIAL_CATALOG_SUCCESS on success, or an
 * error code.
 */
int material_catalog_run(material_catalog_state_t *state);

/**
 * @brief Cleans up the Material Catalog application state.
 *
 * @param state Pointer to the state structure to clean up.
 * @return material_catalog_error_t MATERIAL_CATALOG_SUCCESS on success, or an
 * error code.
 */
int material_catalog_cleanup(material_catalog_state_t *state);

/**
 * @brief Retrieves the global list of material components.
 *
 * @param out_count Pointer to store the number of components.
 * @return const material_component_t* Pointer to the array of components.
 */
const material_component_t *material_catalog_get_components(int *out_count);

/**
 * @brief Retrieves the examples for a given component.
 *
 * @param component_id The ID of the component.
 * @param out_count Pointer to store the number of examples.
 * @return const material_example_t* Pointer to the array of examples.
 */
const material_example_t *material_catalog_get_examples(int component_id,
                                                        int *out_count);

/**
 * @brief Checks if a color is considered light (high luminance).
 * @param color The color to check.
 * @param out_is_light Pointer to a boolean result (1 if light, 0 if dark).
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_is_light_color(const cmp_color_t *color,
                                    int *out_is_light);

/**
 * @brief Determines the appropriate "On" color (black or white) for text on top
 * of the given background color.
 * @param bg_color The background color.
 * @param out_on_color Pointer to the resulting color.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_get_on_color(const cmp_color_t *bg_color,
                                  cmp_color_t *out_on_color);

/**
 * @brief Calculates the contrast ratio between two colors.
 * @param c1 First color.
 * @param c2 Second color.
 * @param out_ratio Pointer to the resulting float ratio.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_get_contrast_ratio(const cmp_color_t *c1,
                                        const cmp_color_t *c2,
                                        float *out_ratio);

/**
 * @brief Generates a full Material 3 palette for the given color and theme.
 * @param id The base palette id.
 * @param theme Light or Dark theme.
 * @param out_palette Pointer to the resulting palette.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_get_palette(catalog_palette_id_t id, catalog_theme_t theme,
                                 struct cmp_palette *out_palette);

/**
 * @brief Retrieves the specific typography styling for a given scale.
 * @param scale The typography scale to retrieve.
 * @param out_style Pointer to the resulting style.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_get_typography_style(theme_typography_scale_t scale,
                                          theme_typography_style_t *out_style);

/**
 * @brief Loads required typography fonts into memory via c-fs.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_load_fonts(void);

/**
 * @brief Resolves the target corner radius or cut size based on shape size.
 * @param size The shape size category.
 * @param out_radius Pointer to the resulting value.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_get_shape_radius(theme_shape_size_t size,
                                      float *out_radius);

/**
 * @brief Retrieves standard system window insets for safe rendering.
 * @param state Pointer to the catalog state struct.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_update_insets(material_catalog_state_t *state);

/**
 * @brief Renders the common Material Top App Bar.
 * @param state Pointer to the catalog state.
 * @param container The container node to attach the app bar to.
 * @param title The title text.
 * @param show_back_button True to render a back navigation icon.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_top_app_bar(material_catalog_state_t *state,
                                        cmp_ui_node_t *container,
                                        const char *title,
                                        int show_back_button);

/**
 * @brief Handles URL launching (simulated abstract http/OS open).
 * @param url The URL string to launch.
 */
void material_catalog_launch_url(const char *url);

/**
 * @brief Renders a transparent scrim blocking underlying input.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_scrim(material_catalog_state_t *state,
                                  cmp_ui_node_t *container);

/**
 * @brief Renders a More (Kebab) menu for the Top App Bar.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_more_menu(material_catalog_state_t *state,
                                      cmp_ui_node_t *container);

/**
 * @brief Applies the interactive Material Ripple effect and state layer logic
 * to a node.
 * @param node The node to make interactive.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_apply_ripple(cmp_ui_node_t *node);

/**
 * @brief Renders the Home Screen (Grid of components).
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_home_screen(material_catalog_state_t *state,
                                        cmp_ui_node_t *container);

/**
 * @brief Renders the Component Details Screen.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @param component_id The ID of the component to detail.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_component_details(material_catalog_state_t *state,
                                              cmp_ui_node_t *container,
                                              int component_id);

/**
 * @brief Creates a generic scrollable list container.
 * @param state Pointer to the catalog state.
 * @param out_scroll_node Pointer to receive the scroll node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_create_scroll_view(material_catalog_state_t *state,
                                        cmp_ui_node_t **out_scroll_node);

/**
 * @brief Renders the "App bars: top" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_top_app_bar_example(material_catalog_state_t *state,
                                                cmp_ui_node_t *container);

/**
 * @brief Renders the "App bars: bottom" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_bottom_app_bar_example(
    material_catalog_state_t *state, cmp_ui_node_t *container);

/**
 * @brief Renders the "Theme Picker" overlay settings menu.
 * @param state Pointer to the catalog state.
 * @param container The container node (assumed modal bottom sheet).
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_theme_picker(material_catalog_state_t *state,
                                         cmp_ui_node_t *container);

/**
 * @brief Renders the "Backdrop" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_backdrop_example(material_catalog_state_t *state,
                                             cmp_ui_node_t *container);

/**
 * @brief Renders the "Badges" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_badges_example(material_catalog_state_t *state,
                                           cmp_ui_node_t *container);

/**
 * @brief Renders the "Bottom navigation" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_bottom_navigation_example(
    material_catalog_state_t *state, cmp_ui_node_t *container);

/**
 * @brief Renders the "Navigation rail" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_navigation_rail_example(
    material_catalog_state_t *state, cmp_ui_node_t *container);

/**
 * @brief Renders the "Buttons" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_buttons_example(material_catalog_state_t *state,
                                            cmp_ui_node_t *container);

/**
 * @brief Renders the "Buttons: floating action button" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_fabs_example(material_catalog_state_t *state,
                                         cmp_ui_node_t *container);

/**
 * @brief Renders the "Cards" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_cards_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container);

/**
 * @brief Renders the "Checkboxes" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_checkboxes_example(material_catalog_state_t *state,
                                               cmp_ui_node_t *container);

/**
 * @brief Renders the "Radio buttons" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_radio_buttons_example(
    material_catalog_state_t *state, cmp_ui_node_t *container);

/**
 * @brief Renders the "Switches" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_switches_example(material_catalog_state_t *state,
                                             cmp_ui_node_t *container);

/**
 * @brief Renders the "Chips" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_chips_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container);

/**
 * @brief Renders the "Dialogs" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_dialogs_example(material_catalog_state_t *state,
                                            cmp_ui_node_t *container);

/**
 * @brief Renders the "Lists" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_lists_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container);

/**
 * @brief Renders the "Menus" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_menus_example(material_catalog_state_t *state,
                                          cmp_ui_node_t *container);

/**
 * @brief Renders the "Drawers & Sheets" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_drawers_sheets_example(
    material_catalog_state_t *state, cmp_ui_node_t *container);

/**
 * @brief Renders the "Progress indicators" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_progress_indicators_example(
    material_catalog_state_t *state, cmp_ui_node_t *container);

/**
 * @brief Renders the "Sliders" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_sliders_example(material_catalog_state_t *state,
                                            cmp_ui_node_t *container);

/**
 * @brief Renders the "Snackbars" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_snackbars_example(material_catalog_state_t *state,
                                              cmp_ui_node_t *container);

/**
 * @brief Renders the "Tabs" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_tabs_example(material_catalog_state_t *state,
                                         cmp_ui_node_t *container);

/**
 * @brief Renders the "Text fields" examples.
 * @param state Pointer to the catalog state.
 * @param container The container node.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_render_text_fields_example(material_catalog_state_t *state,
                                                cmp_ui_node_t *container);

/**
 * @brief Utility for cut corner path generation on CMP Nodes (stub).
 * @param node The UI node to apply the path to.
 * @param family The family of the corner (rounded/cut).
 * @param radius The radius/cut distance to apply.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_apply_shape(cmp_ui_node_t *node,
                                 theme_shape_corner_family_t family,
                                 float radius);

/**
 * @brief Saves the current theme settings to disk.
 * @param saver Pointer to the theme saver configuration.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_save_theme(theme_saver_t *saver);

/**
 * @brief Loads theme settings from disk.
 * @param saver Pointer to the theme saver configuration.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_load_theme(theme_saver_t *saver);

/**
 * @brief Ensures the UI node is populated with the correct semantic
 * descriptions for screen readers.
 * @param state Pointer to the catalog state.
 * @param node The UI node to map.
 * @param name The descriptive name.
 * @param role The ARIA role string.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_map_semantic_node(material_catalog_state_t *state,
                                       cmp_ui_node_t *node, const char *name,
                                       const char *role);

/**
 * @brief Utility for marking nodes hidden if they fall outside the visible
 * scroll port (culling).
 * @param state Pointer to the catalog state.
 * @param scroll_view The container node handling scroll offsets.
 * @return MATERIAL_CATALOG_SUCCESS or error code.
 */
int material_catalog_apply_viewport_culling(material_catalog_state_t *state,
                                            cmp_ui_node_t *scroll_view);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MATERIAL_CATALOG_H */