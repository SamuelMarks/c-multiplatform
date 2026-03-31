/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_cmp_shader_init_rounded_rect(void) {
  cmp_shader_t shader;
  int err;

  shader.internal_handle = NULL;
  err = cmp_shader_init_rounded_rect(&shader);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT_NEQ(NULL, shader.internal_handle);
  ASSERT_EQ(1, *((int *)shader.internal_handle));

  err = cmp_shader_destroy(&shader);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT_EQ(NULL, shader.internal_handle);

  PASS();
}

TEST test_cmp_shader_init_gradient(void) {
  cmp_shader_t shader;
  int err;

  shader.internal_handle = NULL;
  err = cmp_shader_init_gradient(&shader);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT_NEQ(NULL, shader.internal_handle);
  ASSERT_EQ(2, *((int *)shader.internal_handle));

  err = cmp_shader_destroy(&shader);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT_EQ(NULL, shader.internal_handle);

  PASS();
}

TEST test_cmp_shader_init_sdf_text(void) {
  cmp_shader_t shader;
  int err;

  shader.internal_handle = NULL;
  err = cmp_shader_init_sdf_text(&shader);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT_NEQ(NULL, shader.internal_handle);
  ASSERT_EQ(3, *((int *)shader.internal_handle));

  err = cmp_shader_destroy(&shader);
  ASSERT_EQ(CMP_SUCCESS, err);
  ASSERT_EQ(NULL, shader.internal_handle);

  PASS();
}

TEST test_cmp_shader_destroy_null(void) {
  int err = cmp_shader_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, err);
  PASS();
}

TEST test_cmp_shader_init_null(void) {
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_init_rounded_rect(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_init_gradient(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_shader_init_sdf_text(NULL));
  PASS();
}

SUITE(cmp_shader_suite) {
  RUN_TEST(test_cmp_shader_init_rounded_rect);
  RUN_TEST(test_cmp_shader_init_gradient);
  RUN_TEST(test_cmp_shader_init_sdf_text);
  RUN_TEST(test_cmp_shader_destroy_null);
  RUN_TEST(test_cmp_shader_init_null);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_shader_suite);
  GREATEST_MAIN_END();
}
