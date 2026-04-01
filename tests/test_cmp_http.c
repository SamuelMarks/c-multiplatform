/* clang-format off */
#include "cmp.h"
#include "greatest.h"

/* clang-format on */

TEST test_http_lifecycle(void) {
  int res;

  res = cmp_http_init();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_http_shutdown();
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  PASS();
}

TEST test_http_client_creation(void) {
  struct HttpClient *client = NULL;
  cmp_modality_t mod;
  int res;

  cmp_http_init();
  cmp_modality_single_init(&mod);

  res = cmp_http_client_create(&mod, &client);
#if defined(_WIN32) && defined(_MSC_VER) && _MSC_VER < 1600
  if (res == CMP_ERROR_NOT_FOUND) {
    cmp_modality_destroy(&mod);
    PASS();
  }
#endif
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");
  ASSERT(client != NULL);
  ASSERT_EQ_FMT((int)MODALITY_SYNC, (int)client->config.modality, "%d");

  res = cmp_http_client_destroy(client);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  cmp_modality_destroy(&mod);
  cmp_http_shutdown();
  PASS();
}

TEST test_ws_init(void) {
  struct HttpRequest req;
  int res;

  cmp_http_init();
  res = cmp_http_request_init(&req);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_http_ws_init(&req, NULL);
  /* Should succeed natively if it uses the backend */
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  cmp_http_request_free(&req);
  cmp_http_shutdown();
  PASS();
}

TEST test_sse_init(void) {
  struct HttpRequest req;
  int res;

  cmp_http_init();
  res = cmp_http_request_init(&req);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  res = cmp_http_sse_init(&req, NULL);
  ASSERT_EQ_FMT(CMP_SUCCESS, res, "%d");

  cmp_http_request_free(&req);
  cmp_http_shutdown();
  PASS();
}

SUITE(http_suite) {
  RUN_TEST(test_http_lifecycle);
  RUN_TEST(test_http_client_creation);
  RUN_TEST(test_ws_init);
  RUN_TEST(test_sse_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(http_suite);
  GREATEST_MAIN_END();
}
