/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_doc_mock(void) {
  ASSERT_EQ(CMP_SUCCESS, cmp_documentation_mock_init());
  PASS();
}

SUITE(documentation_dx_suite) { RUN_TEST(test_doc_mock); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(documentation_dx_suite);
  GREATEST_MAIN_END();
}
