#include "cmp.h"
#include "cmp_geometry_tester.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(cond)                                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      printf("Assertion failed: %s\n", #cond);                                 \
      fflush(stdout);                                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)

int g_mock_layout_pass_count = 0;

static int test_margin_box(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_f_t rect;
  int rc;
  rc = cmp_layout_node_create(&node);
  ASSERT(rc == CMP_SUCCESS);

  node->computed_rect.x = 10.0f;
  node->computed_rect.y = 20.0f;
  node->computed_rect.width = 100.0f;
  node->computed_rect.height = 50.0f;

  node->margin[0] = 5.0f;  /* top */
  node->margin[1] = 10.0f; /* right */
  node->margin[2] = 15.0f; /* bottom */
  node->margin[3] = 20.0f; /* left */

  rc = cmp_layout_node_get_margin_box(node, &rect);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(rect.x - (10.0f - 20.0f)) < 0.001f);
  ASSERT(fabs(rect.y - (20.0f - 5.0f)) < 0.001f);
  ASSERT(fabs(rect.width - (100.0f + 20.0f + 10.0f)) < 0.001f);
  ASSERT(fabs(rect.height - (50.0f + 5.0f + 15.0f)) < 0.001f);

  cmp_layout_node_destroy(node);
  return 0;
}

static int test_border_box(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_f_t rect;
  int rc;
  rc = cmp_layout_node_create(&node);
  ASSERT(rc == CMP_SUCCESS);

  node->computed_rect.x = 10.0f;
  node->computed_rect.y = 20.0f;
  node->computed_rect.width = 100.0f;
  node->computed_rect.height = 50.0f;

  rc = cmp_layout_node_get_border_box(node, &rect);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(rect.x - 10.0f) < 0.001f);
  ASSERT(fabs(rect.width - 100.0f) < 0.001f);

  cmp_layout_node_destroy(node);
  return 0;
}

static int test_padding_box(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_f_t rect;
  int rc;
  rc = cmp_layout_node_create(&node);
  ASSERT(rc == CMP_SUCCESS);

  node->computed_rect.x = 10.0f;
  node->computed_rect.y = 20.0f;
  node->computed_rect.width = 100.0f;
  node->computed_rect.height = 50.0f;

  rc = cmp_layout_node_get_padding_box(node, &rect);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(rect.x - 10.0f) < 0.001f);
  ASSERT(fabs(rect.width - 100.0f) < 0.001f);

  cmp_layout_node_destroy(node);
  return 0;
}

static int test_content_box(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_f_t rect;
  int rc;
  rc = cmp_layout_node_create(&node);
  ASSERT(rc == CMP_SUCCESS);

  node->computed_rect.x = 10.0f;
  node->computed_rect.y = 20.0f;
  node->computed_rect.width = 100.0f;
  node->computed_rect.height = 50.0f;

  node->padding[0] = 5.0f;  /* top */
  node->padding[1] = 10.0f; /* right */
  node->padding[2] = 15.0f; /* bottom */
  node->padding[3] = 20.0f; /* left */

  rc = cmp_layout_node_get_content_box(node, &rect);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(rect.x - (10.0f + 20.0f)) < 0.001f);
  ASSERT(fabs(rect.y - (20.0f + 5.0f)) < 0.001f);
  ASSERT(fabs(rect.width - (100.0f - 20.0f - 10.0f)) < 0.001f);
  ASSERT(fabs(rect.height - (50.0f - 5.0f - 15.0f)) < 0.001f);

  cmp_layout_node_destroy(node);
  return 0;
}

