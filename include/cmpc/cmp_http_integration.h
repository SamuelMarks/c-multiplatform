#ifndef CMP_HTTP_INTEGRATION_H
#define CMP_HTTP_INTEGRATION_H

/**
 * @file cmp_http_integration.h
 * @brief HTTP event loop integration for LibCMPC.
 */

/* clang-format off */
#ifdef __cplusplus
extern "C" {
#endif

#include "cmpc/cmp_event.h"

#ifndef C_ABSTRACT_HTTP_NO_MULTIPLATFORM_INTEGRATION
#define C_ABSTRACT_HTTP_NO_MULTIPLATFORM_INTEGRATION
#endif
#include <c_abstract_http/c_abstract_http.h>
/* clang-format on */

/**
 * @brief HTTP integration loop state.
 */
typedef struct CMPHttpIntegration {
  struct ModalityEventLoop *loop; /**< HTTP modality event loop pointer. */
  CMPEventDispatcher *dispatcher; /**< UI event dispatcher. */
} CMPHttpIntegration;

/**
 * @brief Initialize an HTTP integration loop.
 * @param integration Integration instance.
 * @param dispatcher UI event dispatcher.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_http_integration_init(CMPHttpIntegration *integration,
                                               CMPEventDispatcher *dispatcher);

/**
 * @brief Shut down an HTTP integration loop.
 * @param integration Integration instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL
cmp_http_integration_shutdown(CMPHttpIntegration *integration);

/**
 * @brief Tick the HTTP integration loop.
 *
 * This should be called once per frame in the main application loop.
 * It will poll the HTTP network state and fire custom events via the
 * dispatcher.
 *
 * @param integration Integration instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_http_integration_tick(CMPHttpIntegration *integration);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CMP_HTTP_INTEGRATION_H */
