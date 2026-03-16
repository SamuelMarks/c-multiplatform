/* clang-format off */
#include "cmpc/cmp_http_integration.h"
#include <stddef.h>
/* clang-format on */

CMP_API int CMP_CALL cmp_http_integration_init(CMPHttpIntegration *integration,
                                               CMPEventDispatcher *dispatcher) {
  if (!integration || !dispatcher) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  integration->dispatcher = dispatcher;
  integration->loop = NULL;
  if (http_loop_init(&integration->loop) != 0) {
    return CMP_ERR_UNKNOWN;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL
cmp_http_integration_shutdown(CMPHttpIntegration *integration) {
  if (!integration) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  if (integration->loop) {
    http_loop_free(integration->loop);
    integration->loop = NULL;
  }
  integration->dispatcher = NULL;
  return CMP_OK;
}

CMP_API int CMP_CALL
cmp_http_integration_tick(CMPHttpIntegration *integration) {
  if (!integration) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  /* http_loop_tick runs the loop iteration without blocking */
  if (integration->loop) {
    http_loop_tick(integration->loop);
  }
  return CMP_OK;
}
