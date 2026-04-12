/* clang-format off */
#include "m3_ripple.h"
#include <stddef.h>

static void ripple_event_handler(cmp_event_t *evt, cmp_ui_node_t *node, void *user_data) {
  material_catalog_state_t *state = (material_catalog_state_t *)user_data;

  if (evt == NULL || node == NULL)
    return;

  if (evt->action == CMP_ACTION_DOWN) {
    node->is_pressed = 1;
    node->is_hovered = 1;
    node->press_opacity = 0.10f; /* 10% pressed state layer */
    node->ripple_radius = 0.0f;
    node->ripple_x = (float)evt->x - node->layout->computed_rect.x;
    node->ripple_y = (float)evt->y - node->layout->computed_rect.y;
    
    if (state) {
      material_catalog_invalidate_ui(state);
    }
  } else if (evt->action == CMP_ACTION_UP ||
             evt->action == CMP_ACTION_CANCEL) {
    node->is_pressed = 0;
    node->hover_opacity = 0.08f; /* 8% hovered state layer */
    
    if (state) {
      material_catalog_invalidate_ui(state);
    }
  } else if (evt->action == CMP_ACTION_MOVE) {
    /* If pointer moved out of bounds, clear hover */
    if (evt->x < node->layout->computed_rect.x || 
        evt->x > node->layout->computed_rect.x + node->layout->computed_rect.width ||
        evt->y < node->layout->computed_rect.y || 
        evt->y > node->layout->computed_rect.y + node->layout->computed_rect.height) {
      node->is_hovered = 0;
      node->is_pressed = 0;
    } else {
      node->is_hovered = 1;
      node->hover_opacity = 0.08f;
    }
    
    if (state) {
      material_catalog_invalidate_ui(state);
    }
  }
}

int material_catalog_apply_ripple(material_catalog_state_t *state,
                                  cmp_ui_node_t *node) {
  if (!node)
    return 1; /* MATERIAL_CATALOG_ERROR_NULL_POINTER */

  cmp_ui_node_add_event_listener(node, CMP_EVENT_TYPE_MOUSE, 0,
                                 ripple_event_handler, state);
  cmp_ui_node_add_event_listener(node, CMP_EVENT_TYPE_TOUCH, 0,
                                 ripple_event_handler, state);

  return 0; /* MATERIAL_CATALOG_SUCCESS */
}
/* clang-format on */
