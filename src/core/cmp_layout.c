/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_layout_node_create(cmp_layout_node_t **out_node) {
  int rc = CMP_SUCCESS;
  cmp_layout_node_t *node;

  if (out_node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_layout_node_create: %s\n", err_str);
    }
    return rc;
  }

  rc = CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&node);
  if (rc != CMP_SUCCESS) {
    if (rc == CMP_SUCCESS)
      rc = CMP_ERROR_OOM;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_layout_node_create CMP_MALLOC: %s\n", err_str);
    }
    return rc;
  }

  memset(node, 0, sizeof(cmp_layout_node_t));
  node->id = 0;

  node->display = CMP_DISPLAY_FLEX;
  node->direction = CMP_FLEX_COLUMN;
  node->flex_wrap = CMP_FLEX_NOWRAP;
  node->justify_content = CMP_FLEX_ALIGN_START;
  node->align_items = CMP_FLEX_ALIGN_STRETCH;
  node->align_content = CMP_FLEX_ALIGN_START;
  node->align_self = CMP_FLEX_ALIGN_AUTO;
  node->position_type = CMP_POSITION_RELATIVE;

  node->order = 0;

  node->box_sizing = CMP_BOX_SIZING_CONTENT_BOX;
  node->is_stacking_context = 0;
  node->z_index = 0;

  node->width = CMP_LAYOUT_AUTO;
  node->height = CMP_LAYOUT_AUTO;
  node->min_width = CMP_LAYOUT_AUTO;
  node->min_height = CMP_LAYOUT_AUTO;
  node->max_width = CMP_LAYOUT_AUTO;
  node->max_height = CMP_LAYOUT_AUTO;
  node->flex_grow = 0.0f;
  node->flex_shrink = 1.0f;
  node->flex_basis = CMP_LAYOUT_AUTO;

  node->measured_width = 0.0f;
  node->measured_height = 0.0f;
  node->dirty = 1;

  node->flex_lines = NULL;

  node->grid_ctx = NULL;

  node->grid_column_start = 0;
  node->grid_column_end = 0;
  node->grid_row_start = 0;
  node->grid_row_end = 0;

  node->width_calc = NULL;
  node->height_calc = NULL;

  *out_node = node;
  return rc;
}

int cmp_layout_node_destroy(cmp_layout_node_t *node) {
  int rc = CMP_SUCCESS;
  size_t i;

  if (node == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    {
      const char *err_str;
      int rc2;
      rc2 = cmp_strerror(rc, &err_str);
      if (rc2 != CMP_SUCCESS) {
        err_str = "Unknown";
      }
      LOG_DEBUG("cmp_layout_node_destroy: %s\n", err_str);
    }
    return rc;
  }

  for (i = 0; i < node->child_count; i++) {
    (void)cmp_layout_node_destroy(node->children[i]);
  }

  if (node->children != NULL) {
    CMP_FREE(node->children);
  }

  if (node->flex_lines != NULL) {
    if (node->flex_lines->lines != NULL) {
      CMP_FREE(node->flex_lines->lines);
    }
    CMP_FREE(node->flex_lines);
  }

  if (node->grid_ctx != NULL) {
    cmp_grid_ctx_destroy(node->grid_ctx);
  }

  if (node->width_calc != NULL)
    CMP_FREE(node->width_calc);
  if (node->height_calc != NULL)
    CMP_FREE(node->height_calc);

  CMP_FREE(node);
  return rc;
}

int cmp_layout_node_add_child(cmp_layout_node_t *parent,
                              cmp_layout_node_t *child) {
  int rc = CMP_SUCCESS;
  if (parent == NULL || child == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (parent->child_count >= parent->child_capacity) {
    size_t new_cap =
        parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    cmp_layout_node_t **new_children;

    if (CMP_MALLOC(sizeof(cmp_layout_node_t *) * new_cap,
                   (void **)&new_children) != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }

    if (parent->children != NULL) {
      memcpy(new_children, parent->children,
             sizeof(cmp_layout_node_t *) * parent->child_count);
      CMP_FREE(parent->children);
    }

    parent->children = new_children;
    parent->child_capacity = new_cap;
  }

  parent->children[parent->child_count++] = child;
  child->parent = parent;

  return rc;
}

static void translate_descendants(cmp_layout_node_t *node, float dx) {
  size_t i;
  if (!node)
    return;
  for (i = 0; i < node->child_count; i++) {
    node->children[i]->computed_rect.x += dx;
    translate_descendants(node->children[i], dx);
  }
}

static void apply_rtl_mirroring(cmp_layout_node_t *node) {
  size_t i;
  if (!node)
    return;

  if (node->direction == CMP_FLEX_ROW) {
    for (i = 0; i < node->child_count; i++) {
      cmp_layout_node_t *child = node->children[i];
      if (child->position_type != CMP_POSITION_ABSOLUTE) {
        float local_x = child->computed_rect.x - node->computed_rect.x;
        float new_x = node->computed_rect.x + node->computed_rect.width -
                      local_x - child->computed_rect.width;
        float dx = new_x - child->computed_rect.x;
        if (dx != 0.0f) {
          child->computed_rect.x = new_x;
          translate_descendants(child, dx);
        }
      }
    }
  }

  for (i = 0; i < node->child_count; i++) {
    apply_rtl_mirroring(node->children[i]);
  }
}

static float g_viewport_width = 0.0f;
static float g_viewport_height = 0.0f;

static float cmp_resolve_unit(float val, cmp_unit_t unit, float parent_val) {
  if (val < 0.0f)
    return val; /* Auto / Unset */
  switch (unit) {
  case CMP_UNIT_PERCENT:
    if (parent_val >= 0.0f)
      return val * 0.01f * parent_val;
    return val;
  case CMP_UNIT_VW:
    return val * 0.01f * g_viewport_width;
  case CMP_UNIT_VH:
    return val * 0.01f * g_viewport_height;
  case CMP_UNIT_VMIN:
    return val * 0.01f *
           (g_viewport_width < g_viewport_height ? g_viewport_width
                                                 : g_viewport_height);
  case CMP_UNIT_VMAX:
    return val * 0.01f *
           (g_viewport_width > g_viewport_height ? g_viewport_width
                                                 : g_viewport_height);
  case CMP_UNIT_EM:
  case CMP_UNIT_REM:
    if (parent_val >= 0.0f)
      return val * 0.01f * parent_val;
    return val;
  case CMP_UNIT_PIXELS:
  default:
    return val;
  }
}

static int cmp_layout_measure_pass(cmp_layout_node_t *node,
                                   float max_available_width) {
  int rc = CMP_SUCCESS;
  float intrinsic_w = 0.0f;
  float intrinsic_h = 0.0f;
  size_t i;
  float sum_child_main = 0.0f;
  float max_child_cross = 0.0f;
  float aspect_ratio = node->aspect_ratio;
  int is_row = (node->direction == CMP_FLEX_ROW);
  float child_avail_w;

  if (node->measure_cb != NULL) {
    rc = node->measure_cb(node->measure_ctx, max_available_width, &intrinsic_w,
                          &intrinsic_h);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("measure_cb failed: %d\n", rc);
      return rc;
    }
    node->measured_width = intrinsic_w;
    node->measured_height = intrinsic_h;
  } else {
    child_avail_w = max_available_width;
    if (child_avail_w >= 0.0f) {
      child_avail_w -= (node->padding[1] + node->padding[3] + node->margin[1] +
                        node->margin[3]);
      if (child_avail_w < 0.0f) {
        child_avail_w = 0.0f;
      }
    }

    for (i = 0; i < node->child_count; i++) {
      rc = cmp_layout_measure_pass(node->children[i], child_avail_w);
      if (rc != CMP_SUCCESS) {
        return rc;
      }
    }

    for (i = 0; i < node->child_count; i++) {
      cmp_layout_node_t *child = node->children[i];
      float w = child->measured_width;
      float h = child->measured_height;

      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED)
        continue;

      if (child->width >= 0.0f)
        w = child->width;
      if (child->height >= 0.0f)
        h = child->height;

      w += child->margin[1] + child->margin[3];
      h += child->margin[0] + child->margin[2];

      if (is_row) {
        sum_child_main += w;
        if (h > max_child_cross)
          max_child_cross = h;
      } else {
        sum_child_main += h;
        if (w > max_child_cross)
          max_child_cross = w;
      }
    }

    if (is_row) {
      node->measured_width =
          sum_child_main + node->padding[1] + node->padding[3];
      node->measured_height =
          max_child_cross + node->padding[0] + node->padding[2];
    } else {
      node->measured_height =
          sum_child_main + node->padding[0] + node->padding[2];
      node->measured_width =
          max_child_cross + node->padding[1] + node->padding[3];
    }
  }

  if (node->width >= 0.0f)
    node->measured_width = node->width;
  if (node->height >= 0.0f)
    node->measured_height = node->height;

  /* For overflow nodes, if intrinsic measured dimensions exceed max, we do NOT
     clamp them initially, they represent scroll bounds. They will be clamped
     during position/flex. */
  if (node->overflow_x != 1 && node->max_width >= 0.0f &&
      node->measured_width > node->max_width)
    node->measured_width = node->max_width;
  if (node->overflow_y != 1 && node->max_height >= 0.0f &&
      node->measured_height > node->max_height)
    node->measured_height = node->max_height;

  if (node->min_width >= 0.0f && node->measured_width < node->min_width)
    node->measured_width = node->min_width;
  if (node->min_height >= 0.0f && node->measured_height < node->min_height)
    node->measured_height = node->min_height;

  if (aspect_ratio > 0.0f) {
    if (node->width >= 0.0f && node->height < 0.0f) {
      node->measured_height = node->width / aspect_ratio;
    } else if (node->height >= 0.0f && node->width < 0.0f) {
      node->measured_width = node->height * aspect_ratio;
    }
  }

  return CMP_SUCCESS;
}

