/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "ui_cssom_view.h"
#include "ui_layout.h"
#include "ui_geometry.h"
/* clang-format on */

static int test_get_bounding_client_rect(void) {
  struct ui_layout_node node;
  struct ui_dom_rect rect;
  ui_error_t rc;

  memset(&node, 0, sizeof(node));
  node.x = 100.0f;
  node.y = 50.0f;
  node.width = 200.0f;
  node.height = 300.0f;

  rc = ui_cssom_view_get_bounding_client_rect(&node, &rect);
  if (rc != UI_ERROR_NONE) {
    printf("test_get_bounding_client_rect failed with error %d\n", rc);
    return 1;
  }

  if (rect.x != 100.0 || rect.y != 50.0 || rect.width != 200.0 ||
      rect.height != 300.0 || rect.top != 50.0 || rect.right != 300.0 ||
      rect.bottom != 350.0 || rect.left != 100.0) {
    printf("test_get_bounding_client_rect failed geometry check\n");
    return 1;
  }

  return 0;
}

static int test_get_client_width_height(void) {
  struct ui_layout_node node;
  float client_width = 0.0f;
  float client_height = 0.0f;
  ui_error_t rc;

  memset(&node, 0, sizeof(node));
  node.content_width = 100.0f;
  node.content_height = 150.0f;
  node.padding[UI_BOX_EDGE_TOP] = 10.0f;
  node.padding[UI_BOX_EDGE_RIGHT] = 20.0f;
  node.padding[UI_BOX_EDGE_BOTTOM] = 30.0f;
  node.padding[UI_BOX_EDGE_LEFT] = 40.0f;
  node.scrollbar_width = 15.0f;
  node.scrollbar_height = 15.0f;

  rc = ui_cssom_view_get_client_width(&node, &client_width);
  if (rc != UI_ERROR_NONE || client_width != (100.0f + 40.0f + 20.0f - 15.0f)) {
    printf("test_get_client_width_height failed width check\n");
    return 1;
  }

  rc = ui_cssom_view_get_client_height(&node, &client_height);
  if (rc != UI_ERROR_NONE ||
      client_height != (150.0f + 10.0f + 30.0f - 15.0f)) {
    printf("test_get_client_width_height failed height check\n");
    return 1;
  }

  return 0;
}

static int test_get_client_top_left(void) {
  struct ui_layout_node node;
  float client_top = 0.0f;
  float client_left = 0.0f;
  ui_error_t rc;

  memset(&node, 0, sizeof(node));
  node.border[UI_BOX_EDGE_TOP] = 5.0f;
  node.border[UI_BOX_EDGE_LEFT] = 8.0f;

  rc = ui_cssom_view_get_client_top(&node, &client_top);
  if (rc != UI_ERROR_NONE || client_top != 5.0f) {
    printf("test_get_client_top_left failed top check\n");
    return 1;
  }

  rc = ui_cssom_view_get_client_left(&node, &client_left);
  if (rc != UI_ERROR_NONE || client_left != 8.0f) {
    printf("test_get_client_top_left failed left check\n");
    return 1;
  }

  return 0;
}

static int test_get_scroll_width_height(void) {
  struct ui_layout_node node;
  float scroll_width = 0.0f;
  float scroll_height = 0.0f;
  ui_error_t rc;

  memset(&node, 0, sizeof(node));
  node.content_width = 500.0f;
  node.content_height = 600.0f;
  node.padding[UI_BOX_EDGE_TOP] = 10.0f;
  node.padding[UI_BOX_EDGE_RIGHT] = 20.0f;
  node.padding[UI_BOX_EDGE_BOTTOM] = 30.0f;
  node.padding[UI_BOX_EDGE_LEFT] = 40.0f;

  rc = ui_cssom_view_get_scroll_width(&node, &scroll_width);
  if (rc != UI_ERROR_NONE || scroll_width != (500.0f + 40.0f + 20.0f)) {
    printf("test_get_scroll_width_height failed width check\n");
    return 1;
  }

  rc = ui_cssom_view_get_scroll_height(&node, &scroll_height);
  if (rc != UI_ERROR_NONE || scroll_height != (600.0f + 10.0f + 30.0f)) {
    printf("test_get_scroll_width_height failed height check\n");
    return 1;
  }

  return 0;
}

static int test_null_arguments(void) {
  struct ui_layout_node node;
  struct ui_dom_rect rect;
  float val;

  memset(&node, 0, sizeof(node));

  if (ui_cssom_view_get_bounding_client_rect(NULL, &rect) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_bounding_client_rect(&node, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_cssom_view_get_client_width(NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_client_width(&node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_cssom_view_get_client_height(NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_client_height(&node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_cssom_view_get_client_top(NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_client_top(&node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_cssom_view_get_client_left(NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_client_left(&node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_cssom_view_get_scroll_width(NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_scroll_width(&node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  if (ui_cssom_view_get_scroll_height(NULL, &val) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_cssom_view_get_scroll_height(&node, NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;

  return 0;
}

static int test_negative_client_size(void) {
  struct ui_layout_node node;
  float val;
  ui_error_t rc;

  memset(&node, 0, sizeof(node));
  node.content_width = 10.0f;
  node.padding[UI_BOX_EDGE_LEFT] = 0.0f;
  node.padding[UI_BOX_EDGE_RIGHT] = 0.0f;
  node.scrollbar_width = 20.0f; /* Forces calculation below 0 */

  rc = ui_cssom_view_get_client_width(&node, &val);
  if (rc != UI_ERROR_NONE || val != 0.0f) {
    printf("test_negative_client_size failed width check\n");
    return 1;
  }

  node.content_height = 10.0f;
  node.padding[UI_BOX_EDGE_TOP] = 0.0f;
  node.padding[UI_BOX_EDGE_BOTTOM] = 0.0f;
  node.scrollbar_height = 20.0f;

  rc = ui_cssom_view_get_client_height(&node, &val);
  if (rc != UI_ERROR_NONE || val != 0.0f) {
    printf("test_negative_client_size failed height check\n");
    return 1;
  }

  return 0;
}

int main(void) {
  int failures = 0;
  failures += test_get_bounding_client_rect();
  failures += test_get_client_width_height();
  failures += test_get_client_top_left();
  failures += test_get_scroll_width_height();
  failures += test_null_arguments();
  failures += test_negative_client_size();

  if (failures == 0) {
    printf("test_ui_cssom_view passed\n");
  } else {
    printf("test_ui_cssom_view had %d failures\n", failures);
  }
  return failures;
}
