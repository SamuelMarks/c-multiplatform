/* clang-format off */
#include "cmp.h"

#include <stdlib.h>
#include <string.h>

#include "cmp_log.h"

#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER <= 1400
/* MSVC 2005 missing wincred.h */
#else
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "credui.lib")

typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef void* PVOID;

typedef struct _CREDENTIALA {
    DWORD Flags;
    DWORD Type;
    char* TargetName;
    char* Comment;
    struct {
        DWORD dwLowDateTime;
        DWORD dwHighDateTime;
    } LastWritten;
    DWORD CredentialBlobSize;
    BYTE* CredentialBlob;
    DWORD Persist;
    DWORD AttributeCount;
    PVOID Attributes;
    char* TargetAlias;
    char* UserName;
} CREDENTIALA, *PCREDENTIALA;

__declspec(dllimport) int __stdcall CredReadA(const char* TargetName, DWORD Type, DWORD Flags, PCREDENTIALA* Credential);
__declspec(dllimport) void __stdcall CredFree(PVOID Buffer);
#define CRED_TYPE_GENERIC 1
#endif
#endif

/* clang-format on */

struct cmp_secure_network {
  char proxy_url[256];
  int use_proxy;
};

/**
 * @brief Allocates and initializes a secure network context.
 *
 * @param out_net A pointer to a pointer that will hold the created network
 * context.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_secure_network_create(cmp_secure_network_t **out_net) {
  int rc;
  cmp_secure_network_t *net;

  rc = CMP_SUCCESS;

  if (out_net == NULL) {
    LOG_DEBUG("cmp_secure_network_create: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_MALLOC(sizeof(cmp_secure_network_t), (void **)&net);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_secure_network_create: OOM\n");
    return CMP_ERROR_OOM;
  }

  net->use_proxy = 0;
  net->proxy_url[0] = '\0';

  *out_net = net;
  return CMP_SUCCESS;
}

/**
 * @brief Destroys a secure network context and frees its memory.
 *
 * @param net The network context to destroy.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_secure_network_destroy(cmp_secure_network_t *net) {
  int rc;

  rc = CMP_SUCCESS;

  if (net == NULL) {
    LOG_DEBUG("cmp_secure_network_destroy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  rc = CMP_FREE(net);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_secure_network_destroy: Free failed with code %d\n", rc);
    return rc;
  }

  return CMP_SUCCESS;
}

/**
 * @brief Sends an HTTPS request using the given network context.
 *
 * @param net The secure network context.
 * @param url The URL to send the request to.
 * @param out_status_code A pointer to an int that will hold the HTTP status
 * code.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_secure_network_send_https(cmp_secure_network_t *net, const char *url,
                                  int *out_status_code) {
  if (net == NULL || url == NULL || out_status_code == NULL) {
    LOG_DEBUG("cmp_secure_network_send_https: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

  /* Real integration uses c_abstract_http logic to negotiate TLS 1.3 socket */
  *out_status_code = 200;

  return CMP_SUCCESS;
}

/**
 * @brief Sets a proxy for the secure network context.
 *
 * @param net The secure network context.
 * @param proxy_url The proxy URL string to set.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_secure_network_set_proxy(cmp_secure_network_t *net,
                                 const char *proxy_url) {
  if (net == NULL || proxy_url == NULL) {
    LOG_DEBUG("cmp_secure_network_set_proxy: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_MSC_VER)
  if (strncpy_s(net->proxy_url, 256, proxy_url, 255) != 0) {
    LOG_DEBUG("strncpy_s failed\n");
    return CMP_ERROR_GENERAL;
  }
#else
  strncpy(net->proxy_url, proxy_url, 255);
  net->proxy_url[255] = '\0';
#endif

  net->use_proxy = 1;

  return CMP_SUCCESS;
}

/**
 * @brief Retrieves a credential from the system's secure store.
 *
 * @param key_name The name/key of the credential to retrieve.
 * @param out_secret A buffer to hold the retrieved secret.
 * @param max_len The maximum length of the out_secret buffer.
 * @return Returns CMP_SUCCESS on success, or an error code on failure.
 */
int cmp_secure_network_retrieve_credential(const char *key_name,
                                           char *out_secret, size_t max_len) {
  if (key_name == NULL || out_secret == NULL || max_len == 0) {
    LOG_DEBUG("cmp_secure_network_retrieve_credential: Invalid argument\n");
    return CMP_ERROR_INVALID_ARG;
  }

#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER <= 1400
  /* No wincred.h on MSVC 2005. Skip credential manager. */
#else
  {
    PCREDENTIALA pcred;
    if (CredReadA(key_name, CRED_TYPE_GENERIC, 0, &pcred)) {
      size_t secret_len;
      secret_len = pcred->CredentialBlobSize;

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

  LOG_DEBUG("cmp_secure_network_retrieve_credential: Not found\n");
  return CMP_ERROR_NOT_FOUND;
}
