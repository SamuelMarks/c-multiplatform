/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_modality_async_stubs(void) {
  cmp_modality_t mod;
  int res;

  res = cmp_modality_async_init(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_modality_eventloop_init(&mod);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

SUITE(modality_async_suite) { RUN_TEST(test_modality_async_stubs); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(modality_async_suite);
  GREATEST_MAIN_END();
}
