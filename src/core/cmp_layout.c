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
  node->align_self = CMP_FLEX_ALIGN_START;
  node->position_type = CMP_POSITION_RELATIVE;

  node->order = 0;

  node->width = -1.0f;
  node->height = -1.0f;
  node->min_width = -1.0f;
  node->min_height = -1.0f;
  node->max_width = -1.0f;
  node->max_height = -1.0f;
  node->flex_grow = 0.0f;
  node->flex_shrink = 1.0f;
  node->flex_basis = -1.0f;

  node->measured_width = 0.0f;
  node->measured_height = 0.0f;
  node->dirty = 1;

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

typedef struct {
  size_t start;
  size_t count;
  float main_size;
  float cross_size;
  float total_flex_grow;
  float total_flex_shrink;
} cmp_layout_line_t;

static void cmp_layout_measure_pass(cmp_layout_node_t *node) {
  size_t i;
  float sum_child_main = 0.0f;
  float max_child_cross = 0.0f;
  float aspect_ratio = node->aspect_ratio;
  int is_row = (node->direction == CMP_FLEX_ROW);

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_measure_pass(node->children[i]);
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    float w = child->measured_width;
    float h = child->measured_height;

    if (child->position_type == CMP_POSITION_ABSOLUTE)
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
    node->measured_width = sum_child_main + node->padding[1] + node->padding[3];
    node->measured_height =
        max_child_cross + node->padding[0] + node->padding[2];
  } else {
    node->measured_height =
        sum_child_main + node->padding[0] + node->padding[2];
    node->measured_width =
        max_child_cross + node->padding[1] + node->padding[3];
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
}

