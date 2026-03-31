/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_csp_suite);

TEST test_cmp_csp_create_destroy(void) {
  cmp_csp_t *csp = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_csp_create(&csp));
  ASSERT_NEQ(NULL, csp);

  ASSERT_EQ(CMP_SUCCESS, cmp_csp_destroy(csp));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_csp_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_csp_destroy(NULL));

  PASS();
}

TEST test_cmp_csp_domains(void) {
  cmp_csp_t *csp = NULL;
  int is_allowed = 0;

  ASSERT_EQ(CMP_SUCCESS, cmp_csp_create(&csp));

  /* Initially empty */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_csp_check_domain(csp, "https://example.com",
                                 CMP_CSP_RESOURCE_IMAGE, &is_allowed));
  ASSERT_EQ(0, is_allowed);

  /* Add domain */
  ASSERT_EQ(CMP_SUCCESS, cmp_csp_add_domain(csp, "https://example.com"));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_csp_check_domain(csp, "https://example.com",
                                 CMP_CSP_RESOURCE_IMAGE, &is_allowed));
  ASSERT_EQ(1, is_allowed);

  /* Unmatched domain */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_csp_check_domain(csp, "https://malicious.com",
                                 CMP_CSP_RESOURCE_SCRIPT, &is_allowed));
  ASSERT_EQ(0, is_allowed);

  /* Add multiple domains */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_csp_add_domain(csp, "https://fonts.googleapis.com"));
  ASSERT_EQ(CMP_SUCCESS,
            cmp_csp_check_domain(csp, "https://fonts.googleapis.com",
                                 CMP_CSP_RESOURCE_FONT, &is_allowed));
  ASSERT_EQ(1, is_allowed);

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_csp_add_domain(NULL, "https://test.com"));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_csp_add_domain(csp, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_csp_check_domain(NULL, "https://test.com",
                                 CMP_CSP_RESOURCE_IMAGE, &is_allowed));
  ASSERT_EQ(
      CMP_ERROR_INVALID_ARG,
      cmp_csp_check_domain(csp, NULL, CMP_CSP_RESOURCE_IMAGE, &is_allowed));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_csp_check_domain(csp, "https://test.com",
                                 CMP_CSP_RESOURCE_IMAGE, NULL));

  ASSERT_EQ(CMP_SUCCESS, cmp_csp_destroy(csp));
  PASS();
}

SUITE(cmp_csp_suite) {
  RUN_TEST(test_cmp_csp_create_destroy);
  RUN_TEST(test_cmp_csp_domains);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_csp_suite);
  GREATEST_MAIN_END();
}
