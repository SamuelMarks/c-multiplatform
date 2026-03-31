/* clang-format off */
#include <cmp.h>
#include <greatest.h>
#include <stdlib.h>
/* clang-format on */

SUITE(cmp_network_suite);

TEST test_cmp_network_create_destroy(void) {
  cmp_network_t *network = NULL;

  ASSERT_EQ(CMP_SUCCESS, cmp_network_create(&network));
  ASSERT_NEQ(NULL, network);

  ASSERT_EQ(CMP_SUCCESS, cmp_network_destroy(network));

  /* Null checks */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_network_create(NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_network_destroy(NULL));

  PASS();
}

TEST test_cmp_network_status(void) {
  cmp_network_t *network = NULL;
  cmp_network_status_t status = CMP_NETWORK_STATUS_OFFLINE;

  ASSERT_EQ(CMP_SUCCESS, cmp_network_create(&network));

  /* Default */
  ASSERT_EQ(CMP_SUCCESS, cmp_network_get_status(network, &status));
  ASSERT_EQ(CMP_NETWORK_STATUS_ONLINE, status);

  /* Valid transitions */
  ASSERT_EQ(CMP_SUCCESS,
            cmp_network_set_status(network, CMP_NETWORK_STATUS_OFFLINE));
  ASSERT_EQ(CMP_SUCCESS, cmp_network_get_status(network, &status));
  ASSERT_EQ(CMP_NETWORK_STATUS_OFFLINE, status);

  ASSERT_EQ(CMP_SUCCESS,
            cmp_network_set_status(network, CMP_NETWORK_STATUS_METERED));
  ASSERT_EQ(CMP_SUCCESS, cmp_network_get_status(network, &status));
  ASSERT_EQ(CMP_NETWORK_STATUS_METERED, status);

  /* Null and invalid args */
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_network_get_status(NULL, &status));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG, cmp_network_get_status(network, NULL));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_network_set_status(NULL, CMP_NETWORK_STATUS_ONLINE));
  ASSERT_EQ(CMP_ERROR_INVALID_ARG,
            cmp_network_set_status(network, (cmp_network_status_t)999));

  ASSERT_EQ(CMP_SUCCESS, cmp_network_destroy(network));
  PASS();
}

SUITE(cmp_network_suite) {
  RUN_TEST(test_cmp_network_create_destroy);
  RUN_TEST(test_cmp_network_status);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(cmp_network_suite);
  GREATEST_MAIN_END();
}
