/* clang-format off */
#include "cmpc/cmp_core.h"

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <time.h>
typedef CRITICAL_SECTION cmp_native_mutex_t;
#else
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
typedef pthread_mutex_t cmp_native_mutex_t;
#endif
/* clang-format on */

typedef struct CMPAsyncTimer {
  CMPEventLoopTimerHandle handle;
  cmp_u32 timeout_ms;
  cmp_u32 next_fire_ms;
  CMPBool repeat;
  CMPEventLoopTimerCallback cb;
  void *cb_ctx;
  struct CMPAsyncTimer *next;
} CMPAsyncTimer;

typedef struct CMPAsyncIO {
  CMPEventLoopIOHandle handle;
  int fd;
  cmp_u32 flags;
  CMPEventLoopIOCallback cb;
  void *cb_ctx;
  struct CMPAsyncIO *next;
} CMPAsyncIO;

typedef struct CMPAsyncWakeItem {
  CMPEventLoopWakeCallback cb;
  void *cb_ctx;
  struct CMPAsyncWakeItem *next;
} CMPAsyncWakeItem;

typedef struct CMPAsyncLoopState {
  CMPAsyncEventLoopConfig config;
  CMPAllocator allocator;
  CMPBool running;
  CMPAsyncTimer *timers;
  CMPAsyncIO *ios;
  cmp_u32 next_timer_id;
  cmp_u32 next_io_id;

  cmp_native_mutex_t wake_mutex;
  CMPAsyncWakeItem *wake_queue_head;
  CMPAsyncWakeItem *wake_queue_tail;
#if defined(_WIN32)
  HANDLE wake_event;
#else
  int wake_fds[2];
#endif
} CMPAsyncLoopState;

static cmp_u32 get_current_time_ms(void) {
#if defined(_WIN32)
  return GetTickCount();
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (cmp_u32)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
#endif
}

static int CMP_CALL async_loop_run(void *ctx) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;
  int rc = CMP_OK;
  CMPBool cont = 1;
  cmp_u32 now;
  cmp_u32 wait_time;
  CMPAsyncTimer *timer;
  CMPAsyncTimer *prev_timer;
  CMPAsyncTimer *next_timer;
  CMPAsyncWakeItem *wake_list;

#if defined(_WIN32)
  /* We just poll for now, real select() logic needs actual sockets */
#else
  fd_set read_fds, write_fds;
  int max_fd;
  struct timeval tv;
  CMPAsyncIO *io;
#endif

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state->running = 1;
  while (state->running) {
    /* 1. Process Wake Queue */
    wake_list = NULL;
#if defined(_WIN32)
    EnterCriticalSection(&state->wake_mutex);
    wake_list = state->wake_queue_head;
    state->wake_queue_head = NULL;
    state->wake_queue_tail = NULL;
    LeaveCriticalSection(&state->wake_mutex);
#else
    pthread_mutex_lock(&state->wake_mutex);
    wake_list = state->wake_queue_head;
    state->wake_queue_head = NULL;
    state->wake_queue_tail = NULL;
    if (state->wake_fds[0] >= 0) {
      char buf[64];
      while (read(state->wake_fds[0], buf, sizeof(buf)) > 0) {
      }
    }
    pthread_mutex_unlock(&state->wake_mutex);
#endif

    while (wake_list != NULL) {
      CMPAsyncWakeItem *next = wake_list->next;
      if (wake_list->cb != NULL) {
        wake_list->cb(wake_list->cb_ctx);
      }
      state->allocator.free(state->allocator.ctx, wake_list);
      wake_list = next;
    }

    /* 2. Poll UI Events */
    if (state->config.on_poll_ui != NULL) {
      cont = 1;
      rc = state->config.on_poll_ui(state->config.ctx, &cont);
      if (rc != CMP_OK || !cont) {
        state->running = 0;
        break;
      }
    }

    now = get_current_time_ms();
    wait_time = 16; /* Cap to 16ms (approx 60 FPS) to keep UI responsive */

    /* Check timers to adjust wait_time */
    for (timer = state->timers; timer != NULL; timer = timer->next) {
      if (now >= timer->next_fire_ms) {
        wait_time = 0;
        break;
      } else {
        cmp_u32 diff = timer->next_fire_ms - now;
        if (diff < wait_time) {
          wait_time = diff;
        }
      }
    }

    /* 3. Wait for IO / Timeout / Wake */
#if defined(_WIN32)
    if (wait_time > 0) {
      WaitForSingleObject(state->wake_event, wait_time);
    }
#else
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    max_fd = -1;

    for (io = state->ios; io != NULL; io = io->next) {
      if ((io->flags & CMP_EVENT_LOOP_IO_READ) != 0) {
        FD_SET(io->fd, &read_fds);
        if (io->fd > max_fd)
          max_fd = io->fd;
      }
      if ((io->flags & CMP_EVENT_LOOP_IO_WRITE) != 0) {
        FD_SET(io->fd, &write_fds);
        if (io->fd > max_fd)
          max_fd = io->fd;
      }
    }

    if (state->wake_fds[0] >= 0) {
      FD_SET(state->wake_fds[0], &read_fds);
      if (state->wake_fds[0] > max_fd)
        max_fd = state->wake_fds[0];
    }

    tv.tv_sec = wait_time / 1000;
    tv.tv_usec = (wait_time % 1000) * 1000;

    if (max_fd >= 0) {
      select(max_fd + 1, &read_fds, &write_fds, NULL, &tv);
    } else {
      if (wait_time > 0) {
        usleep(wait_time * 1000);
      }
    }
#endif

    /* 4. Process Timers */
    now = get_current_time_ms();
    prev_timer = NULL;
    timer = state->timers;
    while (timer != NULL) {
      next_timer = timer->next;
      if (now >= timer->next_fire_ms) {
        /* Fire */
        if (timer->cb != NULL) {
          timer->cb(timer->cb_ctx, timer->handle);
        }

        if (timer->repeat) {
          timer->next_fire_ms = now + timer->timeout_ms;
          prev_timer = timer;
        } else {
          /* Remove timer */
          if (prev_timer == NULL) {
            state->timers = next_timer;
          } else {
            prev_timer->next = next_timer;
          }
          state->allocator.free(state->allocator.ctx, timer);
        }
      } else {
        prev_timer = timer;
      }
      timer = next_timer;
    }

    /* 5. Process IO */
#if !defined(_WIN32)
    for (io = state->ios; io != NULL; io = io->next) {
      if ((io->flags & CMP_EVENT_LOOP_IO_READ) != 0 &&
          FD_ISSET(io->fd, &read_fds)) {
        if (io->cb != NULL)
          io->cb(io->cb_ctx, io->handle);
      }
      if ((io->flags & CMP_EVENT_LOOP_IO_WRITE) != 0 &&
          FD_ISSET(io->fd, &write_fds)) {
        if (io->cb != NULL)
          io->cb(io->cb_ctx, io->handle);
      }
    }
#endif
  }

  return rc;
}

