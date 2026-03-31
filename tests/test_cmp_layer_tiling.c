/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_tiling_create_destroy(void) {
  cmp_layer_tiling_t *tiling = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_create(512, &tiling));
  ASSERT_NEQ(NULL, tiling);

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_destroy(tiling));
  PASS();
}

TEST test_tiling_calculate(void) {
  cmp_layer_tiling_t *tiling = NULL;
  uint32_t count = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_create(512, &tiling));

  /* Fits exactly 1 tile */
  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_calculate(tiling, 512, 512, &count));
  ASSERT_EQ(1, count);

  /* Needs 2x2 = 4 tiles */
  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_calculate(tiling, 600, 600, &count));
  ASSERT_EQ(4, count);

  /* Needs 1x3 = 3 tiles (e.g. extremely tall scrolling layer) */
  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_calculate(tiling, 512, 1500, &count));
  ASSERT_EQ(3, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_destroy(tiling));
  PASS();
}

TEST test_tiling_edge_cases(void) {
  cmp_layer_tiling_t *tiling = NULL;
  uint32_t count = 0;
  cmp_rect_t bounds;

  /* Null pointers */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_layer_tiling_create(512, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_layer_tiling_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_layer_tiling_calculate(NULL, 100, 100, &count));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_layer_tiling_calculate(tiling, 100, 100, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_layer_tiling_get_bounds(NULL, 0, &bounds));

  /* Zero tile size */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_layer_tiling_create(0, &tiling));

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_create(512, &tiling));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_layer_tiling_get_bounds(tiling, 0, NULL));

  /* Zero dimensions should return 0 tiles successfully */
  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_calculate(tiling, 0, 100, &count));
  ASSERT_EQ(0, count);
  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_calculate(tiling, 100, 0, &count));
  ASSERT_EQ(0, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_layer_tiling_destroy(tiling));
  PASS();
}

SUITE(cmp_layer_tiling_suite) {
  RUN_TEST(test_tiling_create_destroy);
  RUN_TEST(test_tiling_calculate);
  RUN_TEST(test_tiling_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_layer_tiling_suite);
  GREATEST_MAIN_END();
}
