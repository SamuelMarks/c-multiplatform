/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <stdio.h>
/* clang-format on */

SUITE(cmp_vfs_theme_suite);

TEST test_vfs_theme_save_load(void) {
  FILE *f;
  const char *valid_theme = "PRIMARY=FF0000\nSECONDARY=00FF00\n";
  const char *corrupted_theme = "PRIMARYFF0000\nNO_EQUALS_HERE\n";
  void *buffer = NULL;
  size_t size = 0;

  cmp_vfs_init();

  /* Test valid theme dat */
  f = fopen("theme_valid.dat", "wb");
  if (f) {
    fwrite(valid_theme, 1, strlen(valid_theme), f);
    fclose(f);
  }

  ASSERT_EQ(CMP_SUCCESS,
            cmp_vfs_read_file_sync("theme_valid.dat", &buffer, &size));
  ASSERT_NEQ(NULL, buffer);
  ASSERT_EQ(strlen(valid_theme), size);
  CMP_FREE(buffer);

  /* Test corrupted theme dat */
  f = fopen("theme_corrupt.dat", "wb");
  if (f) {
    fwrite(corrupted_theme, 1, strlen(corrupted_theme), f);
    fclose(f);
  }

  ASSERT_EQ(CMP_SUCCESS,
            cmp_vfs_read_file_sync("theme_corrupt.dat", &buffer, &size));
  ASSERT_NEQ(NULL, buffer);
  ASSERT_EQ(strlen(corrupted_theme), size);
  CMP_FREE(buffer);

  PASS();
}

SUITE(cmp_vfs_theme_suite) { RUN_TEST(test_vfs_theme_save_load); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_vfs_theme_suite);
  GREATEST_MAIN_END();
}
