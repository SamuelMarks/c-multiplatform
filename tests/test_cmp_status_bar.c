/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_status_bar_create_destroy(void) {
  cmp_status_bar_t *bar = NULL;
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_create(&bar));
  ASSERT_NEQ(NULL, bar);
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_destroy(bar));
  PASS();
}

TEST test_status_bar_update_metrics(void) {
  cmp_status_bar_t *bar = NULL;
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_create(&bar));
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_set_backend_status(bar, "Running"));
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_update_token_usage(bar, 100, 200));
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_update_memory_metrics(bar, 1024.5f, 512.0f));
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_destroy(bar));
  PASS();
}

TEST test_status_bar_null_args(void) {
  cmp_status_bar_t *bar = NULL;
  
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_status_bar_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_status_bar_destroy(NULL));
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_create(&bar));
  
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_status_bar_set_backend_status(NULL, "Status"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_status_bar_set_backend_status(bar, NULL));
  
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_status_bar_update_token_usage(NULL, 10, 10));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_status_bar_update_memory_metrics(NULL, 1.0f, 1.0f));
  
  ASSERT_EQ(CMP_SUCCESS, cmp_status_bar_destroy(bar));
  PASS();
}

SUITE(cmp_status_bar_suite) {
  RUN_TEST(test_status_bar_create_destroy);
  RUN_TEST(test_status_bar_update_metrics);
  RUN_TEST(test_status_bar_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_status_bar_suite);
  GREATEST_MAIN_END();
}
