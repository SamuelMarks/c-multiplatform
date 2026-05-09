/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdlib.h>

#if defined(CMP_OS_DOS) || defined(__WATCOMC__) || defined(__DOS__) || defined(__APPLE__)
/**
 * @brief cmp_coroutine_system_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_init(void) {
int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_coroutine_system_init: Stub on DOS\n");
  return rc;
}
/**
 * @brief cmp_coroutine_system_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_shutdown(void) {
int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_coroutine_system_shutdown: Stub on DOS\n");
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
  LOG_DEBUG("cmp_coroutine_create: Stub on DOS\n");
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
  LOG_DEBUG("cmp_coroutine_resume: Stub on DOS\n");
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
  LOG_DEBUG("cmp_coroutine_yield: Stub on DOS\n");
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
  LOG_DEBUG("cmp_coroutine_destroy: Stub on DOS\n");
  return rc;
}
#else
#if defined(__APPLE__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600
#endif

#if defined(_WIN32)
__declspec(dllimport) void *__stdcall ConvertThreadToFiber(void *lpParameter);
__declspec(dllimport) void *__stdcall CreateFiber(size_t dwStackSize, void (__stdcall *lpStartAddress)(void *), 
void *lpParameter);
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
  int rc;

  co->state = CMP_CORO_RUNNING;
  co->fn(co, co->arg);
  co->state = CMP_CORO_FINISHED;

  /* Switch back to the caller fiber */
  if (co->caller != NULL) {
    rc = cmp_tls_set(g_coro_system_key, co->caller);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_fiber_entry: cmp_tls_set failed\n");
    }
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
      LOG_DEBUG("cmp_coroutine_system_init: Failed to create TLS key\n");
      return rc;
    }

    /* Convert current thread to a fiber if it isn't one already */
    if (GetCurrentFiber() ==
        (void *)0x1E00) { /* Magic value for non-fiber on some Windows */
      main_fiber = ConvertThreadToFiber(NULL);
      if (main_fiber == NULL && GetLastError() != 0 &&
          GetLastError() != 1300 /* ERROR_ALREADY_FIBER */) {
        rc = CMP_ERROR_INVALID_ARG;
        LOG_DEBUG("cmp_coroutine_system_init: ConvertThreadToFiber failed\n");
        return rc;
      }
    }

    g_coro_system_initialized = 1;
  }

  LOG_DEBUG("cmp_coroutine_system_init: Thread initialized as Fiber\n");
  return rc;
}

