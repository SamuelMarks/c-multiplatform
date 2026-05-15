/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

static int g_documentation_mock_initialized = 0;

/**
 * @brief cmp_documentation_mock_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_documentation_mock_init(void) {
  int rc = CMP_SUCCESS;

  if (g_documentation_mock_initialized) {
    cmp_log_debug("cmp_documentation_mock_init: Already initialized\n");
    return CMP_SUCCESS; /* Idempotent */
  }

  /* Allocate or initialize developer experience documentation mocked subsystem
   */
  g_documentation_mock_initialized = 1;
  cmp_log_debug("cmp_documentation_mock_init: Initialized developer experience "
                "documentation mock system\n");

  return rc;
}
