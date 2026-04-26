#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__)
#include "cmp.h"
/**
 * @brief cmp_coroutine_system_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_init(void) { return CMP_SUCCESS; }
/**
 * @brief cmp_coroutine_system_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_shutdown(void) { return CMP_SUCCESS; }
/**
 * @brief cmp_coroutine_create
 *
 * @param out_co Parameter description.
 * @param stack_size Parameter description.
 * @param fn Parameter description.
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  return CMP_ERROR_NOT_FOUND;
}
/**
 * @brief cmp_coroutine_resume
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_resume(cmp_coroutine_t *co) { return CMP_ERROR_NOT_FOUND; }
/**
 * @brief cmp_coroutine_yield
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_yield(cmp_coroutine_t *co) { return CMP_ERROR_NOT_FOUND; }
/**
 * @brief cmp_coroutine_destroy
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_destroy(cmp_coroutine_t *co) { return CMP_ERROR_NOT_FOUND; }

#else
/* clang-format off */
#if defined(__APPLE__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600
#endif
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>

#if defined(_WIN32)
__declspec(dllimport) void *__stdcall ConvertThreadToFiber(void *lpParameter);
__declspec(dllimport) void *__stdcall CreateFiber(size_t dwStackSize, void (__stdcall *lpStartAddress)(void *), void *lpParameter);
__declspec(dllimport) void __stdcall SwitchToFiber(void *lpFiber);
__declspec(dllimport) void __stdcall DeleteFiber(void *lpFiber);
__declspec(dllimport) unsigned long __stdcall GetLastError(void);

#if defined(_M_X64) || defined(__x86_64__)
#define GetCurrentFiber() ((void*)__readgsqword(0x20))
#elif defined(_M_IX86) || defined(__i386__)
#define GetCurrentFiber() ((void*)__readfsdword(0x10))
#else
__declspec(dllimport) void *__stdcall GetCurrentFiber(void);
#endif

#define CMP_CORO_SUPPORTED 1

#elif defined(__linux__) || defined(__APPLE__)

/* MacOS deprecated ucontext in 10.6, but we need it for portable C89 coroutines.
 * Ignoring deprecation warnings for this specific file.
 */
#if defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <ucontext.h>

#define CMP_CORO_SUPPORTED 1

#else

/* Unsupported platforms (e.g. unknown OS without ucontext or fibers) */
#define CMP_CORO_SUPPORTED 0

#endif
/* clang-format on */

#if CMP_CORO_SUPPORTED

#if defined(_WIN32)
/* Windows Fiber implementation */

static cmp_tls_key_t g_coro_system_key;
static int g_coro_system_initialized = 0;

/**
 * @brief cmp_fiber_entry
 *
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void __stdcall cmp_fiber_entry(void *arg) {
  cmp_coroutine_t *co = (cmp_coroutine_t *)arg;

  co->state = CMP_CORO_RUNNING;
  co->fn(co, co->arg);
  co->state = CMP_CORO_FINISHED;

  /* Switch back to the caller fiber */
  if (co->caller != NULL) {
    cmp_tls_set(g_coro_system_key, co->caller);
    SwitchToFiber(co->caller->context);
  }
}

/**
 * @brief cmp_coroutine_system_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_init(void) {
  int rc = CMP_SUCCESS;
  void *main_fiber;

  if (!g_coro_system_initialized) {
    rc = cmp_tls_key_create(&g_coro_system_key);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_coroutine_system_init: Failed to create TLS key\n");
      return CMP_ERROR_OOM;
    }
    g_coro_system_initialized = 1;
  }

  /* Convert current thread to a fiber if it isn't one already */
  if (GetCurrentFiber() ==
      (void *)0x1E00) { /* Magic value for non-fiber on some Windows */
    main_fiber = ConvertThreadToFiber(NULL);
    if (main_fiber == NULL && GetLastError() != 0) {
      LOG_DEBUG(
          "Error in cmp_coroutine_system_init: ConvertThreadToFiber failed\n");
      return CMP_ERROR_INVALID_ARG;
    }
  }

  return rc;
}

/**
 * @brief cmp_coroutine_create
 *
 * @param out_co Parameter description.
 * @param stack_size Parameter description.
 * @param fn Parameter description.
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  int rc = CMP_SUCCESS;
  cmp_coroutine_t *co = NULL;

  if (out_co == NULL || fn == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_create: Invalid argument\n");
    return rc;
  }

  if (!g_coro_system_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_create: System not initialized\n");
    return rc; /* System not initialized on this thread */
  }

  if (stack_size == 0) {
    stack_size = 1024 * 1024; /* 1MB default */
  }

  rc = CMP_MALLOC(sizeof(cmp_coroutine_t), (void **)&co);
  if (rc != CMP_SUCCESS || co == NULL) {
    LOG_DEBUG(
        "Error in cmp_coroutine_create: Out of memory allocating coroutine\n");
    return CMP_ERROR_OOM;
  }

  co->state = CMP_CORO_READY;
  co->fn = fn;
  co->arg = arg;
  co->stack = NULL; /* Not used for Fibers */
  co->stack_size = stack_size;
  co->caller = NULL;

  co->context = CreateFiber(stack_size, cmp_fiber_entry, co);
  if (co->context == NULL) {
    CMP_FREE(co);
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("Error in cmp_coroutine_create: CreateFiber failed\n");
    return rc;
  }

  *out_co = co;
  return rc;
}

