/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"

#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)

/**
 * @brief cmp_tls_key_create
 *
 * @param out_key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key) {
int rc = CMP_SUCCESS;
  if (out_key == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_tls_key_create: out_key is NULL\n");
    return rc;
  }
  *out_key = 0;
  return rc;
}

/**
 * @brief cmp_tls_key_delete
 *
 * @param key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_delete(cmp_tls_key_t key) {
int rc = CMP_SUCCESS;
  (void)key;
  return rc;
}

/**
 * @brief cmp_tls_set
 *
 * @param key Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_set(cmp_tls_key_t key, void *value) {
int rc = CMP_SUCCESS;
  (void)key;
  (void)value;
  return rc;
}

/**
 * @brief cmp_tls_get
 *
 * @param key Parameter description.
 * @param out_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_get(cmp_tls_key_t key, void **out_value) {
int rc = CMP_SUCCESS;
  (void)key;
  if (out_value == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_tls_get: out_value is NULL\n");
    return rc;
  }
  *out_value = NULL;
  return rc;
}

#else

#if defined(_WIN32)

/* Forward declare Windows APIs to avoid <windows.h> */
__declspec(dllimport) unsigned long __stdcall TlsAlloc(void);
__declspec(dllimport) int __stdcall TlsFree(unsigned long dwTlsIndex);
__declspec(dllimport) int __stdcall TlsSetValue(unsigned long dwTlsIndex, void *lpTlsValue);
__declspec(dllimport) void *__stdcall TlsGetValue(unsigned long dwTlsIndex);
__declspec(dllimport) unsigned long __stdcall GetLastError(void);
#define TLS_OUT_OF_INDEXES ((unsigned long)0xFFFFFFFF)

/**
 * @brief cmp_tls_key_create
 *
 * @param out_key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key) {
int rc = CMP_SUCCESS;
  if (out_key == NULL) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_key_create: out_key is NULL\n");
      return rc;
  }

  *out_key = TlsAlloc();
  if (*out_key == TLS_OUT_OF_INDEXES) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_tls_key_create: TlsAlloc failed\n");
      return rc;
  }

  return rc;
}

/**
 * @brief cmp_tls_key_delete
 *
 * @param key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_delete(cmp_tls_key_t key) {
int rc = CMP_SUCCESS;
  if (TlsFree(key) == 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_key_delete: TlsFree failed\n");
      return rc;
  }
  return rc;
}

/**
 * @brief cmp_tls_set
 *
 * @param key Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_set(cmp_tls_key_t key, void *value) {
int rc = CMP_SUCCESS;
  if (TlsSetValue(key, value) == 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_set: TlsSetValue failed\n");
      return rc;
  }
  return rc;
}

/**
 * @brief cmp_tls_get
 *
 * @param key Parameter description.
 * @param out_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_get(cmp_tls_key_t key, void **out_value) {
int rc = CMP_SUCCESS;
  void *val;

  if (out_value == NULL) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_get: out_value is NULL\n");
      return rc;
  }

  val = TlsGetValue(key);
  if (val == NULL && GetLastError() != 0) {
      *out_value = NULL;
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_get: TlsGetValue failed\n");
      return rc;
  }

  *out_value = val;
  return rc;
}

#else

#include <pthread.h>

/**
 * @brief cmp_tls_key_create
 *
 * @param out_key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key) {
int rc = CMP_SUCCESS;
  if (out_key == NULL) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_key_create: out_key is NULL\n");
      return rc;
  }

  if (pthread_key_create(out_key, NULL) != 0) {
      rc = CMP_ERROR_OOM;
      LOG_DEBUG("cmp_tls_key_create: pthread_key_create failed\n");
      return rc;
  }

  return rc;
}

/**
 * @brief cmp_tls_key_delete
 *
 * @param key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_delete(cmp_tls_key_t key) {
int rc = CMP_SUCCESS;
  if (pthread_key_delete(key) != 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_key_delete: pthread_key_delete failed\n");
      return rc;
  }
  return rc;
}

/**
 * @brief cmp_tls_set
 *
 * @param key Parameter description.
 * @param value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_set(cmp_tls_key_t key, void *value) {
int rc = CMP_SUCCESS;
  if (pthread_setspecific(key, value) != 0) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_set: pthread_setspecific failed\n");
      return rc;
  }
  return rc;
}

/**
 * @brief cmp_tls_get
 *
 * @param key Parameter description.
 * @param out_value Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_get(cmp_tls_key_t key, void **out_value) {
int rc = CMP_SUCCESS;
  if (out_value == NULL) {
      rc = CMP_ERROR_INVALID_ARG;
      LOG_DEBUG("cmp_tls_get: out_value is NULL\n");
      return rc;
  }

  *out_value = pthread_getspecific(key);
  return rc;
}

#endif
#endif
/* clang-format on */
