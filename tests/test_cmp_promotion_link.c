/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_promotion_link_vrr(void) {
  cmp_promotion_link_t *link = NULL;
  cmp_frame_rate_t rate;

  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_create(&link));

  /* Test static screen (drops to 10Hz/24Hz) */
  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_evaluate_vrr(link, 0, 0, &rate));
  ASSERT_EQ(CMP_FRAME_RATE_LOW, rate);

  /* Test animating screen (jumps to 120Hz) */
  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_evaluate_vrr(link, 1, 0, &rate));
  ASSERT_EQ(CMP_FRAME_RATE_HIGH, rate);

  /* Test scrolling screen (jumps to 120Hz) */
  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_evaluate_vrr(link, 0, 1, &rate));
  ASSERT_EQ(CMP_FRAME_RATE_HIGH, rate);

  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_destroy(link));
  PASS();
}

TEST test_promotion_drops(void) {
  cmp_promotion_link_t *link = NULL;
  int dropped;

  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_create(&link));

  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_sync(link, 1));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_promotion_link_request_rate(link, CMP_FRAME_RATE_HIGH));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_promotion_link_validate_frame_drops(link, &dropped));
  ASSERT_EQ(0, dropped); /* Perfect A12+ guarantee */

  ASSERT_EQ(CMP_SUCCESS, cmp_promotion_link_destroy(link));
  PASS();
}

TEST test_null_args(void) {
  cmp_promotion_link_t *link = NULL;
  cmp_frame_rate_t r;
  int i;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_promotion_link_create(NULL));
  cmp_promotion_link_create(&link);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_promotion_link_sync(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_promotion_link_request_rate(NULL, CMP_FRAME_RATE_HIGH));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_promotion_link_evaluate_vrr(NULL, 0, 0, &r));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_promotion_link_evaluate_vrr(link, 0, 0, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_promotion_link_validate_frame_drops(NULL, &i));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_promotion_link_validate_frame_drops(link, NULL));

  cmp_promotion_link_destroy(link);
  PASS();
}

SUITE(promotion_link_suite) {
  RUN_TEST(test_promotion_link_vrr);
  RUN_TEST(test_promotion_drops);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(promotion_link_suite);
  GREATEST_MAIN_END();
}
