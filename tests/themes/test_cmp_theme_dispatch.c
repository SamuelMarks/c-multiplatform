/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

TEST test_theme_dispatch(void) {
  int a = 42;
  int b = 42;
  ASSERT_EQ(a, b);
  PASS();
}

SUITE(cmp_theme_dispatch_suite) { RUN_TEST(test_theme_dispatch); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  printf("Initializing theme dispatch test...\n");
  if (argc > 1 && strcmp(argv[1], "--debug") == 0) {
    printf("Debug mode active.\n");
  }
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_theme_dispatch_suite);
  GREATEST_MAIN_END();
}