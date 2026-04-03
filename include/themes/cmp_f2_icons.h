#ifndef CMP_F2_ICONS_H
#define CMP_F2_ICONS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cmp.h"
#include "themes/cmp_f2_theme.h"
/* clang-format on */

/**
 * @brief Represents an active icon instance dynamically scaling size and style.
 */
typedef struct cmp_f2_icon_s {
  /** \brief Documented */
  uint32_t codepoint_regular;
  /** \brief Documented */
  uint32_t codepoint_filled;
  /** \brief Documented */
  int is_filled; /* 0 = Regular, 1 = Filled */
  /** \brief Documented */
  float size; /* Target bounding size (e.g. 10, 12, 16, 20, 24, 28, 32, 48) */
} cmp_f2_icon_t;

/**
 * @brief Initialize a Fluent 2 Icon component capable of state swapping.
 * @param out_node Pointer to receive the allocated node.
 * @param codepoint_regular Unicode codepoint for the outline representation.
 * @param codepoint_filled Unicode codepoint for the solid/active
 * representation.
 * @param size Intrinsic sizing boundary box.
 * @return 0 on success, or an error code.
 */
/**
 * @brief Initialize a Fluent 2 icon_create component.
 * @param out_node
 * Pointer to receive the allocated node.
 * @return 0 on success, or an error
 * code.
 */
CMP_API int cmp_f2_icon_create(cmp_ui_node_t **out_node,
                               uint32_t codepoint_regular,
                               uint32_t codepoint_filled, float size);

/**
 * @brief Triggers a state swap updating the visual representation of the icon.
 * @param icon_node The previously allocated icon component.
 * @param is_filled 0 to show regular, 1 to show filled.
 * @return 0 on success, or an error code.
 */
CMP_API int cmp_f2_icon_set_filled(cmp_ui_node_t *icon_node, int is_filled);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CMP_F2_ICONS_H */
