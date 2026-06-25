#ifndef CMP_GEOMETRY_TESTER_H
#define CMP_GEOMETRY_TESTER_H

#include "cmp.h"
#include <math.h>
#include <stdio.h>

static void _cmp_generate_json_dump(cmp_layout_node_t *node, int depth) {
  size_t i;
  if (!node)
    return;
  for (i = 0; i < (size_t)(depth * 2); i++)
    printf(" ");
  printf("{\n");
  for (i = 0; i < (size_t)(depth * 2 + 2); i++)
    printf(" ");
  printf("\"id\": %d,\n", node->id);
  for (i = 0; i < (size_t)(depth * 2 + 2); i++)
    printf(" ");
  printf("\"rect\": { \"x\": %.2f, \"y\": %.2f, \"w\": %.2f, \"h\": %.2f },\n",
         node->computed_rect.x, node->computed_rect.y,
         node->computed_rect.width, node->computed_rect.height);
  for (i = 0; i < (size_t)(depth * 2 + 2); i++)
    printf(" ");
  printf("\"children\": [\n");
  for (i = 0; i < node->child_count; i++) {
    _cmp_generate_json_dump(node->children[i], depth + 2);
    if (i < node->child_count - 1)
      printf(",\n");
    else
      printf("\n");
  }
  for (i = 0; i < (size_t)(depth * 2 + 2); i++)
    printf(" ");
  printf("]\n");
  for (i = 0; i < (size_t)(depth * 2); i++)
    printf(" ");
  printf("}");
}

static void _cmp_print_ascii_art(cmp_layout_node_t *node) {
  if (!node)
    return;
  printf("\n================ ASCII ART BOUNDS ================\n");
  printf("+-------------------------------------------------+\n");
  printf("| ID: %-43d |\n", node->id);
  printf("| X: %-5.1f Y: %-5.1f W: %-5.1f H: %-5.1f          |\n",
         node->computed_rect.x, node->computed_rect.y,
         node->computed_rect.width, node->computed_rect.height);
  printf("+-------------------------------------------------+\n");
  printf("==================================================\n\n");
}

#define CMP_TEST_FAILURE_DUMP(dump_node)                                       \
  do {                                                                         \
    if (dump_node) {                                                           \
      printf("\n!!! ASSERTION FAILURE !!!\n");                                 \
      printf("Generating JSON Dump:\n");                                       \
      _cmp_generate_json_dump((dump_node), 0);                                 \
      printf("\n\n");                                                          \
      _cmp_print_ascii_art((dump_node));                                       \
      fflush(stdout);                                                          \
    }                                                                          \
  } while (0)
#ifndef CMP_TEST_ABORT
#define CMP_TEST_ABORT(dump_node)                                              \
  do {                                                                         \
    fflush(stdout);                                                            \
    CMP_TEST_FAILURE_DUMP(dump_node);                                          \
    return 1;                                                                  \
  } while (0)
#endif

