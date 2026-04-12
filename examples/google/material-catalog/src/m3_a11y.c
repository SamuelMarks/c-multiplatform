/* clang-format off */
#include "m3_a11y.h"
#include <stddef.h>

int m3_a11y_init(material_catalog_state_t *state) {
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  /* Already initialized state->a11y_tree during general init */
  return MATERIAL_CATALOG_SUCCESS;
}

int m3_a11y_set_content_description(material_catalog_state_t *state,
                                    cmp_ui_node_t *node, const char *role,
                                    const char *content_description) {
  if (!state || !node || !role || !content_description)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  /* Use the generic material catalog mapping that delegates to cmp_a11y_tree_add_node */
  return material_catalog_map_semantic_node(state, node, content_description, role);
}

int m3_a11y_set_state(material_catalog_state_t *state, cmp_ui_node_t *node,
                      const char *property, const char *value) {
  if (!state || !node || !state->a11y_tree)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  /* Mapping this property to the a11y tree node trait/property */
  /* cmp_a11y_tree_set_node_traits or direct state injection */
  (void)property; /* Mock */
  (void)value;    /* Mock */
  return MATERIAL_CATALOG_SUCCESS;
}

int m3_a11y_announce_state_change(material_catalog_state_t *state,
                                  const char *announcement) {
  if (!state || !state->a11y_tree || !announcement)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  cmp_a11y_post_announcement(state->a11y_tree, announcement);
  return MATERIAL_CATALOG_SUCCESS;
}

int m3_a11y_apply_high_contrast(material_catalog_state_t *state) {
  cmp_color_t black;
  cmp_color_t white;

  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  black.r = 0.0f;
  black.g = 0.0f;
  black.b = 0.0f;
  black.a = 1.0f;
  black.space = CMP_COLOR_SPACE_SRGB;

  white.r = 1.0f;
  white.g = 1.0f;
  white.b = 1.0f;
  white.a = 1.0f;
  white.space = CMP_COLOR_SPACE_SRGB;

  /* Flatten primary and secondary colors to deep blacks and whites */
  /* High contrast light mode: very dark text, white background */
  /* High contrast dark mode: very light text, black background */
  if (state->current_theme == CATALOG_THEME_DARK) {
    state->sys_colors.primary = black; /* Mock deep colors for test */
    state->sys_colors.on_primary = white;
  } else {
    state->sys_colors.primary = black;
    state->sys_colors.on_primary = white;
  }

  return MATERIAL_CATALOG_SUCCESS;
}
/* clang-format on */
