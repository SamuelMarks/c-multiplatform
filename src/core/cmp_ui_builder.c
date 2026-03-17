/* clang-format off */
#include "cmpc/cmp_ui_builder.h"
#include <string.h>
/* clang-format on */

typedef struct CMPUIBuilderNode {
  CMPLayoutNode layout;
  CMPWidget *widget;
  struct CMPUIBuilderNode *parent;
  struct CMPUIBuilderNode *next_sibling;
  struct CMPUIBuilderNode *first_child;
  struct CMPUIBuilderNode *last_child;
  cmp_usize child_count;
  CMPLayoutNode **child_array;
  struct CMPUIBuilderNode *next_alloc;
} CMPUIBuilderNode;

CMP_API int CMP_CALL cmp_ui_builder_init(CMPUIBuilder *builder,
                                         const CMPAllocator *allocator) {
  if (builder == NULL || allocator == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  memset(builder, 0, sizeof(*builder));
  builder->allocator = *allocator;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_ui_builder_destroy(CMPUIBuilder *builder) {
  CMPUIBuilderNode *curr;
  CMPUIBuilderNode *next;

  if (builder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  curr = builder->head;
  while (curr != NULL) {
    next = curr->next_alloc;
    if (curr->child_array != NULL) {
      builder->allocator.free(builder->allocator.ctx, curr->child_array);
    }
    builder->allocator.free(builder->allocator.ctx, curr);
    curr = next;
  }

  memset(builder, 0, sizeof(*builder));
  return CMP_OK;
}

static int cmp_ui_builder_alloc_node(CMPUIBuilder *builder,
                                     const CMPLayoutStyle *style,
                                     CMPUIBuilderNode **out_node) {
  CMPUIBuilderNode *node;
  int rc;

  rc = builder->allocator.alloc(builder->allocator.ctx,
                                sizeof(CMPUIBuilderNode), (void **)&node);
  if (rc != CMP_OK) {
    return rc;
  }

  memset(node, 0, sizeof(*node));
  node->next_alloc = builder->head;
  builder->head = node;

  rc = cmp_layout_node_init(&node->layout, style);
  if (rc != CMP_OK) {
    return rc;
  }

  if (builder->current != NULL) {
    node->parent = builder->current;
    if (builder->current->last_child == NULL) {
      builder->current->first_child = node;
    } else {
      builder->current->last_child->next_sibling = node;
    }
    builder->current->last_child = node;
    builder->current->child_count++;
  } else if (builder->root_layout == NULL) {
    builder->root_layout = &node->layout;
  }

  *out_node = node;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_ui_builder_begin_column(CMPUIBuilder *builder,
                                                 const CMPLayoutStyle *style) {
  CMPUIBuilderNode *node;
  int rc;

  if (builder == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_ui_builder_alloc_node(builder, style, &node);
  if (rc != CMP_OK) {
    return rc;
  }

  builder->current = node;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_ui_builder_begin_row(CMPUIBuilder *builder,
                                              const CMPLayoutStyle *style) {
  CMPUIBuilderNode *node;
  int rc;

  if (builder == NULL || style == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_ui_builder_alloc_node(builder, style, &node);
  if (rc != CMP_OK) {
    return rc;
  }

  builder->current = node;
  return CMP_OK;
}

static int CMP_CALL cmp_ui_builder_widget_measure(void *ctx,
                                                  CMPLayoutMeasureSpec width,
                                                  CMPLayoutMeasureSpec height,
                                                  CMPSize *out_size) {
  CMPWidget *widget;
  CMPMeasureSpec w_spec;
  CMPMeasureSpec h_spec;

  widget = (CMPWidget *)ctx;
  if (widget == NULL || widget->vtable == NULL ||
      widget->vtable->measure == NULL) {
    out_size->width = 0.0f;
    out_size->height = 0.0f;
    return CMP_OK;
  }

  w_spec.mode = width.mode;
  w_spec.size = width.size;
  h_spec.mode = height.mode;
  h_spec.size = height.size;

  return widget->vtable->measure(widget->ctx, w_spec, h_spec, out_size);
}

CMP_API int CMP_CALL cmp_ui_builder_add_widget(CMPUIBuilder *builder,
                                               const CMPLayoutStyle *style,
                                               CMPWidget *widget) {
  CMPUIBuilderNode *node;
  int rc;

  if (builder == NULL || style == NULL || widget == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = cmp_ui_builder_alloc_node(builder, style, &node);
  if (rc != CMP_OK) {
    return rc;
  }

  node->widget = widget;
  rc = cmp_layout_node_set_measure(&node->layout, cmp_ui_builder_widget_measure,
                                   widget);
  if (rc != CMP_OK) {
    return rc;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_ui_builder_end(CMPUIBuilder *builder) {
  CMPUIBuilderNode *curr;
  CMPUIBuilderNode *child;
  cmp_usize i;
  int rc;

  if (builder == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  curr = builder->current;
  if (curr == NULL) {
    return CMP_ERR_STATE;
  }

  if (curr->child_count > 0) {
    rc = builder->allocator.alloc(builder->allocator.ctx,
                                  curr->child_count * sizeof(CMPLayoutNode *),
                                  (void **)&curr->child_array);
    if (rc != CMP_OK) {
      return rc;
    }

    child = curr->first_child;
    for (i = 0; i < curr->child_count; i++) {
      curr->child_array[i] = &child->layout;
      child = child->next_sibling;
    }

    rc = cmp_layout_node_set_children(&curr->layout, curr->child_array,
                                      curr->child_count);
    if (rc != CMP_OK) {
      return rc;
    }
  }

  builder->current = curr->parent;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_ui_builder_get_root(CMPUIBuilder *builder,
                                             CMPLayoutNode **out_root) {
  if (builder == NULL || out_root == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (builder->root_layout == NULL) {
    return CMP_ERR_STATE;
  }

  *out_root = builder->root_layout;
  return CMP_OK;
}