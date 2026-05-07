/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include <stdlib.h>
/* clang-format on */

TEST test_icc_profile_parse(void) {
  cmp_icc_profile_t *prof = NULL;
  unsigned char mock_jpg[] = {0xFF, 0xD8, 0xFF, 0xE2, 0x00, 0x00, 'I', 'C', 'C',
                              '_',  'P',  'R',  'O',  'F',  'I',  'L', 'E'};
  int is_wide = 0;
  float mat[9];

  ASSERT_EQ(CMP_SUCCESS,
            cmp_icc_profile_parse(mock_jpg, sizeof(mock_jpg), (void **)&prof));
  ASSERT_NEQ(NULL, prof);

  ASSERT_EQ(CMP_SUCCESS, cmp_icc_profile_is_wide_gamut(prof, &is_wide));
  ASSERT_EQ(1, is_wide);

  ASSERT_EQ(CMP_SUCCESS, cmp_icc_profile_get_matrix(prof, mat));
  ASSERT_EQ(1.2249f, mat[0]);

  ASSERT_EQ(CMP_SUCCESS, cmp_icc_profile_destroy(prof));
  PASS();
}

SUITE(suite_icc_profile) { RUN_TEST(test_icc_profile_parse); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_icc_profile);
  GREATEST_MAIN_END();
}