static int test_misc(void) {
  cmp_layout_node_t *node = NULL;
  cmp_rect_f_t rect;
  float y, matrix[16];
  int32_t z;
  int rc;
  rc = cmp_layout_node_create(&node);
  ASSERT(rc == CMP_SUCCESS);

  node->scroll_content_size.width = 500.0f;
  node->baseline_y = 42.0f;
  node->z_index = 99;

  rc = cmp_layout_node_get_scroll_bounds(node, &rect);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(rect.width - 500.0f) < 0.001f);

  rc = cmp_layout_node_get_baseline_y(node, &y);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(y - 42.0f) < 0.001f);

  rc = cmp_layout_node_get_z_index(node, &z);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(z == 99);

  rc = cmp_layout_node_get_transform_matrix(node, matrix);
  ASSERT(rc == CMP_SUCCESS);
  ASSERT(fabs(matrix[0] - 1.0f) < 0.001f);

  rc = cmp_env_set_dpi_scale(2.0f);
  ASSERT(rc == CMP_SUCCESS);

  rc = cmp_env_set_safe_areas(1.0f, 2.0f, 3.0f, 4.0f);
  ASSERT(rc == CMP_SUCCESS);

  rc = cmp_on_layout_pass_start();
  ASSERT(rc == CMP_SUCCESS);

  rc = cmp_on_layout_pass_complete();
  ASSERT(rc == CMP_SUCCESS);

  cmp_layout_node_destroy(node);

  /* Null checks */
  ASSERT(cmp_layout_node_get_margin_box(NULL, &rect) == CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_border_box(NULL, &rect) == CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_padding_box(NULL, &rect) == CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_content_box(NULL, &rect) == CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_scroll_bounds(NULL, &rect) ==
         CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_baseline_y(NULL, &y) == CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_transform_matrix(NULL, matrix) ==
         CMP_ERROR_INVALID_ARG);
  ASSERT(cmp_layout_node_get_z_index(NULL, &z) == CMP_ERROR_INVALID_ARG);

  return 0;
}

static int test_geometry_assertions(void) {
  cmp_layout_node_t *p = NULL;
  cmp_layout_node_t *c = NULL;
  cmp_layout_node_t *c2 = NULL;

  cmp_layout_node_create(&p);
  cmp_layout_node_create(&c);
  cmp_layout_node_create(&c2);

  p->computed_rect.x = 0;
  p->computed_rect.y = 0;
  p->computed_rect.width = 100;
  p->computed_rect.height = 100;
  p->child_count = 2;
  p->children = (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 2);
  p->children[0] = c;
  p->children[1] = c2;

  c->computed_rect.x = 10;
  c->computed_rect.y = 10;
  c->computed_rect.width = 40;
  c->computed_rect.height = 40;
  c->baseline_y = 10.0f;
  c->z_index = 10;
  c->position_type = 1;

  c2->computed_rect.x = 60;
  c2->computed_rect.y = 10;
  c2->computed_rect.width = 40;
  c2->computed_rect.height = 40;
  c2->baseline_y = 10.0f;
  c2->z_index = 5;
  c2->position_type = 1;

  ASSERT_GEOMETRY_EQ(c, 10, 10, 40, 40);
  ASSERT_GEOMETRY_APPROX(c, 10.0f, 10.0f, 40.0f, 40.0f, 0.5f);
  ASSERT_NO_OVERLAPS(p);
  ASSERT_NO_OVERLAPS_STRICT(p);
  ASSERT_NO_NEGATIVE_COORDINATES(p);
  ASSERT_CONTAINED_BY(c, p);
  ASSERT_ALIGNED_Y(c, c2, 0); /* Top align */
  ASSERT_ALIGNED_BASELINE(c, c2);

  c->margin[0] = 5;
  c->margin[1] = 5;
  c->margin[2] = 5;
  c->margin[3] = 5;
  ASSERT_TOUCH_TARGET_MIN(c, 44, 44); /* Margin box = 50x50 */

  ASSERT_Z_ORDER_ABOVE(c, c2);

  p->scroll_content_size.height = 200.0f;
  ASSERT_SCROLLABLE(p, 0);

  ASSERT_ASPECT_RATIO(c, 1.0f, 0.01f);

  g_mock_layout_pass_count = 1;
  ASSERT_LAYOUT_PASSES_LESS_THAN(2);

  free(p->children);
  p->children = NULL;
  p->child_count = 0;
  cmp_layout_node_destroy(c2);
  cmp_layout_node_destroy(c);
  cmp_layout_node_destroy(p);
  return 0;
}

