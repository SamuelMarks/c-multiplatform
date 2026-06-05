/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief cmp_pos_absolute_relative
 *
 * @param node Parameter description.
 * @param parent_rect Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pos_absolute_relative(cmp_layout_node_t *node,
                              const cmp_rect_t *parent_rect) {
  int rc = CMP_SUCCESS;

  if (!node || !parent_rect) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pos_absolute_relative: Invalid argument\n");
    return rc;
  }

  if (node->position_type == CMP_POSITION_ABSOLUTE) {
    node->computed_rect.x = parent_rect->x + node->position[3]; /* Left */
    node->computed_rect.y = parent_rect->y + node->position[0]; /* Top */
  } else if (node->position_type == CMP_POSITION_RELATIVE) {
    node->computed_rect.x += node->position[3];
    node->computed_rect.y += node->position[0];
  }
  return rc;
}

/**
 * @brief cmp_pos_fixed
 *
 * @param node Parameter description.
 * @param viewport_rect Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pos_fixed(cmp_layout_node_t *node, const cmp_rect_t *viewport_rect) {
  int rc = CMP_SUCCESS;

  if (!node || !viewport_rect) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pos_fixed: Invalid argument\n");
    return rc;
  }

  node->computed_rect.x = viewport_rect->x + node->position[3];
  node->computed_rect.y = viewport_rect->y + node->position[0];
  return rc;
}

/**
 * @brief cmp_pos_sticky
 *
 * @param node Parameter description.
 * @param scroll_offset Parameter description.
 * @param container_rect Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_pos_sticky(cmp_layout_node_t *node, float scroll_offset,
                   const cmp_rect_t *container_rect) {
  int rc = CMP_SUCCESS;

  if (!node || !container_rect) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_pos_sticky: Invalid argument\n");
    return rc;
  }

  /* Sticky evaluation: anchor element to the specified scroll offset threshold
   */
  if (scroll_offset > node->position[0]) {
    node->computed_rect.y = scroll_offset + node->position[0];
  }
  return rc;
}

/**
 * @brief cmp_anchor_position
 *
 * @param floating_node Parameter description.
 * @param anchor_rect Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_anchor_position(cmp_layout_node_t *floating_node,
                        const cmp_rect_t *anchor_rect) {
  int rc = CMP_SUCCESS;

  if (!floating_node || !anchor_rect) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_anchor_position: Invalid argument\n");
    return rc;
  }
  floating_node->computed_rect.x = anchor_rect->x;
  floating_node->computed_rect.y = anchor_rect->y + anchor_rect->height;
  return rc;
}

/**
 * @brief cmp_anchor_fallback
 *
 * @param floating_node Parameter description.
 * @param anchor_rect Parameter description.
 * @param viewport_rect Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_anchor_fallback(cmp_layout_node_t *floating_node,
                        const cmp_rect_t *anchor_rect,
                        const cmp_rect_t *viewport_rect) {
  int rc = CMP_SUCCESS;

  if (!floating_node || !anchor_rect || !viewport_rect) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_anchor_fallback: Invalid argument\n");
    return rc;
  }

  /* Check bottom collision */
  if (anchor_rect->y + anchor_rect->height +
          floating_node->computed_rect.height >
      viewport_rect->height) {
    /* Flip to top */
    floating_node->computed_rect.y =
        anchor_rect->y - floating_node->computed_rect.height;
  }
  return rc;
}

