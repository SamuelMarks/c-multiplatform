/* clang-format off */
#ifndef M3_A11Y_H
#define M3_A11Y_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "material_catalog.h"
#include <cmp.h>

/**
 * @brief Initialize the accessibility tree for the catalog.
 * @param state The catalog state.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_a11y_init(material_catalog_state_t *state);

/**
 * @brief Map a UI component to the a11y semantic tree with a role and description.
 * @param state The catalog state.
 * @param node The UI node to map.
 * @param role The ARIA role string (e.g., "button", "checkbox", "tab").
 * @param content_description The localized string describing the component.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_a11y_set_content_description(material_catalog_state_t *state, cmp_ui_node_t *node, const char *role, const char *content_description);

/**
 * @brief Map an interaction state change (e.g. checked, focused) to the a11y tree.
 * @param state The catalog state.
 * @param node The UI node.
 * @param property The ARIA property to set (e.g., "aria-checked").
 * @param value The value (e.g., "true", "false", "mixed").
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_a11y_set_state(material_catalog_state_t *state, cmp_ui_node_t *node, const char *property, const char *value);

/**
 * @brief Announce a state change to the screen reader.
 * @param state The catalog state.
 * @param announcement The text to announce.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_a11y_announce_state_change(material_catalog_state_t *state, const char *announcement);

/**
 * @brief Applies high contrast mode overrides to the current theme palettes.
 * @param state The catalog state.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_a11y_apply_high_contrast(material_catalog_state_t *state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_A11Y_H */
/* clang-format on */
