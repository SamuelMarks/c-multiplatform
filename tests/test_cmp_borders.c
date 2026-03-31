/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_borders_suite);

TEST test_radius_init_and_uniform(void) {
  cmp_radius_t r;

  ASSERT_EQ(CMP_SUCCESS, cmp_radius_init(&r));
  ASSERT_EQ(CMP_CORNER_ROUND, r.corner_shape);
  ASSERT_EQ(0.0f, r.top_left_x);

  ASSERT_EQ(CMP_SUCCESS, cmp_radius_set_uniform(&r, 10.0f));
  ASSERT_EQ(10.0f, r.top_left_x);
  ASSERT_EQ(10.0f, r.bottom_right_y);
  PASS();
}

TEST test_radius_hit_test(void) {
  cmp_radius_t r;
  int inside;

  cmp_radius_init(&r);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_radius_hit_test(&r, 100.0f, 100.0f, 50.0f, 50.0f, &inside));
  ASSERT_EQ(1, inside);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_radius_hit_test(&r, 100.0f, 100.0f, -10.0f, 50.0f, &inside));
  ASSERT_EQ(0, inside);

  PASS();
}

TEST test_box_shadow(void) {
  cmp_box_shadow_t *s1 = NULL;
  cmp_box_shadow_t *s2 = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_box_shadow_create(&s1));
  ASSERT_NEQ(NULL, s1);

  ASSERT_EQ(CMP_SUCCESS, cmp_box_shadow_create(&s2));

  ASSERT_EQ(CMP_SUCCESS, cmp_box_shadow_append(s1, s2));
  ASSERT_EQ(s2, s1->next);

  ASSERT_EQ(CMP_SUCCESS, cmp_box_shadow_destroy(s1));
  PASS();
}

TEST test_shadow_9patch(void) {
  cmp_shadow_9patch_t shadow;
  ASSERT_EQ(CMP_SUCCESS, cmp_shadow_9patch_generate(4.0f, &shadow));
  ASSERT_EQ(4.0f, shadow.elevation);
  ASSERT_EQ(NULL, shadow.base_texture);
  PASS();
}

TEST test_filters(void) {
  cmp_filter_t *f1 = NULL;
  cmp_filter_t *f2 = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_filter_create(&f1, CMP_FILTER_BLUR, 10.0f));
  ASSERT_EQ(CMP_FILTER_BLUR, f1->op);
  ASSERT_EQ(10.0f, f1->amount);

  ASSERT_EQ(CMP_SUCCESS, cmp_filter_create(&f2, CMP_FILTER_BRIGHTNESS, 1.5f));

  ASSERT_EQ(CMP_SUCCESS, cmp_filter_append(f1, f2));
  ASSERT_EQ(f2, f1->next);

  ASSERT_EQ(CMP_SUCCESS, cmp_filter_destroy(f1));
  PASS();
}

TEST test_backdrop_edge_mirror(void) {
  int clamped;

  ASSERT_EQ(CMP_SUCCESS, cmp_backdrop_edge_mirror(100, 50, &clamped));
  ASSERT_EQ(50, clamped);

  ASSERT_EQ(CMP_SUCCESS, cmp_backdrop_edge_mirror(100, -10, &clamped));
  ASSERT_EQ(10, clamped); /* Mirrors positive */

  ASSERT_EQ(CMP_SUCCESS, cmp_backdrop_edge_mirror(100, 110, &clamped));
  ASSERT_EQ(89, clamped); /* 100 - (110 - 100) - 1 */

  PASS();
}

SUITE(cmp_borders_suite) {
  RUN_TEST(test_radius_init_and_uniform);
  RUN_TEST(test_radius_hit_test);
  RUN_TEST(test_box_shadow);
  RUN_TEST(test_shadow_9patch);
  RUN_TEST(test_filters);
  RUN_TEST(test_backdrop_edge_mirror);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_borders_suite);
  GREATEST_MAIN_END();
}