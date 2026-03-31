/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_system_menu_suite);

TEST test_cmp_system_menu_create_destroy(void) {
  cmp_system_menu_t *menu = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_create(&menu));
  ASSERT_NEQ(NULL, menu);

  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_destroy(menu));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_menu_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_menu_destroy(NULL));

  PASS();
}

TEST test_cmp_system_menu_add_and_show(void) {
  cmp_system_menu_t *menu = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_create(&menu));

  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_add_item(menu, "File", 1));
  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_add_item(menu, "Edit", 2));
  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_add_item(menu, "View", 3));
  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_add_item(menu, "Help", 4));
  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_add_item(menu, "About", 5));

  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_show(menu));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_menu_add_item(NULL, "Test", 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_menu_add_item(menu, NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_system_menu_show(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_system_menu_destroy(menu));
  PASS();
}

SUITE(cmp_system_menu_suite) {
  RUN_TEST(test_cmp_system_menu_create_destroy);
  RUN_TEST(test_cmp_system_menu_add_and_show);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_system_menu_suite);
  GREATEST_MAIN_END();
}
