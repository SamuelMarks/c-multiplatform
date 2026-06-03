/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_android_renderer_create_destroy(void) {
  cmp_android_renderer_t *r = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_android_renderer_create(&r));
  ASSERT_NEQ(NULL, r);

  ASSERT_EQ(CMP_SUCCESS, cmp_android_renderer_destroy(r));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_renderer_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_renderer_destroy(NULL));

  PASS();
}

TEST test_android_renderer_init(void) {
  cmp_android_renderer_t *r = NULL;
  cmp_window_t *w = NULL; /* Mock window since we just need a pointer */
  int backend = 0;
  int w_placeholder = 0;
  w = (cmp_window_t *)&w_placeholder;

  ASSERT_EQ(CMP_SUCCESS, cmp_android_renderer_create(&r));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_android_renderer_initialize_fallback(r, w, &backend));
  ASSERT_EQ(1, backend);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_renderer_initialize_fallback(NULL, w, &backend));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_renderer_initialize_fallback(r, NULL, &backend));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_renderer_initialize_fallback(r, w, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_android_renderer_destroy(r));
  PASS();
}

SUITE(android_renderer_suite) {
  RUN_TEST(test_android_renderer_create_destroy);
  RUN_TEST(test_android_renderer_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(android_renderer_suite);
  GREATEST_MAIN_END();
}
