/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_tls_lifecycle(void) {
  cmp_tls_key_t key;
  void *val = NULL;
  int dummy = 42;
  int res;

  res = cmp_tls_key_create(&key);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_tls_get(key, &val);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(val == NULL);

  res = cmp_tls_set(key, &dummy);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_tls_get(key, &val);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(val == &dummy);

  res = cmp_tls_key_delete(key);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

SUITE(tls_suite) { RUN_TEST(test_tls_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(tls_suite);
  GREATEST_MAIN_END();
}
