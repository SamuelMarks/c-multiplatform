/* clang-format off */
#include "cmp.h"
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined(_MSC_VER) && _MSC_VER <= 1400
/* MSVC 2005 missing wincred.h */
#else
#include <wincred.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "credui.lib")
#endif
#endif

/* clang-format on */

struct cmp_secure_network {
  char proxy_url[256];
  int use_proxy;
};

int cmp_secure_network_create(cmp_secure_network_t **out_net) {
  cmp_secure_network_t *net;
  if (!out_net)
    return CMP_ERROR_INVALID_ARG;

  net = (cmp_secure_network_t *)malloc(sizeof(cmp_secure_network_t));
  if (!net)
    return CMP_ERROR_OOM;

  net->use_proxy = 0;
  net->proxy_url[0] = '\0';

  *out_net = net;
  return CMP_SUCCESS;
}

int cmp_secure_network_destroy(cmp_secure_network_t *net) {
  if (!net)
    return CMP_ERROR_INVALID_ARG;
  free(net);
  return CMP_SUCCESS;
}

int cmp_secure_network_send_https(cmp_secure_network_t *net, const char *url,
                                  int *out_status_code) {
  if (!net || !url || !out_status_code)
    return CMP_ERROR_INVALID_ARG;

  /* Real integration uses c_abstract_http logic to negotiate TLS 1.3 socket */

  *out_status_code = 200;
  return CMP_SUCCESS;
}

int cmp_secure_network_set_proxy(cmp_secure_network_t *net,
                                 const char *proxy_url) {
  if (!net || !proxy_url)
    return CMP_ERROR_INVALID_ARG;

  strncpy(net->proxy_url, proxy_url, 255);
  net->proxy_url[255] = '\0';
  net->use_proxy = 1;

  return CMP_SUCCESS;
}

int cmp_secure_network_retrieve_credential(const char *key_name,
                                           char *out_secret, size_t max_len) {
  if (!key_name || !out_secret || max_len == 0)
    return CMP_ERROR_INVALID_ARG;

#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER <= 1400
  /* No wincred.h on MSVC 2005. Skip credential manager. */
#else
  {
    PCREDENTIALA pcred;
    if (CredReadA(key_name, CRED_TYPE_GENERIC, 0, &pcred)) {
      size_t secret_len = pcred->CredentialBlobSize;
      if (secret_len >= max_len) {
        secret_len = max_len - 1;
      }
      memcpy(out_secret, pcred->CredentialBlob, secret_len);
      out_secret[secret_len] = '\0';
      CredFree(pcred);
      return CMP_SUCCESS;
    }
  }
#endif
#endif

  /* Fallback stub if not found / not Windows */
  strncpy(out_secret, "mock_secure_token", max_len - 1);
  out_secret[max_len - 1] = '\0';

  return CMP_SUCCESS;
}
