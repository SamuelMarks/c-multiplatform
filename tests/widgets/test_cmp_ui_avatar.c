/* clang-format off */
#include "cmp_ui_avatar.h"
#include "cmp.h"
#include "cmp_log.h"
#include "greatest.h"
/* clang-format on */

TEST test_avatar_create_destroy(void) {
  cmp_ui_avatar_t *avatar = NULL;
  int rc;
  rc = cmp_ui_avatar_create(&avatar, "SM", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(avatar != NULL);

  rc = cmp_ui_avatar_destroy(avatar);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_avatar_create_null_args(void) {
  cmp_ui_avatar_t *avatar = NULL;
  int rc;

  rc = cmp_ui_avatar_create(NULL, "SM", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_avatar_create(&avatar, NULL, 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_avatar_destroy(avatar);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_avatar_destroy_null(void) {
  int rc;
  rc = cmp_ui_avatar_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);
  PASS();
}

TEST test_avatar_get_node(void) {
  cmp_ui_avatar_t *avatar = NULL;
  cmp_ui_node_t *node = NULL;
  int rc;

  rc = cmp_ui_avatar_create(&avatar, "SM", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_avatar_get_node(avatar, &node);
  ASSERT_EQ(CMP_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cmp_ui_avatar_get_node(NULL, &node);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_avatar_get_node(avatar, NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_avatar_destroy(avatar);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

TEST test_avatar_set_initials(void) {
  cmp_ui_avatar_t *avatar = NULL;
  int rc;

  rc = cmp_ui_avatar_create(&avatar, "SM", 0xFF0000FF, 0xFFFFFFFF);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_avatar_set_initials(avatar, "AB");
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_avatar_set_initials(avatar, NULL);
  ASSERT_EQ(CMP_SUCCESS, rc);

  rc = cmp_ui_avatar_set_initials(NULL, "Fail");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, rc);

  rc = cmp_ui_avatar_destroy(avatar);
  ASSERT_EQ(CMP_SUCCESS, rc);

  PASS();
}

SUITE(avatar_suite) {
  RUN_TEST(test_avatar_create_destroy);
  RUN_TEST(test_avatar_create_null_args);
  RUN_TEST(test_avatar_destroy_null);
  RUN_TEST(test_avatar_get_node);
  RUN_TEST(test_avatar_set_initials);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(avatar_suite);
  GREATEST_MAIN_END();
}