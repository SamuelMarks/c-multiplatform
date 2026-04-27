
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_mipmap_lifecycle(void) {
  cmp_mipmap_generator_t *gen = NULL;
  void *mipmaps[8] = {0};
  size_t levels = 0;
  char dummy_data[4 * 4] = {0};

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generator_create(&gen));
  ASSERT_NEQ(NULL, gen);

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generator_generate(gen, dummy_data, 4, 4,
                                                       mipmaps, &levels));
  ASSERT_EQ(2, levels);

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generator_destroy(gen));
  PASS();
}

TEST test_mipmap_null_args(void) {
  cmp_mipmap_generator_t *gen = NULL;
  void *mipmaps[8] = {0};
  size_t levels = 0;
  char dummy_data[4 * 4] = {0};

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_generator_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_generator_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generator_create(&gen));

  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_mipmap_generator_generate(NULL, dummy_data, 4, 4, mipmaps, &levels));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_mipmap_generator_generate(gen, NULL, 4, 4, mipmaps, &levels));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_mipmap_generator_generate(gen, dummy_data, 0, 4, mipmaps, &levels));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_mipmap_generator_generate(gen, dummy_data, 4, 0, mipmaps, &levels));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_generator_generate(
                                       gen, dummy_data, 4, 4, NULL, &levels));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mipmap_generator_generate(
                                       gen, dummy_data, 4, 4, mipmaps, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_mipmap_generator_destroy(gen));
  PASS();
}

SUITE(cmp_mipmap_suite) {
  RUN_TEST(test_mipmap_lifecycle);
  RUN_TEST(test_mipmap_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_mipmap_suite);
  GREATEST_MAIN_END();
}