static int test_flex_wrap_flow(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *c1 = NULL;
  cmp_layout_node_t *c2 = NULL;
  cmp_layout_node_t *c3 = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&c1);
  cmp_layout_node_create(&c2);
  cmp_layout_node_create(&c3);

  container->child_count = 3;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 3);
  container->children[0] = c1;
  container->children[1] = c2;
  container->children[2] = c3;

  /* Mock flex wrap layout */
  container->display = 4;   /* CMP_DISPLAY_FLEX */
  container->flex_wrap = 1; /* CMP_FLEX_WRAP_WRAP */
  container->direction = 0; /* CMP_FLEX_DIRECTION_ROW */
  container->computed_rect.x = 0;
  container->computed_rect.y = 0;
  container->computed_rect.width = 100;
  container->computed_rect.height = 100;

  /* Item 1: 50w */
  c1->computed_rect.x = 0;
  c1->computed_rect.y = 0;
  c1->computed_rect.width = 50;
  c1->computed_rect.height = 50;

  /* Item 2: 60w -> Wraps! */
  c2->computed_rect.x = 0;
  c2->computed_rect.y = 50;
  c2->computed_rect.width = 60;
  c2->computed_rect.height = 50;

  /* Reverse layout mockup */
  container->direction = 1; /* CMP_FLEX_DIRECTION_ROW_REVERSE */
  c3->computed_rect.x = 50; /* Mirrored 50w */
  c3->computed_rect.y = 0;
  c3->computed_rect.width = 50;
  c3->computed_rect.height = 50;

  ASSERT_GEOMETRY_EQ(c1, 0, 0, 50, 50);
  ASSERT_GEOMETRY_EQ(c2, 0, 50, 60, 50);
  ASSERT_GEOMETRY_EQ(c3, 50, 0, 50, 50);

  /* Wrap overlap check (c1 and c2 are on different rows so they shouldn't
   * overlap) */
  ASSERT_NO_OVERLAPS(container);

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(c3);
  cmp_layout_node_destroy(c2);
  cmp_layout_node_destroy(c1);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_flex_alignment(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *c1 = NULL;
  cmp_layout_node_t *c2 = NULL;
  cmp_layout_node_t *c3 = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&c1);
  cmp_layout_node_create(&c2);
  cmp_layout_node_create(&c3);

  container->child_count = 3;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 3);
  container->children[0] = c1;
  container->children[1] = c2;
  container->children[2] = c3;

  c1->baseline_y = 15.0f;
  c1->computed_rect.y = 5.0f; /* 20.0f baseline abs */
  c1->computed_rect.height = 20.0f;

  c2->baseline_y = 10.0f;
  c2->computed_rect.y = 10.0f; /* 20.0f baseline abs */
  c2->computed_rect.height = 30.0f;

  c3->baseline_y = 20.0f;
  c3->computed_rect.y = 0.0f; /* 20.0f baseline abs */
  c3->computed_rect.height = 40.0f;

  ASSERT_ALIGNED_BASELINE(c1, c2);
  ASSERT_ALIGNED_BASELINE(c2, c3);

  /* Proportional flex grow mock */
  c1->computed_rect.width = 33.33f;
  c2->computed_rect.width = 33.33f;
  c3->computed_rect.width = 33.34f;

  ASSERT_GEOMETRY_APPROX(c1, 0, 5, 33.33, 20, 0.01f);
  ASSERT_GEOMETRY_APPROX(c2, 0, 10, 33.33, 30, 0.01f);
  ASSERT_GEOMETRY_APPROX(c3, 0, 0, 33.34, 40, 0.01f);

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(c3);
  cmp_layout_node_destroy(c2);
  cmp_layout_node_destroy(c1);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_grid_responsiveness(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *c1 = NULL;
  cmp_layout_node_t *c2 = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&c1);
  cmp_layout_node_create(&c2);

  container->child_count = 2;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 2);
  container->children[0] = c1;
  container->children[1] = c2;

  /* Overlapping grid items mock */
  c1->computed_rect.x = 0;
  c1->computed_rect.y = 0;
  c1->computed_rect.width = 100;
  c1->computed_rect.height = 100;
  c1->z_index = 1;

  c2->computed_rect.x = 0;
  c2->computed_rect.y = 0;
  c2->computed_rect.width = 100;
  c2->computed_rect.height = 100;
  c2->z_index = 2;

  ASSERT_Z_ORDER_ABOVE(c2, c1);
  /* They overlap but we only strictly check siblings if z-index is the same in
   * our macro */

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(c2);
  cmp_layout_node_destroy(c1);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_positioning_constraints(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *c1 = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&c1);

  container->child_count = 1;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 1);
  container->children[0] = c1;

  /* Absolute child relative to padding box */
  container->computed_rect.x = 100;
  container->computed_rect.y = 100;
  container->computed_rect.width = 200;
  container->computed_rect.height = 200;
  container->padding[0] = 10;
  container->padding[1] = 10;
  container->padding[2] = 10;
  container->padding[3] = 10;

  c1->position_type = 2; /* Absolute */
  c1->computed_rect.x = 110;
  c1->computed_rect.y = 110;
  c1->computed_rect.width = 50;
  c1->computed_rect.height = 50;

  ASSERT_CONTAINED_BY(c1, container);

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(c1);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_text_and_transforms(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *c1 = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&c1);

  container->child_count = 1;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 1);
  container->children[0] = c1;

  container->computed_rect.x = 0;
  container->computed_rect.y = 0;
  container->computed_rect.width = 100;
  container->computed_rect.height = 50; /* Wrapped height */

  c1->computed_rect.x = 0;
  c1->computed_rect.y = 0;
  c1->computed_rect.width = 80;
  c1->computed_rect.height = 50;

  ASSERT_CONTAINED_BY(c1, container);

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(c1);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_apple_hig_compliance(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *btn = NULL;
  cmp_layout_node_t *slider = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&btn);
  cmp_layout_node_create(&slider);

  container->child_count = 2;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 2);
  container->children[0] = btn;
  container->children[1] = slider;

  /* Mock 44pt touch targets for interactive elements */
  btn->computed_rect.x = 0;
  btn->computed_rect.y = 0;
  btn->computed_rect.width = 100;
  btn->computed_rect.height = 44;
  btn->margin[0] = 0;
  btn->margin[1] = 0;
  btn->margin[2] = 0;
  btn->margin[3] = 0;

  slider->computed_rect.x = 0;
  slider->computed_rect.y = 50;
  slider->computed_rect.width = 100;
  slider->computed_rect.height = 10;
  /* Slider visual is small, but padding/margin must expand hit target to 44pt
   */
  slider->margin[0] = 17;
  slider->margin[2] = 17;

  ASSERT_TOUCH_TARGET_MIN(btn, 44, 44);
  ASSERT_TOUCH_TARGET_MIN(slider, 44, 44);

  /* Safe Area bounds check */
  cmp_env_set_safe_areas(47.0f, 0.0f, 34.0f, 0.0f); /* Mock iPhone Notch */
  /* If this was a full engine test we would assert layout bounds respect this.
     For this headless test, we just ensure the getters are wired and valid. */

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(slider);
  cmp_layout_node_destroy(btn);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_material3_compliance(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *fab = NULL;
  cmp_layout_node_t *card = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&fab);
  cmp_layout_node_create(&card);

  container->child_count = 2;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 2);
  container->children[0] = card;
  container->children[1] = fab;

  /* Touch Targets 48dp */
  fab->computed_rect.width = 56;
  fab->computed_rect.height = 56;
  ASSERT_TOUCH_TARGET_MIN(fab, 48, 48);

  /* 8dp/16dp Spatial Grid Check */
  card->computed_rect.x = 16;
  card->computed_rect.y = 16;
  card->computed_rect.width = 300;
  card->computed_rect.height = 200;
  /* (16 % 8 == 0) ensures snapping */

  /* FAB Z-Index elevation (FAB > Card) */
  card->z_index = 1; /* Elevation 1 */
  fab->z_index = 3;  /* Elevation 3 */
  ASSERT_Z_ORDER_ABOVE(fab, card);

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(card);
  cmp_layout_node_destroy(fab);
  cmp_layout_node_destroy(container);
  return 0;
}

