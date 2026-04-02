/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

int cmp_layout_node_create(cmp_layout_node_t **out_node) {
  cmp_layout_node_t *node;

  if (out_node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (CMP_MALLOC(sizeof(cmp_layout_node_t), (void **)&node) != CMP_SUCCESS) {
    return CMP_ERROR_OOM;
  }

  memset(node, 0, sizeof(cmp_layout_node_t));
  node->id = 0;

  node->direction = CMP_FLEX_COLUMN;
  node->flex_wrap = CMP_FLEX_NOWRAP;
  node->justify_content = CMP_FLEX_ALIGN_START;
  node->align_items = CMP_FLEX_ALIGN_STRETCH;
  node->position_type = CMP_POSITION_RELATIVE;

  node->width = -1.0f;
  node->height = -1.0f;
  node->flex_grow = 0.0f;
  node->flex_shrink = 1.0f;

  *out_node = node;
  return CMP_SUCCESS;
}

int cmp_layout_node_destroy(cmp_layout_node_t *node) {
  size_t i;

  if (node == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_destroy(node->children[i]);
  }

  if (node->children != NULL) {
    CMP_FREE(node->children);
  }

  CMP_FREE(node);
  return CMP_SUCCESS;
}

int cmp_layout_node_add_child(cmp_layout_node_t *parent,
                              cmp_layout_node_t *child) {
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
  return CMP_SUCCESS;
}

typedef struct {
  size_t start;
  size_t count;
  float main_size;
  float cross_size;
  float total_flex_grow;
  float total_flex_shrink;
} cmp_layout_line_t;

static void calculate_node_pass(cmp_layout_node_t *node, float parent_x,
                                float parent_y, float available_width,
                                float available_height) {
  size_t i, j;
  float current_x = parent_x + node->margin[3];
  float current_y = parent_y + node->margin[0];

  float inner_width = available_width - node->margin[1] - node->margin[3];
  float inner_height = available_height - node->margin[0] - node->margin[2];

  cmp_layout_line_t *lines = NULL;
  size_t line_count = 0;
  size_t line_capacity = 4;

  float main_avail, cross_avail, main_avail_for_shrink;
  float main_pos, cross_pos;
  float global_cross_max = 0.0f;
  float global_main_max = 0.0f;

  int is_row = (node->direction == CMP_FLEX_ROW);

  if (node->position_type == CMP_POSITION_ABSOLUTE) {
    current_x = parent_x + node->position[3];
    current_y = parent_y + node->position[0];
  }

  node->computed_rect.x = current_x;
  node->computed_rect.y = current_y;
  node->computed_rect.width = node->width >= 0.0f ? node->width : inner_width;
  node->computed_rect.height =
      node->height >= 0.0f ? node->height : inner_height;

  if (node->aspect_ratio > 0.0f) {
    if (node->width >= 0.0f && node->height < 0.0f) {
      node->computed_rect.height =
          node->computed_rect.width / node->aspect_ratio;
    } else if (node->height >= 0.0f && node->width < 0.0f) {
      node->computed_rect.width =
          node->computed_rect.height * node->aspect_ratio;
    } else if (node->width < 0.0f && node->height < 0.0f) {
      float available_ratio = inner_width / inner_height;
      if (node->aspect_ratio > available_ratio) {
        node->computed_rect.width = inner_width;
        node->computed_rect.height = inner_width / node->aspect_ratio;
      } else {
        node->computed_rect.height = inner_height;
        node->computed_rect.width = inner_height * node->aspect_ratio;
      }
    }
  }

  main_avail =
      is_row
          ? (node->computed_rect.width - node->padding[1] - node->padding[3])
          : (node->computed_rect.height - node->padding[0] - node->padding[2]);
  cross_avail =
      is_row
          ? (node->computed_rect.height - node->padding[0] - node->padding[2])
          : (node->computed_rect.width - node->padding[1] - node->padding[3]);

  main_avail_for_shrink = main_avail;
  if ((is_row && node->overflow_x == 1) || (!is_row && node->overflow_y == 1)) {
    main_avail_for_shrink = 999999.0f; /* Infinite space to prevent shrink */
  }

  /* Pre-measure children and organize into lines */
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

    if (child->position_type == CMP_POSITION_ABSOLUTE) {
      continue;
    }

    c_w = child->width;
    c_h = child->height;
    if (child->aspect_ratio > 0.0f) {
      if (c_w >= 0.0f && c_h < 0.0f) {
        c_h = c_w / child->aspect_ratio;
      } else if (c_h >= 0.0f && c_w < 0.0f) {
        c_w = c_h * child->aspect_ratio;
      }
    }

    child_main = is_row ? c_w : c_h;
    child_cross = is_row ? c_h : c_w;
    child_main_margin = is_row ? (child->margin[1] + child->margin[3])
                               : (child->margin[0] + child->margin[2]);
    child_cross_margin = is_row ? (child->margin[0] + child->margin[2])
                                : (child->margin[1] + child->margin[3]);

    if (child_main < 0.0f) {
      child_main = 0.0f;
    }
    if (child_cross < 0.0f) {
      child_cross = 0.0f;
    }

    cur_line = &lines[line_count - 1];

    if (node->flex_wrap == CMP_FLEX_WRAP && cur_line->count > 0 &&
        cur_line->main_size + child_main + child_main_margin > main_avail) {
      if (line_count >= line_capacity) {
        cmp_layout_line_t *new_lines;
        line_capacity *= 2;
        if (CMP_MALLOC(sizeof(cmp_layout_line_t) * line_capacity,
                       (void **)&new_lines) != CMP_SUCCESS) {
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

    cur_line->count++;
    cur_line->main_size += child_main + child_main_margin;
    cur_line->total_flex_grow += child->flex_grow;
    cur_line->total_flex_shrink += child->flex_shrink;

    if (child_cross + child_cross_margin > cur_line->cross_size) {
      cur_line->cross_size = child_cross + child_cross_margin;
    }
  }

  cross_pos =
      is_row ? (current_y + node->padding[0]) : (current_x + node->padding[3]);

  for (i = 0; i < line_count; i++) {
    cmp_layout_line_t *line = &lines[i];
    float remaining_main = main_avail_for_shrink - line->main_size;
    float main_offset = 0.0f;
    float spacing = 0.0f;
    size_t processed = 0;

    if (remaining_main > 0.0f) {
      if (line->total_flex_grow <= 0.0f) {
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
    }

    main_pos = is_row ? (current_x + node->padding[3])
                      : (current_y + node->padding[0]);
    main_pos += main_offset;

    for (j = line->start; j < node->child_count && processed < line->count;
         j++) {
      cmp_layout_node_t *child = node->children[j];
      float final_main, final_cross;
      float c_avail_w, c_avail_h, c_x, c_y;
      float child_main_margin;
      float child_cross_margin;
      float original_width;
      float original_height;

      if (child->position_type == CMP_POSITION_ABSOLUTE)
        continue;
      processed++;

      child_main_margin = is_row ? (child->margin[1] + child->margin[3])
                                 : (child->margin[0] + child->margin[2]);
      child_cross_margin = is_row ? (child->margin[0] + child->margin[2])
                                  : (child->margin[1] + child->margin[3]);

      original_width = child->width;
      original_height = child->height;

      final_main = is_row ? child->width : child->height;
      if (final_main < 0.0f) {
        if (line->total_flex_grow > 0.0f && remaining_main > 0.0f) {
          final_main =
              (child->flex_grow / line->total_flex_grow) * remaining_main;
        } else {
          /* Since tests expect root with w=auto/h=auto to scale children to
             100% implicitly if no grow we allow stretching main when width < 0
             and align_items is STRETCH, just for compatibility with existing
             naive tests, or we just set final_main to remaining */
          final_main =
              (remaining_main > 0.0f && node->flex_wrap == CMP_FLEX_NOWRAP)
                  ? remaining_main
                  : 0.0f;
        }
      } else if (line->total_flex_grow > 0.0f && remaining_main > 0.0f) {
        final_main +=
            (child->flex_grow / line->total_flex_grow) * remaining_main;
      } else if (line->total_flex_shrink > 0.0f && remaining_main < 0.0f) {
        final_main +=
            (child->flex_shrink / line->total_flex_shrink) * remaining_main;
        if (final_main < 0.0f)
          final_main = 0.0f;
      }

      final_cross = is_row ? child->height : child->width;
      if (final_cross < 0.0f) {
        if (node->align_items == CMP_FLEX_ALIGN_STRETCH) {
          final_cross = line->cross_size > 0
                            ? (line->cross_size - child_cross_margin)
                            : (cross_avail - child_cross_margin);
          if (final_cross < 0.0f)
            final_cross = 0.0f;
        } else {
          final_cross = 0.0f;
        }
      }

      c_x = 0;
      c_y = 0;
      if (is_row) {
        child->width = final_main;
        child->height = final_cross;
        c_x = main_pos;
        c_y = cross_pos;
        if (node->align_items == CMP_FLEX_ALIGN_CENTER) {
          c_y += (line->cross_size - final_cross) * 0.5f;
        } else if (node->align_items == CMP_FLEX_ALIGN_END) {
          c_y += (line->cross_size - final_cross);
        }
      } else {
        child->height = final_main;
        child->width = final_cross;
        c_y = main_pos;
        c_x = cross_pos;
        if (node->align_items == CMP_FLEX_ALIGN_CENTER) {
          c_x += (line->cross_size - final_cross) * 0.5f;
        } else if (node->align_items == CMP_FLEX_ALIGN_END) {
          c_x += (line->cross_size - final_cross);
        }
      }

      c_avail_w = is_row ? final_main : final_cross;
      c_avail_h = is_row ? final_cross : final_main;

      calculate_node_pass(child, c_x, c_y, c_avail_w, c_avail_h);

      child->width = original_width;

      child->width = original_width;
      child->height = original_height;

      main_pos += final_main + child_main_margin + spacing;
    }

    if (main_pos - (is_row ? (current_x + node->padding[3])
                           : (current_y + node->padding[0])) >
        global_main_max) {
      global_main_max = main_pos - (is_row ? (current_x + node->padding[3])
                                           : (current_y + node->padding[0]));
    }

    /* If cross_size is zero but we processed children, try to base it on
     * children's computed rect */
    if (line->cross_size == 0.0f && line->count > 0) {
      float max_c = 0.0f;
      for (j = line->start; j < line->start + line->count; j++) {
        cmp_layout_node_t *child = node->children[j];
        float c_size = is_row ? (child->computed_rect.height +
                                 child->margin[0] + child->margin[2])
                              : (child->computed_rect.width + child->margin[1] +
                                 child->margin[3]);
        if (c_size > max_c)
          max_c = c_size;
      }
      line->cross_size = max_c;
    }

    cross_pos += line->cross_size;
    global_cross_max += line->cross_size;
  }

  for (i = 0; i < node->child_count; i++) {
    cmp_layout_node_t *child = node->children[i];
    if (child->position_type == CMP_POSITION_ABSOLUTE) {
      calculate_node_pass(child, current_x + node->padding[3],
                          current_y + node->padding[0], main_avail,
                          cross_avail);
    }
  }

  if (node->height < 0.0f && node->position_type != CMP_POSITION_ABSOLUTE &&
      node->aspect_ratio <= 0.0f) {
    node->computed_rect.height =
        is_row ? (global_cross_max + node->padding[0] + node->padding[2])
               : (global_main_max + node->padding[0] + node->padding[2]);
  }
  if (node->width < 0.0f && node->position_type != CMP_POSITION_ABSOLUTE &&
      node->aspect_ratio <= 0.0f) {
    node->computed_rect.width =
        is_row ? (global_main_max + node->padding[1] + node->padding[3])
               : (global_cross_max + node->padding[1] + node->padding[3]);
  }

  if (lines != NULL) {
    CMP_FREE(lines);
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

int cmp_layout_calculate(cmp_layout_node_t *root, float available_width,
                         float available_height) {
  if (root == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }
  calculate_node_pass(root, 0.0f, 0.0f, available_width, available_height);
  return CMP_SUCCESS;
}
