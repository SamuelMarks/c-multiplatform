/* clang-format off */
#include "m3_i18n.h"
#include <stddef.h>
#include <string.h>

int m3_i18n_init(material_catalog_state_t *state) {
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  state->is_rtl = 0; /* Default Left-to-Right */
  return MATERIAL_CATALOG_SUCCESS;
}

int m3_i18n_set_rtl(material_catalog_state_t *state, int is_rtl) {
  if (!state)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  if (state->is_rtl != is_rtl) {
    state->is_rtl = is_rtl;
    material_catalog_invalidate_ui(state);
  }
  return MATERIAL_CATALOG_SUCCESS;
}

static void mirror_node_layout(cmp_ui_node_t *node) {
  if (!node || !node->layout)
    return;

  /* Since Flexbox in cmp.h uses flex-direction column vs row natively,
     but doesn't have ROW_REVERSE explicitly, we can either manually
     reverse the children array during drawing, or map padding left/right. */
  
  /* Swap Left and Right Padding (1 = Right, 3 = Left) */
  {
    float temp = node->layout->padding[1];
    node->layout->padding[1] = node->layout->padding[3];
    node->layout->padding[3] = temp;
  }
  
  /* Swap Left and Right Margin */
  {
    float temp = node->layout->margin[1];
    node->layout->margin[1] = node->layout->margin[3];
    node->layout->margin[3] = temp;
  }

  /* Swap Left and Right absolute positioning offsets */
  if (node->layout->position_type == CMP_POSITION_ABSOLUTE) {
    /* Instead of swapping, we should re-calculate relative to parent,
       but for basic mirroring swapping right/left is a fallback */
    float temp = node->layout->position[1];
    node->layout->position[1] = node->layout->position[3];
    node->layout->position[3] = temp;
  }

  /* Note: Reverse children manually here if direction is ROW */
  if (node->layout->direction == CMP_FLEX_ROW) {
    size_t i, j;
    for (i = 0, j = node->child_count > 0 ? node->child_count - 1 : 0; i < j; ++i, --j) {
      cmp_ui_node_t *temp_child = node->children[i];
      node->children[i] = node->children[j];
      node->children[j] = temp_child;
    }
  }

  /* Check if it's an image node with a directional icon */
  if (node->type == 2 && node->properties) {
    /* If type=2 is an Image, and it's a directional icon we would mark it to be drawn mirrored.
       Since we lack a direct `transform: scaleX(-1)` attribute on `cmp_ui_node_t`,
       a real implementation would set a bit in `design_language_override` or similar. */
  }

  /* Recurse */
  {
    size_t i;
    for (i = 0; i < node->child_count; ++i) {
      mirror_node_layout(node->children[i]);
    }
  }
}

int m3_i18n_apply_rtl_mirroring(material_catalog_state_t *state, cmp_ui_node_t *root) {
  if (!state || !root)
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;

  if (state->is_rtl) {
    mirror_node_layout(root);
  }

  return MATERIAL_CATALOG_SUCCESS;
}

int m3_i18n_is_directional_icon(const char *svg_path) {
  if (!svg_path) return 0;
  
  if (strstr(svg_path, "arrow_back") != NULL ||
      strstr(svg_path, "arrow_forward") != NULL ||
      strstr(svg_path, "chevron_left") != NULL ||
      strstr(svg_path, "chevron_right") != NULL ||
      strstr(svg_path, "backspace") != NULL ||
      strstr(svg_path, "forward") != NULL ||
      strstr(svg_path, "reply") != NULL) {
    return 1;
  }
  
  return 0;
}
/* clang-format on */
