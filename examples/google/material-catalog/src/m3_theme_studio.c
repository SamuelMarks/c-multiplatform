/* clang-format off */
#include "material_catalog.h"
#include <string.h>
/* clang-format on */

int material_catalog_render_theme_studio(material_catalog_state_t *state,
                                         cmp_ui_node_t *container) {
  cmp_ui_node_t *box = NULL, *title = NULL, *color_picker = NULL,
                *save_btn = NULL;

  if (!state || !container) {
    return MATERIAL_CATALOG_ERROR_NULL_POINTER;
  }

  cmp_ui_box_create(&box);
  box->layout->direction = CMP_FLEX_COLUMN;
  box->layout->width = state->content_width; /* 100% */
  box->layout->padding[0] = dp_to_px(state, 16.0f);
  box->layout->padding[1] = dp_to_px(state, 16.0f);
  box->layout->padding[2] = dp_to_px(state, 16.0f);
  box->layout->padding[3] = dp_to_px(state, 16.0f);

  cmp_ui_text_create(&title, "Theme Studio", -1);
  /* Assume h4 or headline small */
  title->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(box, title);

  /* Color picker mock */
  cmp_ui_box_create(&color_picker);
  color_picker->type = 4; /* Text input mock for color hex */
  color_picker->layout->width = 200.0f;
  color_picker->layout->height = 56.0f;
  color_picker->layout->margin[2] = dp_to_px(state, 16.0f);
  cmp_ui_node_add_child(box, color_picker);

  /* Save button */
  cmp_ui_box_create(&save_btn);
  save_btn->type = 3; /* Button mock */
  save_btn->layout->width = 120.0f;
  save_btn->layout->height = 40.0f;
  cmp_ui_node_add_child(box, save_btn);

  cmp_ui_node_add_child(container, box);

  return MATERIAL_CATALOG_SUCCESS;
}
