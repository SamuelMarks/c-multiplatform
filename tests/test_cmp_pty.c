/* clang-format off */
#include "cmp.h"
#include "cmp_pty.h"
#include "greatest.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

SUITE(cmp_pty_suite);

TEST test_pty_lifecycle(void) {
  cmp_pty_t *pty = NULL;

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_create(NULL, "cmd", 80, 24));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_create(&pty, NULL, 80, 24));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS, cmp_pty_create(&pty, "bash", 80, 24));
  ASSERT_NEQ(NULL, pty);

  /* Null argument for destroy */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_destroy(NULL));

  /* Successful destroy */
  ASSERT_EQ(CMP_SUCCESS, cmp_pty_destroy(pty));

  PASS();
}

TEST test_pty_resize(void) {
  cmp_pty_t *pty = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_pty_create(&pty, "bash", 80, 24));

  /* Null/invalid arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_resize(NULL, 100, 30));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_resize(pty, 0, 30));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_resize(pty, -1, 30));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_resize(pty, 100, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_resize(pty, 100, -1));

  /* Successful resize */
  ASSERT_EQ(CMP_SUCCESS, cmp_pty_resize(pty, 100, 30));

  ASSERT_EQ(CMP_SUCCESS, cmp_pty_destroy(pty));

  PASS();
}

TEST test_pty_read_write(void) {
  cmp_pty_t *pty = NULL;
  char read_buf[2048];
  unsigned int read_len = 0;
  const char *test_data = "Hello PTY";
  unsigned int test_len = (unsigned int)strlen(test_data);
  char huge_data[2048];
  unsigned int i;

  ASSERT_EQ(CMP_SUCCESS, cmp_pty_create(&pty, "bash", 80, 24));

  /* Null arguments */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_write(NULL, test_data, test_len));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_pty_write(pty, NULL, test_len));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pty_read(NULL, read_buf, sizeof(read_buf), &read_len));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pty_read(pty, NULL, sizeof(read_buf), &read_len));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_pty_read(pty, read_buf, sizeof(read_buf), NULL));
  /* Read initial mock data */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pty_read(pty, read_buf, sizeof(read_buf), &read_len));
  ASSERT_EQ(19, read_len); /* "Simulated PTY start" */
  read_buf[read_len] = '\0';
  ASSERT_STR_EQ("Simulated PTY start", read_buf);

  /* Read again when empty */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pty_read(pty, read_buf, sizeof(read_buf), &read_len));
  ASSERT_EQ(0, read_len);

  /* Write data */
  ASSERT_EQ(CMP_SUCCESS, cmp_pty_write(pty, test_data, test_len));

  /* Read back data partially */
  ASSERT_EQ(CMP_SUCCESS, cmp_pty_read(pty, read_buf, 5, &read_len));
  ASSERT_EQ(5, read_len);
  read_buf[read_len] = '\0';
  ASSERT_STR_EQ("Hello", read_buf);

  /* Read remaining */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pty_read(pty, read_buf, sizeof(read_buf), &read_len));
  ASSERT_EQ(4, read_len);
  read_buf[read_len] = '\0';
  ASSERT_STR_EQ(" PTY", read_buf);

  /* Test truncation of large writes (mock behavior clamps at 1023) */
  for (i = 0; i < sizeof(huge_data) - 1; i++) {
    huge_data[i] = 'A';
  }
  huge_data[sizeof(huge_data) - 1] = '\0';

  ASSERT_EQ(CMP_SUCCESS, cmp_pty_write(pty, huge_data, sizeof(huge_data)));

  /* Read it back */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_pty_read(pty, read_buf, sizeof(read_buf), &read_len));
  ASSERT_EQ(1023, read_len);

  ASSERT_EQ(CMP_SUCCESS, cmp_pty_destroy(pty));

  PASS();
}

SUITE(cmp_pty_suite) {
  RUN_TEST(test_pty_lifecycle);
  RUN_TEST(test_pty_resize);
  RUN_TEST(test_pty_read_write);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_pty_suite);
  GREATEST_MAIN_END();
}