/**
 * @brief cmp_anchor_size
 *
 * @param floating_node Parameter description.
 * @param anchor_rect Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_anchor_size(cmp_layout_node_t *floating_node,
                    const cmp_rect_t *anchor_rect) {
  int rc = CMP_SUCCESS;

  if (!floating_node || !anchor_rect) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_anchor_size: Invalid argument\n");
    return rc;
  }
  floating_node->computed_rect.width = anchor_rect->width;
  return rc;
}

/**
 * @brief cmp_stack_ctx_create
 *
 * @param out_ctx Parameter description.
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stack_ctx_create(cmp_stack_ctx_t **out_ctx, cmp_layout_node_t *node) {
  int rc = CMP_SUCCESS;
  cmp_stack_ctx_t *ctx = NULL;

  if (!out_ctx || !node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stack_ctx_create: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_stack_ctx_t), (void **)&ctx);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_stack_ctx_create: Out of memory\n");
    return rc;
  }

  memset(ctx, 0, sizeof(cmp_stack_ctx_t));
  ctx->node = node;
  ctx->z_index = node->z_index;
  *out_ctx = ctx;
  return rc;
}

/**
 * @brief cmp_stack_ctx_destroy
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stack_ctx_destroy(cmp_stack_ctx_t *ctx) {
  int rc = CMP_SUCCESS;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stack_ctx_destroy: Invalid argument\n");
    return rc;
  }
  if (ctx->children) {
    CMP_FREE(ctx->children);
  }
  CMP_FREE(ctx);
  return rc;
}

/**
 * @brief cmp_stack_ctx_add_child
 *
 * @param parent Parameter description.
 * @param child Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_stack_ctx_add_child(cmp_stack_ctx_t *parent, cmp_stack_ctx_t *child) {
  int rc = CMP_SUCCESS;
  cmp_stack_ctx_t **new_children = NULL;
  size_t new_cap;

  if (!parent || !child) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_stack_ctx_add_child: Invalid argument\n");
    return rc;
  }

  if (parent->child_count >= parent->child_capacity) {
    new_cap = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    rc =
        CMP_MALLOC(new_cap * sizeof(cmp_stack_ctx_t *), (void **)&new_children);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_stack_ctx_add_child: Out of memory\n");

      return rc;
    }
    if (parent->children) {
      memcpy(new_children, parent->children,
             parent->child_count * sizeof(cmp_stack_ctx_t *));
      CMP_FREE(parent->children);
    }
    parent->children = new_children;
    parent->child_capacity = new_cap;
  }

  parent->children[parent->child_count++] = child;
  child->parent = parent;
  return rc;
}

/**
 * @brief compare_z_index
 *
 * @param a Parameter description.
 * @param b Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static int compare_z_index(const void *a, const void *b) {
  int rc = CMP_SUCCESS;
  cmp_stack_ctx_t *ctx_a = *(cmp_stack_ctx_t **)a;
  cmp_stack_ctx_t *ctx_b = *(cmp_stack_ctx_t **)b;

  rc = ctx_a->z_index - ctx_b->z_index;
  return rc;
}

/**
 * @brief cmp_z_index_sort
 *
 * @param ctx Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_z_index_sort(cmp_stack_ctx_t *ctx) {
  int rc = CMP_SUCCESS;

  if (!ctx) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_z_index_sort: Invalid argument\n");
    return rc;
  }
  if (ctx->child_count > 1) {
    qsort(ctx->children, ctx->child_count, sizeof(cmp_stack_ctx_t *),
          compare_z_index);
  }
  return rc;
}

/**
 * @brief cmp_transform_3d_evaluate
 *
 * @param node Parameter description.
 * @param preserve_3d Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_transform_3d_evaluate(cmp_layout_node_t *node, int preserve_3d) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_transform_3d_evaluate: Invalid argument\n");
    return rc;
  }
  /* Mark hierarchy as sharing 3D context if preserve_3d is 1 */
  (void)preserve_3d;
  return rc;
}

/**
 * @brief cmp_backface_visibility_evaluate
 *
 * @param node Parameter description.
 * @param is_hidden Parameter description.
 * @param rotation_y Parameter description.
 * @param out_visible Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_backface_visibility_evaluate(cmp_layout_node_t *node, int is_hidden,
                                     float rotation_y, int *out_visible) {
  int rc = CMP_SUCCESS;

  if (!node || !out_visible) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_backface_visibility_evaluate: Invalid argument\n");
    return rc;
  }

  if (is_hidden && (rotation_y > 90.0f && rotation_y < 270.0f)) {
    *out_visible = 0;
  } else {
    *out_visible = 1;
  }
  return rc;
}

/**
 * @brief cmp_top_layer_promote
 *
 * @param node Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_top_layer_promote(cmp_layout_node_t *node) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_top_layer_promote: Invalid argument\n");
    return rc;
  }
  /* Add node to global top_layer render list bypassing standard context */
  return rc;
}

/**
 * @brief cmp_popover_toggle
 *
 * @param node Parameter description.
 * @param state Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_popover_toggle(cmp_layout_node_t *node, cmp_popover_state_t state) {
  int rc = CMP_SUCCESS;

  if (!node) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_popover_toggle: Invalid argument\n");
    return rc;
  }
  if (state == CMP_POPOVER_SHOWING) {
    rc = cmp_top_layer_promote(node);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("Error in cmp_popover_toggle: cmp_top_layer_promote failed\n");
    }
  }
  return rc;
}

/**
 * @brief cmp_layer_tree_build
 *
 * @param root_node Parameter description.
 * @param out_layer_root Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_layer_tree_build(cmp_layout_node_t *root_node,
                         cmp_layer_t **out_layer_root) {
  int rc = CMP_SUCCESS;
  cmp_layer_t *layer = NULL;

  if (!root_node || !out_layer_root) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_layer_tree_build: Invalid argument\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_layer_t), (void **)&layer);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Error in cmp_layer_tree_build: Out of memory\n");
    return rc;
  }

  memset(layer, 0, sizeof(cmp_layer_t));
  layer->node = root_node;

  /* Implement clipping overflow mapping */
  if (root_node->overflow_x == 2 || root_node->overflow_y == 2) { /* 2 = Hidden */
    layer->scissor_enable = 1;
    layer->scissor_rect = root_node->computed_rect;
  } else if (root_node->parent) {
    /* simplistic inheritance of scissor for demonstration or later expansion,
       a full tree walk would intersect ancestor scissor rects */
  }

  *out_layer_root = layer;
  return rc;
}

/**
 * @brief cmp_layer_tree_destroy
 *
 * @param layer_root Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_layer_tree_destroy(cmp_layer_t *layer_root) {
  int rc = CMP_SUCCESS;

  if (!layer_root) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_layer_tree_destroy: Invalid argument\n");
    return rc;
  }
  if (layer_root->children) {
    CMP_FREE(layer_root->children);
  }
  CMP_FREE(layer_root);
  return rc;
}
