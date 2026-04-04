/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_editable_lifecycle(void) {
  cmp_editable_t *editable = NULL;
  int res = cmp_editable_create(&editable);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, editable);

  res = cmp_editable_destroy(editable);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_editable_null_args(void) {
  int res = cmp_editable_create(NULL);
  cmp_editable_t *editable = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_editable_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_editable_insert_text(NULL, "test");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_editable_create(&editable);
  res = cmp_editable_insert_text(editable, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  cmp_editable_destroy(editable);
  PASS();
}

TEST test_editable_insert(void) {
  cmp_editable_t *editable = NULL;
  int res;
  cmp_editable_create(&editable);

  res = cmp_editable_insert_text(editable, "hello ");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_editable_insert_text(editable, "world");
  ASSERT_EQ(CMP_SUCCESS, res);

  cmp_editable_destroy(editable);
  PASS();
}

SUITE(editable_suite) {
  RUN_TEST(test_editable_lifecycle);
  RUN_TEST(test_editable_null_args);
  RUN_TEST(test_editable_insert);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(editable_suite);
  GREATEST_MAIN_END();
}
