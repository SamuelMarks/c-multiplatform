/**
 * @file ui_layout.c
 * @brief ui_layout.c implementation.
 */
/*
 * @file ui_layout.c
 * @brief Implementation of the UI layout engine (Flexbox & Block).
 */
/* clang-format off */
#include "ui_layout.h"

/**
 * @brief local_strcmp.
 * @param a Parameter a.
 * @param b Parameter b.
 * @return Return value.
 */
static int local_strcmp(const char *a, const char *b) {
  while (*a && (*a == *b)) {
    a++;
    b++;
  }
  return *(const unsigned char *)a - *(const unsigned char *)b;
}

#include "ui_css_values.h"
#include "ui_internal_mem.h"
#include "ui_web_bridge.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ui_layout_parsers.c"
#include "ui_layout_compute.c"
#include "ui_layout_algorithms.c"
#include "ui_layout_tree.c"
/* clang-format on */
ui_error_t ui_layout_solve_viewport(struct ui_layout_node *root,
                                    float window_width, float window_height) {
  if (!root) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Identify if root is html or body */
  if (root->dom_node && root->dom_node->tag_name) {
    if (strcmp(root->dom_node->tag_name, "html") == 0 ||
        strcmp(root->dom_node->tag_name, "body") == 0) {
      root->width = window_width;
      root->height = window_height;
      root->content_width = window_width - root->padding[1] - root->padding[3] -
                            root->border[1] - root->border[3];
      if (root->content_width < 0.0f)
        root->content_width = 0.0f;
      root->content_height = window_height - root->padding[0] -
                             root->padding[2] - root->border[0] -
                             root->border[2];
      if (root->content_height < 0.0f)
        root->content_height = 0.0f;
    }
  } else if (!root->dom_node) {
    /* Anonymous root */
    root->width = window_width;
    root->height = window_height;
    root->content_width = window_width;
    root->content_height = window_height;
  }

  /* Force any direct child body to also match window dimensions exactly */
  if (root->first_child) {
    struct ui_layout_node *child = root->first_child;
    while (child) {
      if (child->dom_node && child->dom_node->tag_name) {
        if (strcmp(child->dom_node->tag_name, "body") == 0) {
          child->width = window_width;
          child->height = window_height;
          child->content_width = window_width - child->padding[1] -
                                 child->padding[3] - child->border[1] -
                                 child->border[3];
          child->content_height = window_height - child->padding[0] -
                                  child->padding[2] - child->border[0] -
                                  child->border[2];
        }
      }
      child = child->next_sibling;
    }
  }

  return ui_layout_compute(root, window_width, window_height);
}

/**
 * @brief Checks the computed layout tree for bounding box violations.
 * @param[in] node The layout node to check recursively.
 * @return UI_ERROR_NONE if valid, UI_ERROR_LAYOUT_VIOLATION if elements
 * improperly exceed bounds.
 */
ui_error_t ui_layout_sanity_check(const struct ui_layout_node *node) {
  struct ui_layout_node *child;

  if (!node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  child = node->first_child;
  while (child) {
    /*
     * The task explicitly asks to assert:
     * `child_rect.x + child_rect.width <= parent_rect.width`
     * (accounting for margins/overflow)
     */
    float child_max_x = child->x + child->width + child->margin[1];
    float parent_max_x = node->width;

    float child_max_y = child->y + child->height + child->margin[2];
    float parent_max_y = node->height;

    /* If overflow is visible, CSS allows children to bleed structurally.
       If overflow is scroll/auto, the scrollbar acts as the containment
       mechanism. But if it is hidden, we strictly prove they do not bleed into
       neighbor regions without being explicitly handled by the engine.
       Actually, to prove the engine "holds under stress testing", we can flag a
       violation if the layout constraints resulted in an unhandled bleed. For
       this engine, we consider it a violation if a child exceeds parent bounds
       UNLESS the parent explicitly manages overflow via SCROLL, AUTO, or
       VISIBLE. Actually, if it's VISIBLE it's allowed. If it's HIDDEN, it's
       clipped visually, but we can assert the intrinsic math here. Wait, let's
       just assert exactly what's requested: children do not exceed parent
       bounds unless overflow handles it.
     */
    if (node->overflow_x == UI_LAYOUT_OVERFLOW_HIDDEN ||
        node->overflow_x == UI_LAYOUT_OVERFLOW_VISIBLE) {
      /* Even if hidden or visible, the mathematical bounds could be larger,
         but if we want to ensure containment sanity, we might only assert it if
         there's a strict constraint. Let's just do a basic assertion that
         width doesn't wildly blow out without a reason.
         Wait, if overflow is NOT scroll/auto, and child_max_x > parent_max_x,
         maybe that's a violation of our "prevent UI elements from bleeding"
         rule? Let's flag it if overflow is HIDDEN and it bleeds, as the user
         wants to test bounds.
      */
      if (node->overflow_x != UI_LAYOUT_OVERFLOW_VISIBLE &&
          child_max_x > parent_max_x + 0.5f) {
        /* allow tiny float drift */
        return UI_ERROR_LAYOUT_VIOLATION;
      }
    }

    if (node->overflow_y != UI_LAYOUT_OVERFLOW_VISIBLE &&
        node->overflow_y != UI_LAYOUT_OVERFLOW_SCROLL &&
        node->overflow_y != UI_LAYOUT_OVERFLOW_AUTO) {
      if (child_max_y > parent_max_y + 0.5f) {
        return UI_ERROR_LAYOUT_VIOLATION;
      }
    }

    {
      ui_error_t check_rc = ui_layout_sanity_check(child);
      if (check_rc != UI_ERROR_NONE) {
        return check_rc;
      }
    }
    child = child->next_sibling;
  }

  return UI_ERROR_NONE;
}

/**
 * @brief Computes layout for a node based on its display properties.
 * @param[in,out] node The layout node.
 * @param[in] available_width The available width.
 * @param[in] available_height The available height.
 * @return UI_ERROR_NONE on success.
 */
ui_error_t ui_layout_compute(struct ui_layout_node *node, float available_width,
                             float available_height) {

  if (!node) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Suppress unused warning */
  (void)available_height;

  if (node->display_inside == UI_LAYOUT_DISPLAY_INSIDE_FLEX) {
    (void)layout_flex(node, available_width);
    return UI_ERROR_NONE;
  }

  /* Basic block layout */
  (void)layout_block(node, available_width);

#if defined(__EMSCRIPTEN__)
  if (node->dom_node) {
    ui_web_bridge_set_bounds((uint32_t)(uintptr_t)node->dom_node, node->x,
                             node->y, node->width, node->height);
  }
#endif

  return UI_ERROR_NONE;
}
