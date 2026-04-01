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

int cmp_tls_key_create(cmp_tls_key_t *out_key) {
    if (out_key == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    *out_key = TlsAlloc();
    if (*out_key == TLS_OUT_OF_INDEXES) {
        return CMP_ERROR_OOM;
    }

    return CMP_SUCCESS;
}

int cmp_tls_key_delete(cmp_tls_key_t key) {
    if (TlsFree(key) == 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    return CMP_SUCCESS;
}

int cmp_tls_set(cmp_tls_key_t key, void *value) {
    if (TlsSetValue(key, value) == 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    return CMP_SUCCESS;
}

int cmp_tls_get(cmp_tls_key_t key, void **out_value) {
    void *val;

    if (out_value == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    val = TlsGetValue(key);
    if (val == NULL && GetLastError() != 0) {
        *out_value = NULL;
        return CMP_ERROR_INVALID_ARG;
    }

    *out_value = val;
    return CMP_SUCCESS;
}

#else

int cmp_tls_key_create(cmp_tls_key_t *out_key) {
    if (out_key == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    if (pthread_key_create(out_key, NULL) != 0) {
        return CMP_ERROR_OOM;
    }

    return CMP_SUCCESS;
}

int cmp_tls_key_delete(cmp_tls_key_t key) {
    if (pthread_key_delete(key) != 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    return CMP_SUCCESS;
}

int cmp_tls_set(cmp_tls_key_t key, void *value) {
    if (pthread_setspecific(key, value) != 0) {
        return CMP_ERROR_INVALID_ARG;
    }
    return CMP_SUCCESS;
}

int cmp_tls_get(cmp_tls_key_t key, void **out_value) {
    if (out_value == NULL) {
        return CMP_ERROR_INVALID_ARG;
    }

    *out_value = pthread_getspecific(key);
    return CMP_SUCCESS;
}

#endif
/* clang-format on */
