/* clang-format off */
#include "cmp.h"
#include "cmp_ui_avatar.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_ui_avatar_suite);

TEST test_ui_avatar_lifecycle(void) {
  cmp_ui_avatar_t *avatar = NULL;
  cmp_ui_node_t *node = NULL;

  /* Null arg checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_ui_avatar_create(NULL, "SM", 0xFF000000, 0xFFFFFFFF));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_avatar_destroy(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_avatar_get_node(NULL, &node));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_avatar_set_initials(NULL, "AB"));

  /* Successful creation */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_avatar_create(&avatar, "SM", 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, avatar);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_ui_avatar_get_node(avatar, NULL));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_avatar_get_node(avatar, &node));
  ASSERT_NEQ(NULL, node);

  /* Set initials */
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_avatar_set_initials(avatar, "JD"));
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_avatar_set_initials(avatar, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_ui_avatar_destroy(avatar));

  PASS();
}

TEST test_ui_avatar_null_initials_init(void) {
  cmp_ui_avatar_t *avatar = NULL;

  ASSERT_EQ(CMP_SUCCESS,
            cmp_ui_avatar_create(&avatar, NULL, 0xFF000000, 0xFFFFFFFF));
  ASSERT_NEQ(NULL, avatar);
  ASSERT_EQ(CMP_SUCCESS, cmp_ui_avatar_destroy(avatar));

  PASS();
}

SUITE(cmp_ui_avatar_suite) {
  RUN_TEST(test_ui_avatar_lifecycle);
  RUN_TEST(test_ui_avatar_null_initials_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_ui_avatar_suite);
  GREATEST_MAIN_END();
}
