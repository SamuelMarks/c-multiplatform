#include "cmp.h"
#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
/**
 * @brief cmp_tls_key_create
 *
 * @param out_key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key) {
  int rc;
  rc = 0;
  *out_key = 0;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
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
  int rc;
  rc = 0;
  (void)key;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
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
  int rc;
  rc = 0;
  (void)key;
  (void)value;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
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
  int rc;
  rc = 0;
  (void)key;
  *out_value = NULL;
  if (rc != 0) {
    if (rc != 0) {
      return rc;
    }
    return rc;
  }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
#else
#include "cmp.h"
#if 0 || defined(__WATCOMC__) || defined(__DOS__)
/**
 * @brief cmp_tls_key_create
 *
 * @param out_key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key) {
  int rc;
  rc = 0;*out_key = 0; if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;(void)key; if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;(void)key; (void)value; if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;(void)key; *out_value = NULL; if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
#else
#if 0 || defined(__WATCOMC__) || defined(__DOS__)
#include "cmp.h"
#else
/* clang-format off */
#include "cmp.h"

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
  int rc;
  rc = 0;if (out_key == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    *out_key = TlsAlloc();
    if (*out_key == TLS_OUT_OF_INDEXES) {
        return CMP_ERROR_OOM;
    }

    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;if (TlsFree(key) == 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;if (TlsSetValue(key, value) == 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;void *val;

    if (out_value == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    val = TlsGetValue(key);
    if (val == NULL && GetLastError() != 0) {
        *out_value = NULL;
        return CMP_ERROR_INVALID_ARG;
    }

    *out_value = val;
    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
#else

/**
 * @brief cmp_tls_key_create
 *
 * @param out_key Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_tls_key_create(cmp_tls_key_t *out_key) {
  int rc;
  rc = 0;if (out_key == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    if (pthread_key_create(out_key, NULL) != 0) {
        return CMP_ERROR_OOM;
    }

    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;if (pthread_key_delete(key) != 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;if (pthread_setspecific(key, value) != 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
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
  int rc;
  rc = 0;if (out_value == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    *out_value = pthread_getspecific(key);
    if (rc != 0) { if (rc != 0) {   return rc; } return rc; }
  if (rc != 0) {
    return rc;
  }
  return rc;
}
#endif
/* clang-format on */

#endif

#endif

#endif