static void cmp_layout_resolve_flex_pass(cmp_layout_node_t *node,
                                         cmp_layout_constraints_t constraints) {
  size_t i, j;
  int is_row = (node->direction == CMP_FLEX_ROW);
  float inner_width, inner_height;
  float main_avail, main_avail_for_shrink;
  float global_cross_max = 0.0f;
  float global_main_max = 0.0f;

  if (node->flex_lines != NULL) {
    if (node->flex_lines->lines != NULL) {
      CMP_FREE(node->flex_lines->lines);
      node->flex_lines->lines = NULL;
    }
  } else {
    CMP_MALLOC(sizeof(cmp_layout_lines_t), (void **)&node->flex_lines);
    memset(node->flex_lines, 0, sizeof(cmp_layout_lines_t));
  }

  node->flex_lines->count = 0;
  node->flex_lines->capacity = 0;

  node->computed_rect.width = node->measured_width;
  node->computed_rect.height = node->measured_height;

  /* Block-level flex containers (default) stretch to fill available width if
   * auto */
  if (node->display == CMP_DISPLAY_FLEX && node->width < 0.0f) {
    if (constraints.max_width >= 0.0f &&
        node->computed_rect.width < constraints.max_width) {
      /* If parent is row and we are not grow, we shouldn't stretch unless
         align-items stretch, but if we have no parent (root), we stretch to
         screen. */
      if (node->parent == NULL) {
        node->computed_rect.width = constraints.max_width;
        if (node->height < 0.0f && constraints.max_height >= 0.0f) {
          node->computed_rect.height = constraints.max_height;
        }
      } else {
        /* If parent is column, stretch to parent width */
        if (node->parent->direction == CMP_FLEX_COLUMN &&
            node->parent->align_items == CMP_FLEX_ALIGN_STRETCH) {
          node->computed_rect.width = constraints.max_width;
        }
      }
    }
  }

  /* Clamp to parent constraints */
  if (constraints.max_width >= 0.0f &&
      node->computed_rect.width > constraints.max_width) {
    node->computed_rect.width = constraints.max_width;
  }
  if (constraints.max_height >= 0.0f &&
      node->computed_rect.height > constraints.max_height) {
    node->computed_rect.height = constraints.max_height;
  }
  if (constraints.min_width >= 0.0f &&
      node->computed_rect.width < constraints.min_width) {
    node->computed_rect.width = constraints.min_width;
  }
  if (constraints.min_height >= 0.0f &&
      node->computed_rect.height < constraints.min_height) {
    node->computed_rect.height = constraints.min_height;
  }

  inner_width = node->computed_rect.width - node->padding[1] - node->padding[3];
  inner_height =
      node->computed_rect.height - node->padding[0] - node->padding[2];

  main_avail = is_row ? inner_width : inner_height;
  main_avail_for_shrink = main_avail;
  if ((is_row && node->overflow_x == 1) || (!is_row && node->overflow_y == 1)) {
    main_avail_for_shrink = 999999.0f;
  }

  if (node->child_count > 0) {
    node->flex_lines->capacity = 4;
    CMP_MALLOC(sizeof(cmp_layout_line_t) * node->flex_lines->capacity,
               (void **)&node->flex_lines->lines);
    memset(&node->flex_lines->lines[0], 0, sizeof(cmp_layout_line_t));
    node->flex_lines->count = 1;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    float child_main, child_cross, child_main_margin, child_cross_margin;
    cmp_layout_line_t *cur_line;
    float c_w, c_h;

    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED)
      continue;

    c_w = child->measured_width;
    c_h = child->measured_height;

    child_main = is_row ? c_w : c_h;
    child_cross = is_row ? c_h : c_w;
    child_main_margin = is_row ? (child->margin[1] + child->margin[3])
                               : (child->margin[0] + child->margin[2]);
    child_cross_margin = is_row ? (child->margin[0] + child->margin[2])
                                : (child->margin[1] + child->margin[3]);

    cur_line = &node->flex_lines->lines[node->flex_lines->count - 1];

    if (node->flex_wrap == CMP_FLEX_WRAP && cur_line->count > 0 &&
        cur_line->main_size + child_main + child_main_margin > main_avail) {
      if (node->flex_lines->count >= node->flex_lines->capacity) {
        cmp_layout_line_t *new_lines;
        node->flex_lines->capacity *= 2;
        if (CMP_MALLOC(sizeof(cmp_layout_line_t) * node->flex_lines->capacity,
                       (void **)&new_lines) != CMP_SUCCESS) {
          if (node->flex_lines->lines)
            CMP_FREE(node->flex_lines->lines);
          node->flex_lines->lines = NULL;
          node->flex_lines->count = 0;
          return;
        }
        memcpy(new_lines, node->flex_lines->lines,
               sizeof(cmp_layout_line_t) * node->flex_lines->count);
        CMP_FREE(node->flex_lines->lines);
        node->flex_lines->lines = new_lines;
      }
      cur_line = &node->flex_lines->lines[node->flex_lines->count++];
      memset(cur_line, 0, sizeof(cmp_layout_line_t));
      cur_line->start_index = i;
    }

    if (cur_line->count > 0)
      cur_line->main_size += (is_row ? node->column_gap : node->row_gap);
    cur_line->count++;
    cur_line->main_size += child_main + child_main_margin;
    cur_line->total_flex_grow += child->flex_grow;
    cur_line->total_flex_shrink += child->flex_shrink;

    if (child_cross + child_cross_margin > cur_line->cross_size) {
      cur_line->cross_size = child_cross + child_cross_margin;
    }
  }

  if (node->flex_lines->count == 1) {
    float parent_cross = is_row ? inner_height : inner_width;
    if (parent_cross > node->flex_lines->lines[0].cross_size) {
      node->flex_lines->lines[0].cross_size = parent_cross;
    }
  }

  for (i = 0; i < node->flex_lines->count; i++) {
    cmp_layout_line_t *line = &node->flex_lines->lines[i];
    float current_main_pos = 0.0f;
    size_t processed = 0;
    int items_frozen_this_pass = 1;
    float unfrozen_remaining_main = main_avail_for_shrink - line->main_size;
    float unfrozen_grow = line->total_flex_grow;
    float unfrozen_shrink = line->total_flex_shrink;

    /* Initialize all line items to unfrozen and set their initial final_main */
    for (j = line->start_index;
         j < node->child_count && processed < line->count; j++) {
      cmp_layout_node_t *child = node->children[j];
      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED)
        continue;
      processed++;
      child->is_frozen = 0;
      if (is_row) {
        child->computed_rect.width = child->measured_width;
      } else {
        child->computed_rect.height = child->measured_height;
      }
    }

    /* W3C Flexbox sizing algorithm */
    while (items_frozen_this_pass) {
      items_frozen_this_pass = 0;
      processed = 0;
      for (j = line->start_index;
           j < node->child_count && processed < line->count; j++) {
        cmp_layout_node_t *child = node->children[j];
        float final_main, min_main, max_main;

        if (child->position_type == CMP_POSITION_ABSOLUTE ||
            child->position_type == CMP_POSITION_FIXED)
          continue;
        processed++;

        if (child->is_frozen)
          continue;

        /* Always base growth/shrink on the original intrinsic flex basis */
        final_main = is_row ? child->measured_width : child->measured_height;
        min_main = is_row ? child->min_width : child->min_height;
        max_main = is_row ? child->max_width : child->max_height;
        if (min_main < 0.0f)
          min_main = 0.0f;

        if (unfrozen_grow > 0.0f && unfrozen_remaining_main > 0.0f &&
            unfrozen_remaining_main != 999999.0f) {
          final_main +=
              (child->flex_grow / unfrozen_grow) * unfrozen_remaining_main;
        } else if (unfrozen_shrink > 0.0f && unfrozen_remaining_main < 0.0f) {
          final_main +=
              (child->flex_shrink / unfrozen_shrink) * unfrozen_remaining_main;
        }

        /* Check for freeze constraints */
        if (max_main >= 0.0f && final_main > max_main) {
          final_main = max_main;
          child->is_frozen = 1;
        } else if (final_main < min_main) {
          final_main = min_main;
          child->is_frozen = 1;
        }

        if (child->is_frozen) {
          items_frozen_this_pass = 1;
          unfrozen_grow -= child->flex_grow;
          unfrozen_shrink -= child->flex_shrink;

          /* Subtract the difference between the frozen size and the original
             flex basis from the available free space. Since
             `unfrozen_remaining_main` originally included this difference as
             available space, we must re-calculate it or simply update it. */
          unfrozen_remaining_main -=
              (final_main -
               (is_row ? child->measured_width : child->measured_height));

          if (is_row) {
            child->computed_rect.width = final_main;
          } else {
            child->computed_rect.height = final_main;
          }

          /* Break out to recalculate remaining unfrozen items with the new
           * remaining space and factors */
          break;
        }

        if (is_row) {
          child->computed_rect.width = final_main;
        } else {
          child->computed_rect.height = final_main;
        }
      }
    }

    processed = 0;
    for (j = line->start_index;
         j < node->child_count && processed < line->count; j++) {
      cmp_layout_node_t *child = node->children[j];
      float final_main, final_cross;
      cmp_layout_constraints_t child_constraints;
      cmp_flex_align_t stretch_align;

      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED)
        continue;
      processed++;

      final_main =
          is_row ? child->computed_rect.width : child->computed_rect.height;
      final_cross = is_row ? child->measured_height : child->measured_width;

      stretch_align = child->align_self != CMP_FLEX_ALIGN_AUTO
                          ? child->align_self
                          : node->align_items;

      /* Stretch */
      if (stretch_align == CMP_FLEX_ALIGN_STRETCH &&
          (is_row ? child->height < 0.0f : child->width < 0.0f)) {
        float child_cross_margin = is_row
                                       ? (child->margin[0] + child->margin[2])
                                       : (child->margin[1] + child->margin[3]);
        float target_cross =
            line->cross_size > 0
                ? (line->cross_size - child_cross_margin)
                : (is_row ? inner_height : inner_width) - child_cross_margin;
        if (target_cross > final_cross) {
          final_cross = target_cross;
        }
      }

      child_constraints.min_width = is_row ? final_main : final_cross;
      child_constraints.max_width = child_constraints.min_width;
      child_constraints.min_height = is_row ? final_cross : final_main;
      child_constraints.max_height = child_constraints.min_height;

      cmp_layout_calculate_node(child, &child_constraints);

      current_main_pos +=
          final_main + (is_row ? (child->margin[1] + child->margin[3])
                               : (child->margin[0] + child->margin[2]));
      if (processed < line->count)
        current_main_pos += (is_row ? node->column_gap : node->row_gap);
    }

    if (current_main_pos > global_main_max)
      global_main_max = current_main_pos;

    if (line->cross_size == 0.0f && line->count > 0) {
      float max_c = 0.0f;
      size_t processed_c = 0;
      for (j = line->start_index;
           j < node->child_count && processed_c < line->count; j++) {
        float c_size;
        cmp_layout_node_t *child = node->children[j];
        if (child->position_type == CMP_POSITION_ABSOLUTE ||
            child->position_type == CMP_POSITION_FIXED)
          continue;
        processed_c++;
        c_size = is_row ? (child->computed_rect.height + child->margin[0] +
                           child->margin[2])
                        : (child->computed_rect.width + child->margin[1] +
                           child->margin[3]);
        if (c_size > max_c)
          max_c = c_size;
      }
      line->cross_size = max_c;
    }

    global_cross_max += line->cross_size;
    if (i < node->flex_lines->count - 1) {
      global_cross_max += is_row ? node->row_gap : node->column_gap;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED) {
      cmp_layout_constraints_t abs_constraints = {-1.0f, -1.0f, -1.0f, -1.0f};
      cmp_layout_calculate_node(child, &abs_constraints);
    }
  }

  if (node->width < 0.0f && node->aspect_ratio <= 0.0f) {
    node->computed_rect.width =
        is_row ? (global_main_max + node->padding[1] + node->padding[3])
               : (global_cross_max + node->padding[1] + node->padding[3]);
  }
  if (node->height < 0.0f && node->aspect_ratio <= 0.0f) {
    node->computed_rect.height =
        is_row ? (global_cross_max + node->padding[0] + node->padding[2])
               : (global_main_max + node->padding[0] + node->padding[2]);
  }

  if (constraints.max_width >= 0.0f &&
      node->computed_rect.width > constraints.max_width)
    node->computed_rect.width = constraints.max_width;
  if (constraints.max_height >= 0.0f &&
      node->computed_rect.height > constraints.max_height)
    node->computed_rect.height = constraints.max_height;
  if (constraints.min_width >= 0.0f &&
      node->computed_rect.width < constraints.min_width)
    node->computed_rect.width = constraints.min_width;
  if (constraints.min_height >= 0.0f &&
      node->computed_rect.height < constraints.min_height)
    node->computed_rect.height = constraints.min_height;
}