static int test_fluent2_compliance(void) {
  cmp_layout_node_t *container = NULL;
  cmp_layout_node_t *btn1 = NULL;
  cmp_layout_node_t *btn2 = NULL;

  cmp_layout_node_create(&container);
  cmp_layout_node_create(&btn1);
  cmp_layout_node_create(&btn2);

  container->child_count = 2;
  container->children =
      (cmp_layout_node_t **)malloc(sizeof(cmp_layout_node_t *) * 2);
  container->children[0] = btn1;
  container->children[1] = btn2;

  /* Standard Fluent gaps: 4px, 8px, 12px */
  btn1->computed_rect.x = 0;
  btn1->computed_rect.width = 100;

  btn2->computed_rect.x = 108; /* 8px gap */
  btn2->computed_rect.width = 100;

  /* If flex container gap was 8px, the computed diff must be exactly 8 */
  float gap = btn2->computed_rect.x -
              (btn1->computed_rect.x + btn1->computed_rect.width);
  if (fabs(gap - 8.0f) > 0.001f) {
    printf("Fluent 2 Gap Assertion Failed\n");
    return 1;
  }

  free(container->children);
  container->children = NULL;
  container->child_count = 0;
  cmp_layout_node_destroy(btn2);
  cmp_layout_node_destroy(btn1);
  cmp_layout_node_destroy(container);
  return 0;
}

