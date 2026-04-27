/* clang-format off */
#include <cmp_screen_capture.h>
#include <cmp.h>
#include <greatest.h>
/* clang-format on */

SUITE(cmp_screen_capture_suite);

TEST test_cmp_screen_capture_create_destroy(void) {
  cmp_screen_capture_t *capture = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_create(&capture));
  ASSERT_NEQ(NULL, capture);

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_destroy(capture));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_screen_capture_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_screen_capture_destroy(NULL));

  PASS();
}

TEST test_cmp_screen_capture_active_window(void) {
  cmp_screen_capture_t *capture = NULL;
  unsigned char *pixels = NULL;
  int width = 0, height = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_create(&capture));

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_active_window(capture, &pixels,
                                                          &width, &height));
  ASSERT_NEQ(NULL, pixels);
  ASSERT_EQ(800, width);
  ASSERT_EQ(600, height);
  ASSERT_EQ(255, pixels[0]);

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_free_pixels(pixels));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_active_window(NULL, &pixels, &width, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_active_window(capture, NULL, &width, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_active_window(capture, &pixels, NULL, &height));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_active_window(capture, &pixels, &width, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_free_pixels(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_destroy(capture));

  PASS();
}

TEST test_cmp_screen_capture_region(void) {
  cmp_screen_capture_t *capture = NULL;
  unsigned char *pixels = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_create(&capture));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_screen_capture_region(capture, 10, 20, 100, 200, &pixels));
  ASSERT_NEQ(NULL, pixels);
  ASSERT_EQ(128, pixels[0]);

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_free_pixels(pixels));

  /* Null/bounds checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_region(NULL, 10, 20, 100, 200, &pixels));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_region(capture, 10, 20, 100, 200, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_region(capture, 10, 20, -1, 200, &pixels));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_screen_capture_region(capture, 10, 20, 100, -1, &pixels));

  ASSERT_EQ(CMP_SUCCESS, cmp_screen_capture_destroy(capture));

  PASS();
}

SUITE(cmp_screen_capture_suite) {
  RUN_TEST(test_cmp_screen_capture_create_destroy);
  RUN_TEST(test_cmp_screen_capture_active_window);
  RUN_TEST(test_cmp_screen_capture_region);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_screen_capture_suite);
  GREATEST_MAIN_END();
}