/**
 * @brief cmp_coroutine_system_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_shutdown(void) {
  int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_coroutine_system_shutdown: Teardown skipped on Windows Fiber "
            "mock\n");
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
  int free_rc;

  if (out_co == NULL || fn == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_coroutine_create: Invalid argument\n");
    return rc;
  }

  if (!g_coro_system_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_coroutine_create: System not initialized\n");
    return rc; /* System not initialized on this thread */
  }

  if (stack_size == 0) {
    stack_size = 1024 * 1024; /* 1MB default */
  }

  rc = CMP_MALLOC(sizeof(cmp_coroutine_t), (void **)&co);
  if (rc != CMP_SUCCESS || co == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_coroutine_create: Out of memory allocating coroutine\n");
    return rc;
  }

  co->state = CMP_CORO_READY;
  co->fn = fn;
  co->arg = arg;
  co->stack = NULL; /* Not used for Fibers */
  co->stack_size = stack_size;
  co->caller = NULL;

  co->context = CreateFiber(stack_size, cmp_fiber_entry, co);
  if (co->context == NULL) {
    free_rc = CMP_FREE(co);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE failed during recovery\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_coroutine_create: CreateFiber failed\n");
    return rc;
  }

  *out_co = co;
  LOG_DEBUG("cmp_coroutine_create: Successfully created fiber coroutine\n");
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
    LOG_DEBUG("cmp_coroutine_resume: Invalid argument or finished\n");
    return rc;
  }

  rc = cmp_tls_get(g_coro_system_key, (void **)&current_co);
  if (rc != CMP_SUCCESS) {
    current_co = NULL;
    rc = CMP_SUCCESS; /* We will just assume it's the main fiber if TLS isn't
                         set yet */
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

    rc = cmp_tls_set(g_coro_system_key, co);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_resume: cmp_tls_set failed\n");
      return rc;
    }
    SwitchToFiber(co->context);

    /* When we return here, we are back in the caller */
    rc = cmp_tls_set(g_coro_system_key, current_co);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_resume: cmp_tls_set restore failed\n");
      /* Restore TLS failure shouldn't necessarily crash resume entirely */
      rc = CMP_SUCCESS;
    }
  }

  LOG_DEBUG("cmp_coroutine_resume: Resumed execution cleanly\n");
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
    LOG_DEBUG("cmp_coroutine_yield: Invalid argument\n");
    return rc;
  }

  co->state = CMP_CORO_SUSPENDED;
  rc = cmp_tls_set(g_coro_system_key, co->caller);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_coroutine_yield: cmp_tls_set failed\n");
    return rc;
  }
  SwitchToFiber(co->caller->context);

  LOG_DEBUG("cmp_coroutine_yield: Yielded fiber cleanly\n");
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
    LOG_DEBUG("cmp_coroutine_destroy: Invalid argument (co=NULL)\n");
    return rc;
  }

  if (co->context != NULL) {
    DeleteFiber(co->context);
    co->context = NULL;
  }

  rc = CMP_FREE(co);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_coroutine_destroy: CMP_FREE failed\n");
    return rc;
  }

  LOG_DEBUG("cmp_coroutine_destroy: Successfully destroyed fiber\n");
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
  int rc;

  co->state = CMP_CORO_RUNNING;
  co->fn(co, co->arg);
  co->state = CMP_CORO_FINISHED;

  if (co->caller != NULL) {
    rc = cmp_tls_set(g_coro_system_key, co->caller);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_ucontext_entry: cmp_tls_set failed\n");
    }
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
      LOG_DEBUG("cmp_coroutine_system_init: Failed to create TLS key\n");
      return rc;
    }
    g_coro_system_initialized = 1;
  }
  LOG_DEBUG("cmp_coroutine_system_init: System init successful\n");
  return rc;
}

