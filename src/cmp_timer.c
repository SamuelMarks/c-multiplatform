/* clang-format off */
#include "cmp.h"
#include <stdlib.h>

#if defined(_WIN32)
__declspec(dllimport) void *__stdcall CreateThread(void *lpThreadAttributes, size_t dwStackSize, unsigned long (__stdcall *lpStartAddress)(void *), void *lpParameter, unsigned long dwCreationFlags, unsigned long *lpThreadId);
__declspec(dllimport) unsigned long __stdcall WaitForSingleObject(void *hHandle, unsigned long dwMilliseconds);
__declspec(dllimport) int __stdcall CloseHandle(void *hObject);
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#else
#include <unistd.h>
#endif
/* clang-format on */

struct cmp_timer {
  unsigned int interval_ms;
  int repeat;
  cmp_task_fn_t fn;
  void *arg;
  int is_running;
  cmp_thread_t thread;
};

static int g_timer_system_initialized = 0;

int cmp_timer_system_init(void) {
  if (g_timer_system_initialized) {
    return CMP_SUCCESS;
  }
  g_timer_system_initialized = 1;
  return CMP_SUCCESS;
}

int cmp_timer_system_shutdown(void) {
  if (!g_timer_system_initialized) {
    return CMP_SUCCESS;
  }
  g_timer_system_initialized = 0;
  return CMP_SUCCESS;
}

#if defined(_WIN32)
static unsigned long __stdcall cmp_timer_thread_func(void *arg) {
#else
static void *cmp_timer_thread_func(void *arg) {
#endif
  cmp_timer_t *timer = (cmp_timer_t *)arg;

  while (timer->is_running) {
#if defined(_WIN32)
    Sleep(timer->interval_ms);
#else
    usleep(timer->interval_ms * 1000);
#endif

    if (!timer->is_running) {
      break;
    }

    timer->fn(timer->arg);

    if (!timer->repeat) {
      timer->is_running = 0;
      break;
    }
  }

#if defined(_WIN32)
  return 0;
#else
  return NULL;
#endif
}

int cmp_timer_start(cmp_timer_t **out_timer, unsigned int interval_ms,
                    int repeat, cmp_task_fn_t fn, void *arg) {
  cmp_timer_t *timer;
  int res;

  if (out_timer == NULL || fn == NULL || interval_ms == 0) {
    return CMP_ERROR_INVALID_ARG;
  }

  if (!g_timer_system_initialized) {
    return CMP_ERROR_INVALID_ARG;
  }

  res = CMP_MALLOC(sizeof(struct cmp_timer), (void **)&timer);
  if (res != CMP_SUCCESS || timer == NULL) {
    return CMP_ERROR_OOM;
  }

  timer->interval_ms = interval_ms;
  timer->repeat = repeat;
  timer->fn = fn;
  timer->arg = arg;
  timer->is_running = 1;

#if defined(_WIN32)
  timer->thread = CreateThread(NULL, 0, cmp_timer_thread_func, timer, 0, NULL);
  if (timer->thread == NULL) {
    CMP_FREE(timer);
    return CMP_ERROR_OOM;
  }
#else
  if (pthread_create(&timer->thread, NULL, cmp_timer_thread_func, timer) != 0) {
    CMP_FREE(timer);
    return CMP_ERROR_OOM;
  }
#endif

  *out_timer = timer;
  return CMP_SUCCESS;
}

int cmp_timer_stop(cmp_timer_t *timer) {
  if (timer == NULL) {
    return CMP_ERROR_INVALID_ARG;
  }

  timer->is_running = 0;

#if defined(_WIN32)
  WaitForSingleObject(timer->thread, 0xFFFFFFFF);
  CloseHandle(timer->thread);
#else
  pthread_join(timer->thread, NULL);
#endif

  CMP_FREE(timer);

  return CMP_SUCCESS;
}
