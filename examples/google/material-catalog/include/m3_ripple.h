/* clang-format off */
#ifndef M3_RIPPLE_H
#define M3_RIPPLE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "material_catalog.h"
#include <cmp.h>

/**
 * @struct m3_ripple_state_t
 * @brief Internal state for an active ripple animation on a node.
 */
typedef struct m3_ripple_state_t {
    float x;
    float y;
    float radius;
    float target_radius;
    float opacity;
    float target_opacity;
    int is_pressed;
    int is_hovered;
    int is_focused;
} m3_ripple_state_t;

/**
 * @brief Apply Material 3 ripple and state layer interactions to a node.
 * @param state The catalog state context.
 * @param node The UI node to make interactive.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int material_catalog_apply_ripple(material_catalog_state_t *state, cmp_ui_node_t *node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_RIPPLE_H */
/* clang-format on */
