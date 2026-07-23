#ifndef UI_CSSOM_VIEW_H
#define UI_CSSOM_VIEW_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "ui_error.h"
#include "ui_geometry.h"
#include "ui_layout.h"
/* clang-format on */

/**
 * @brief Retrieves the size of an element and its position relative to the
 * viewport.
 *
 * Maps to Element.getBoundingClientRect().
 *
 * @param node The layout node.
 * @param out_rect Pointer to receive the bounding client rect.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error
ui_cssom_view_get_bounding_client_rect(const struct ui_layout_node *node,
                                       struct ui_dom_rect *out_rect);

/**
 * @brief Retrieves the inner width of an element in pixels, including padding
 * but excluding borders, margins, and vertical scrollbars.
 *
 * Maps to Element.clientWidth.
 *
 * @param node The layout node.
 * @param out_width Pointer to receive the client width.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_cssom_view_get_client_width(const struct ui_layout_node *node,
                                             float *out_width);

/**
 * @brief Retrieves the inner height of an element in pixels, including padding
 * but excluding borders, margins, and horizontal scrollbars.
 *
 * Maps to Element.clientHeight.
 *
 * @param node The layout node.
 * @param out_height Pointer to receive the client height.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_cssom_view_get_client_height(const struct ui_layout_node *node,
                                              float *out_height);

/**
 * @brief Retrieves the width of the top border of an element in pixels.
 *
 * Maps to Element.clientTop.
 *
 * @param node The layout node.
 * @param out_top Pointer to receive the client top.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_cssom_view_get_client_top(const struct ui_layout_node *node,
                                           float *out_top);

/**
 * @brief Retrieves the width of the left border of an element in pixels.
 *
 * Maps to Element.clientLeft.
 *
 * @param node The layout node.
 * @param out_left Pointer to receive the client left.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_cssom_view_get_client_left(const struct ui_layout_node *node,
                                            float *out_left);

/**
 * @brief Retrieves the total width of an element's content, including content
 * not visible on the screen due to overflow.
 *
 * Maps to Element.scrollWidth.
 *
 * @param node The layout node.
 * @param out_width Pointer to receive the scroll width.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_cssom_view_get_scroll_width(const struct ui_layout_node *node,
                                             float *out_width);

/**
 * @brief Retrieves the total height of an element's content, including content
 * not visible on the screen due to overflow.
 *
 * Maps to Element.scrollHeight.
 *
 * @param node The layout node.
 * @param out_height Pointer to receive the scroll height.
 * @return UI_ERROR_NONE on success.
 */
enum ui_error ui_cssom_view_get_scroll_height(const struct ui_layout_node *node,
                                              float *out_height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UI_CSSOM_VIEW_H */