static void cmp_layout_position_pass(cmp_layout_node_t *node, float parent_x,
                                     float parent_y) {
  size_t i, j;
  float current_x = parent_x + node->margin[3];
  float current_y = parent_y + node->margin[0];
  int is_row = (node->direction == CMP_FLEX_ROW);
  float inner_width, inner_height;
  float main_avail, cross_pos;

  float total_cross_size = 0.0f;
  float remaining_cross = 0.0f;
  float cross_offset = 0.0f;
  float cross_spacing = 0.0f;

  if (node->position_type == CMP_POSITION_ABSOLUTE) {
    /* Step 2 Absolute Positioning Solver:
       Calculate top/right/bottom/left relative to nearest positioned ancestor.
       If an offset is set, we defer to it over flow coordinates. */

    /* Left / Right */
    if (node->position[3] >= 0.0f) {
      current_x = parent_x + node->position[3];
    } else if (node->position[1] >= 0.0f) {
      if (node->parent != NULL) {
        current_x = parent_x + node->parent->computed_rect.width -
                    node->computed_rect.width - node->position[1];
      }
    }

    /* Top / Bottom */
    if (node->position[0] >= 0.0f) {
      current_y = parent_y + node->position[0];
    } else if (node->position[2] >= 0.0f) {
      if (node->parent != NULL) {
        current_y = parent_y + node->parent->computed_rect.height -
                    node->computed_rect.height - node->position[2];
      }
    }
  }

  node->computed_rect.x = current_x;
  node->computed_rect.y = current_y;

  if (node->position_type == CMP_POSITION_STICKY) {
    /* Step 3 Sticky Positioning:
       Acts relatively but clamps to nearest scroll container when crossed.
       For a fully accurate implementation, we walk up to the scroll parent.
       Since we assume `parent` might be the scroller here for minimal MVP: */
    if (node->parent != NULL) {
      float scroll_x = node->parent->scroll_offset_x;
      float scroll_y = node->parent->scroll_offset_y;

      /* Clamp Top */
      if (node->position[0] >= 0.0f) {
        float sticky_top_bound = parent_y + scroll_y + node->position[0];
        if (node->computed_rect.y < sticky_top_bound) {
          node->computed_rect.y = sticky_top_bound;
        }
      }
      /* Clamp Left */
      if (node->position[3] >= 0.0f) {
        float sticky_left_bound = parent_x + scroll_x + node->position[3];
        if (node->computed_rect.x < sticky_left_bound) {
          node->computed_rect.x = sticky_left_bound;
        }
      }
    }
  }

  inner_width = node->computed_rect.width - node->padding[1] - node->padding[3];
  inner_height =
      node->computed_rect.height - node->padding[0] - node->padding[2];
  main_avail = is_row ? inner_width : inner_height;

  for (i = 0; i < node->flex_lines->count; i++) {
    total_cross_size += node->flex_lines->lines[i].cross_size;
    if (i < node->flex_lines->count - 1) {
      total_cross_size += is_row ? node->row_gap : node->column_gap;
    }
  }

  remaining_cross = (is_row ? inner_height : inner_width) - total_cross_size;
  if (remaining_cross > 0.0f && node->flex_lines->count > 0) {
    if (node->align_content == CMP_FLEX_ALIGN_CENTER) {
      cross_offset = remaining_cross * 0.5f;
    } else if (node->align_content == CMP_FLEX_ALIGN_END) {
      cross_offset = remaining_cross;
    } else if (node->align_content == CMP_FLEX_ALIGN_SPACE_BETWEEN &&
               node->flex_lines->count > 1) {
      cross_spacing = remaining_cross / (node->flex_lines->count - 1);
    } else if (node->align_content == CMP_FLEX_ALIGN_SPACE_AROUND) {
      cross_spacing = remaining_cross / node->flex_lines->count;
      cross_offset = cross_spacing * 0.5f;
    }
  }

  cross_pos = is_row ? (current_y + node->padding[0] + cross_offset)
                     : (current_x + node->padding[3] + cross_offset);

  for (i = 0; i < node->flex_lines->count; i++) {
    cmp_layout_line_t *line = &node->flex_lines->lines[i];
    float line_actual_main_size = 0.0f;
    float remaining_main;
    float main_offset = 0.0f;
    float spacing = 0.0f;
    float main_pos;
    size_t processed = 0;

    for (j = line->start_index;
         j < node->child_count && processed < line->count; j++) {
      cmp_layout_node_t *child = node->children[j];
      float child_main, child_main_margin;
      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED)
        continue;

      child_main =
          is_row ? child->computed_rect.width : child->computed_rect.height;
      child_main_margin = is_row ? (child->margin[1] + child->margin[3])
                                 : (child->margin[0] + child->margin[2]);

      if (processed > 0) {
        line_actual_main_size += (is_row ? node->column_gap : node->row_gap);
      }
      line_actual_main_size += child_main + child_main_margin;
      processed++;
    }

    remaining_main = main_avail - line_actual_main_size;
    processed = 0;

    if (remaining_main > 0.0f) {
      if (node->justify_content == CMP_FLEX_ALIGN_CENTER) {
        main_offset = remaining_main * 0.5f;
      } else if (node->justify_content == CMP_FLEX_ALIGN_END) {
        main_offset = remaining_main;
      } else if (node->justify_content == CMP_FLEX_ALIGN_SPACE_BETWEEN &&
                 line->count > 1) {
        spacing = remaining_main / (line->count - 1);
      } else if (node->justify_content == CMP_FLEX_ALIGN_SPACE_AROUND &&
                 line->count > 0) {
        spacing = remaining_main / line->count;
        main_offset = spacing * 0.5f;
      }
    }

    main_pos = is_row ? (current_x + node->padding[3])
                      : (current_y + node->padding[0]);
    main_pos += main_offset;

    for (j = line->start_index;
         j < node->child_count && processed < line->count; j++) {
      cmp_layout_node_t *child = node->children[j];
      float final_main =
          is_row ? child->computed_rect.width : child->computed_rect.height;
      float final_cross =
          is_row ? child->computed_rect.height : child->computed_rect.width;
      float c_x, c_y;
      cmp_flex_align_t alignment = node->align_items;

      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED)
        continue;
      processed++;

      if (child->align_self != CMP_FLEX_ALIGN_AUTO) {
        alignment = child->align_self;
      }

      c_x = 0;
      c_y = 0;
      if (is_row) {
        c_x = main_pos;
        c_y = cross_pos;
        if (alignment == CMP_FLEX_ALIGN_CENTER) {
          c_y += (line->cross_size - final_cross) * 0.5f;
        } else if (alignment == CMP_FLEX_ALIGN_END) {
          c_y += (line->cross_size - final_cross);
        }
      } else {
        c_y = main_pos;
        c_x = cross_pos;
        if (alignment == CMP_FLEX_ALIGN_CENTER) {
          c_x += (line->cross_size - final_cross) * 0.5f;
        } else if (alignment == CMP_FLEX_ALIGN_END) {
          c_x += (line->cross_size - final_cross);
        }
      }

      cmp_layout_position_pass(child, c_x, c_y);

      main_pos += final_main +
                  (is_row ? (child->margin[1] + child->margin[3])
                          : (child->margin[0] + child->margin[2])) +
                  spacing;
      if (processed < line->count)
        main_pos += (is_row ? node->column_gap : node->row_gap);
    }

    cross_pos += line->cross_size + cross_spacing;
    if (i < node->flex_lines->count - 1) {
      cross_pos += is_row ? node->row_gap : node->column_gap;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED) {
      cmp_layout_position_pass(child, current_x + node->padding[3],
                               current_y + node->padding[0]);
    }
  }

  if (node->overflow_x == 1 || node->overflow_y == 1) {
    float content_w = 0.0f;
    float content_h = 0.0f;
    for (i = 0; i < node->child_count; i++) {
      cmp_layout_node_t *child = node->children[i];
      float child_right = child->computed_rect.x + child->computed_rect.width -
                          node->computed_rect.x;
      float child_bottom = child->computed_rect.y +
                           child->computed_rect.height - node->computed_rect.y;
      if (child_right > content_w)
        content_w = child_right;
      if (child_bottom > content_h)
        content_h = child_bottom;
    }
    node->scroll_content_size.width = content_w;
    node->scroll_content_size.height = content_h;
  }
}

