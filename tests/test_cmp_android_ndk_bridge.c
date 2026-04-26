/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_android_ndk_bridge_create_destroy(void) {
  cmp_android_ndk_bridge_t *br = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_android_ndk_bridge_create(&br));
  ASSERT_NEQ(NULL, br);

  ASSERT_EQ(CMP_SUCCESS, cmp_android_ndk_bridge_destroy(br));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_ndk_bridge_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_ndk_bridge_destroy(NULL));

  PASS();
}

TEST test_android_ndk_bridge_tick(void) {
  cmp_android_ndk_bridge_t *br = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_android_ndk_bridge_create(&br));

  ASSERT_EQ(CMP_SUCCESS, cmp_android_ndk_bridge_tick(br, 0.016f));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_android_ndk_bridge_tick(NULL, 0.016f));

  ASSERT_EQ(CMP_SUCCESS, cmp_android_ndk_bridge_destroy(br));
  PASS();
}

SUITE(android_ndk_bridge_suite) {
  RUN_TEST(test_android_ndk_bridge_create_destroy);
  RUN_TEST(test_android_ndk_bridge_tick);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(android_ndk_bridge_suite);
  GREATEST_MAIN_END();
}
