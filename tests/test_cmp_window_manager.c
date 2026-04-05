/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_cmp_window_manager_create_destroy(void) {
  cmp_window_manager_t *manager = NULL;
  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_create(&manager));
  ASSERT(manager != NULL);
  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_destroy(manager));
  PASS();
}

TEST test_cmp_window_manager_add_remove(void) {
  cmp_window_manager_t *manager = NULL;
  unsigned int count = 0;
  cmp_window_t *w1 = (cmp_window_t *)0x1234;
  cmp_window_t *w2 = (cmp_window_t *)0x5678;
  cmp_window_t *out_w = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_create(&manager));

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_get_window_count(manager, &count));
  ASSERT_EQ(0, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_add_window(manager, w1));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_add_window(manager, w2));

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_get_window_count(manager, &count));
  ASSERT_EQ(2, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_get_window_at(manager, 0, &out_w));
  ASSERT_EQ(w1, out_w);

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_get_window_at(manager, 1, &out_w));
  ASSERT_EQ(w2, out_w);

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_remove_window(manager, w1));

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_get_window_count(manager, &count));
  ASSERT_EQ(1, count);

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_get_window_at(manager, 0, &out_w));
  ASSERT_EQ(w2, out_w);

  /* we do not destroy the manager normally here because it would try to destroy
   * w1 and w2 which are fake pointers. Wait, our mock does not actually destroy
   * window properly unless it is a valid window! Let us use mock windows or
   * just remove before destroy. */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_remove_window(manager, w2));

  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_destroy(manager));
  PASS();
}

TEST test_cmp_window_manager_should_close(void) {
  cmp_window_manager_t *manager = NULL;
  /* We cannot really test should_close on fake pointers because it calls
   * cmp_window_should_close which will crash on fake pointers. Let us just test
   * an empty manager. */
  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_create(&manager));
  ASSERT_EQ(1, cmp_window_manager_should_close(manager));
  ASSERT_EQ(CMP_SUCCESS, cmp_window_manager_destroy(manager));
  PASS();
}

SUITE(cmp_window_manager_suite) {
  RUN_TEST(test_cmp_window_manager_create_destroy);
  RUN_TEST(test_cmp_window_manager_add_remove);
  RUN_TEST(test_cmp_window_manager_should_close);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_window_manager_suite);
  GREATEST_MAIN_END();
}
