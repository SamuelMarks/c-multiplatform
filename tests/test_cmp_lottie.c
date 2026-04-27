/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include "cmp_lottie.h"
#include <stdlib.h>
/* clang-format on */

TEST test_lottie_lifecycle(void) {
  cmp_lottie_t *lottie = NULL;
  const char *mock_json = "{}";

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_lottie_create(NULL, 2, &lottie));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_lottie_create(mock_json, 0, &lottie));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_lottie_create(mock_json, 2, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_lottie_create(mock_json, 2, &lottie));
  ASSERT_NEQ(NULL, lottie);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_lottie_step(NULL, 16.0f));
  ASSERT_EQ(CMP_SUCCESS, cmp_lottie_step(lottie, 16.0f));
  ASSERT_EQ(
      CMP_SUCCESS,
      cmp_lottie_step(lottie, 2000.0f)); /* triggers wrap around loop branch */

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_lottie_destroy(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_lottie_destroy(lottie));
  PASS();
}

SUITE(suite_lottie) { RUN_TEST(test_lottie_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_lottie);
  GREATEST_MAIN_END();
}