static void cmp_layout_responsive_pass(cmp_layout_node_t *node,
                                       float available_width) {
  size_t i;
  if (!node)
    return;

  if (available_width < 300.0f) {
    if (node->direction == CMP_FLEX_ROW) {
      node->direction = CMP_FLEX_COLUMN;
    }
    if (node->parent == NULL) {
      node->font_size = available_width * 0.05f;
    }
  } else if (available_width >= 300.0f && available_width < 600.0f) {
    if (node->parent == NULL) {
      node->font_size = available_width * 0.045f;
    }
  } else if (available_width >= 600.0f && available_width < 900.0f) {
    if (node->direction == CMP_FLEX_ROW) {
      node->flex_wrap = CMP_FLEX_WRAP;
    }
    if (node->parent == NULL) {
      node->font_size = available_width * 0.04f;
    }
  } else if (available_width >= 900.0f && available_width < 1200.0f) {
    if (node->parent == NULL) {
      node->font_size = available_width * 0.03f;
    }
  } else if (available_width >= 1200.0f && available_width < 1920.0f) {
    if (node->parent == NULL) {
      node->font_size = available_width * 0.02f;
    }
  } else if (available_width >= 1920.0f) {
    if (node->parent == NULL) {
      node->margin[0] = available_width * 0.05f;
      node->margin[1] = available_width * 0.05f;
      node->margin[2] = available_width * 0.05f;
      node->margin[3] = available_width * 0.05f;
      node->font_size = available_width * 0.015f;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_responsive_pass(node->children[i], available_width);
  }
}

static void apply_rtl_mirroring(cmp_layout_node_t *node);
static void cmp_layout_responsive_pass(cmp_layout_node_t *node,
                                       float available_width);

int cmp_layout_get_min_content(cmp_layout_node_t *node, float *out_width) {
  int rc = CMP_SUCCESS;
  float max_child_min = 0.0f;
  float sum_child_min = 0.0f;
  size_t i;

  if (node == NULL || out_width == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (node->width >= 0.0f) {
    *out_width = node->width;
    return CMP_SUCCESS;
  }

  if (node->measure_cb != NULL) {
    float h = 0.0f;
    rc = node->measure_cb(node->measure_ctx, 0.0f, out_width, &h);
    return rc;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    float child_min = 0.0f;

    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED)
      continue;

    rc = cmp_layout_get_min_content(child, &child_min);
    if (rc != CMP_SUCCESS)
      return rc;

    child_min += child->margin[1] + child->margin[3] + child->padding[1] +
                 child->padding[3];

    if (child_min > max_child_min)
      max_child_min = child_min;
    sum_child_min += child_min;
  }

  if (node->display == CMP_DISPLAY_INLINE ||
      node->display == CMP_DISPLAY_INLINE_BLOCK ||
      (node->display == CMP_DISPLAY_FLEX && node->direction == CMP_FLEX_ROW)) {
    *out_width = sum_child_min;
  } else {
    *out_width = max_child_min;
  }

  if (node->min_width >= 0.0f && *out_width < node->min_width) {
    *out_width = node->min_width;
  }
  if (node->max_width >= 0.0f && *out_width > node->max_width) {
    *out_width = node->max_width;
  }

  return CMP_SUCCESS;
}

int cmp_layout_get_max_content(cmp_layout_node_t *node, float *out_width) {
  int rc = CMP_SUCCESS;
  float max_child_max = 0.0f;
  float sum_child_max = 0.0f;
  size_t i;

  if (node == NULL || out_width == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (node->width >= 0.0f) {
    *out_width = node->width;
    return CMP_SUCCESS;
  }

  if (node->measure_cb != NULL) {
    float h = 0.0f;
    /* Pass a large width to get ideal layout without wrapping */
    rc = node->measure_cb(node->measure_ctx, 999999.0f, out_width, &h);
    return rc;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    float child_max = 0.0f;

    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED)
      continue;

    rc = cmp_layout_get_max_content(child, &child_max);
    if (rc != CMP_SUCCESS)
      return rc;

    child_max += child->margin[1] + child->margin[3] + child->padding[1] +
                 child->padding[3];

    if (child_max > max_child_max)
      max_child_max = child_max;
    sum_child_max += child_max;
  }

  if (node->display == CMP_DISPLAY_INLINE ||
      node->display == CMP_DISPLAY_INLINE_BLOCK ||
      (node->display == CMP_DISPLAY_FLEX && node->direction == CMP_FLEX_ROW)) {
    *out_width = sum_child_max;
  } else {
    *out_width = max_child_max;
  }

  if (node->min_width >= 0.0f && *out_width < node->min_width) {
    *out_width = node->min_width;
  }
  if (node->max_width >= 0.0f && *out_width > node->max_width) {
    *out_width = node->max_width;
  }

  return CMP_SUCCESS;
}

int cmp_layout_bfc(cmp_layout_node_t *node,
                   const cmp_layout_constraints_t *constraints) {
  int rc = CMP_SUCCESS;
  size_t i;
  float current_y = 0.0f;
  float max_child_width = 0.0f;
  float prev_margin_bottom = 0.0f;
  float available_child_w;

  if (node == NULL || constraints == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  available_child_w = constraints->max_width;
  if (available_child_w >= 0.0f) {
    available_child_w -= (node->padding[1] + node->padding[3]);
    if (available_child_w < 0.0f) {
      available_child_w = 0.0f;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    cmp_layout_constraints_t child_constraints;
    float child_margin_top;
    float collapsed_margin;

    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED) {
      continue;
    }

    child_constraints.min_width = CMP_LAYOUT_AUTO;
    child_constraints.max_width = available_child_w;
    child_constraints.min_height = CMP_LAYOUT_AUTO;
    child_constraints.max_height = constraints->max_height;

    rc = cmp_layout_calculate_node(child, &child_constraints);
    if (rc != CMP_SUCCESS) {
      return rc;
    }

    child_margin_top = child->margin[0];

    /* W3C Margin Collapsing (Adjacent siblings) */
    if (prev_margin_bottom > child_margin_top) {
      collapsed_margin = prev_margin_bottom;
    } else {
      collapsed_margin = child_margin_top;
    }

    current_y += collapsed_margin;

    child->computed_rect.x = child->margin[3] + node->padding[3];
    child->computed_rect.y = current_y + node->padding[0];
    child->computed_rect.width = child->measured_width;
    child->computed_rect.height = child->measured_height;

    if (child->width != CMP_LAYOUT_AUTO && child->width > 0.0f) {
      child->computed_rect.width = child->width;
    } else if (child->display == CMP_DISPLAY_BLOCK) {
      /* Block takes 100% of parent width by default */
      child->computed_rect.width =
          available_child_w - (child->margin[1] + child->margin[3]);
    }

    current_y += child->computed_rect.height;
    prev_margin_bottom = child->margin[2];

    if (child->computed_rect.width + child->margin[1] + child->margin[3] >
        max_child_width) {
      max_child_width =
          child->computed_rect.width + child->margin[1] + child->margin[3];
    }
  }

  current_y += prev_margin_bottom;

  node->measured_width = max_child_width + node->padding[1] + node->padding[3];
  node->measured_height = current_y + node->padding[0] + node->padding[2];

  if (node->width != CMP_LAYOUT_AUTO && node->width >= 0.0f) {
    node->measured_width = node->width;
  }
  if (node->height != CMP_LAYOUT_AUTO && node->height >= 0.0f) {
    node->measured_height = node->height;
  }

  node->computed_rect.width = node->measured_width;
  node->computed_rect.height = node->measured_height;

  return rc;
}

int cmp_layout_ifc(cmp_layout_node_t *node,
                   const cmp_layout_constraints_t *constraints) {
  int rc = CMP_SUCCESS;
  size_t i;
  float current_x = 0.0f;
  float current_y = 0.0f;
  float max_line_height = 0.0f;
  float max_width = 0.0f;
  float available_child_w;

  if (node == NULL || constraints == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  available_child_w = constraints->max_width;
  if (available_child_w >= 0.0f) {
    available_child_w -= (node->padding[1] + node->padding[3]);
    if (available_child_w < 0.0f) {
      available_child_w = 0.0f;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    cmp_layout_constraints_t child_constraints;
    float child_total_w;

    if (child->position_type == CMP_POSITION_ABSOLUTE ||
        child->position_type == CMP_POSITION_FIXED) {
      continue;
    }

    child_constraints.min_width = CMP_LAYOUT_AUTO;
    child_constraints.max_width = available_child_w;
    child_constraints.min_height = CMP_LAYOUT_AUTO;
    child_constraints.max_height = constraints->max_height;

    rc = cmp_layout_calculate_node(child, &child_constraints);
    if (rc != CMP_SUCCESS) {
      return rc;
    }

    child_total_w = child->measured_width + child->margin[1] + child->margin[3];

    if (current_x + child_total_w > available_child_w && current_x > 0.0f) {
      /* Line wrap */
      current_x = 0.0f;
      current_y += max_line_height;
      max_line_height = 0.0f;
    }

    child->computed_rect.x = current_x + child->margin[3] + node->padding[3];
    child->computed_rect.y = current_y + child->margin[0] + node->padding[0];
    child->computed_rect.width = child->measured_width;
    child->computed_rect.height = child->measured_height;

    current_x += child_total_w;
    if (current_x > max_width) {
      max_width = current_x;
    }

    if (child->measured_height + child->margin[0] + child->margin[2] >
        max_line_height) {
      max_line_height =
          child->measured_height + child->margin[0] + child->margin[2];
    }
  }

  current_y += max_line_height;

  node->measured_width = max_width + node->padding[1] + node->padding[3];
  node->measured_height = current_y + node->padding[0] + node->padding[2];

  if (node->width != CMP_LAYOUT_AUTO && node->width >= 0.0f) {
    node->measured_width = node->width;
  }
  if (node->height != CMP_LAYOUT_AUTO && node->height >= 0.0f) {
    node->measured_height = node->height;
  }

  node->computed_rect.width = node->measured_width;
  node->computed_rect.height = node->measured_height;

  return rc;
}

int cmp_layout_gfc(cmp_layout_node_t *node,
                   const cmp_layout_constraints_t *constraints) {
  size_t i;
  float total_width = 0.0f;
  float total_height = 0.0f;

  if (node->grid_ctx == NULL) {
    /* If no explicitly configured grid context exists, behave like a block
     * wrapper */
    return cmp_layout_bfc(node, constraints);
  }

  /* 1. Track Sizing Definitions
   * For MVP: Initialize simple explicit tracks. In full implementations this
   * uses cmp_grid_track_evaluate on minmax constraints. */
  if (node->grid_ctx->computed_col_sizes)
    CMP_FREE(node->grid_ctx->computed_col_sizes);
  if (node->grid_ctx->computed_row_sizes)
    CMP_FREE(node->grid_ctx->computed_row_sizes);

  node->grid_ctx->computed_col_sizes = NULL;
  node->grid_ctx->computed_row_sizes = NULL;

  /* Initial explicit counts */
  node->grid_ctx->computed_col_count = node->grid_ctx->template_columns_count;
  node->grid_ctx->computed_row_count = node->grid_ctx->template_rows_count;

  /* Scan children to determine if implicit tracks are required (Step 4) */
  {
    int max_col = (int)node->grid_ctx->template_columns_count;
    int max_row = (int)node->grid_ctx->template_rows_count;
    int current_auto_col = 0;
    int current_auto_row = 0;

    for (i = 0; i < node->child_count; i++) {
      cmp_layout_node_t *child = node->children[i];
      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED)
        continue;

      if (child->grid_column_start != 0 || child->grid_row_start != 0 ||
          child->grid_column_end != 0 || child->grid_row_end != 0) {
        /* Explicit placement */
        int end_c = child->grid_column_end > child->grid_column_start
                        ? child->grid_column_end
                        : child->grid_column_start;
        int end_r = child->grid_row_end > child->grid_row_start
                        ? child->grid_row_end
                        : child->grid_row_start;
        if (end_c > max_col)
          max_col = end_c;
        if (end_r > max_row)
          max_row = end_r;
      } else {
        /* Auto placement */
        current_auto_col++;
        if (current_auto_col >= max_col && max_col > 0) {
          current_auto_col = 0;
          current_auto_row++;
        }
        if (current_auto_row + 1 > max_row) {
          max_row = current_auto_row + 1;
        }
      }
    }
    node->grid_ctx->computed_col_count = max_col;
    node->grid_ctx->computed_row_count = max_row;
  }

  if (node->grid_ctx->computed_col_count > 0) {
    CMP_MALLOC(sizeof(float) * node->grid_ctx->computed_col_count,
               (void **)&node->grid_ctx->computed_col_sizes);
    memset(node->grid_ctx->computed_col_sizes, 0,
           sizeof(float) * node->grid_ctx->computed_col_count);
  }

  if (node->grid_ctx->computed_row_count > 0) {
    CMP_MALLOC(sizeof(float) * node->grid_ctx->computed_row_count,
               (void **)&node->grid_ctx->computed_row_sizes);
    memset(node->grid_ctx->computed_row_sizes, 0,
           sizeof(float) * node->grid_ctx->computed_row_count);
  }

  /* 2. Fractional Units (`fr`) Distribute:
   * Normally handled via minmax/fr distributions. For this minimal layout
   * skeleton we measure children to derive auto sizing, then distribute
   * remaining fr. */

  /* First pass: Measure intrinsic children */
  for (i = 0; i < node->child_count; i++) {
    cmp_layout_measure_pass(node->children[i], constraints->max_width);
  }

  /* Evaluate Fixed & Percent Tracks (Explicit and Implicit) */
  for (i = 0; i < node->grid_ctx->computed_col_count; i++) {
    cmp_grid_track_size_t track;
    if (i < node->grid_ctx->template_columns_count) {
      track = node->grid_ctx->template_columns[i];
    } else {
      /* Implicit Column Track */
      if (node->grid_ctx->auto_columns_count > 0) {
        track =
            node->grid_ctx
                ->auto_columns[(i - node->grid_ctx->template_columns_count) %
                               node->grid_ctx->auto_columns_count];
      } else {
        track.type = CMP_GRID_TRACK_AUTO;
        track.value = 0.0f;
      }
    }

    if (track.type == CMP_GRID_TRACK_FIXED) {
      node->grid_ctx->computed_col_sizes[i] = track.value;
    } else if (track.type == CMP_GRID_TRACK_PERCENTAGE) {
      node->grid_ctx->computed_col_sizes[i] =
          track.value * 0.01f * constraints->max_width;
    }
  }

  for (i = 0; i < node->grid_ctx->computed_row_count; i++) {
    cmp_grid_track_size_t track;
    if (i < node->grid_ctx->template_rows_count) {
      track = node->grid_ctx->template_rows[i];
    } else {
      /* Implicit Row Track */
      if (node->grid_ctx->auto_rows_count > 0) {
        track = node->grid_ctx
                    ->auto_rows[(i - node->grid_ctx->template_rows_count) %
                                node->grid_ctx->auto_rows_count];
      } else {
        track.type = CMP_GRID_TRACK_AUTO;
        track.value = 0.0f;
      }
    }

    if (track.type == CMP_GRID_TRACK_FIXED) {
      node->grid_ctx->computed_row_sizes[i] = track.value;
    } else if (track.type == CMP_GRID_TRACK_PERCENTAGE) {
      node->grid_ctx->computed_row_sizes[i] =
          track.value * 0.01f * constraints->max_height;
    }
  }

  /* Distribute fr tracks - simple implementation */
  if (constraints->max_width >= 0.0f) {
    float used_w = 0.0f;
    float fr_sum = 0.0f;
    float fr_value = 0.0f;
    for (i = 0; i < node->grid_ctx->computed_col_count; i++) {
      cmp_grid_track_size_t track;
      if (i < node->grid_ctx->template_columns_count)
        track = node->grid_ctx->template_columns[i];
      else if (node->grid_ctx->auto_columns_count > 0)
        track =
            node->grid_ctx
                ->auto_columns[(i - node->grid_ctx->template_columns_count) %
                               node->grid_ctx->auto_columns_count];
      else {
        track.type = CMP_GRID_TRACK_AUTO;
        track.value = 0;
      }

      if (track.type == CMP_GRID_TRACK_FR) {
        fr_sum += track.value;
      } else {
        used_w += node->grid_ctx->computed_col_sizes[i];
      }
    }
    if (fr_sum > 0.0f) {
      fr_value = (constraints->max_width - used_w) / fr_sum;
      if (fr_value < 0.0f)
        fr_value = 0.0f;
      for (i = 0; i < node->grid_ctx->computed_col_count; i++) {
        cmp_grid_track_size_t track;
        if (i < node->grid_ctx->template_columns_count)
          track = node->grid_ctx->template_columns[i];
        else if (node->grid_ctx->auto_columns_count > 0)
          track =
              node->grid_ctx
                  ->auto_columns[(i - node->grid_ctx->template_columns_count) %
                                 node->grid_ctx->auto_columns_count];
        else {
          track.type = CMP_GRID_TRACK_AUTO;
          track.value = 0;
        }

        if (track.type == CMP_GRID_TRACK_FR) {
          node->grid_ctx->computed_col_sizes[i] = track.value * fr_value;
        }
      }
    }
  }

  /* 3. Grid Line Placement & Positioning pass */
  {
    int current_auto_row = 0;
    int current_auto_col = 0;

    for (i = 0; i < node->child_count; i++) {
      cmp_layout_node_t *child = node->children[i];
      float c_x = node->margin[3] + node->padding[3];
      float c_y = node->margin[0] + node->padding[0];
      int col_idx = current_auto_col;
      int row_idx = current_auto_row;
      size_t c_idx;

      if (child->position_type == CMP_POSITION_ABSOLUTE ||
          child->position_type == CMP_POSITION_FIXED) {
        /* Phase 6 Out-of-Flow Deferral - Abs positions skip track placement */
        continue;
      }

      /* Explicit vs Auto Placement */
      if (child->grid_column_start != 0 || child->grid_row_start != 0) {
        col_idx =
            child->grid_column_start > 0 ? child->grid_column_start - 1 : 0;
        row_idx = child->grid_row_start > 0 ? child->grid_row_start - 1 : 0;
      } else {
        col_idx = current_auto_col;
        row_idx = current_auto_row;

        /* Auto-placement simple bump */
        current_auto_col++;
        if (current_auto_col >= (int)node->grid_ctx->computed_col_count) {
          current_auto_col = 0;
          current_auto_row++;
        }
      }

      for (c_idx = 0; c_idx < (size_t)col_idx &&
                      c_idx < node->grid_ctx->computed_col_count;
           c_idx++) {
        c_x += node->grid_ctx->computed_col_sizes[c_idx] +
               node->grid_ctx->column_gap;
      }
      for (c_idx = 0; c_idx < (size_t)row_idx &&
                      c_idx < node->grid_ctx->computed_row_count;
           c_idx++) {
        c_y +=
            node->grid_ctx->computed_row_sizes[c_idx] + node->grid_ctx->row_gap;
      }

      child->computed_rect.x = c_x + child->margin[3];
      child->computed_rect.y = c_y + child->margin[0];

      /* Handle spanning / ending constraints */
      {
        float explicit_w = 0.0f;
        float explicit_h = 0.0f;
        int end_col =
            child->grid_column_end > 0 ? child->grid_column_end - 1 : col_idx;
        int end_row =
            child->grid_row_end > 0 ? child->grid_row_end - 1 : row_idx;

        if (end_col < col_idx)
          end_col = col_idx;
        if (end_row < row_idx)
          end_row = row_idx;

        for (c_idx = col_idx; c_idx <= (size_t)end_col &&
                              c_idx < node->grid_ctx->computed_col_count;
             c_idx++) {
          explicit_w += node->grid_ctx->computed_col_sizes[c_idx];
          if (c_idx < (size_t)end_col)
            explicit_w += node->grid_ctx->column_gap;
        }

        for (c_idx = row_idx; c_idx <= (size_t)end_row &&
                              c_idx < node->grid_ctx->computed_row_count;
             c_idx++) {
          explicit_h += node->grid_ctx->computed_row_sizes[c_idx];
          if (c_idx < (size_t)end_row)
            explicit_h += node->grid_ctx->row_gap;
        }

        if (col_idx < (int)node->grid_ctx->computed_col_count) {
          child->computed_rect.width =
              explicit_w - child->margin[1] - child->margin[3];
        } else {
          child->computed_rect.width = child->measured_width;
        }

        if (row_idx < (int)node->grid_ctx->computed_row_count) {
          child->computed_rect.height =
              explicit_h - child->margin[0] - child->margin[2];
        } else {
          child->computed_rect.height = child->measured_height;
        }
      }
    }
  }

  /* Compute intrinsic parent size */
  for (i = 0; i < node->grid_ctx->computed_col_count; i++) {
    total_width += node->grid_ctx->computed_col_sizes[i];
    if (i < node->grid_ctx->computed_col_count - 1)
      total_width += node->grid_ctx->column_gap;
  }
  for (i = 0; i < node->grid_ctx->computed_row_count; i++) {
    total_height += node->grid_ctx->computed_row_sizes[i];
    if (i < node->grid_ctx->computed_row_count - 1)
      total_height += node->grid_ctx->row_gap;
  }

  node->measured_width = total_width + node->padding[1] + node->padding[3];
  node->measured_height = total_height + node->padding[0] + node->padding[2];

  if (node->width >= 0.0f)
    node->measured_width = node->width;
  if (node->height >= 0.0f)
    node->measured_height = node->height;

  node->computed_rect.width = node->measured_width;
  node->computed_rect.height = node->measured_height;

  return CMP_SUCCESS;
}

int cmp_layout_ffc(cmp_layout_node_t *node,
                   const cmp_layout_constraints_t *constraints) {
  int rc;
  rc = cmp_layout_measure_pass(node, constraints->max_width);
  if (rc != CMP_SUCCESS) {
    return rc;
  }
  cmp_layout_resolve_flex_pass(node, *constraints);
  cmp_layout_position_pass(node, 0.0f, 0.0f);
  return CMP_SUCCESS;
}

int cmp_layout_calculate_node(cmp_layout_node_t *node,
                              const cmp_layout_constraints_t *constraints) {
  int j;
  float parent_width;
  float parent_height;
  float v1, v2;
  if (node == NULL || constraints == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (node->display == CMP_DISPLAY_NONE) {
    node->measured_width = 0.0f;
    node->measured_height = 0.0f;
    return CMP_SUCCESS;
  }

  parent_width = constraints->max_width >= 0.0f ? constraints->max_width : 0.0f;
  parent_height =
      constraints->max_height >= 0.0f ? constraints->max_height : 0.0f;

  /* Calc Resolution */
  if (node->width_calc != NULL) {
    v1 = cmp_resolve_unit(node->width_calc->val1, node->width_calc->unit1,
                          parent_width);
    v2 = cmp_resolve_unit(node->width_calc->val2, node->width_calc->unit2,
                          parent_width);
    switch (node->width_calc->op) {
    case CMP_CALC_OP_ADD:
      node->width = v1 + v2;
      break;
    case CMP_CALC_OP_SUB:
      node->width = v1 - v2;
      break;
    case CMP_CALC_OP_MUL:
      node->width = v1 * v2;
      break;
    case CMP_CALC_OP_DIV:
      node->width = v2 != 0.0f ? v1 / v2 : 0.0f;
      break;
    default:
      break;
    }
    node->width_unit = CMP_UNIT_PIXELS;
  }

  if (node->height_calc != NULL) {
    v1 = cmp_resolve_unit(node->height_calc->val1, node->height_calc->unit1,
                          parent_height);
    v2 = cmp_resolve_unit(node->height_calc->val2, node->height_calc->unit2,
                          parent_height);
    switch (node->height_calc->op) {
    case CMP_CALC_OP_ADD:
      node->height = v1 + v2;
      break;
    case CMP_CALC_OP_SUB:
      node->height = v1 - v2;
      break;
    case CMP_CALC_OP_MUL:
      node->height = v1 * v2;
      break;
    case CMP_CALC_OP_DIV:
      node->height = v2 != 0.0f ? v1 / v2 : 0.0f;
      break;
    default:
      break;
    }
    node->height_unit = CMP_UNIT_PIXELS;
  }

  /* Unit Resolution */
  node->width = cmp_resolve_unit(node->width, node->width_unit, parent_width);
  if (node->width_unit != CMP_UNIT_PIXELS && node->width >= 0.0f)
    node->width_unit = CMP_UNIT_PIXELS;

  node->height =
      cmp_resolve_unit(node->height, node->height_unit, parent_height);
  if (node->height_unit != CMP_UNIT_PIXELS && node->height >= 0.0f)
    node->height_unit = CMP_UNIT_PIXELS;

  node->min_width =
      cmp_resolve_unit(node->min_width, node->min_width_unit, parent_width);
  if (node->min_width_unit != CMP_UNIT_PIXELS && node->min_width >= 0.0f)
    node->min_width_unit = CMP_UNIT_PIXELS;

  node->max_width =
      cmp_resolve_unit(node->max_width, node->max_width_unit, parent_width);
  if (node->max_width_unit != CMP_UNIT_PIXELS && node->max_width >= 0.0f)
    node->max_width_unit = CMP_UNIT_PIXELS;

  node->min_height =
      cmp_resolve_unit(node->min_height, node->min_height_unit, parent_height);
  if (node->min_height_unit != CMP_UNIT_PIXELS && node->min_height >= 0.0f)
    node->min_height_unit = CMP_UNIT_PIXELS;

  node->max_height =
      cmp_resolve_unit(node->max_height, node->max_height_unit, parent_height);
  if (node->max_height_unit != CMP_UNIT_PIXELS && node->max_height >= 0.0f)
    node->max_height_unit = CMP_UNIT_PIXELS;

  /* According to W3C, even height percentage is resolved against parent width
   * for margin/padding. */
  for (j = 0; j < 4; j++) {
    node->margin[j] =
        cmp_resolve_unit(node->margin[j], node->margin_unit[j], parent_width);
    if (node->margin_unit[j] != CMP_UNIT_PIXELS && node->margin[j] >= 0.0f)
      node->margin_unit[j] = CMP_UNIT_PIXELS;

    node->padding[j] =
        cmp_resolve_unit(node->padding[j], node->padding_unit[j], parent_width);
    if (node->padding_unit[j] != CMP_UNIT_PIXELS && node->padding[j] >= 0.0f)
      node->padding_unit[j] = CMP_UNIT_PIXELS;
  }

  /* Clamp / Min / Max Enforcement */
  if (node->width >= 0.0f) {
    if (node->min_width >= 0.0f && node->width < node->min_width)
      node->width = node->min_width;
    if (node->max_width >= 0.0f && node->width > node->max_width)
      node->width = node->max_width;
  }

  if (node->height >= 0.0f) {
    if (node->min_height >= 0.0f && node->height < node->min_height)
      node->height = node->min_height;
    if (node->max_height >= 0.0f && node->height > node->max_height)
      node->height = node->max_height;
  }

  /* Intrinsic keywords resolution for width */
  if (node->width == CMP_LAYOUT_MIN_CONTENT) {
    cmp_layout_get_min_content(node, &node->width);
  } else if (node->width == CMP_LAYOUT_MAX_CONTENT) {
    cmp_layout_get_max_content(node, &node->width);
  } else if (node->width == CMP_LAYOUT_FIT_CONTENT) {
    float min_w = 0.0f;
    float max_w = 0.0f;
    float fit_w = parent_width;
    cmp_layout_get_min_content(node, &min_w);
    cmp_layout_get_max_content(node, &max_w);
    if (fit_w < min_w)
      fit_w = min_w;
    if (fit_w > max_w)
      fit_w = max_w;
    node->width = fit_w;
  }

  /* Step 4: Stacking Contexts (Painter's Algorithm)
   * Evaluate `is_stacking_context` based on z-index applicability.
   * `z-index` only applies if `position != static` or `display == flex/grid`.
   */
  node->is_stacking_context = 0;
  if (node->z_index != 0) {
    if (node->position_type != CMP_POSITION_STATIC) {
      node->is_stacking_context = 1;
    } else if (node->parent != NULL &&
               (node->parent->display == CMP_DISPLAY_FLEX ||
                node->parent->display == CMP_DISPLAY_GRID)) {
      /* Flex and Grid items establish a stacking context if z-index is not auto
       * (0 for us) */
      node->is_stacking_context = 1;
    }
  }

  switch (node->display) {
  case CMP_DISPLAY_BLOCK:
    return cmp_layout_bfc(node, constraints);
  case CMP_DISPLAY_INLINE:
  case CMP_DISPLAY_INLINE_BLOCK:
    return cmp_layout_ifc(node, constraints);
  case CMP_DISPLAY_GRID:
    return cmp_layout_gfc(node, constraints);
  case CMP_DISPLAY_FLEX:
  case CMP_DISPLAY_INLINE_FLEX:
  default:
    return cmp_layout_ffc(node, constraints);
  }
}

int cmp_layout_calculate(cmp_layout_node_t *root, float available_width,
                         float available_height) {
  int rc = CMP_SUCCESS;
  int is_rtl = 0;
  cmp_layout_constraints_t root_constraints;

  if (root == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  /* Dirty flag optimization: only recalculate if size changed or explicitly
   * dirty */
  if (!root->dirty && root->computed_rect.width == available_width &&
      root->computed_rect.height == available_height) {
    return CMP_SUCCESS;
  }

  /* Reset dirty flag as we are about to calculate */
  root->dirty = 0;

  root_constraints.min_width = CMP_LAYOUT_AUTO;
  root_constraints.max_width = available_width;
  root_constraints.min_height = CMP_LAYOUT_AUTO;
  root_constraints.max_height = available_height;

  g_viewport_width = available_width >= 0.0f ? available_width : 0.0f;
  g_viewport_height = available_height >= 0.0f ? available_height : 0.0f;

  cmp_layout_responsive_pass(root, available_width);
  rc = cmp_layout_calculate_node(root, &root_constraints);
  if (rc != CMP_SUCCESS) {
    return rc;
  }

  (void)cmp_i18n_is_rtl(&is_rtl);
  if (is_rtl) {
    apply_rtl_mirroring(root);
  }

  return rc;
}
