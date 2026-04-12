/* clang-format off */
#ifndef M3_I18N_H
#define M3_I18N_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "material_catalog.h"
#include <cmp.h>

/**
 * @brief Initialize internationalization features for the catalog.
 * @param state The catalog state.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_i18n_init(material_catalog_state_t *state);

/**
 * @brief Sets the layout mirroring direction.
 * @param state The catalog state.
 * @param is_rtl 1 for RTL (Right-to-Left), 0 for LTR (Left-to-Right).
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_i18n_set_rtl(material_catalog_state_t *state, int is_rtl);

/**
 * @brief Traverses the UI tree to flip directional layout parameters automatically.
 * @param state The catalog state.
 * @param root The root node to traverse.
 * @return MATERIAL_CATALOG_SUCCESS or an error code.
 */
int m3_i18n_apply_rtl_mirroring(material_catalog_state_t *state, cmp_ui_node_t *root);

/**
 * @brief Helper to determine if an SVG icon path requires mirroring in RTL mode.
 * @param svg_path The path to the SVG asset.
 * @return 1 if directional (should be flipped), 0 otherwise.
 */
int m3_i18n_is_directional_icon(const char *svg_path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* M3_I18N_H */
/* clang-format on */
