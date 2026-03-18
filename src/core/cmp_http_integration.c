/* clang-format off */
#include "cmpc/cmp_http_integration.h"
#include "cmpc/cmp_text_field.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
/* clang-format on */

static char *cmp_internal_strdup(const char *s) {
  size_t len;
  char *d;
  if (!s)
    return NULL;
  len = strlen(s);
  d = (char *)malloc(len + 1);
  if (!d)
    return NULL;
  memcpy(d, s, len + 1);
  return d;
}

static int cmp_http_form_internal_submit(void *ctx, CMPForm *form) {
  CMPHttpForm *http_form = (CMPHttpForm *)ctx;
  cmp_usize i;
  int rc;
  struct HttpRequest *req_arr[1];
  struct HttpFuture *fut_arr[1];

  if (!http_form || !http_form->integration) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (http_form->request || http_form->future) {
    return CMP_OK; /* Already active */
  }

  http_form->request = calloc(1, sizeof(struct HttpRequest));
  if (!http_form->request)
    return CMP_ERR_OUT_OF_MEMORY;

  http_form->future = calloc(1, sizeof(struct HttpFuture));
  if (!http_form->future) {
    free(http_form->request);
    http_form->request = NULL;
    return CMP_ERR_OUT_OF_MEMORY;
  }

  http_request_init(http_form->request);
  http_future_init(http_form->future);

  if (http_form->url) {
    http_form->request->url = cmp_internal_strdup(http_form->url);
  }

  if (http_form->method) {
    if (strcmp(http_form->method, "POST") == 0) {
      http_form->request->method = HTTP_POST;
    } else if (strcmp(http_form->method, "PUT") == 0) {
      http_form->request->method = HTTP_PUT;
    } else {
      http_form->request->method = HTTP_GET;
    }
  } else {
    http_form->request->method = HTTP_GET;
  }

  for (i = 0; i < form->input_count; ++i) {
    if (cmp_widget_is_text_field(form->inputs[i])) {
      CMPTextField *tf = (CMPTextField *)form->inputs[i];
      if (tf->utf8_label && tf->utf8) {
        char *field_name = cmp_internal_strdup(tf->utf8_label);
        if (field_name) {
          http_request_add_part(http_form->request, field_name, NULL, NULL,
                                tf->utf8, tf->utf8_len);
          free(field_name);
        }
      }
    }
  }

  http_request_flatten_parts(http_form->request);

  if (!http_form->client) {
    http_form->client = calloc(1, sizeof(struct HttpClient));
    if (http_form->client) {
      http_client_init(http_form->client);
      http_form->client->config.modality = MODALITY_ASYNC;
      http_form->client->loop = http_form->integration->loop;
    }
  }

  if (http_form->client) {
    req_arr[0] = http_form->request;
    fut_arr[0] = http_form->future;
    rc = http_client_send_multi(http_form->client, req_arr, 1, fut_arr, NULL,
                                NULL, 0);
    if (rc == 0) {
      cmp_form_set_submitting(form, CMP_TRUE);
    }
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_http_form_init(CMPHttpForm *http_form,
                                        CMPHttpIntegration *integration,
                                        CMPEventDispatcher *dispatcher,
                                        CMPWidget *root_widget,
                                        const char *method, const char *url) {
  int rc;
  if (!http_form || !integration || !dispatcher || !root_widget) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  memset(http_form, 0, sizeof(*http_form));
  rc = cmp_form_init(&http_form->form, dispatcher, root_widget);
  if (rc != CMP_OK) {
    return rc;
  }

  http_form->integration = integration;
  http_form->method = method;
  http_form->url = url;

  cmp_form_set_submit(&http_form->form, NULL, cmp_http_form_internal_submit,
                      http_form);

  return CMP_OK;
}

CMP_API int CMP_CALL
cmp_http_form_set_on_done(CMPHttpForm *http_form,
                          CMPHttpFormSubmitCallback on_done, void *user_data) {
  if (!http_form) {
    return CMP_ERR_INVALID_ARGUMENT;
  }
  http_form->on_done = on_done;
  http_form->user_data = user_data;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_http_form_set_loading_indicator(
    CMPHttpForm *http_form, CMPWidget *loading_indicator) {
  if (!http_form)
    return CMP_ERR_INVALID_ARGUMENT;
  http_form->loading_indicator = loading_indicator;
  if (loading_indicator) {
    loading_indicator->flags |= CMP_WIDGET_FLAG_DISABLED;
  }
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_http_form_shutdown(CMPHttpForm *http_form) {
  if (!http_form) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (http_form->request) {
    if (http_form->request->url) {
      free(http_form->request->url);
      http_form->request->url = NULL;
    }
    http_request_free(http_form->request);
    free(http_form->request);
    http_form->request = NULL;
  }

  if (http_form->future) {
    http_future_free(http_form->future);
    free(http_form->future);
    http_form->future = NULL;
  }

  if (http_form->client) {
    http_client_free(http_form->client);
    free(http_form->client);
    http_form->client = NULL;
  }

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_http_form_tick(CMPHttpForm *http_form) {
  if (!http_form) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  if (http_form->future && http_form->future->is_ready) {
    int status_code = 0;
    const void *body = NULL;
    cmp_usize body_size = 0;

    if (http_form->future->error_code == 0 && http_form->future->response) {
      status_code = http_form->future->response->status_code;
      body = http_form->future->response->body;
      body_size = (cmp_usize)http_form->future->response->body_len;
    }

    if (http_form->on_done) {
      http_form->on_done(http_form->user_data, http_form, status_code, body,
                         body_size);
    }

    cmp_form_set_submitting(&http_form->form, CMP_FALSE);

    /* Clean up so another request can be made */
    if (http_form->request->url) {
      free(http_form->request->url);
      http_form->request->url = NULL;
    }
    http_request_free(http_form->request);
    free(http_form->request);
    http_form->request = NULL;

    http_future_free(http_form->future);
    free(http_form->future);
    http_form->future = NULL;
  }

  return CMP_OK;
}

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