/**
 * @brief cmp_coroutine_system_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_shutdown(void) {
  int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_coroutine_system_shutdown: System shutdown successful\n");
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
  int free_rc;

  if (out_co == NULL || fn == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_coroutine_create: Invalid argument\n");
    return rc;
  }

  if (!g_coro_system_initialized) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_coroutine_create: System not initialized\n");
    return rc;
  }

  if (stack_size == 0) {
    stack_size = 1024 * 1024;
  }

  rc = CMP_MALLOC(sizeof(cmp_coroutine_t), (void **)&co);
  if (rc != CMP_SUCCESS || co == NULL) {
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_coroutine_create: Out of memory allocating coroutine\n");
    return rc;
  }

  rc = CMP_MALLOC(sizeof(ucontext_t), (void **)&co->context);
  if (rc != CMP_SUCCESS || co->context == NULL) {
    free_rc = CMP_FREE(co);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_coroutine_create: Out of memory allocating ucontext\n");
    return rc;
  }

  rc = CMP_MALLOC(stack_size, (void **)&co->stack);
  if (rc != CMP_SUCCESS || co->stack == NULL) {
    free_rc = CMP_FREE(co->context);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE ctx failed\n");
    }
    free_rc = CMP_FREE(co);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE co failed\n");
    }
    rc = CMP_ERROR_OOM;
    LOG_DEBUG("cmp_coroutine_create: Out of memory allocating stack\n");
    return rc;
  }

  co->state = CMP_CORO_READY;
  co->fn = fn;
  co->arg = arg;
  co->stack_size = stack_size;
  co->caller = NULL;

  uc = (ucontext_t *)co->context;
  if (getcontext(uc) != 0) {
    free_rc = CMP_FREE(co->stack);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE stack failed\n");
    }
    free_rc = CMP_FREE(co->context);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE ctx failed\n");
    }
    free_rc = CMP_FREE(co);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_create: CMP_FREE co failed\n");
    }
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_coroutine_create: getcontext failed\n");
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
  LOG_DEBUG("cmp_coroutine_create: Successfully created ucontext coroutine\n");
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
    LOG_DEBUG("cmp_coroutine_resume: Invalid argument or finished\n");
    return rc;
  }

  rc = cmp_tls_get(g_coro_system_key, (void **)&current_co);
  if (rc != CMP_SUCCESS) {
    current_co = NULL;
    rc = CMP_SUCCESS;
  }

  if (current_co == NULL) {
    cmp_coroutine_t main_co;
    main_co.context = &main_ctx;
    co->caller = &main_co;
    rc = cmp_tls_set(g_coro_system_key, co);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_resume: cmp_tls_set failed\n");
      return rc;
    }
    swapcontext(&main_ctx, (ucontext_t *)co->context);
    rc = cmp_tls_set(g_coro_system_key, NULL);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_resume: cmp_tls_set NULL failed\n");
      rc = CMP_SUCCESS;
    }
  } else {
    co->caller = current_co;
    rc = cmp_tls_set(g_coro_system_key, co);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_resume: cmp_tls_set failed\n");
      return rc;
    }
    swapcontext((ucontext_t *)current_co->context, (ucontext_t *)co->context);
    rc = cmp_tls_set(g_coro_system_key, current_co);
    if (rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_resume: cmp_tls_set restore failed\n");
      rc = CMP_SUCCESS;
    }
  }

  LOG_DEBUG("cmp_coroutine_resume: Successfully resumed execution\n");
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
    LOG_DEBUG("cmp_coroutine_yield: Invalid argument\n");
    return rc;
  }

  co->state = CMP_CORO_SUSPENDED;
  rc = cmp_tls_set(g_coro_system_key, co->caller);
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_coroutine_yield: cmp_tls_set failed\n");
    return rc;
  }
  swapcontext((ucontext_t *)co->context, (ucontext_t *)co->caller->context);

  LOG_DEBUG("cmp_coroutine_yield: Yielded ucontext cleanly\n");
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
  int free_rc;

  if (co == NULL) {
    rc = CMP_ERROR_INVALID_ARG;
    LOG_DEBUG("cmp_coroutine_destroy: Invalid argument (co=NULL)\n");
    return rc;
  }

  if (co->stack != NULL) {
    free_rc = CMP_FREE(co->stack);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_destroy: CMP_FREE stack failed\n");
    }
    co->stack = NULL;
  }

  if (co->context != NULL) {
    free_rc = CMP_FREE(co->context);
    if (free_rc != CMP_SUCCESS) {
      LOG_DEBUG("cmp_coroutine_destroy: CMP_FREE context failed\n");
    }
    co->context = NULL;
  }

  free_rc = CMP_FREE(co);
  if (free_rc != CMP_SUCCESS) {
    LOG_DEBUG("cmp_coroutine_destroy: CMP_FREE co failed\n");
    rc = free_rc;
  }

  LOG_DEBUG("cmp_coroutine_destroy: Successfully destroyed ucontext\n");
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
  LOG_DEBUG(
      "cmp_coroutine_system_init: Coroutines not supported on this platform\n");
  return rc;
}
/**
 * @brief cmp_coroutine_system_shutdown
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_coroutine_system_shutdown(void) {
  int rc = CMP_SUCCESS;
  LOG_DEBUG("cmp_coroutine_system_shutdown: Coroutines not supported on this "
            "platform\n");
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
  LOG_DEBUG(
      "cmp_coroutine_create: Coroutines not supported on this platform\n");
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
  LOG_DEBUG(
      "cmp_coroutine_resume: Coroutines not supported on this platform\n");
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
  LOG_DEBUG("cmp_coroutine_yield: Coroutines not supported on this platform\n");
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
  LOG_DEBUG(
      "cmp_coroutine_destroy: Coroutines not supported on this platform\n");
  return rc;
}
#endif

#endif
