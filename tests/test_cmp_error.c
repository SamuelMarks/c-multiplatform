/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_crash_handler_init(void) {
  int res = cmp_crash_handler_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  PASS();
}

SUITE(error_suite) { RUN_TEST(test_crash_handler_init); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  int *p = NULL;

  if (argc > 1 && strcmp(argv[1], "--crash") == 0) {
    cmp_crash_handler_init();
    *p = 1; /* Trigger SIGSEGV */
    return 0;
  }
  if (argc > 1 && strcmp(argv[1], "--assert") == 0) {
    CMP_ASSERT(0 == 1); /* Trigger assert fail */
    return 0;
  }

  GREATEST_MAIN_BEGIN();
  RUN_SUITE(error_suite);
  GREATEST_MAIN_END();
}
