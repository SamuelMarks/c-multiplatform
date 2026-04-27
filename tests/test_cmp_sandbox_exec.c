/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <string.h>
/* clang-format on */

SUITE(cmp_sandbox_exec_suite);

TEST test_cmp_sandbox_exec_create_destroy(void) {
  cmp_sandbox_exec_t *sandbox = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_sandbox_exec_create(&sandbox));
  ASSERT_NEQ(NULL, sandbox);

  ASSERT_EQ(CMP_SUCCESS, cmp_sandbox_exec_destroy(sandbox));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_sandbox_exec_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_sandbox_exec_destroy(NULL));

  PASS();
}

TEST test_cmp_sandbox_exec_run(void) {
  cmp_sandbox_exec_t *sandbox = NULL;
  char *output = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_sandbox_exec_create(&sandbox));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_sandbox_exec_run(sandbox, "python", "print('hello')", &output));
  ASSERT_NEQ(NULL, output);
  ASSERT_STR_EQ("Hello world (mocked output)\n", output);

  ASSERT_EQ(CMP_SUCCESS, cmp_sandbox_exec_free_output(output));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sandbox_exec_run(NULL, "python", "code", &output));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sandbox_exec_run(sandbox, NULL, "code", &output));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sandbox_exec_run(sandbox, "python", NULL, &output));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_sandbox_exec_run(sandbox, "python", "code", NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_sandbox_exec_free_output(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_sandbox_exec_destroy(sandbox));

  PASS();
}

SUITE(cmp_sandbox_exec_suite) {
  RUN_TEST(test_cmp_sandbox_exec_create_destroy);
  RUN_TEST(test_cmp_sandbox_exec_run);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_sandbox_exec_suite);
  GREATEST_MAIN_END();
}
