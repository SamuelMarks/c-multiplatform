/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief cmp_documentation_mock_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_documentation_mock_init(void) {
  int rc = CMP_SUCCESS;
  /* Documentation is primarily an inline effort, no complex logic required */
  cmp_log_debug(
      "cmp_documentation_mock_init: Initialized documentation stubs\n");

  return rc;
}
