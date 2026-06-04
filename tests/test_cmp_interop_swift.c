/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_swift_interop_macros_compilation(void) {
  /* This test primarily exists to ensure the macro expansions do not break
   * C89/MSVC syntax */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-extension"
#endif

  CMP_ASSUME_NONNULL_BEGIN
  void *CMP_NONNULL test_ptr = (void *)1;
  int *CMP_NULLABLE opt_ptr = NULL;
  (void)test_ptr;
  (void)opt_ptr;
  CMP_ASSUME_NONNULL_END

#ifdef __clang__
#pragma clang diagnostic pop
#endif

  PASS();
}

TEST test_interop_cfstring_and_arc(void) {
  void *retained_obj = NULL;

  /* Mock an aligned pointer */
  void *aligned_ptr = (void *)(sizeof(void *) * 2);
  /* Mock an unaligned pointer */
  void *unaligned_ptr = (void *)((char *)aligned_ptr + 1);

  ASSERT_EQ(CMP_SUCCESS, cmp_interop_mock_init());
  ASSERT_EQ(CMP_SUCCESS, cmp_interop_cfstring_bridge(aligned_ptr));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_interop_cfstring_bridge(unaligned_ptr));

  ASSERT_EQ(CMP_SUCCESS, cmp_interop_allocate_retained_object(&retained_obj));
  ASSERT_NEQ(NULL, retained_obj);
  /* Validate reference count initialization */
  ASSERT_EQ(1, ((size_t *)retained_obj)[0]);

  ASSERT_EQ(CMP_SUCCESS, cmp_interop_release_object(retained_obj));

  PASS();
}

TEST test_interop_null_args(void) {
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_interop_cfstring_bridge(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_interop_allocate_retained_object(NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_interop_release_object(NULL));
  PASS();
}

SUITE(interop_swift_suite) {
  RUN_TEST(test_swift_interop_macros_compilation);
  RUN_TEST(test_interop_cfstring_and_arc);
  RUN_TEST(test_interop_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(interop_swift_suite);
  GREATEST_MAIN_END();
}
