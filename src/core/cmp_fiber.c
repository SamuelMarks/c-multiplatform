/* clang-format off */
#include "cmpc/cmp_fiber.h"

#include <stdlib.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <ucontext.h>
#endif
/* clang-format on */

struct CMPFiber {
  CMPFiberScheduler *scheduler;
  CMPFiberRoutine routine;
  void *arg;
  cmp_usize stack_size;
  void *stack;
  CMPBool is_finished;
  struct CMPFiber *next;
  struct CMPFiber *prev;

#if defined(_WIN32)
  LPVOID platform_fiber;
#else
  ucontext_t context;
#endif
};

struct CMPFiberScheduler {
  CMPAllocator allocator;
  CMPFiber *active_fibers;
  CMPFiber *current_fiber;
#if defined(_WIN32)
  LPVOID main_fiber;
#else
  ucontext_t main_context;
#endif
};

#if defined(_WIN32)
static void WINAPI fiber_startup(LPVOID param) {
  CMPFiber *fiber = (CMPFiber *)param;
  fiber->routine(fiber->arg);
  fiber->is_finished = 1;
  SwitchToFiber(fiber->scheduler->main_fiber);
}
#else
static void fiber_startup(void) {
  /* Using a global is unsafe if schedulers run on multiple OS threads.
   * This is a simplified C89 ucontext implementation for MVP. */
}
#endif

/* Hack for C89 to pass fiber to ucontext without TLS/globals if possible, but
 * makecontext only takes int. We'll use a hack where the scheduler runs and
 * assumes the current fiber. For a production cross-platform lib,
 * Boost.Context/libco assembly is better. This provides the ucontext MVP. */
#if !defined(_WIN32)
/* We use thread-local storage for the current running fiber if available, but
 * C89 doesn't have it natively. For now, rely on a static global which makes
 * this implementation single-threaded only for the scheduler. */
static CMPFiber *g_current_fiber = NULL;

static void ucontext_startup(void) {
  CMPFiber *fiber = g_current_fiber;
  if (fiber != NULL) {
    fiber->routine(fiber->arg);
    fiber->is_finished = 1;
    swapcontext(&fiber->context, &fiber->scheduler->main_context);
  }
}
#endif

#if defined(_MSC_VER)
static __declspec(thread) CMPFiberScheduler *g_thread_scheduler = NULL;
#elif defined(__GNUC__) || defined(__clang__)
static __thread CMPFiberScheduler *g_thread_scheduler = NULL;
#else
/* Fallback global for C89 non-windows */
static CMPFiberScheduler *g_thread_scheduler = NULL;
#endif

