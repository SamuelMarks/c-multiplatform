/* clang-format off */
#include "cmp.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_tab_nav_lifecycle(void) {
  cmp_tab_nav_t *nav = NULL;
  int res;

  res = cmp_tab_nav_create(&nav);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, nav);

  res = cmp_tab_nav_destroy(nav);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_tab_nav_null_args(void) {
  cmp_tab_nav_t *nav = NULL;
  cmp_editor_tab_t *tab = NULL;
  size_t count;
  int res;

  res = cmp_tab_nav_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_create(&nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tab_nav_add_tab(NULL, "title", "path");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_add_tab(nav, NULL, "path");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_add_tab(nav, "title", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_close_tab(NULL, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_move_tab(NULL, 0, 1);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_get_count(NULL, &count);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_get_count(nav, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_get_tab(NULL, 0, &tab);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_get_tab(nav, 0, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_destroy(nav);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

TEST test_tab_nav_operations(void) {
  cmp_tab_nav_t *nav = NULL;
  cmp_editor_tab_t *tab = NULL;
  size_t count;
  int res;

  res = cmp_tab_nav_create(&nav);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Add tabs */
  res = cmp_tab_nav_add_tab(nav, "main.c", "/src/main.c");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tab_nav_add_tab(nav, "utils.h", "/include/utils.h");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tab_nav_get_count(nav, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(2, (int)count);

  /* Second tab should be active */
  res = cmp_tab_nav_get_tab(nav, 1, &tab);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("utils.h", tab->title);
  ASSERT_EQ(1, tab->is_active);

  /* First tab should be inactive */
  res = cmp_tab_nav_get_tab(nav, 0, &tab);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("main.c", tab->title);
  ASSERT_EQ(0, tab->is_active);

  /* Move tab */
  res = cmp_tab_nav_move_tab(nav, 1, 0);
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Now first tab is utils.h and active */
  res = cmp_tab_nav_get_tab(nav, 0, &tab);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("utils.h", tab->title);
  ASSERT_EQ(1, tab->is_active);

  /* Out of bounds operations */
  res = cmp_tab_nav_get_tab(nav, 5, &tab);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_close_tab(nav, 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_move_tab(nav, 0, 5);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_tab_nav_move_tab(nav, 5, 0);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  /* Close tab */
  res = cmp_tab_nav_close_tab(nav, 0); /* closes utils.h */
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_tab_nav_get_count(nav, &count);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_EQ(1, (int)count);

  /* main.c should now be active */
  res = cmp_tab_nav_get_tab(nav, 0, &tab);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_STR_EQ("main.c", tab->title);
  ASSERT_EQ(1, tab->is_active);

  res = cmp_tab_nav_destroy(nav);
  ASSERT_EQ(CMP_SUCCESS, res);
  PASS();
}

SUITE(tab_navigation_suite) {
  RUN_TEST(test_tab_nav_lifecycle);
  RUN_TEST(test_tab_nav_null_args);
  RUN_TEST(test_tab_nav_operations);
}
#ifdef __cplusplus
extern "C" {
#endif
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(tab_navigation_suite);
  GREATEST_MAIN_END();
}
#ifdef __cplusplus
}
#endif
