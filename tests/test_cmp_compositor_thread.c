/* clang-format off */
#include "greatest.h"
#include "cmp.h"
#include "cmp_compositor_thread.h"
#include <stdlib.h>
/* clang-format on */

TEST test_compositor_thread_lifecycle(void) {
  cmp_compositor_thread_t *thread = NULL;
  void *dummy_tree = (void *)(size_t)0xDEADBEEF;
  ASSERT_EQ(CMP_SUCCESS, cmp_compositor_thread_create(&thread));
  ASSERT_NEQ(NULL, thread);

  ASSERT_EQ(CMP_SUCCESS, cmp_compositor_thread_push_tree(thread, dummy_tree));

  ASSERT_EQ(CMP_SUCCESS, cmp_compositor_thread_destroy(thread));
  PASS();
}

SUITE(suite_compositor_thread) { RUN_TEST(test_compositor_thread_lifecycle); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(suite_compositor_thread);
  GREATEST_MAIN_END();
}
