/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_embedded_pty_lifecycle(void) {
  cmp_embedded_pty_t *pty = NULL;
  int res;

  res = cmp_embedded_pty_create(&pty);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, pty);

  res = cmp_embedded_pty_destroy(pty);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_embedded_pty_null_args(void) {
  cmp_embedded_pty_t *pty = NULL;
  char buf[32];
  size_t read_bytes;
  int res;

  res = cmp_embedded_pty_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_create(&pty);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_embedded_pty_spawn(NULL, "cmd");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_spawn(pty, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_write(NULL, "input", 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* PTY allows NULL input if length is 0, but fails if length > 0 */
  res = cmp_embedded_pty_write(pty, NULL, 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_read(NULL, buf, sizeof(buf), &read_bytes);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_read(pty, NULL, sizeof(buf), &read_bytes);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_read(pty, buf, 0, &read_bytes);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_read(pty, buf, sizeof(buf), NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_embedded_pty_destroy(pty);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_embedded_pty_operations(void) {
  cmp_embedded_pty_t *pty = NULL;
  char buf[128];
  size_t read_bytes;
  int res;

  res = cmp_embedded_pty_create(&pty);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Write before spawn should fail */
  res = cmp_embedded_pty_write(pty, "test", 4);
  ASSERT_EQ(CMP_ERROR_INVALID_STATE, res);

  /* Read before spawn should just return 0 bytes */
  res = cmp_embedded_pty_read(pty, buf, sizeof(buf), &read_bytes);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, (int)read_bytes);

  /* Spawn */
  res = cmp_embedded_pty_spawn(pty, "cmd.exe");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Initial read should give welcome string */
  res = cmp_embedded_pty_read(pty, buf, sizeof(buf), &read_bytes);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT(read_bytes > 0);
  buf[read_bytes] = '\0';
  ASSERT_STR_EQ("C:\\> ", buf);

  /* Write some input */
  res = cmp_embedded_pty_write(pty, "dir", 3);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Read back the echoed input */
  res = cmp_embedded_pty_read(pty, buf, sizeof(buf), &read_bytes);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(3, (int)read_bytes);
  buf[read_bytes] = '\0';
  ASSERT_STR_EQ("dir", buf);

  /* Second read should give 0 bytes available */
  res = cmp_embedded_pty_read(pty, buf, sizeof(buf), &read_bytes);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(0, (int)read_bytes);

  res = cmp_embedded_pty_destroy(pty);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(embedded_pty_suite) {
  RUN_TEST(test_embedded_pty_lifecycle);
  RUN_TEST(test_embedded_pty_null_args);
  RUN_TEST(test_embedded_pty_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(embedded_pty_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
