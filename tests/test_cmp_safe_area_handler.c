/* clang-format off */
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_safe_area_handler_suite);

TEST test_cmp_safe_area_handler_create_destroy(void) {
  cmp_safe_area_handler_t *handler = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_area_handler_create(&handler));
  ASSERT_NEQ(NULL, handler);

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_area_handler_destroy(handler));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_safe_area_handler_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_safe_area_handler_destroy(NULL));

  PASS();
}

TEST test_cmp_safe_area_handler_query_insets(void) {
  cmp_safe_area_handler_t *handler = NULL;
  cmp_window_t *window = (cmp_window_t *)1; /* Mock window pointer */
  int top = -1, bottom = -1, left = -1, right = -1;

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_area_handler_create(&handler));

  /* Mock the underlying areas by getting them?
     The structure is opaque, so we can't easily mock the inside here
     without poking through it. But the default is 0.0f */
  ASSERT_EQ(CMP_SUCCESS, cmp_safe_area_handler_query_insets(
                             handler, window, &top, &bottom, &left, &right));

  ASSERT_EQ(0, top);
  ASSERT_EQ(0, bottom);
  ASSERT_EQ(0, left);
  ASSERT_EQ(0, right);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_safe_area_handler_query_insets(NULL, window, &top, &bottom,
                                               &left, &right));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_safe_area_handler_query_insets(handler, NULL, &top, &bottom,
                                               &left, &right));

  /* Null pointers for out params are allowed per the implementation,
     they just shouldn't crash and should return SUCCESS if args are valid */
  ASSERT_EQ(CMP_SUCCESS, cmp_safe_area_handler_query_insets(
                             handler, window, NULL, NULL, NULL, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_safe_area_handler_destroy(handler));

  PASS();
}

SUITE(cmp_safe_area_handler_suite) {
  RUN_TEST(test_cmp_safe_area_handler_create_destroy);
  RUN_TEST(test_cmp_safe_area_handler_query_insets);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_safe_area_handler_suite);
  GREATEST_MAIN_END();
}
