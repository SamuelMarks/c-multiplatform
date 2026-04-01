/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_auth_siwa(void) {
  cmp_siwa_ctx_t *siwa = NULL;
  char token[256];
  int valid;

  ASSERT_EQ(CMP_SUCCESS, cmp_siwa_create(&siwa));

  ASSERT_EQ(CMP_SUCCESS, cmp_siwa_request(siwa, 1, token, 256));
  ASSERT_STR_EQ("apple_jwt_fake_token", token);

  ASSERT_EQ(CMP_SUCCESS, cmp_siwa_destroy(siwa));

  /* Test branding validation */
  cmp_a11y_tree_t *tree = NULL;
  cmp_a11y_tree_create(&tree);
  cmp_a11y_tree_add_node(tree, 5, "button", "Sign In");

  ASSERT_EQ(CMP_SUCCESS, cmp_tree_validate_siwa_branding(tree, 5, &valid));
  ASSERT_EQ(1, valid);

  cmp_a11y_tree_destroy(tree);

  PASS();
}

TEST test_auth_biometric(void) {
  cmp_local_auth_t *auth = NULL;
  int success;

  ASSERT_EQ(CMP_SUCCESS, cmp_local_auth_create(&auth));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_local_auth_request(auth, "Unlock to see balances", &success));
  ASSERT_EQ(1, success);
  ASSERT_EQ(CMP_SUCCESS, cmp_local_auth_destroy(auth));

  PASS();
}

TEST test_auth_keychain(void) {
  cmp_keychain_t *keychain = NULL;
  char buf[128];

  ASSERT_EQ(CMP_SUCCESS, cmp_keychain_create(&keychain));

  ASSERT_EQ(CMP_SUCCESS,
            cmp_keychain_save(keychain, "user_token", "super_secret_payload"));

  ASSERT_EQ(CMP_SUCCESS, cmp_keychain_load(keychain, "user_token", buf, 128));
  ASSERT_STR_EQ("super_secret_payload", buf);

  ASSERT_EQ(CMP_ERROR_NOT_FOUND,
            cmp_keychain_load(keychain, "invalid_key", buf, 128));

  ASSERT_EQ(CMP_SUCCESS, cmp_keychain_destroy(keychain));

  PASS();
}

TEST test_auth_ui_security(void) {
  cmp_a11y_tree_t *tree = NULL;
  int is_sensitive;

  cmp_a11y_tree_create(&tree);
  cmp_a11y_tree_add_node(tree, 10, "textfield", "Code");

  ASSERT_EQ(CMP_SUCCESS, cmp_a11y_tree_set_node_text_content_type(
                             tree, 10, CMP_TEXT_CONTENT_TYPE_ONE_TIME_CODE));

  /* Simulated blurred bg */
  cmp_window_t *win = (cmp_window_t *)1; /* Fake window handle */

  ASSERT_EQ(CMP_SUCCESS, cmp_window_set_secure_background_obscure(win, 1));

  cmp_a11y_tree_destroy(tree);

  /* Sensitive content filtering */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_visuals_check_sensitive_content("data", 4, &is_sensitive));
  ASSERT_EQ(0, is_sensitive);

  PASS();
}

TEST test_null_args(void) {
  cmp_siwa_ctx_t *siwa = NULL;
  cmp_local_auth_t *auth = NULL;
  cmp_keychain_t *kc = NULL;

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_siwa_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_local_auth_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keychain_create(NULL));

  cmp_siwa_create(&siwa);
  cmp_local_auth_create(&auth);
  cmp_keychain_create(&kc);

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_siwa_request(NULL, 1, NULL, 0));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_tree_validate_siwa_branding(NULL, 0, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_local_auth_request(NULL, NULL, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keychain_save(NULL, NULL, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_keychain_load(NULL, NULL, NULL, 0));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_a11y_tree_set_node_text_content_type(
                                       NULL, 0, CMP_TEXT_CONTENT_TYPE_NONE));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_window_set_secure_background_obscure(NULL, 1));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_visuals_check_sensitive_content(NULL, 0, NULL));

  cmp_siwa_destroy(siwa);
  cmp_local_auth_destroy(auth);
  cmp_keychain_destroy(kc);

  PASS();
}

SUITE(auth_suite) {
  RUN_TEST(test_auth_siwa);
  RUN_TEST(test_auth_biometric);
  RUN_TEST(test_auth_keychain);
  RUN_TEST(test_auth_ui_security);
  RUN_TEST(test_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(auth_suite);
  GREATEST_MAIN_END();
}
