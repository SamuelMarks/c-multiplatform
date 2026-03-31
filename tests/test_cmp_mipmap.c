/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_mipmap_create_destroy(void) {
  cmp_mipmap_t *mipmap = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_create(&mipmap));
  ASSERT_NEQ(NULL, mipmap);

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_destroy(mipmap));
  PASS();
}

TEST test_mipmap_generate(void) {
  cmp_mipmap_t *mipmap = NULL;
  cmp_texture_t tex = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_create(&mipmap));

  tex.width = 1024;
  tex.height = 1024;
  tex.internal_handle = (void *)(size_t)0x1234;

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generate(mipmap, &tex));

  /* Asymmetric dimensions test */
  tex.width = 512;
  tex.height = 2048;
  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generate(mipmap, &tex));

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_destroy(mipmap));
  PASS();
}

TEST test_mipmap_set_anisotropy(void) {
  cmp_texture_t tex = {0};
  tex.width = 1024;
  tex.height = 1024;
  tex.internal_handle = (void *)(size_t)0x1234;

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_set_anisotropy(&tex, 1.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_set_anisotropy(&tex, 4.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_set_anisotropy(&tex, 16.0f));

  /* Testing implicit clamping to 16.0 */
  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_set_anisotropy(&tex, 32.0f));

  PASS();
}

TEST test_mipmap_edge_cases(void) {
  cmp_mipmap_t *mipmap = NULL;
  cmp_texture_t tex = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_create(&mipmap));

  /* NULL pointers */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_generate(NULL, &tex));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_generate(mipmap, NULL));

  /* Out of bounds dimensions */
  tex.width = 0;
  tex.height = 1024;
  ASSERT_EQ(CMP_ERROR_BOUNDS, cmp_mipmap_generate(mipmap, &tex));

  tex.width = 1024;
  tex.height = -5;
  ASSERT_EQ(CMP_ERROR_BOUNDS, cmp_mipmap_generate(mipmap, &tex));

  /* Anisotropy edge cases */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_set_anisotropy(NULL, 4.0f));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_mipmap_set_anisotropy(&tex, 0.0f)); /* Less than 1.0 is invalid */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_set_anisotropy(&tex, -4.0f));

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_destroy(mipmap));
  PASS();
}

SUITE(cmp_mipmap_suite) {
  RUN_TEST(test_mipmap_create_destroy);
  RUN_TEST(test_mipmap_generate);
  RUN_TEST(test_mipmap_set_anisotropy);
  RUN_TEST(test_mipmap_edge_cases);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_mipmap_suite);
  GREATEST_MAIN_END();
}
