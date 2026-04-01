/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_pos_absolute_relative(cmp_layout_node_t *node,
                              const cmp_rect_t *parent_rect) {
  if (!node || !parent_rect)
    return CMP_ERROR_INVALID_ARG;

  if (node->position_type == CMP_POSITION_ABSOLUTE) {
    node->computed_rect.x = parent_rect->x + node->position[3]; /* Left */
    node->computed_rect.y = parent_rect->y + node->position[0]; /* Top */
  } else if (node->position_type == CMP_POSITION_RELATIVE) {
    node->computed_rect.x += node->position[3];
    node->computed_rect.y += node->position[0];
  }
  return CMP_SUCCESS;
}

int cmp_pos_fixed(cmp_layout_node_t *node, const cmp_rect_t *viewport_rect) {
  if (!node || !viewport_rect)
    return CMP_ERROR_INVALID_ARG;

  node->computed_rect.x = viewport_rect->x + node->position[3];
  node->computed_rect.y = viewport_rect->y + node->position[0];
  return CMP_SUCCESS;
}

int cmp_pos_sticky(cmp_layout_node_t *node, float scroll_offset,
                   const cmp_rect_t *container_rect) {
  if (!node || !container_rect)
    return CMP_ERROR_INVALID_ARG;

  /* Basic sticky evaluation stub */
  if (scroll_offset > node->position[0]) {
    node->computed_rect.y = scroll_offset + node->position[0];
  }

  return CMP_SUCCESS;
}

int cmp_anchor_position(cmp_layout_node_t *floating_node,
                        const cmp_rect_t *anchor_rect) {
  if (!floating_node || !anchor_rect)
    return CMP_ERROR_INVALID_ARG;
  floating_node->computed_rect.x = anchor_rect->x;
  floating_node->computed_rect.y = anchor_rect->y + anchor_rect->height;
  return CMP_SUCCESS;
}

int cmp_anchor_fallback(cmp_layout_node_t *floating_node,
                        const cmp_rect_t *anchor_rect,
                        const cmp_rect_t *viewport_rect) {
  if (!floating_node || !anchor_rect || !viewport_rect)
    return CMP_ERROR_INVALID_ARG;

  /* Check bottom collision */
  if (anchor_rect->y + anchor_rect->height +
          floating_node->computed_rect.height >
      viewport_rect->height) {
    /* Flip to top */
    floating_node->computed_rect.y =
        anchor_rect->y - floating_node->computed_rect.height;
  }
  return CMP_SUCCESS;
}

int cmp_anchor_size(cmp_layout_node_t *floating_node,
                    const cmp_rect_t *anchor_rect) {
  if (!floating_node || !anchor_rect)
    return CMP_ERROR_INVALID_ARG;
  floating_node->computed_rect.width = anchor_rect->width;
  return CMP_SUCCESS;
}

int cmp_stack_ctx_create(cmp_stack_ctx_t **out_ctx, cmp_layout_node_t *node) {
  cmp_stack_ctx_t *ctx;
  if (!out_ctx || !node)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_stack_ctx_t), (void **)&ctx) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(ctx, 0, sizeof(cmp_stack_ctx_t));
  ctx->node = node;
  ctx->z_index = node->z_index;
  *out_ctx = ctx;
  return CMP_SUCCESS;
}

int cmp_stack_ctx_destroy(cmp_stack_ctx_t *ctx) {
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->children)
    CMP_FREE(ctx->children);
  CMP_FREE(ctx);
  return CMP_SUCCESS;
}

int cmp_stack_ctx_add_child(cmp_stack_ctx_t *parent, cmp_stack_ctx_t *child) {
  cmp_stack_ctx_t **new_children;
  size_t new_cap;
  if (!parent || !child)
    return CMP_ERROR_INVALID_ARG;

  if (parent->child_count >= parent->child_capacity) {
    new_cap = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    if (CMP_MALLOC(new_cap * sizeof(cmp_stack_ctx_t *),
                   (void **)&new_children) != CMP_SUCCESS)
      return CMP_ERROR_OOM;
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
  return CMP_SUCCESS;
}

static int compare_z_index(const void *a, const void *b) {
  cmp_stack_ctx_t *ctx_a = *(cmp_stack_ctx_t **)a;
  cmp_stack_ctx_t *ctx_b = *(cmp_stack_ctx_t **)b;
  return ctx_a->z_index - ctx_b->z_index;
}

int cmp_z_index_sort(cmp_stack_ctx_t *ctx) {
  if (!ctx)
    return CMP_ERROR_INVALID_ARG;
  if (ctx->child_count > 1) {
    qsort(ctx->children, ctx->child_count, sizeof(cmp_stack_ctx_t *),
          compare_z_index);
  }
  return CMP_SUCCESS;
}

int cmp_transform_3d_evaluate(cmp_layout_node_t *node, int preserve_3d) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Mark hierarchy as sharing 3D context if preserve_3d is 1 */
  (void)preserve_3d;
  return CMP_SUCCESS;
}

int cmp_backface_visibility_evaluate(cmp_layout_node_t *node, int is_hidden,
                                     float rotation_y, int *out_visible) {
  if (!node || !out_visible)
    return CMP_ERROR_INVALID_ARG;

  if (is_hidden && (rotation_y > 90.0f && rotation_y < 270.0f)) {
    *out_visible = 0;
  } else {
    *out_visible = 1;
  }
  return CMP_SUCCESS;
}

int cmp_top_layer_promote(cmp_layout_node_t *node) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  /* Add node to global top_layer render list bypassing standard context */
  return CMP_SUCCESS;
}

int cmp_popover_toggle(cmp_layout_node_t *node, cmp_popover_state_t state) {
  if (!node)
    return CMP_ERROR_INVALID_ARG;
  if (state == CMP_POPOVER_SHOWING) {
    cmp_top_layer_promote(node);
  }
  return CMP_SUCCESS;
}

int cmp_layer_tree_build(cmp_layout_node_t *root_node,
                         cmp_layer_t **out_layer_root) {
  cmp_layer_t *layer;
  if (!root_node || !out_layer_root)
    return CMP_ERROR_INVALID_ARG;

  if (CMP_MALLOC(sizeof(cmp_layer_t), (void **)&layer) != CMP_SUCCESS)
    return CMP_ERROR_OOM;

  memset(layer, 0, sizeof(cmp_layer_t));
  layer->node = root_node;
  *out_layer_root = layer;
  return CMP_SUCCESS;
}

int cmp_layer_tree_destroy(cmp_layer_t *layer_root) {
  if (!layer_root)
    return CMP_ERROR_INVALID_ARG;
  if (layer_root->children)
    CMP_FREE(layer_root->children);
  CMP_FREE(layer_root);
  return CMP_SUCCESS;
}