static void cmp_layout_resolve_flex_pass(cmp_layout_node_t *node,
                                         cmp_layout_constraints_t constraints) {
  size_t i, j;
  int is_row = (node->direction == CMP_FLEX_ROW);
  float inner_width, inner_height;
  cmp_layout_line_t *lines = NULL;
  size_t line_count = 0;
  size_t line_capacity = 4;
  float main_avail, main_avail_for_shrink;
  float global_cross_max = 0.0f;
  float global_main_max = 0.0f;

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
    CMP_MALLOC(sizeof(cmp_layout_line_t) * line_capacity, (void **)&lines);
    memset(&lines[0], 0, sizeof(cmp_layout_line_t));
    line_count = 1;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    float child_main, child_cross, child_main_margin, child_cross_margin;
    cmp_layout_line_t *cur_line;
    float c_w, c_h;

    if (child->position_type == CMP_POSITION_ABSOLUTE)
      continue;

    c_w = child->measured_width;
    c_h = child->measured_height;

    child_main = is_row ? c_w : c_h;
    child_cross = is_row ? c_h : c_w;
    child_main_margin = is_row ? (child->margin[1] + child->margin[3])
                               : (child->margin[0] + child->margin[2]);
    child_cross_margin = is_row ? (child->margin[0] + child->margin[2])
                                : (child->margin[1] + child->margin[3]);

    cur_line = &lines[line_count - 1];

    if (node->flex_wrap == CMP_FLEX_WRAP && cur_line->count > 0 &&
        cur_line->main_size + child_main + child_main_margin > main_avail) {
      if (line_count >= line_capacity) {
        cmp_layout_line_t *new_lines;
        line_capacity *= 2;
        if (CMP_MALLOC(sizeof(cmp_layout_line_t) * line_capacity,
                       (void **)&new_lines) != CMP_SUCCESS) {
          if (lines)
            CMP_FREE(lines);
          return;
        }
        memcpy(new_lines, lines, sizeof(cmp_layout_line_t) * line_count);
        CMP_FREE(lines);
        lines = new_lines;
      }
      cur_line = &lines[line_count++];
      memset(cur_line, 0, sizeof(cmp_layout_line_t));
      cur_line->start = i;
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

  for (i = 0; i < line_count; i++) {
    cmp_layout_line_t *line = &lines[i];
    float remaining_main = main_avail_for_shrink - line->main_size;
    float current_main_pos = 0.0f;

    size_t processed = 0;
    for (j = line->start; j < node->child_count && processed < line->count;
         j++) {
      cmp_layout_node_t *child = node->children[j];
      float final_main, final_cross;
      cmp_layout_constraints_t child_constraints;
      float c_w, c_h;

      if (child->position_type == CMP_POSITION_ABSOLUTE)
        continue;
      processed++;

      c_w = child->measured_width;
      c_h = child->measured_height;

      final_main = is_row ? c_w : c_h;

      if (child->width < 0.0f && child->height < 0.0f && final_main <= 0.0f &&
          remaining_main > 0.0f && line->total_flex_grow == 0.0f &&
          node->flex_wrap == CMP_FLEX_NOWRAP) {
        final_main = remaining_main;
      } else if ((is_row ? child->width < 0.0f : child->height < 0.0f)) {
        if (line->total_flex_grow > 0.0f && remaining_main > 0.0f) {
          final_main =
              (child->flex_grow / line->total_flex_grow) * remaining_main;
        } else {
          /* Fallback size logic */
          final_main =
              (remaining_main > 0.0f && node->flex_wrap == CMP_FLEX_NOWRAP)
                  ? remaining_main
                  : (is_row ? child->measured_width : child->measured_height);

          /* If shrink must occur and we have shrink factors, shrink it. */
          if (line->total_flex_shrink > 0.0f && remaining_main < 0.0f) {
            final_main +=
                (child->flex_shrink / line->total_flex_shrink) * remaining_main;
          }
          if (final_main < 0.0f)
            final_main = 0.0f;
        }
      } else if (line->total_flex_grow > 0.0f && remaining_main > 0.0f &&
                 remaining_main != 999999.0f) {
        final_main +=
            (child->flex_grow / line->total_flex_grow) * remaining_main;
      } else if (line->total_flex_shrink > 0.0f && remaining_main < 0.0f) {
        final_main +=
            (child->flex_shrink / line->total_flex_shrink) * remaining_main;
        if (final_main < 0.0f)
          final_main = 0.0f;
      }

      final_cross = is_row ? c_h : c_w;

      /* Stretch */
      if (node->align_items == CMP_FLEX_ALIGN_STRETCH &&
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

      cmp_layout_resolve_flex_pass(child, child_constraints);

      /* Clamp child to its constraints immediately */
      if (child_constraints.max_width >= 0.0f &&
          child->computed_rect.width > child_constraints.max_width) {
        child->computed_rect.width = child_constraints.max_width;
      }
      if (child_constraints.max_height >= 0.0f &&
          child->computed_rect.height > child_constraints.max_height) {
        child->computed_rect.height = child_constraints.max_height;
      }

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
      for (j = line->start; j < node->child_count && processed_c < line->count;
           j++) {
        float c_size;
        cmp_layout_node_t *child = node->children[j];
        if (child->position_type == CMP_POSITION_ABSOLUTE)
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
    if (i < line_count - 1) {
      global_cross_max += is_row ? node->row_gap : node->column_gap;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    if (child->position_type == CMP_POSITION_ABSOLUTE) {
      cmp_layout_constraints_t abs_constraints = {-1.0f, -1.0f, -1.0f, -1.0f};
      cmp_layout_resolve_flex_pass(child, abs_constraints);
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

  if (lines != NULL) {
    CMP_FREE(lines);
  }
}

static void cmp_layout_position_pass(cmp_layout_node_t *node, float parent_x,
                                     float parent_y) {
  size_t i, j;
  float current_x = parent_x + node->margin[3];
  float current_y = parent_y + node->margin[0];
  int is_row = (node->direction == CMP_FLEX_ROW);
  float inner_width, inner_height;
  cmp_layout_line_t *lines = NULL;
  size_t line_count = 0;
  size_t line_capacity = 4;
  float main_avail, cross_pos;

  if (node->position_type == CMP_POSITION_ABSOLUTE) {
    current_x = parent_x + node->position[3];
    current_y = parent_y + node->position[0];
  }

  node->computed_rect.x = current_x;
  node->computed_rect.y = current_y;

  inner_width = node->computed_rect.width - node->padding[1] - node->padding[3];
  inner_height =
      node->computed_rect.height - node->padding[0] - node->padding[2];
  main_avail = is_row ? inner_width : inner_height;

  if (node->child_count > 0) {
    CMP_MALLOC(sizeof(cmp_layout_line_t) * line_capacity, (void **)&lines);
    memset(&lines[0], 0, sizeof(cmp_layout_line_t));
    line_count = 1;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    float child_main, child_cross, child_main_margin, child_cross_margin;
    cmp_layout_line_t *cur_line;

    if (child->position_type == CMP_POSITION_ABSOLUTE)
      continue;

    child_main =
        is_row ? child->computed_rect.width : child->computed_rect.height;
    child_cross =
        is_row ? child->computed_rect.height : child->computed_rect.width;
    child_main_margin = is_row ? (child->margin[1] + child->margin[3])
                               : (child->margin[0] + child->margin[2]);
    child_cross_margin = is_row ? (child->margin[0] + child->margin[2])
                                : (child->margin[1] + child->margin[3]);

    cur_line = &lines[line_count - 1];

    if (node->flex_wrap == CMP_FLEX_WRAP && cur_line->count > 0 &&
        cur_line->main_size + child_main + child_main_margin > main_avail) {
      if (line_count >= line_capacity) {
        cmp_layout_line_t *new_lines;
        line_capacity *= 2;
        if (CMP_MALLOC(sizeof(cmp_layout_line_t) * line_capacity,
                       (void **)&new_lines) != CMP_SUCCESS) {
          if (lines)
            CMP_FREE(lines);
          return;
        }
        memcpy(new_lines, lines, sizeof(cmp_layout_line_t) * line_count);
        CMP_FREE(lines);
        lines = new_lines;
      }
      cur_line = &lines[line_count++];
      memset(cur_line, 0, sizeof(cmp_layout_line_t));
      cur_line->start = i;
    }

    if (cur_line->count > 0)
      cur_line->main_size += (is_row ? node->column_gap : node->row_gap);
    cur_line->count++;
    cur_line->main_size += child_main + child_main_margin;
    if (child_cross + child_cross_margin > cur_line->cross_size) {
      cur_line->cross_size = child_cross + child_cross_margin;
    }
  }

  cross_pos =
      is_row ? (current_y + node->padding[0]) : (current_x + node->padding[3]);

  for (i = 0; i < line_count; i++) {
    cmp_layout_line_t *line = &lines[i];
    float remaining_main = main_avail - line->main_size;
    float main_offset = 0.0f;
    float spacing = 0.0f;
    float main_pos;
    size_t processed = 0;

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

    for (j = line->start; j < node->child_count && processed < line->count;
         j++) {
      cmp_layout_node_t *child = node->children[j];
      float final_main =
          is_row ? child->computed_rect.width : child->computed_rect.height;
      float final_cross =
          is_row ? child->computed_rect.height : child->computed_rect.width;
      float c_x, c_y;

      if (child->position_type == CMP_POSITION_ABSOLUTE)
        continue;
      processed++;

      c_x = 0;
      c_y = 0;
      if (is_row) {
        c_x = main_pos;
        c_y = cross_pos;
        if (node->align_items == CMP_FLEX_ALIGN_CENTER) {
          c_y += (line->cross_size - final_cross) * 0.5f;
        } else if (node->align_items == CMP_FLEX_ALIGN_END) {
          c_y += (line->cross_size - final_cross);
        }
      } else {
        c_y = main_pos;
        c_x = cross_pos;
        if (node->align_items == CMP_FLEX_ALIGN_CENTER) {
          c_x += (line->cross_size - final_cross) * 0.5f;
        } else if (node->align_items == CMP_FLEX_ALIGN_END) {
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

    cross_pos += line->cross_size;
    if (i < line_count - 1) {
      cross_pos += is_row ? node->row_gap : node->column_gap;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    if (child->position_type == CMP_POSITION_ABSOLUTE) {
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

  if (lines != NULL) {
    CMP_FREE(lines);
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
  } else if (available_width >= 300.0f && available_width < 600.0f) {
  } else if (available_width >= 600.0f && available_width < 900.0f) {
    if (node->direction == CMP_FLEX_ROW) {
      node->flex_wrap = CMP_FLEX_WRAP;
    }
  } else if (available_width >= 900.0f && available_width < 1200.0f) {
  } else if (available_width >= 1200.0f && available_width < 1920.0f) {
  } else if (available_width >= 1920.0f) {
    if (node->parent == NULL) {
      node->margin[0] = available_width * 0.05f;
      node->margin[1] = available_width * 0.05f;
      node->margin[2] = available_width * 0.05f;
      node->margin[3] = available_width * 0.05f;
      node->font_size = 24.0f;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_responsive_pass(node->children[i], available_width);
  }
}

static void apply_rtl_mirroring(cmp_layout_node_t *node);
static void cmp_layout_responsive_pass(cmp_layout_node_t *node,
                                       float available_width);

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

  root_constraints.min_width = -1.0f;
  root_constraints.max_width = available_width;
  root_constraints.min_height = -1.0f;
  root_constraints.max_height = available_height;

  cmp_layout_responsive_pass(root, available_width);
  cmp_layout_measure_pass(root);
  cmp_layout_resolve_flex_pass(root, root_constraints);
  cmp_layout_position_pass(root, 0.0f, 0.0f);

  (void)cmp_i18n_is_rtl(&is_rtl);
  if (is_rtl) {
    apply_rtl_mirroring(root);
  }

  return rc;
}
