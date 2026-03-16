/* clang-format off */
#include "cmpc/cmp_http_integration.h"
#include "test_utils.h"
#include <string.h>

#ifndef CMP_TESTING
#define CMP_TESTING
#endif
/* clang-format on */

static int test_http_integration_init(void) {
  CMPHttpIntegration integration;
  CMPEventDispatcher dispatcher;

  CMP_TEST_EXPECT(cmp_http_integration_init(NULL, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_http_integration_init(&integration, NULL),
                  CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_http_integration_init(NULL, &dispatcher),
                  CMP_ERR_INVALID_ARGUMENT);

  cmp_event_dispatcher_init(&dispatcher);
  CMP_TEST_OK(cmp_http_integration_init(&integration, &dispatcher));

  cmp_http_integration_shutdown(&integration);
  return 0;
}

static int test_http_integration_lifecycle(void) {
  CMPHttpIntegration integration;
  CMPEventDispatcher dispatcher;

  cmp_event_dispatcher_init(&dispatcher);
  cmp_http_integration_init(&integration, &dispatcher);

  /* Bad arguments */
  CMP_TEST_EXPECT(cmp_http_integration_tick(NULL), CMP_ERR_INVALID_ARGUMENT);
  CMP_TEST_EXPECT(cmp_http_integration_shutdown(NULL),
                  CMP_ERR_INVALID_ARGUMENT);

  /* Valid tick */
  CMP_TEST_OK(cmp_http_integration_tick(&integration));

  /* Shutdown */
  CMP_TEST_OK(cmp_http_integration_shutdown(&integration));

  return 0;
}

int main(void) {
  int fails = 0;
  fails += test_http_integration_init();
  fails += test_http_integration_lifecycle();
  return fails;
}
