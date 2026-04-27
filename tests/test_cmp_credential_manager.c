/* clang-format off */
#include "cmp.h"
#include "cmp_credential_manager.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_credential_manager_lifecycle(void) {
  cmp_credential_manager_t *manager = NULL;
  char *secret = NULL;
  int res;

  res = cmp_credential_manager_create(&manager);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, manager);

  res = cmp_credential_manager_set_secret(manager, "test_svc", "test_acc",
                                          "my_secret");
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_credential_manager_get_secret(manager, "test_svc", "test_acc",
                                          &secret);
  ASSERT_EQ(CMP_SUCCESS, res);
  ASSERT_NEQ(NULL, secret);
  ASSERT_STR_EQ("my_secret", secret);

  res = cmp_credential_manager_free_secret(secret);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_credential_manager_delete_secret(manager, "test_svc", "test_acc");
  ASSERT_EQ(CMP_SUCCESS, res);

  /* Get after delete should fail */
  res = cmp_credential_manager_get_secret(manager, "test_svc", "test_acc",
                                          &secret);
  ASSERT_EQ(CMP_ERROR_NOT_FOUND, res);

  res = cmp_credential_manager_destroy(manager);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

TEST test_credential_manager_null_args(void) {
  cmp_credential_manager_t *manager = NULL;
  char *secret = NULL;
  int res;

  res = cmp_credential_manager_create(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_destroy(NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_create(&manager);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_credential_manager_set_secret(NULL, "svc", "acc", "sec");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_get_secret(NULL, "svc", "acc", &secret);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_get_secret(manager, NULL, "acc", &secret);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_get_secret(manager, "svc", NULL, &secret);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_get_secret(manager, "svc", "acc", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_delete_secret(NULL, "svc", "acc");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_delete_secret(manager, NULL, "acc");
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_delete_secret(manager, "svc", NULL);
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, res);

  res = cmp_credential_manager_free_secret(NULL);
  ASSERT_EQ(CMP_SUCCESS, res);

  res = cmp_credential_manager_destroy(manager);
  ASSERT_EQ(CMP_SUCCESS, res);

  PASS();
}

SUITE(credential_manager_suite) {
  RUN_TEST(test_credential_manager_lifecycle);
  RUN_TEST(test_credential_manager_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(credential_manager_suite);
  GREATEST_MAIN_END();
}
