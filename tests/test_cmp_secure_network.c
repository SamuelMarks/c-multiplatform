/* clang-format off */
#include "cmp.h"
#include "greatest.h"
/* clang-format on */

TEST test_secure_network_create_destroy(void) {
  cmp_secure_network_t *net = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_create(&net));
  ASSERT_NEQ(NULL, net);

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_destroy(net));
  PASS();
}

TEST test_secure_network_set_proxy(void) {
  cmp_secure_network_t *net = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_create(&net));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_secure_network_set_proxy(net, "http://localhost:8080"));

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_destroy(net));
  PASS();
}

TEST test_secure_network_send_https(void) {
  cmp_secure_network_t *net = NULL;
  int status_code = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_create(&net));

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_send_https(
                             net, "https://example.com", &status_code));
  ASSERT_EQ(200, status_code);

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_destroy(net));
  PASS();
}

TEST test_secure_network_retrieve_credential(void) {
  char secret[256];
  int rc;

  /* Since this uses CredReadA, it will likely return NOT_FOUND in CI/tests
     unless a specific test credential exists. We assert NOT_FOUND or SUCCESS
     just to ensure it doesn't crash. */
  rc = cmp_secure_network_retrieve_credential("NonExistentKey", secret,
                                              sizeof(secret));
  ASSERT(rc == CMP_ERROR_NOT_FOUND || rc == CMP_SUCCESS);

  PASS();
}

TEST test_secure_network_null_args(void) {
  cmp_secure_network_t *net = NULL;
  int status;
  char secret[256];

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_secure_network_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_secure_network_destroy(NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_create(&net));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_secure_network_set_proxy(NULL, "proxy"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_secure_network_set_proxy(net, NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_secure_network_send_https(NULL, "url", &status));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_secure_network_send_https(net, NULL, &status));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_secure_network_send_https(net, "url", NULL));

  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_secure_network_retrieve_credential(NULL, secret, 256));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_secure_network_retrieve_credential("key", NULL, 256));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_secure_network_retrieve_credential("key", secret, 0));

  ASSERT_EQ(CMP_SUCCESS, cmp_secure_network_destroy(net));
  PASS();
}

SUITE(cmp_secure_network_suite) {
  RUN_TEST(test_secure_network_create_destroy);
  RUN_TEST(test_secure_network_set_proxy);
  RUN_TEST(test_secure_network_send_https);
  RUN_TEST(test_secure_network_retrieve_credential);
  RUN_TEST(test_secure_network_null_args);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_secure_network_suite);
  GREATEST_MAIN_END();
}
