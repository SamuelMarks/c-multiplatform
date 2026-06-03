/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_android_storage_create_destroy(void) {
  cmp_android_storage_t *st = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_android_storage_create(&st));
  ASSERT_NEQ(NULL, st);

  ASSERT_EQ(CMP_SUCCESS, cmp_android_storage_destroy(st));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_storage_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_storage_destroy(NULL));

  PASS();
}

TEST test_android_storage_access(void) {
  cmp_android_storage_t *st = NULL;
  char *uri = NULL;
  int can_write = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_android_storage_create(&st));

  ASSERT_EQ(CMP_SUCCESS, cmp_android_storage_request_tree_access(st, &uri));
  ASSERT_NEQ(NULL, uri);

  ASSERT_EQ(CMP_SUCCESS, cmp_android_storage_check_access(st, uri, &can_write));
  ASSERT_EQ(1, can_write);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_android_storage_check_access(st, "file://test", &can_write));
  ASSERT_EQ(0, can_write);

  CMP_FREE(uri);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_storage_request_tree_access(NULL, &uri));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_storage_request_tree_access(st, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_storage_check_access(
                                       NULL, "content://test", &can_write));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_storage_check_access(st, NULL, &can_write));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_android_storage_check_access(st, "content://test", NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_android_storage_destroy(st));
  PASS();
}

SUITE(android_storage_suite) {
  RUN_TEST(test_android_storage_create_destroy);
  RUN_TEST(test_android_storage_access);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(android_storage_suite);
  GREATEST_MAIN_END();
}