static int CMP_CALL async_loop_stop(void *ctx) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state->running = 0;
#if defined(_WIN32)
  SetEvent(state->wake_event);
#else
  if (state->wake_fds[1] >= 0) {
    char c = 's';
    write(state->wake_fds[1], &c, 1);
  }
#endif

  return CMP_OK;
}

static int CMP_CALL async_loop_add_io(void *ctx, int fd, cmp_u32 flags,
                                      CMPEventLoopIOCallback cb, void *cb_ctx,
                                      CMPEventLoopIOHandle *out_handle) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;
  CMPAsyncIO *io;
  int rc;

  if (state == NULL || out_handle == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = state->allocator.alloc(state->allocator.ctx, sizeof(CMPAsyncIO),
                              (void **)&io);
  if (rc != CMP_OK || io == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  state->next_io_id++;
  io->handle = (CMPEventLoopIOHandle)(cmp_usize)state->next_io_id;
  io->fd = fd;
  io->flags = flags;
  io->cb = cb;
  io->cb_ctx = cb_ctx;
  io->next = state->ios;
  state->ios = io;

  *out_handle = io->handle;
  return CMP_OK;
}

static int CMP_CALL async_loop_remove_io(void *ctx,
                                         CMPEventLoopIOHandle handle) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;
  CMPAsyncIO *io, *prev;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  prev = NULL;
  for (io = state->ios; io != NULL; io = io->next) {
    if (io->handle == handle) {
      if (prev == NULL) {
        state->ios = io->next;
      } else {
        prev->next = io->next;
      }
      state->allocator.free(state->allocator.ctx, io);
      return CMP_OK;
    }
    prev = io;
  }

  return CMP_ERR_NOT_FOUND;
}

static int CMP_CALL async_loop_set_timer(void *ctx, cmp_u32 timeout_ms,
                                         CMPBool repeat,
                                         CMPEventLoopTimerCallback cb,
                                         void *cb_ctx,
                                         CMPEventLoopTimerHandle *out_handle) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;
  CMPAsyncTimer *timer;
  int rc;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = state->allocator.alloc(state->allocator.ctx, sizeof(CMPAsyncTimer),
                              (void **)&timer);
  if (rc != CMP_OK || timer == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  state->next_timer_id++;
  timer->handle = (CMPEventLoopTimerHandle)(cmp_usize)state->next_timer_id;
  timer->timeout_ms = timeout_ms;
  timer->next_fire_ms = get_current_time_ms() + timeout_ms;
  timer->repeat = repeat;
  timer->cb = cb;
  timer->cb_ctx = cb_ctx;
  timer->next = state->timers;
  state->timers = timer;

  if (out_handle != NULL) {
    *out_handle = timer->handle;
  }

  return CMP_OK;
}

static int CMP_CALL async_loop_clear_timer(void *ctx,
                                           CMPEventLoopTimerHandle handle) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;
  CMPAsyncTimer *timer, *prev;

  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  prev = NULL;
  for (timer = state->timers; timer != NULL; timer = timer->next) {
    if (timer->handle == handle) {
      if (prev == NULL) {
        state->timers = timer->next;
      } else {
        prev->next = timer->next;
      }
      state->allocator.free(state->allocator.ctx, timer);
      return CMP_OK;
    }
    prev = timer;
  }

  return CMP_ERR_NOT_FOUND;
}

