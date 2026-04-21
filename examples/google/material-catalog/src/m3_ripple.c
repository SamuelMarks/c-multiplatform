/* clang-format off */
#include "m3_ripple.h"
#include "cmp_ink_ripple.h"
#include <stddef.h>

#define MAX_ACTIVE_RIPPLES 64
static struct {
    cmp_ui_node_t *node;
    cmp_ink_ripple_t *ctx;
} g_active_ripples[MAX_ACTIVE_RIPPLES];

static void add_ripple_tracking(cmp_ui_node_t *node, cmp_ink_ripple_t *ripple) {
    int i;
    for (i = 0; i < MAX_ACTIVE_RIPPLES; i++) {
        if (g_active_ripples[i].node == NULL) {
            g_active_ripples[i].node = node;
            g_active_ripples[i].ctx = ripple;
            break;
        }
    }
}

static cmp_ink_ripple_t *get_ripple_for_node(cmp_ui_node_t *node) {
    int i;
    for (i = 0; i < MAX_ACTIVE_RIPPLES; i++) {
        if (g_active_ripples[i].node == node) {
            return g_active_ripples[i].ctx;
        }
    }
    return NULL;
}

static void remove_ripple_tracking(cmp_ui_node_t *node) {
    int i;
    for (i = 0; i < MAX_ACTIVE_RIPPLES; i++) {
        if (g_active_ripples[i].node == node) {
            cmp_ink_ripple_destroy(g_active_ripples[i].ctx);
            g_active_ripples[i].node = NULL;
            g_active_ripples[i].ctx = NULL;
        }
    }
}

void material_catalog_update_ripples(material_catalog_state_t *state, float dt_ms) {
    int i;
    for (i = 0; i < MAX_ACTIVE_RIPPLES; i++) {
        if (g_active_ripples[i].node != NULL) {
            cmp_ink_ripple_t *r = g_active_ripples[i].ctx;
            float radius = 0.0f;
            float opacity = 0.0f;
            float x = 0.0f;
            float y = 0.0f;
            
            cmp_ink_ripple_update(r, dt_ms);
            cmp_ink_ripple_get_state(r, &radius, &opacity, &x, &y);
            
            g_active_ripples[i].node->ripple_radius = radius;
            g_active_ripples[i].node->ripple_x = x;
            g_active_ripples[i].node->ripple_y = y;
            
            /* Only apply opacity if the ripple is visible */
            if (opacity > 0.0f || radius > 0.0f) {
                g_active_ripples[i].node->press_opacity = opacity;
                if (state) material_catalog_invalidate_paint(state);
            }
            
            if (opacity <= 0.01f && g_active_ripples[i].node->is_pressed == 0) {
                /* Finished fading, remove tracking */
                g_active_ripples[i].node->press_opacity = 0.0f;
                remove_ripple_tracking(g_active_ripples[i].node);
            }
        }
    }
}

static void ripple_event_handler(cmp_event_t *evt, cmp_ui_node_t *node, void *user_data) {
  material_catalog_state_t *state = (material_catalog_state_t *)user_data;
  cmp_ink_ripple_t *ripple;

  if (evt == NULL || node == NULL)
    return;

  if (evt->action == CMP_ACTION_DOWN) {
    node->is_pressed = 1;
    node->is_hovered = 1;
    node->hover_opacity = 0.08f; /* 8% hovered state layer */

    /* Remove any old ripple */
    remove_ripple_tracking(node);

    if (cmp_ink_ripple_create(&ripple) == 0) {
        float rx = (float)evt->x - node->layout->computed_rect.x;
        float ry = (float)evt->y - node->layout->computed_rect.y;
        cmp_ink_ripple_trigger(ripple, rx, ry, node->layout->computed_rect.width, node->layout->computed_rect.height);
        add_ripple_tracking(node, ripple);
    }

    if (state) {
      material_catalog_invalidate_paint(state);
    }
  } else if (evt->action == CMP_ACTION_UP ||
             evt->action == CMP_ACTION_CANCEL) {
    node->is_pressed = 0;
    
    ripple = get_ripple_for_node(node);
    if (ripple) {
        cmp_ink_ripple_release(ripple);
    }

    if (state) {
      material_catalog_invalidate_paint(state);
    }
  } else if (evt->action == CMP_ACTION_MOVE) {
    /* If pointer moved out of bounds, clear hover */
    if (evt->x < node->layout->computed_rect.x ||
        evt->x > node->layout->computed_rect.x + node->layout->computed_rect.width ||
        evt->y < node->layout->computed_rect.y ||
        evt->y > node->layout->computed_rect.y + node->layout->computed_rect.height) {
      node->is_hovered = 0;
      node->hover_opacity = 0.0f;
    } else {
      node->is_hovered = 1;
      node->hover_opacity = 0.08f;
    }

    if (state) {
      material_catalog_invalidate_paint(state);
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