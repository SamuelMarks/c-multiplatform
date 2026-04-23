
/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST dummy(void) { PASS(); }
SUITE(cmp_mipmap_suite) { RUN_TEST(dummy); }
GREATEST_MAIN_DEFS();
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_mipmap_suite);
  GREATEST_MAIN_END();
}
