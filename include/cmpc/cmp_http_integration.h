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
#include "cmpc/cmp_form.h"

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

struct CMPHttpForm;

/**
 * @brief HTTP form submit completion callback.
 * @param ctx User callback context.
 * @param form HTTP form instance.
 * @param status_code HTTP response status code (or 0 on failure).
 * @param body Response body payload (may be NULL).
 * @param body_size Response body size in bytes.
 * @return CMP_OK on success or a failure code.
 */
typedef int(CMP_CALL *CMPHttpFormSubmitCallback)(void *ctx,
                                                 struct CMPHttpForm *form,
                                                 int status_code,
                                                 const void *body,
                                                 cmp_usize body_size);

/**
 * @brief High-level HTTP form wrapper.
 */
typedef struct CMPHttpForm {
  CMPForm form;                      /**< Base form widget. */
  CMPHttpIntegration *integration;   /**< HTTP integration loop. */
  const char *method;                /**< Target HTTP method (e.g. "POST"). */
  const char *url;                   /**< Target URL. */
  CMPHttpFormSubmitCallback on_done; /**< Completion callback. */
  void *user_data;                   /**< User context for callback. */
  struct HttpClient *client;         /**< HTTP client instance (internal). */
  struct HttpRequest *request;       /**< Active HTTP request handle. */
  struct HttpFuture *future;         /**< Active request future. */
} CMPHttpForm;

/**
 * @brief Initialize an HTTP form wrapper.
 * @param http_form HTTP form instance.
 * @param integration HTTP integration instance.
 * @param dispatcher UI event dispatcher.
 * @param root_widget Root widget for the form visual tree.
 * @param method Target HTTP method (e.g. "POST").
 * @param url Target URL.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_http_form_init(CMPHttpForm *http_form,
                                        CMPHttpIntegration *integration,
                                        CMPEventDispatcher *dispatcher,
                                        CMPWidget *root_widget,
                                        const char *method, const char *url);

/**
 * @brief Configure the HTTP form submit completion callback.
 * @param http_form HTTP form instance.
 * @param on_done Completion callback.
 * @param user_data User context.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_http_form_set_on_done(
    CMPHttpForm *http_form, CMPHttpFormSubmitCallback on_done, void *user_data);

/**
 * @brief Shut down an HTTP form wrapper, freeing internal HTTP state.
 * @param http_form HTTP form instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_http_form_shutdown(CMPHttpForm *http_form);

/**
 * @brief Tick the HTTP form to process asynchronous responses.
 * @param http_form HTTP form instance.
 * @return CMP_OK on success or a failure code.
 */
CMP_API int CMP_CALL cmp_http_form_tick(CMPHttpForm *http_form);

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