/**
 * @brief cmp_coroutine_resume
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_resume(cmp_coroutine_t *co) {
  int rc = CMP_SUCCESS;
  cmp_coroutine_t *current_co = NULL;

  if (co == NULL || co->state == CMP_CORO_FINISHED) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_resume: Invalid argument or finished\n");
    return rc;
  }

  if (cmp_tls_get(g_coro_system_key, (void **)&current_co) != CMP_SUCCESS) {
    current_co = NULL;
  }

  /* Wait, the local variable issue: if we are yielding multiple times,
     `main_co` goes out of scope? No, `cmp_coroutine_resume` blocks until
     `cmp_coroutine_yield` is called! So the stack variable is completely safe
     because `SwitchToFiber` blocks this thread's stack execution. */

  {
    cmp_coroutine_t main_co;

    if (current_co == NULL) {
      main_co.context = GetCurrentFiber();
      co->caller = &main_co;
    } else {
      co->caller = current_co;
    }

    cmp_tls_set(g_coro_system_key, co);
    SwitchToFiber(co->context);

    /* When we return here, we are back in the caller */
    cmp_tls_set(g_coro_system_key, current_co);
  }

  return rc;
}

/**
 * @brief cmp_coroutine_yield
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_yield(cmp_coroutine_t *co) {
  int rc = CMP_SUCCESS;

  if (co == NULL || co->caller == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_yield: Invalid argument\n");
    return rc;
  }

  co->state = CMP_CORO_SUSPENDED;
  cmp_tls_set(g_coro_system_key, co->caller);
  SwitchToFiber(co->caller->context);

  return rc;
}

/**
 * @brief cmp_coroutine_destroy
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_destroy(cmp_coroutine_t *co) {
  int rc = CMP_SUCCESS;

  if (co == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_destroy: Invalid argument (co=NULL)\n");
    return rc;
  }

  if (co->context != NULL) {
    DeleteFiber(co->context);
    co->context = NULL;
  }

  CMP_FREE(co);
  return rc;
}

#else
/* POSIX ucontext implementation */

static cmp_tls_key_t g_coro_system_key;
static int g_coro_system_initialized = 0;

/**
 * @brief cmp_ucontext_entry
 *
 * @param arg_ptr_lo Parameter description.
 * @param arg_ptr_hi Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void cmp_ucontext_entry(int arg_ptr_lo, int arg_ptr_hi) {
  /* Reconstruct pointer from 32-bit ints to support 64-bit platforms cleanly
   * via makecontext */
  uint64_t ptr_val =
      ((uint64_t)(uint32_t)arg_ptr_hi << 32) | (uint32_t)arg_ptr_lo;
  cmp_coroutine_t *co = (cmp_coroutine_t *)(uintptr_t)ptr_val;

  co->state = CMP_CORO_RUNNING;
  co->fn(co, co->arg);
  co->state = CMP_CORO_FINISHED;

  if (co->caller != NULL) {
    cmp_tls_set(g_coro_system_key, co->caller);
    setcontext((ucontext_t *)co->caller->context);
  }
}

/**
 * @brief cmp_coroutine_system_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_init(void) {
  int rc = CMP_SUCCESS;

  if (!g_coro_system_initialized) {
    rc = cmp_tls_key_create(&g_coro_system_key);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG(
          "Error in cmp_coroutine_system_init: Failed to create TLS key\n");
      return CMP_ERROR_OOM;
    }
    g_coro_system_initialized = 1;
  }
  return rc;
}

/**
 * @brief cmp_coroutine_create
 *
 * @param out_co Parameter description.
 * @param stack_size Parameter description.
 * @param fn Parameter description.
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  int rc = CMP_SUCCESS;
  cmp_coroutine_t *co = NULL;
  ucontext_t *uc = NULL;
  uint64_t ptr_val;

  if (out_co == NULL || fn == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_create: Invalid argument\n");
    return rc;
  }

  if (!g_coro_system_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_create: System not initialized\n");
    return rc;
  }

  if (stack_size == 0) {
    stack_size = 1024 * 1024;
  }

  rc = CMP_MALLOC(sizeof(cmp_coroutine_t), (void **)&co);
  if (rc != CMP_SUCCESS || co == NULL) {
    LOG_DEBUG(
        "Error in cmp_coroutine_create: Out of memory allocating coroutine\n");
    return CMP_ERROR_OOM;
  }

  rc = CMP_MALLOC(sizeof(ucontext_t), (void **)&co->context);
  if (rc != CMP_SUCCESS || co->context == NULL) {
    CMP_FREE(co);
    LOG_DEBUG(
        "Error in cmp_coroutine_create: Out of memory allocating ucontext\n");
    return CMP_ERROR_OOM;
  }

  rc = CMP_MALLOC(stack_size, (void **)&co->stack);
  if (rc != CMP_SUCCESS || co->stack == NULL) {
    CMP_FREE(co->context);
    CMP_FREE(co);
    LOG_DEBUG(
        "Error in cmp_coroutine_create: Out of memory allocating stack\n");
    return CMP_ERROR_OOM;
  }

  co->state = CMP_CORO_READY;
  co->fn = fn;
  co->arg = arg;
  co->stack_size = stack_size;
  co->caller = NULL;

  uc = (ucontext_t *)co->context;
  if (getcontext(uc) != 0) {
    CMP_FREE(co->stack);
    CMP_FREE(co->context);
    CMP_FREE(co);
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_create: getcontext failed\n");
    return rc;
  }

  uc->uc_stack.ss_sp = co->stack;
  uc->uc_stack.ss_size = stack_size;
  uc->uc_link = NULL;

  ptr_val = (uint64_t)(uintptr_t)co;
  /* Use (void (*)(void)) cast to satisfy strict-prototypes while makecontext
     expects void (*)() in some systems. Both work for calling with args. */
  makecontext(uc, (void (*)(void))cmp_ucontext_entry, 2, (int)(uint32_t)ptr_val,
              (int)(uint32_t)(ptr_val >> 32));

  *out_co = co;
  return rc;
}

