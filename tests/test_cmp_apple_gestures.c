/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

/* We need a mock cmp_window_apple_enable_gestures since it's probably missing
 * or we need to test null logic */
/* Actually, the build process handles linking, but we test what we can. */

TEST test_apple_gestures_create_destroy(void) {
  cmp_apple_gestures_t *gestures = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_apple_gestures_create(&gestures));
  ASSERT_NEQ(NULL, gestures);

  ASSERT_EQ(CMP_SUCCESS, cmp_apple_gestures_destroy(gestures));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_apple_gestures_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_apple_gestures_destroy(NULL));

  PASS();
}

TEST test_apple_gestures_enable(void) {
  cmp_apple_gestures_t *gestures = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_apple_gestures_create(&gestures));
  ASSERT_NEQ(NULL, gestures);

  /* Can't easily test the actual cmp_window_apple_enable_gestures without a
   * mock window. But we can test the null arg paths. */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_apple_gestures_enable(NULL, NULL, 1, 1, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_apple_gestures_enable(gestures, NULL, 1, 1, 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_apple_gestures_destroy(gestures));
  PASS();
}

SUITE(apple_gestures_suite) {
  RUN_TEST(test_apple_gestures_create_destroy);
  RUN_TEST(test_apple_gestures_enable);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(apple_gestures_suite);
  GREATEST_MAIN_END();
}
