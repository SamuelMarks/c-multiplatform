/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_material3_dummy(void) { PASS(); }

SUITE(cmp_material3_suite) { RUN_TEST(test_material3_dummy); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_material3_suite);
  GREATEST_MAIN_END();
}