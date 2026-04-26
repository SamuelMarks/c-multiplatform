/* clang-format off */
#include "cmp_mmap.h"
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_mmap_lifecycle(void) {
  cmp_mmap_t *mmap = NULL;
  void *data = NULL;
  size_t size = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_mmap_open(&mmap, "dummy.txt"));
  ASSERT_NEQ(NULL, mmap);

  ASSERT_EQ(CMP_SUCCESS, cmp_mmap_get_data(mmap, &data, &size));
  ASSERT_NEQ(NULL, data);
  ASSERT_EQ(1024, size);

  ASSERT_EQ(CMP_SUCCESS, cmp_mmap_close(mmap));
  PASS();
}

TEST test_mmap_null_args(void) {
  cmp_mmap_t *mmap = NULL;
  void *data = NULL;
  size_t size = 0;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mmap_open(NULL, "dummy.txt"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mmap_open(&mmap, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mmap_close(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_mmap_open(&mmap, "dummy.txt"));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mmap_get_data(NULL, &data, &size));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mmap_get_data(mmap, NULL, &size));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_mmap_get_data(mmap, &data, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_mmap_close(mmap));
  PASS();
}

SUITE(mmap_suite) {
  RUN_TEST(test_mmap_lifecycle);
  RUN_TEST(test_mmap_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(mmap_suite);
  GREATEST_MAIN_END();
}
