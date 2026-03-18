/* clang-format off */
#include "greatest.h"
/* clang-format on */

GREATEST_MAIN_DEFS();

extern SUITE(oauth2_gui_suite);

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(oauth2_gui_suite);
  GREATEST_MAIN_END();
}
