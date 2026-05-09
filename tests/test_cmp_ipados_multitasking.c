/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_ipados_multitasking_lifecycle(void) {
  cmp_ipados_multitasking_t *mt = NULL;
  cmp_size_class_t h, v;

  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_multitasking_create(&mt));
  ASSERT_NEQ(NULL, mt);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ipados_multitasking_request_scene(mt, "com.example.activity"));

  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_multitasking_resolve_layout(
                             mt, 1024.0f, 768.0f, &h, &v));

  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_multitasking_destroy(mt));
  PASS();
}

TEST test_ipados_multitasking_null_args(void) {
  cmp_ipados_multitasking_t *mt = NULL;
  cmp_size_class_t h, v;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_multitasking_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_multitasking_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_ipados_multitasking_create(&mt));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_multitasking_request_scene(
                                       NULL, "com.example.activity"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ipados_multitasking_request_scene(mt, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_multitasking_resolve_layout(
                                       NULL, 1024.0f, 768.0f, &h, &v));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_multitasking_resolve_layout(
                                       mt, 1024.0f, 768.0f, NULL, &v));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ipados_multitasking_resolve_layout(
                                       mt, 1024.0f, 768.0f, &h, NULL));

  cmp_ipados_multitasking_destroy(mt);
  PASS();
}

SUITE(ipados_multitasking_suite) {
  RUN_TEST(test_ipados_multitasking_lifecycle);
  RUN_TEST(test_ipados_multitasking_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ipados_multitasking_suite);
  GREATEST_MAIN_END();
}
