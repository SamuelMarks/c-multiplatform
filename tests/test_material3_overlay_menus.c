#include "greatest.h"
TEST test_dummy(void) { PASS(); }
SUITE(md3_overlay_menus_suite) { RUN_TEST(test_dummy); }
GREATEST_MAIN_DEFS();
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(md3_overlay_menus_suite);
  GREATEST_MAIN_END();
}