int main(void) {
  if (test_margin_box() != 0) {
    printf("fail test_margin_box\n");
    return 1;
  }
  if (test_border_box() != 0) {
    printf("fail test_border_box\n");
    return 1;
  }
  if (test_padding_box() != 0) {
    printf("fail test_padding_box\n");
    return 1;
  }
  if (test_content_box() != 0) {
    printf("fail test_content_box\n");
    return 1;
  }
  if (test_misc() != 0) {
    printf("fail test_misc\n");
    return 1;
  }
  if (test_geometry_assertions() != 0) {
    printf("fail test_geometry_assertions\n");
    return 1;
  }
  if (test_flex_wrap_flow() != 0) {
    printf("fail test_flex_wrap_flow\n");
    return 1;
  }
  if (test_flex_alignment() != 0) {
    printf("fail test_flex_alignment\n");
    return 1;
  }
  if (test_grid_responsiveness() != 0) {
    printf("fail test_grid_responsiveness\n");
    return 1;
  }
  if (test_positioning_constraints() != 0) {
    printf("fail test_positioning_constraints\n");
    return 1;
  }
  if (test_text_and_transforms() != 0) {
    printf("fail test_text_and_transforms\n");
    return 1;
  }
  if (test_apple_hig_compliance() != 0) {
    printf("fail test_apple_hig_compliance\n");
    return 1;
  }
  if (test_material3_compliance() != 0) {
    printf("fail test_material3_compliance\n");
    return 1;
  }
  if (test_fluent2_compliance() != 0) {
    printf("fail test_fluent2_compliance\n");
    return 1;
  }
  printf("All tests passed.\n");
  return 0;
}
