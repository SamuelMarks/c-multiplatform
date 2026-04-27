/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_image_preview_suite);

TEST test_image_preview_create_destroy(void) {
  cmp_image_preview_t *preview = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_image_preview_create(&preview));
  ASSERT_NEQ(NULL, preview);
  ASSERT_EQ(CMP_SUCCESS, cmp_image_preview_destroy(preview));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_image_preview_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_image_preview_destroy(NULL));
  PASS();
}

TEST test_image_preview_load_base64(void) {
  cmp_image_preview_t *preview = NULL;
  unsigned char *pixels = NULL;
  int w, h;
  /* "A" base64 encoded is "QQ==" */
  const char *base64 = "QQ==";

  ASSERT_EQ(CMP_SUCCESS, cmp_image_preview_create(&preview));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(NULL, base64, &pixels, &w, &h));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(preview, NULL, &pixels, &w, &h));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(preview, base64, NULL, &w, &h));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(preview, base64, &pixels, NULL, &h));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(preview, base64, &pixels, &w, NULL));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_image_preview_load_base64(preview, base64, &pixels, &w, &h));
  ASSERT_NEQ(NULL, pixels);
  ASSERT_EQ(0, w); /* we expect 1 byte out, w = 1/4 = 0 theoretically via the
                      simple integer division */
  ASSERT_EQ('A', pixels[0]);

  ASSERT_EQ(CMP_SUCCESS, cmp_image_preview_free_pixels(pixels));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_image_preview_free_pixels(NULL));

  /* Test invalid length */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(preview, "QQ=", &pixels, &w, &h));

  /* Test invalid char */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_image_preview_load_base64(preview, "Q@==", &pixels, &w, &h));

  ASSERT_EQ(CMP_SUCCESS, cmp_image_preview_destroy(preview));
  PASS();
}

SUITE(cmp_image_preview_suite) {
  RUN_TEST(test_image_preview_create_destroy);
  RUN_TEST(test_image_preview_load_base64);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_image_preview_suite);
  GREATEST_MAIN_END();
}
