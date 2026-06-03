/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include "cmp_fluent.h"
#include <stdlib.h>
/* clang-format on */

TEST test_fluent_reveal_lifecycle(void) {
  cmp_fluent_reveal_t *reveal = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_fluent_reveal_create(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_fluent_reveal_create(&reveal));
  ASSERT_NEQ(NULL, reveal);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_fluent_reveal_update_pointer(NULL, 100.0f, 200.0f));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_fluent_reveal_update_pointer(reveal, 100.0f, 200.0f));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_fluent_reveal_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_fluent_reveal_destroy(reveal));
  PASS();
}

TEST test_acrylic_noise_lifecycle(void) {
  cmp_acrylic_noise_t *noise = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_acrylic_noise_create(0, 256, &noise));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_acrylic_noise_create(256, 0, &noise));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_acrylic_noise_create(256, 256, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_acrylic_noise_create(256, 256, &noise));
  ASSERT_NEQ(NULL, noise);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_acrylic_noise_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_acrylic_noise_destroy(noise));
  PASS();
}

SUITE(suite_fluent) {
  RUN_TEST(test_fluent_reveal_lifecycle);
  RUN_TEST(test_acrylic_noise_lifecycle);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_fluent);
  GREATEST_MAIN_END();
}