#define ASSERT_GEOMETRY_EQ(node, ex, ey, ew, eh)                               \
  do {                                                                         \
    cmp_rect_f_t rect;                                                         \
    if (cmp_layout_node_get_border_box((node), &rect) != CMP_SUCCESS) {        \
      printf("ASSERT_GEOMETRY_EQ failed: Could not get border box\n");         \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    if ((int)rect.x != (ex) || (int)rect.y != (ey) ||                          \
        (int)rect.width != (ew) || (int)rect.height != (eh)) {                 \
      printf("ASSERT_GEOMETRY_EQ failed: Expected (%d,%d %dx%d), got "         \
             "(%.1f,%.1f %.1fx%.1f)\n",                                        \
             (ex), (ey), (ew), (eh), rect.x, rect.y, rect.width, rect.height); \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

#define ASSERT_GEOMETRY_APPROX(node, ex, ey, ew, eh, tol)                      \
  do {                                                                         \
    cmp_rect_f_t rect;                                                         \
    if (cmp_layout_node_get_border_box((node), &rect) != CMP_SUCCESS) {        \
      printf("ASSERT_GEOMETRY_APPROX failed: Could not get border box\n");     \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    if (fabs(rect.x - (ex)) > (tol) || fabs(rect.y - (ey)) > (tol) ||          \
        fabs(rect.width - (ew)) > (tol) || fabs(rect.height - (eh)) > (tol)) { \
      printf("ASSERT_GEOMETRY_APPROX failed: Expected (~%.1f,~%.1f "           \
             "~%.1fx~%.1f), got (%.1f,%.1f %.1fx%.1f)\n",                      \
             (float)(ex), (float)(ey), (float)(ew), (float)(eh), rect.x,       \
             rect.y, rect.width, rect.height);                                 \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

static int _cmp_check_no_overlaps(cmp_layout_node_t *container, int strict) {
  size_t i, j;
  if (!container)
    return CMP_SUCCESS;
  for (i = 0; i < container->child_count; i++) {
    cmp_layout_node_t *a = container->children[i];
    if (!strict && (a->position_type == 2 || a->position_type == 3))
      continue;
    for (j = i + 1; j < container->child_count; j++) {
      cmp_layout_node_t *b = container->children[j];
      if (!strict && (b->position_type == 2 || b->position_type == 3))
        continue;
      if (a->z_index != b->z_index)
        continue;
      if (a->computed_rect.x < b->computed_rect.x + b->computed_rect.width &&
          a->computed_rect.x + a->computed_rect.width > b->computed_rect.x &&
          a->computed_rect.y < b->computed_rect.y + b->computed_rect.height &&
          a->computed_rect.y + a->computed_rect.height > b->computed_rect.y) {
        return CMP_ERROR_GENERAL; /* Overlap detected */
      }
    }
    if (_cmp_check_no_overlaps(a, strict) != CMP_SUCCESS)
      return CMP_ERROR_GENERAL;
  }
  return CMP_SUCCESS;
}

#define ASSERT_NO_OVERLAPS(container)                                          \
  do {                                                                         \
    if (_cmp_check_no_overlaps((container), 0) != CMP_SUCCESS) {               \
      printf("ASSERT_NO_OVERLAPS failed: Siblings overlap\n");                 \
      CMP_TEST_ABORT(container);                                               \
    }                                                                          \
  } while (0)

#define ASSERT_NO_OVERLAPS_STRICT(container)                                   \
  do {                                                                         \
    if (_cmp_check_no_overlaps((container), 1) != CMP_SUCCESS) {               \
      printf("ASSERT_NO_OVERLAPS_STRICT failed: Siblings overlap\n");          \
      CMP_TEST_ABORT(container);                                               \
    }                                                                          \
  } while (0)

#define ASSERT_CONTAINED_BY(child, parent)                                     \
  do {                                                                         \
    cmp_rect_f_t c_box, p_box;                                                 \
    if (cmp_layout_node_get_border_box((child), &c_box) != CMP_SUCCESS ||      \
        cmp_layout_node_get_content_box((parent), &p_box) != CMP_SUCCESS) {    \
      printf("ASSERT_CONTAINED_BY failed: Could not get boxes\n");             \
      CMP_TEST_ABORT(parent);                                                  \
    }                                                                          \
    if (c_box.x < p_box.x || c_box.y < p_box.y ||                              \
        c_box.x + c_box.width > p_box.x + p_box.width ||                       \
        c_box.y + c_box.height > p_box.y + p_box.height) {                     \
      printf("ASSERT_CONTAINED_BY failed: Child exceeds parent content "       \
             "bounds\n");                                                      \
      CMP_TEST_ABORT(parent);                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_ALIGNED_Y(node_a, node_b, align)                                \
  do {                                                                         \
    cmp_rect_f_t a_box, b_box;                                                 \
    if (cmp_layout_node_get_border_box((node_a), &a_box) != CMP_SUCCESS ||     \
        cmp_layout_node_get_border_box((node_b), &b_box) != CMP_SUCCESS) {     \
      printf("ASSERT_ALIGNED_Y failed: Could not get boxes\n");                \
      CMP_TEST_ABORT(node_a);                                                  \
    }                                                                          \
    if ((align) == 0 && fabs(a_box.y - b_box.y) > 0.1f) {                      \
      printf("ASSERT_ALIGNED_Y failed: Top mismatch\n");                       \
      CMP_TEST_ABORT(node_a);                                                  \
    } else if ((align) == 1 && fabs((a_box.y + a_box.height / 2.0f) -          \
                                    (b_box.y + b_box.height / 2.0f)) > 0.1f) { \
      printf("ASSERT_ALIGNED_Y failed: Center mismatch\n");                    \
      CMP_TEST_ABORT(node_a);                                                  \
    } else if ((align) == 2 && fabs((a_box.y + a_box.height) -                 \
                                    (b_box.y + b_box.height)) > 0.1f) {        \
      printf("ASSERT_ALIGNED_Y failed: Bottom mismatch\n");                    \
      CMP_TEST_ABORT(node_a);                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_ALIGNED_X(node_a, node_b, align)                                \
  do {                                                                         \
    cmp_rect_f_t a_box, b_box;                                                 \
    if (cmp_layout_node_get_border_box((node_a), &a_box) != CMP_SUCCESS ||     \
        cmp_layout_node_get_border_box((node_b), &b_box) != CMP_SUCCESS) {     \
      printf("ASSERT_ALIGNED_X failed: Could not get boxes\n");                \
      CMP_TEST_ABORT(node_a);                                                  \
    }                                                                          \
    if ((align) == 0 && fabs(a_box.x - b_box.x) > 0.1f) {                      \
      printf("ASSERT_ALIGNED_X failed: Left mismatch\n");                      \
      CMP_TEST_ABORT(node_a);                                                  \
    } else if ((align) == 1 && fabs((a_box.x + a_box.width / 2.0f) -           \
                                    (b_box.x + b_box.width / 2.0f)) > 0.1f) {  \
      printf("ASSERT_ALIGNED_X failed: Center mismatch\n");                    \
      CMP_TEST_ABORT(node_a);                                                  \
    } else if ((align) == 2 && fabs((a_box.x + a_box.width) -                  \
                                    (b_box.x + b_box.width)) > 0.1f) {         \
      printf("ASSERT_ALIGNED_X failed: Right mismatch\n");                     \
      CMP_TEST_ABORT(node_a);                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_ALIGNED_BASELINE(node_a, node_b)                                \
  do {                                                                         \
    float a_base, b_base;                                                      \
    cmp_rect_f_t a_box, b_box;                                                 \
    if (cmp_layout_node_get_baseline_y((node_a), &a_base) != CMP_SUCCESS ||    \
        cmp_layout_node_get_baseline_y((node_b), &b_base) != CMP_SUCCESS ||    \
        cmp_layout_node_get_border_box((node_a), &a_box) != CMP_SUCCESS ||     \
        cmp_layout_node_get_border_box((node_b), &b_box) != CMP_SUCCESS) {     \
      printf("ASSERT_ALIGNED_BASELINE failed: Could not get baselines or "     \
             "boxes\n");                                                       \
      CMP_TEST_ABORT(node_a);                                                  \
    }                                                                          \
    if (fabs((a_box.y + a_base) - (b_box.y + b_base)) > 0.1f) {                \
      printf(                                                                  \
          "ASSERT_ALIGNED_BASELINE failed: Baselines differ %.1f vs %.1f\n",   \
          a_box.y + a_base, b_box.y + b_base);                                 \
      CMP_TEST_ABORT(node_a);                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_TOUCH_TARGET_MIN(node, min_w, min_h)                            \
  do {                                                                         \
    cmp_rect_f_t rect;                                                         \
    if (cmp_layout_node_get_margin_box((node), &rect) != CMP_SUCCESS) {        \
      printf("ASSERT_TOUCH_TARGET_MIN failed: Could not get margin box\n");    \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    if (rect.width < (min_w) || rect.height < (min_h)) {                       \
      printf("ASSERT_TOUCH_TARGET_MIN failed: Touch target %.1fx%.1f < min "   \
             "%.1fx%.1f\n",                                                    \
             rect.width, rect.height, (float)(min_w), (float)(min_h));         \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

#define ASSERT_Z_ORDER_ABOVE(node_above, node_below)                           \
  do {                                                                         \
    int z_a, z_b;                                                              \
    if (cmp_layout_node_get_z_index((node_above), &z_a) != CMP_SUCCESS ||      \
        cmp_layout_node_get_z_index((node_below), &z_b) != CMP_SUCCESS) {      \
      printf("ASSERT_Z_ORDER_ABOVE failed: Could not get Z indices\n");        \
      CMP_TEST_ABORT(node_above);                                              \
    }                                                                          \
    if (z_a <= z_b) {                                                          \
      printf("ASSERT_Z_ORDER_ABOVE failed: Node A (%d) is not strictly above " \
             "Node B (%d)\n",                                                  \
             z_a, z_b);                                                        \
      CMP_TEST_ABORT(node_above);                                              \
    }                                                                          \
  } while (0)

#define ASSERT_SCROLLABLE(node, axis)                                          \
  do {                                                                         \
    cmp_rect_f_t rect, scroll;                                                 \
    if (cmp_layout_node_get_content_box((node), &rect) != CMP_SUCCESS ||       \
        cmp_layout_node_get_scroll_bounds((node), &scroll) != CMP_SUCCESS) {   \
      printf("ASSERT_SCROLLABLE failed: Could not get bounds\n");              \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    if ((axis) == 0 && scroll.height <= rect.height) {                         \
      printf("ASSERT_SCROLLABLE failed: Node is not scrollable vertically\n"); \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    if ((axis) == 1 && scroll.width <= rect.width) {                           \
      printf(                                                                  \
          "ASSERT_SCROLLABLE failed: Node is not scrollable horizontally\n");  \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

#define ASSERT_ASPECT_RATIO(node, ratio, tol)                                  \
  do {                                                                         \
    cmp_rect_f_t rect;                                                         \
    if (cmp_layout_node_get_border_box((node), &rect) != CMP_SUCCESS) {        \
      printf("ASSERT_ASPECT_RATIO failed: Could not get bounds\n");            \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    if (rect.height <= 0.0f) {                                                 \
      printf("ASSERT_ASPECT_RATIO failed: Height is zero\n");                  \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
    float cur_ratio = rect.width / rect.height;                                \
    if (fabs(cur_ratio - (ratio)) > (tol)) {                                   \
      printf("ASSERT_ASPECT_RATIO failed: %.2f != %.2f\n", cur_ratio,          \
             (float)(ratio));                                                  \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

extern int g_mock_layout_pass_count;

#define ASSERT_LAYOUT_PASSES_LESS_THAN(count)                                  \
  do {                                                                         \
    if (g_mock_layout_pass_count >= (count)) {                                 \
      printf("ASSERT_LAYOUT_PASSES_LESS_THAN failed: Pass count %d >= limit "  \
             "%d\n",                                                           \
             g_mock_layout_pass_count, (count));                               \
      CMP_TEST_ABORT(NULL);                                                    \
    }                                                                          \
  } while (0)

static int _cmp_check_no_negative_coords(cmp_layout_node_t *node) {
  size_t i;
  if (!node)
    return CMP_SUCCESS;
  if (node->computed_rect.x < 0.0f || node->computed_rect.y < 0.0f) {
    return CMP_ERROR_GENERAL;
  }
  for (i = 0; i < node->child_count; i++) {
    if (_cmp_check_no_negative_coords(node->children[i]) != CMP_SUCCESS) {
      return CMP_ERROR_GENERAL;
    }
  }
  return CMP_SUCCESS;
}

#define ASSERT_NO_NEGATIVE_COORDINATES(node)                                   \
  do {                                                                         \
    if (_cmp_check_no_negative_coords((node)) != CMP_SUCCESS) {                \
      printf("ASSERT_NO_NEGATIVE_COORDINATES failed: Node or descendant has "  \
             "negative X/Y\n");                                                \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

static int _cmp_check_strict_containment(cmp_layout_node_t *node) {
  size_t i;
  cmp_rect_f_t p_box, c_box;
  if (!node)
    return CMP_SUCCESS;

  if (cmp_layout_node_get_content_box(node, &p_box) == CMP_SUCCESS) {
    for (i = 0; i < node->child_count; i++) {
      cmp_layout_node_t *child = node->children[i];
      /* Skip explicitly positioned elements if they form a new stacking context
       * out of flow */
      if (child->position_type == 2 || child->position_type == 3) {
        continue;
      }

      if (cmp_layout_node_get_margin_box(child, &c_box) == CMP_SUCCESS) {
        if (c_box.x < p_box.x || c_box.y < p_box.y ||
            c_box.x + c_box.width > p_box.x + p_box.width ||
            c_box.y + c_box.height > p_box.y + p_box.height) {
          /* Allow overflow if explicitly requested */
          if (node->overflow_x == 0 &&
              node->overflow_y == 0) { /* 0 usually implies visible */
            /* We might want to warn or we can just fail if it's strict */
            return CMP_ERROR_GENERAL;
          }
        }
      }
      if (_cmp_check_strict_containment(child) != CMP_SUCCESS) {
        return CMP_ERROR_GENERAL;
      }
    }
  }
  return CMP_SUCCESS;
}

#define ASSERT_STRICT_CONTAINMENT(node)                                        \
  do {                                                                         \
    if (_cmp_check_strict_containment((node)) != CMP_SUCCESS) {                \
      printf(                                                                  \
          "ASSERT_STRICT_CONTAINMENT failed: A child overflows its parent\n"); \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

static int _cmp_check_within_viewport(cmp_layout_node_t *node, float vw,
                                      float vh) {
  size_t i;
  cmp_rect_f_t c_box;
  if (!node)
    return CMP_SUCCESS;

  if (cmp_layout_node_get_margin_box(node, &c_box) == CMP_SUCCESS) {
    if (c_box.x < 0 || c_box.y < 0 || c_box.x + c_box.width > vw ||
        c_box.y + c_box.height > vh) {
      return CMP_ERROR_GENERAL;
    }
  }

  for (i = 0; i < node->child_count; i++) {
    if (_cmp_check_within_viewport(node->children[i], vw, vh) != CMP_SUCCESS) {
      return CMP_ERROR_GENERAL;
    }
  }
  return CMP_SUCCESS;
}

#define ASSERT_WITHIN_VIEWPORT(node, vw, vh)                                   \
  do {                                                                         \
    if (_cmp_check_within_viewport((node), (vw), (vh)) != CMP_SUCCESS) {       \
      printf("ASSERT_WITHIN_VIEWPORT failed: Node or descendant exceeds "      \
             "viewport bounds\n");                                             \
      CMP_TEST_ABORT(node);                                                    \
    }                                                                          \
  } while (0)

#define ASSERT_TEXT_OVERFLOW_HANDLED(text_node, parent_node)                   \
  do {                                                                         \
    cmp_rect_f_t t_box, p_box;                                                 \
    if (cmp_layout_node_get_margin_box((text_node), &t_box) != CMP_SUCCESS ||  \
        cmp_layout_node_get_content_box((parent_node), &p_box) !=              \
            CMP_SUCCESS) {                                                     \
      printf("ASSERT_TEXT_OVERFLOW_HANDLED failed: Could not get bounds\n");   \
      CMP_TEST_ABORT(text_node);                                               \
    }                                                                          \
    if (t_box.width > p_box.width) {                                           \
      if ((parent_node)->overflow_x == 0) {                                    \
        printf("ASSERT_TEXT_OVERFLOW_HANDLED failed: Text overflows "          \
               "horizontally and parent does not handle it\n");                \
        CMP_TEST_ABORT(text_node);                                             \
      }                                                                        \
    }                                                                          \
  } while (0)

#endif