/**
 * @brief cmp_coroutine_resume
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_resume(cmp_coroutine_t *co) {
  int rc = CMP_SUCCESS;
  cmp_coroutine_t *current_co = NULL;
  ucontext_t main_ctx;

  if (co == NULL || co->state == CMP_CORO_FINISHED) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_resume: Invalid argument or finished\n");
    return rc;
  }

  if (cmp_tls_get(g_coro_system_key, (void **)&current_co) != CMP_SUCCESS) {
    current_co = NULL;
  }

  if (current_co == NULL) {
    cmp_coroutine_t main_co;
    main_co.context = &main_ctx;
    co->caller = &main_co;
    cmp_tls_set(g_coro_system_key, co);
    swapcontext(&main_ctx, (ucontext_t *)co->context);
    cmp_tls_set(g_coro_system_key, NULL);
  } else {
    co->caller = current_co;
    cmp_tls_set(g_coro_system_key, co);
    swapcontext((ucontext_t *)current_co->context, (ucontext_t *)co->context);
    cmp_tls_set(g_coro_system_key, current_co);
  }

  return rc;
}

/**
 * @brief cmp_coroutine_yield
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_yield(cmp_coroutine_t *co) {
  int rc = CMP_SUCCESS;

  if (co == NULL || co->caller == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_yield: Invalid argument\n");
    return rc;
  }

  co->state = CMP_CORO_SUSPENDED;
  cmp_tls_set(g_coro_system_key, co->caller);
  swapcontext((ucontext_t *)co->context, (ucontext_t *)co->caller->context);

  return rc;
}

/**
 * @brief cmp_coroutine_destroy
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_destroy(cmp_coroutine_t *co) {
  int rc = CMP_SUCCESS;

  if (co == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("Error in cmp_coroutine_destroy: Invalid argument (co=NULL)\n");
    return rc;
  }

  if (co->stack != NULL) {
    CMP_FREE(co->stack);
    co->stack = NULL;
  }

  if (co->context != NULL) {
    CMP_FREE(co->context);
    co->context = NULL;
  }

  CMP_FREE(co);
  return rc;
}

#endif /* POSIX vs WIN32 */

#else /* CMP_CORO_SUPPORTED == 0 */

/**
 * @brief cmp_coroutine_system_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_init(void) {
  int rc = CMP_ERROR_NOT_FOUND;
  LOG_DEBUG("Error in cmp_coroutine_system_init: Coroutines not supported on "
            "this platform\n");
  return rc;
}
/**
 * @brief cmp_coroutine_create
 *
 * @param out_co Parameter description.
 * @param stack_size Parameter description.
 * @param fn Parameter description.
 * @param arg Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  int rc = CMP_ERROR_NOT_FOUND;
  (void)out_co;
  (void)stack_size;
  (void)fn;
  (void)arg;
  LOG_DEBUG("Error in cmp_coroutine_create: Coroutines not supported on this "
            "platform\n");
  return rc;
}
/**
 * @brief cmp_coroutine_resume
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_resume(cmp_coroutine_t *co) {
  int rc = CMP_ERROR_NOT_FOUND;
  (void)co;
  LOG_DEBUG("Error in cmp_coroutine_resume: Coroutines not supported on this "
            "platform\n");
  return rc;
}
/**
 * @brief cmp_coroutine_yield
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_yield(cmp_coroutine_t *co) {
  int rc = CMP_ERROR_NOT_FOUND;
  (void)co;
  LOG_DEBUG("Error in cmp_coroutine_yield: Coroutines not supported on this "
            "platform\n");
  return rc;
}
/**
 * @brief cmp_coroutine_destroy
 *
 * @param co Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_destroy(cmp_coroutine_t *co) {
  int rc = CMP_ERROR_NOT_FOUND;
  (void)co;
  LOG_DEBUG("Error in cmp_coroutine_destroy: Coroutines not supported on this "
            "platform\n");
  return rc;
}

#endif

#endif
