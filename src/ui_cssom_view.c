/* clang-format off */
#include "ui_cssom_view.h"
/* clang-format on */

/*
 * @brief ui_cssom_view_get_bounding_client_rect.
 * @param node Parameter node.
 * @param out_rect Parameter out_rect.
 * @return Return value.
 */
ui_error_t
ui_cssom_view_get_bounding_client_rect(const struct ui_layout_node *node,
                                       struct ui_dom_rect *out_rect) {
  float abs_x = 0.0f;
  float abs_y = 0.0f;
  const struct ui_layout_node *curr = node;

  if (!node || !out_rect) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  while (curr) {
    abs_x += curr->x;
    abs_y += curr->y;
    curr = curr->parent;
  }

  /* width/height on ui_layout_node is typically the border-box size,
     but if it's the content size, we need to add padding and border.
     Assuming layout_node->width and height are the final border-box dimension.
   */
  {
    ui_error_t rc =
        ui_dom_rect_init(out_rect, abs_x, abs_y, node->width, node->height);
    (void)rc;
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_view_get_client_width.
 * @param node Parameter node.
 * @param out_width Parameter out_width.
 * @return Return value.
 */
ui_error_t ui_cssom_view_get_client_width(const struct ui_layout_node *node,
                                          float *out_width) {
  if (!node || !out_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* clientWidth = inner width = width + padding - scrollbar_width (assuming
     width is content width) or if layout->width is border-box: clientWidth =
     width - border_left - border_right - scrollbar_width */

  *out_width = node->content_width + node->padding[UI_BOX_EDGE_LEFT] +
               node->padding[UI_BOX_EDGE_RIGHT] - node->scrollbar_width;

  if (*out_width < 0.0f) {
    *out_width = 0.0f;
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_view_get_client_height.
 * @param node Parameter node.
 * @param out_height Parameter out_height.
 * @return Return value.
 */
ui_error_t ui_cssom_view_get_client_height(const struct ui_layout_node *node,
                                           float *out_height) {
  if (!node || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_height = node->content_height + node->padding[UI_BOX_EDGE_TOP] +
                node->padding[UI_BOX_EDGE_BOTTOM] - node->scrollbar_height;

  if (*out_height < 0.0f) {
    *out_height = 0.0f;
  }

  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_view_get_client_top.
 * @param node Parameter node.
 * @param out_top Parameter out_top.
 * @return Return value.
 */
ui_error_t ui_cssom_view_get_client_top(const struct ui_layout_node *node,
                                        float *out_top) {
  if (!node || !out_top) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_top = node->border[UI_BOX_EDGE_TOP];
  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_view_get_client_left.
 * @param node Parameter node.
 * @param out_left Parameter out_left.
 * @return Return value.
 */
ui_error_t ui_cssom_view_get_client_left(const struct ui_layout_node *node,
                                         float *out_left) {
  if (!node || !out_left) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_left = node->border[UI_BOX_EDGE_LEFT];
  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_view_get_scroll_width.
 * @param node Parameter node.
 * @param out_width Parameter out_width.
 * @return Return value.
 */
ui_error_t ui_cssom_view_get_scroll_width(const struct ui_layout_node *node,
                                          float *out_width) {
  if (!node || !out_width) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* The scroll width is the max of the clientWidth and the actual content width
   */
  *out_width = node->content_width + node->padding[UI_BOX_EDGE_LEFT] +
               node->padding[UI_BOX_EDGE_RIGHT];
  return UI_ERROR_NONE;
}

/*
 * @brief ui_cssom_view_get_scroll_height.
 * @param node Parameter node.
 * @param out_height Parameter out_height.
 * @return Return value.
 */
ui_error_t ui_cssom_view_get_scroll_height(const struct ui_layout_node *node,
                                           float *out_height) {
  if (!node || !out_height) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* The scroll height is the max of the clientHeight and the actual content
   * height */
  *out_height = node->content_height + node->padding[UI_BOX_EDGE_TOP] +
                node->padding[UI_BOX_EDGE_BOTTOM];
  return UI_ERROR_NONE;
}
