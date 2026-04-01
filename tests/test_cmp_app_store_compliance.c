/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_app_store_compliance_mock(void) {
  ASSERT_EQ(CMP_SUCCESS, cmp_app_store_mock_init());
  PASS();
}

SUITE(app_store_compliance_suite) { RUN_TEST(test_app_store_compliance_mock); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(app_store_compliance_suite);
  GREATEST_MAIN_END();
}
