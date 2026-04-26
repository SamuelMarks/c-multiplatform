/* clang-format off */
#include "greatest.h"
#include "cmp.h"
/* clang-format on */

TEST test_hardware_accel(void) {
  int fallback = 0;
  int rc;
  rc = cmp_verify_hardware_acceleration(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  rc = cmp_verify_hardware_acceleration(&fallback);
  if (rc != CMP_SUCCESS && rc != CMP_ERROR_NOT_FOUND) {
    FAIL();
  }
  PASS();
}

SUITE(hardware_accel_suite) { RUN_TEST(test_hardware_accel); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(hardware_accel_suite);
  GREATEST_MAIN_END();
}
