/* clang-format off */
#if defined(__APPLE__) && !defined(_XOPEN_SOURCE)
#define _XOPEN_SOURCE 600
#endif
#include "cmp.h"
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

int cmp_coroutine_system_init(void) {
  if (!g_coro_system_initialized) {
    if (cmp_tls_key_create(&g_coro_system_key) != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    g_coro_system_initialized = 1;
  }

  /* Convert current thread to a fiber if it isn't one already */
  if (GetCurrentFiber() ==
      (void *)0x1E00) { /* Magic value for non-fiber on some Windows */
    void *main_fiber = ConvertThreadToFiber(NULL);
    if (main_fiber == NULL && GetLastError() != 0) {
      return CMP_ERROR_INVALID_ARG;
    }
  }

  return CMP_SUCCESS;
}

int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  cmp_coroutine_t *co;
  int res;

  if (out_co == NULL || fn == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!g_coro_system_initialized) {
    return CMP_ERROR_INVALID_ARG; /* System not initialized on this thread */
  }

  if (stack_size == 0) {
    stack_size = 1024 * 1024; /* 1MB default */
  }

  res = CMP_MALLOC(sizeof(cmp_coroutine_t), (void **)&co);
  if (res != CMP_SUCCESS || co == NULL) {
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
    return CMP_ERROR_OOM;
  }

  *out_co = co;
  return CMP_SUCCESS;
}

int cmp_coroutine_resume(cmp_coroutine_t *co) {
  cmp_coroutine_t *current_co;

  if (co == NULL || co->state == CMP_CORO_FINISHED) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (cmp_tls_get(g_coro_system_key, (void **)&current_co) != CMP_SUCCESS) {
    current_co = NULL;
  }

  /* We don't have a cmp_coroutine_t for the main thread, so caller might be
     NULL, but we save the OS context anyway */
  if (current_co == NULL) {
    /* This is the main thread resuming a fiber for the first time */
    /* Note: We need a static/heap allocated main_co if we want it to persist
       across yields correctly, but since we only swap back directly to it from
       the fiber, a local stack variable works if we block */
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

  return CMP_SUCCESS;
}

int cmp_coroutine_yield(cmp_coroutine_t *co) {
  if (co == NULL || co->caller == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  co->state = CMP_CORO_SUSPENDED;
  cmp_tls_set(g_coro_system_key, co->caller);
  SwitchToFiber(co->caller->context);

  return CMP_SUCCESS;
}

int cmp_coroutine_destroy(cmp_coroutine_t *co) {
  if (co == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (co->context != NULL) {
    DeleteFiber(co->context);
    co->context = NULL;
  }

  CMP_FREE(co);
  return CMP_SUCCESS;
}

#else
/* POSIX ucontext implementation */

static cmp_tls_key_t g_coro_system_key;
static int g_coro_system_initialized = 0;

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

int cmp_coroutine_system_init(void) {
  if (!g_coro_system_initialized) {
    if (cmp_tls_key_create(&g_coro_system_key) != CMP_SUCCESS) {
      return CMP_ERROR_OOM;
    }
    g_coro_system_initialized = 1;
  }
  return CMP_SUCCESS;
}

int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  cmp_coroutine_t *co;
  ucontext_t *uc;
  uint64_t ptr_val;
  int res;

  if (out_co == NULL || fn == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!g_coro_system_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (stack_size == 0) {
    stack_size = 1024 * 1024;
  }

  res = CMP_MALLOC(sizeof(cmp_coroutine_t), (void **)&co);
  if (res != CMP_SUCCESS || co == NULL) {
    return CMP_ERROR_OOM;
  }

  res = CMP_MALLOC(sizeof(ucontext_t), (void **)&co->context);
  if (res != CMP_SUCCESS || co->context == NULL) {
    CMP_FREE(co);
    return CMP_ERROR_OOM;
  }

  res = CMP_MALLOC(stack_size, (void **)&co->stack);
  if (res != CMP_SUCCESS || co->stack == NULL) {
    CMP_FREE(co->context);
    CMP_FREE(co);
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
    return CMP_ERROR_INVALID_ARG;
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
  return CMP_SUCCESS;
}

int cmp_coroutine_resume(cmp_coroutine_t *co) {
  cmp_coroutine_t *current_co;
  ucontext_t main_ctx;

  if (co == NULL || co->state == CMP_CORO_FINISHED) {
    return CMP_ERROR_INVALID_ARG;
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

  return CMP_SUCCESS;
}

int cmp_coroutine_yield(cmp_coroutine_t *co) {
  if (co == NULL || co->caller == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  co->state = CMP_CORO_SUSPENDED;
  cmp_tls_set(g_coro_system_key, co->caller);
  swapcontext((ucontext_t *)co->context, (ucontext_t *)co->caller->context);

  return CMP_SUCCESS;
}

int cmp_coroutine_destroy(cmp_coroutine_t *co) {
  if (co == NULL) {
    return CMP_ERROR_INVALID_ARG;
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
  return CMP_SUCCESS;
}

#endif /* POSIX vs WIN32 */

#else /* CMP_CORO_SUPPORTED == 0 */

int cmp_coroutine_system_init(void) { return CMP_ERROR_NOT_FOUND; }
int cmp_coroutine_create(cmp_coroutine_t **out_co, size_t stack_size,
                         cmp_coroutine_fn_t fn, void *arg) {
  return CMP_ERROR_NOT_FOUND;
}
int cmp_coroutine_resume(cmp_coroutine_t *co) { return CMP_ERROR_NOT_FOUND; }
int cmp_coroutine_yield(cmp_coroutine_t *co) { return CMP_ERROR_NOT_FOUND; }
int cmp_coroutine_destroy(cmp_coroutine_t *co) { return CMP_ERROR_NOT_FOUND; }

#endif
