/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_dynamic_type_lifecycle(void) {
  cmp_dynamic_type_t *dyn = NULL;
  int res = cmp_dynamic_type_create(&dyn);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, dyn);

  res = cmp_dynamic_type_destroy(dyn);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_dynamic_type_null_args(void) {
  int res = cmp_dynamic_type_create(NULL);
  cmp_dynamic_type_t *dyn = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_dynamic_type_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_dynamic_type_create(&dyn);

  res = cmp_dynamic_type_set_scale(NULL, 1.5f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_dynamic_type_set_scale(dyn, -1.0f);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_dynamic_type_apply(NULL, (void *)0x1234);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_dynamic_type_apply(dyn, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_dynamic_type_destroy(dyn);
  PASS();
}

TEST test_dynamic_type_operations(void) {
  cmp_dynamic_type_t *dyn = NULL;
  int res;
  cmp_dynamic_type_create(&dyn);

  res = cmp_dynamic_type_set_scale(dyn, 1.5f);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_dynamic_type_apply(dyn, (void *)0x1234);
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_dynamic_type_destroy(dyn);
  PASS();
}

SUITE(dynamic_type_suite) {
  RUN_TEST(test_dynamic_type_lifecycle);
  RUN_TEST(test_dynamic_type_null_args);
  RUN_TEST(test_dynamic_type_operations);
}

#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(dynamic_type_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
