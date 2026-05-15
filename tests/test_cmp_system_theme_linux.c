/* clang-format off */
#include "cmp.h"
#include "cmp_system_theme.h"
#include "greatest.h"
/* clang-format on */

TEST test_system_theme_linux_stubs(void) { PASS(); }

SUITE(system_theme_linux_suite) { RUN_TEST(test_system_theme_linux_stubs); }
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(system_theme_linux_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