static int CMP_CALL async_loop_wake(void *ctx, CMPEventLoopWakeCallback cb,
                                    void *cb_ctx) {
  CMPAsyncLoopState *state = (CMPAsyncLoopState *)ctx;
  CMPAsyncWakeItem *item;
  int rc;

  if (state == NULL || cb == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = state->allocator.alloc(state->allocator.ctx, sizeof(CMPAsyncWakeItem),
                              (void **)&item);
  if (rc != CMP_OK || item == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  item->cb = cb;
  item->cb_ctx = cb_ctx;
  item->next = NULL;

#if defined(_WIN32)
  EnterCriticalSection(&state->wake_mutex);
#else
  pthread_mutex_lock(&state->wake_mutex);
#endif

  if (state->wake_queue_tail == NULL) {
    state->wake_queue_head = item;
    state->wake_queue_tail = item;
  } else {
    state->wake_queue_tail->next = item;
    state->wake_queue_tail = item;
  }

#if defined(_WIN32)
  SetEvent(state->wake_event);
  LeaveCriticalSection(&state->wake_mutex);
#else
  if (state->wake_fds[1] >= 0) {
    char c = 'w';
    if (write(state->wake_fds[1], &c, 1) < 0) {
      /* ignore write error on non-blocking pipe if full */
    }
  }
  pthread_mutex_unlock(&state->wake_mutex);
#endif

  return CMP_OK;
}

static const CMPEventLoopVTable g_cmp_async_loop_vtable = {
    async_loop_run,       async_loop_stop,      async_loop_add_io,
    async_loop_remove_io, async_loop_set_timer, async_loop_clear_timer,
    async_loop_wake};

CMP_API int CMP_CALL cmp_event_loop_async_init(
    CMPAllocator *alloc, const CMPAsyncEventLoopConfig *config,
    CMPEventLoop *out_loop) {
  CMPAsyncLoopState *state;
  int rc;

  if (alloc == NULL || alloc->alloc == NULL || out_loop == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  rc = alloc->alloc(alloc->ctx, sizeof(CMPAsyncLoopState), (void **)&state);
  if (rc != CMP_OK || state == NULL) {
    return CMP_ERR_OUT_OF_MEMORY;
  }

  memset(state, 0, sizeof(*state));
  if (config != NULL) {
    state->config = *config;
  }
  state->allocator = *alloc;
  state->running = 0;

#if defined(_WIN32)
  InitializeCriticalSection(&state->wake_mutex);
  state->wake_event = CreateEventA(NULL, FALSE, FALSE, NULL);
#else
  pthread_mutex_init(&state->wake_mutex, NULL);
  if (pipe(state->wake_fds) == 0) {
    fcntl(state->wake_fds[0], F_SETFL,
          fcntl(state->wake_fds[0], F_GETFL) | O_NONBLOCK);
    fcntl(state->wake_fds[1], F_SETFL,
          fcntl(state->wake_fds[1], F_GETFL) | O_NONBLOCK);
  } else {
    state->wake_fds[0] = -1;
    state->wake_fds[1] = -1;
  }
#endif

  out_loop->ctx = state;
  out_loop->vtable = &g_cmp_async_loop_vtable;

  return CMP_OK;
}

CMP_API int CMP_CALL cmp_event_loop_async_destroy(CMPAllocator *alloc,
                                                  CMPEventLoop *loop) {
  CMPAsyncLoopState *state;
  CMPAsyncTimer *timer, *next_timer;
  CMPAsyncIO *io, *next_io;
  CMPAsyncWakeItem *wake_item, *next_wake;

  if (alloc == NULL || alloc->free == NULL || loop == NULL ||
      loop->vtable != &g_cmp_async_loop_vtable) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  state = (CMPAsyncLoopState *)loop->ctx;
  if (state == NULL) {
    return CMP_ERR_INVALID_ARGUMENT;
  }

  timer = state->timers;
  while (timer != NULL) {
    next_timer = timer->next;
    alloc->free(alloc->ctx, timer);
    timer = next_timer;
  }

  io = state->ios;
  while (io != NULL) {
    next_io = io->next;
    alloc->free(alloc->ctx, io);
    io = next_io;
  }

  wake_item = state->wake_queue_head;
  while (wake_item != NULL) {
    next_wake = wake_item->next;
    alloc->free(alloc->ctx, wake_item);
    wake_item = next_wake;
  }

#if defined(_WIN32)
  if (state->wake_event != NULL) {
    CloseHandle(state->wake_event);
  }
  DeleteCriticalSection(&state->wake_mutex);
#else
  if (state->wake_fds[0] >= 0) {
    close(state->wake_fds[0]);
  }
  if (state->wake_fds[1] >= 0) {
    close(state->wake_fds[1]);
  }
  pthread_mutex_destroy(&state->wake_mutex);
#endif

  alloc->free(alloc->ctx, state);
  loop->ctx = NULL;
  loop->vtable = NULL;

  return CMP_OK;
}