CMP_API int CMP_CALL cmp_fiber_scheduler_create(
    CMPAllocator *alloc, CMPFiberScheduler **out_scheduler) {
  CMPFiberScheduler *sched;
  int rc;

  if (alloc == NULL || alloc->alloc == NULL || out_scheduler == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPFiberScheduler), (void **)&sched);
  if (rc != CMP_OK || sched == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  sched->allocator = *alloc;
  sched->active_fibers = NULL;
  sched->current_fiber = NULL;

#if defined(_WIN32)
  sched->main_fiber = ConvertThreadToFiber(NULL);
  if (sched->main_fiber == NULL) {
    alloc->free(alloc->ctx, sched);
    return CMP_ERR_UNKNOWN;
  }
#endif

  g_thread_scheduler = sched;
  *out_scheduler = sched;
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_fiber_scheduler_destroy(CMPAllocator *alloc,
                                                 CMPFiberScheduler *scheduler) {
  CMPFiber *current;
  CMPFiber *next;

  if (alloc == NULL || alloc->free == NULL || scheduler == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  current = scheduler->active_fibers;
  while (current != NULL) {
    next = current->next;
#if defined(_WIN32)
    DeleteFiber(current->platform_fiber);
#else
    alloc->free(alloc->ctx, current->stack);
#endif
    alloc->free(alloc->ctx, current);
    current = next;
  }

#if defined(_WIN32)
  if (scheduler->main_fiber != NULL) {
    /* Cannot easily un-convert thread if it wasn't a fiber before, but for
     * clean exit we often just leave it. Windows 2000+ has ConvertFiberToThread
     */
    ConvertFiberToThread();
  }
#endif

  if (g_thread_scheduler == scheduler) {
    g_thread_scheduler = NULL;
  }
  alloc->free(alloc->ctx, scheduler);
  return CMP_OK;
}

CMP_API int CMP_CALL cmp_fiber_spawn(CMPFiberScheduler *scheduler,
                                     const CMPFiberConfig *config,
                                     CMPFiber **out_fiber) {
  CMPFiber *fiber;
  int rc;
  cmp_usize stack_size;

  if (scheduler == NULL || config == NULL || config->routine == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  stack_size = config->stack_size > 0 ? config->stack_size : 65536;

  rc = scheduler->allocator.alloc(scheduler->allocator.ctx, sizeof(CMPFiber),
                                  (void **)&fiber);
  if (rc != CMP_OK || fiber == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  fiber->scheduler = scheduler;
  fiber->routine = config->routine;
  fiber->arg = config->arg;
  fiber->stack_size = stack_size;
  fiber->is_finished = 0;

#if defined(_WIN32)
  fiber->stack = NULL; /* Windows manages the stack */
  fiber->platform_fiber = CreateFiber(stack_size, fiber_startup, fiber);
  if (fiber->platform_fiber == NULL) {
    scheduler->allocator.free(scheduler->allocator.ctx, fiber);
    return CMP_ERR_OUT_OF_MEMORY;
  }
#else
  rc = scheduler->allocator.alloc(scheduler->allocator.ctx, stack_size,
                                  &fiber->stack);
  if (rc != CMP_OK || fiber->stack == NULL) {
    scheduler->allocator.free(scheduler->allocator.ctx, fiber);
    return CMP_ERR_OUT_OF_MEMORY;
  }

  if (getcontext(&fiber->context) != 0) {
    scheduler->allocator.free(scheduler->allocator.ctx, fiber->stack);
    scheduler->allocator.free(scheduler->allocator.ctx, fiber);
    return CMP_ERR_UNKNOWN;
  }

  fiber->context.uc_stack.ss_sp = fiber->stack;
  fiber->context.uc_stack.ss_size = stack_size;
  fiber->context.uc_link = &scheduler->main_context;
  makecontext(&fiber->context, ucontext_startup, 0);
#endif

  /* Add to queue */
  fiber->prev = NULL;
  fiber->next = scheduler->active_fibers;
  if (scheduler->active_fibers != NULL) {
    scheduler->active_fibers->prev = fiber;
  }
  scheduler->active_fibers = fiber;

  if (out_fiber != NULL) {
    *out_fiber = fiber;
  }

  return CMP_OK;
}

CMP_API void CMP_CALL cmp_fiber_yield(void) {
  CMPFiberScheduler *sched = g_thread_scheduler;
  if (sched == NULL || sched->current_fiber == NULL) {
    return; /* Not in a fiber */
  }

#if defined(_WIN32)
  SwitchToFiber(sched->main_fiber);
#else
  swapcontext(&sched->current_fiber->context, &sched->main_context);
#endif
}

CMP_API int CMP_CALL
cmp_fiber_scheduler_run_pass(CMPFiberScheduler *scheduler) {
  CMPFiber *current;
  CMPFiber *next;
  int count = 0;

  if (scheduler == NULL) {
    return -1;
  }

  current = scheduler->active_fibers;
  while (current != NULL) {
    next = current->next;

    if (!current->is_finished) {
      scheduler->current_fiber = current;
#if defined(_WIN32)
      SwitchToFiber(current->platform_fiber);
#else
      g_current_fiber = current;
      swapcontext(&scheduler->main_context, &current->context);
      g_current_fiber = NULL;
#endif
      scheduler->current_fiber = NULL;
      count++;
    }

    if (current->is_finished) {
      /* Remove from list */
      if (current->prev != NULL) {
        current->prev->next = current->next;
      } else {
        scheduler->active_fibers = current->next;
      }
      if (current->next != NULL) {
        current->next->prev = current->prev;
      }

      /* Free */
#if defined(_WIN32)
      DeleteFiber(current->platform_fiber);
#else
      scheduler->allocator.free(scheduler->allocator.ctx, current->stack);
#endif
      scheduler->allocator.free(scheduler->allocator.ctx, current);
    }

    current = next;
  }

  return count;
}

#include "cmpc/cmp_fs.h"

#if defined(_WIN32)
#else
#include <sys/time.h>
#endif

static cmp_u32 get_current_time_ms(void) {
#if defined(_WIN32)
  return GetTickCount();
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (cmp_u32)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
#endif
}

CMP_API void CMP_CALL cmp_fiber_sleep(cmp_u32 ms) {
  cmp_u32 start = get_current_time_ms();
  while (get_current_time_ms() - start < ms) {
    cmp_fiber_yield();
  }
}

typedef struct FiberFSReadCtx {
  int is_done;
  void *data;
  cmp_usize size;
  int status;
} FiberFSReadCtx;

static void CMP_CALL fiber_fs_read_cb(void *ctx, const char *utf8_path,
                                      void *data, cmp_usize size, int status,
                                      void *user) {
  FiberFSReadCtx *fctx = (FiberFSReadCtx *)user;
  CMP_UNUSED(ctx);
  CMP_UNUSED(utf8_path);

  if (fctx != NULL) {
    fctx->data = data;
    fctx->size = size;
    fctx->status = status;
    fctx->is_done = 1;
  } else {
    /* Shouldn't happen but free memory if it does */
    CMPAllocator alloc;
    if (cmp_get_default_allocator(&alloc) == CMP_OK) {
      alloc.free(alloc.ctx, data);
    }
  }
}

CMP_API int CMP_CALL cmp_fiber_fs_read(CMPEnv *env, CMPTasks *tasks,
                                       CMPEventLoop *loop,
                                       const CMPAllocator *alloc,
                                       const char *utf8_path, void **out_data,
                                       cmp_usize *out_size) {
  FiberFSReadCtx fctx;
  int rc;

  fctx.is_done = 0;
  fctx.data = NULL;
  fctx.size = 0;
  fctx.status = CMP_ERR_UNKNOWN;

  rc = cmp_fs_read_async(env, tasks, loop, alloc, utf8_path, fiber_fs_read_cb,
                         &fctx);
  if (rc != CMP_OK) {
    return rc;
  }

  while (!fctx.is_done) {
    cmp_fiber_yield();
  }

  if (fctx.status == CMP_OK) {
    if (out_data != NULL) {
      *out_data = fctx.data;
    } else {
      alloc->free(alloc->ctx, fctx.data);
    }
    if (out_size != NULL) {
      *out_size = fctx.size;
    }
  } else {
    if (fctx.data != NULL) {
      alloc->free(alloc->ctx, fctx.data);
    }
  }

  return fctx.status;
}