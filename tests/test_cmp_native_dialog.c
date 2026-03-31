/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_native_dialog_suite);

TEST test_cmp_native_dialog_create_destroy(void) {
  cmp_native_dialog_t *dialog = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_native_dialog_create(&dialog));
  ASSERT_NEQ(NULL, dialog);

  ASSERT_EQ(CMP_SUCCESS, cmp_native_dialog_destroy(dialog));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_native_dialog_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_native_dialog_destroy(NULL));

  PASS();
}

TEST test_cmp_native_dialog_show_and_result(void) {
  cmp_native_dialog_t *dialog = NULL;
  char *result = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_native_dialog_create(&dialog));

  /* Show dialog (stubbed) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_native_dialog_show(dialog, CMP_DIALOG_TYPE_FILE_OPEN));

  /* Initial result should be NULL */
  ASSERT_EQ(CMP_SUCCESS, cmp_native_dialog_get_result_string(dialog, &result));
  ASSERT_EQ(NULL, result);

  /* Set result (simulating OS response) */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_native_dialog_set_result_string(dialog, "C:\\test.txt"));
  ASSERT_EQ(CMP_SUCCESS, cmp_native_dialog_get_result_string(dialog, &result));
  ASSERT_NEQ(NULL, result);
  ASSERT_STR_EQ("C:\\test.txt", result);
  CMP_FREE(result);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_native_dialog_show(NULL, CMP_DIALOG_TYPE_FILE_OPEN));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_native_dialog_get_result_string(NULL, &result));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_native_dialog_get_result_string(dialog, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_native_dialog_set_result_string(NULL, "test"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_native_dialog_set_result_string(dialog, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_native_dialog_destroy(dialog));
  PASS();
}

SUITE(cmp_native_dialog_suite) {
  RUN_TEST(test_cmp_native_dialog_create_destroy);
  RUN_TEST(test_cmp_native_dialog_show_and_result);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_native_dialog_suite);
  GREATEST_MAIN_END();
}
