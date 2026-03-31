/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

SUITE(cmp_clipboard_suite);

TEST test_cmp_clipboard_create_destroy(void) {
  cmp_clipboard_t *clipboard = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_create(&clipboard));
  ASSERT_NEQ(NULL, clipboard);

  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_destroy(clipboard));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_destroy(NULL));

  PASS();
}

TEST test_cmp_clipboard_set_get_clear(void) {
  cmp_clipboard_t *clipboard = NULL;
  char *text = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_create(&clipboard));

  /* Initial get should be NULL */
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_get_text(clipboard, &text));
  ASSERT_EQ(NULL, text);

  /* Set text */
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_set_text(clipboard, "Hello World"));

  /* Get text */
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_get_text(clipboard, &text));
  ASSERT_NEQ(NULL, text);
  ASSERT_STR_EQ("Hello World", text);
  CMP_FREE(text);

  /* Overwrite text */
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_set_text(clipboard, "New Text"));
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_get_text(clipboard, &text));
  ASSERT_NEQ(NULL, text);
  ASSERT_STR_EQ("New Text", text);
  CMP_FREE(text);

  /* Clear text */
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_clear(clipboard));
  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_get_text(clipboard, &text));
  ASSERT_EQ(NULL, text);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_set_text(NULL, "Test"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_set_text(clipboard, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_get_text(NULL, &text));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_get_text(clipboard, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_clipboard_clear(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_clipboard_destroy(clipboard));
  PASS();
}

SUITE(cmp_clipboard_suite) {
  RUN_TEST(test_cmp_clipboard_create_destroy);
  RUN_TEST(test_cmp_clipboard_set_get_clear);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_clipboard_suite);
  GREATEST_MAIN_END();
}
