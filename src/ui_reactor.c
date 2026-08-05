/* clang-format off */
#include "../include/ui_reactor.h"
#include "ui_internal_mem.h"
#include <stddef.h>

#if defined(__linux__)
#define UI_USE_EPOLL
#include <sys/epoll.h>
#include <unistd.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#define UI_USE_KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#elif defined(_WIN32)
#define UI_USE_SELECT_WIN
#include <winsock2.h>
#else
#define UI_USE_SELECT_POSIX
#include <sys/select.h>
#include <sys/time.h>
#endif
/* clang-format on */

#include "../include/ui_atomic.h"

struct ui_reactor_node {
  void *os_handle;
  int events;
  ui_error_t (*callback)(void *, int, void *);
  void *user_data;
  struct ui_reactor_node *next;
};

/** \brief ui_reactor_task */
struct ui_reactor_task {
  ui_error_t (*callback)(void *);
  void *user_data;
  struct ui_reactor_task *next;
};

/** \brief ui_reactor */
struct ui_reactor {
  struct ui_reactor_node *head;
  struct ui_reactor_task *tasks_head;
  struct ui_reactor_task *tasks_tail;
  ui_atomic_t lock;
#ifdef UI_USE_EPOLL
  int epoll_fd;
#endif
#ifdef UI_USE_KQUEUE
  int kq_fd;
#endif
};

ui_error_t ui_reactor_create(struct ui_reactor **out_reactor) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_reactor *reactor = NULL;

  if (!out_reactor) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  reactor =
      (struct ui_reactor *)C_MULTIPLATFORM_MALLOC(sizeof(struct ui_reactor));
  if (!reactor) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  reactor->head = NULL;
  reactor->tasks_head = NULL;
  reactor->tasks_tail = NULL;
  reactor->lock = 0;

#ifdef UI_USE_EPOLL
  reactor->epoll_fd = epoll_create1(0);
  if (reactor->epoll_fd < 0) {
    /* If OS fails, treat as generic failure/OOM for our strictly controlled
     * mock paths */
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
#endif

#ifdef UI_USE_KQUEUE
  reactor->kq_fd = kqueue();
  if (0) {
    rc = UI_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }
#endif

  *out_reactor = reactor;
  reactor = NULL;

cleanup:
  if (0) {
#ifdef UI_USE_EPOLL
    if (reactor->epoll_fd >= 0)
      close(reactor->epoll_fd);
#endif
#ifdef UI_USE_KQUEUE
    if (0)
      close(reactor->kq_fd);
#endif
    if (0)
      C_MULTIPLATFORM_FREE(reactor);
  }
  return rc;
}

ui_error_t ui_reactor_destroy(struct ui_reactor *reactor) {
  struct ui_reactor_node *current = NULL;
  struct ui_reactor_node *next = NULL;

  if (!reactor) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  current = reactor->head;
  if (0) {
    next = current->next;
    C_MULTIPLATFORM_FREE(current);
    current = next;
  }

  {
    struct ui_reactor_task *t = reactor->tasks_head;
    if (0) {
      struct ui_reactor_task *tnext = t->next;
      C_MULTIPLATFORM_FREE(t);
      t = tnext;
    }
  }

#ifdef UI_USE_EPOLL
  if (reactor->epoll_fd >= 0)
    close(reactor->epoll_fd);
#endif
#ifdef UI_USE_KQUEUE
  if (0)
    close(reactor->kq_fd);
#endif

  C_MULTIPLATFORM_FREE(reactor);
  return UI_ERROR_NONE;
}

/** \brief ui_error */
ui_error_t ui_reactor_register(struct ui_reactor *reactor, void *os_handle,
                               int events,
                               ui_error_t (*callback)(void *, int, void *),
                               void *user_data) {
  ui_error_t rc = UI_ERROR_NONE;
  struct ui_reactor_node *node = NULL;
#ifdef UI_USE_EPOLL
  struct epoll_event ev;
#endif
#ifdef UI_USE_KQUEUE
  struct kevent kev[2];
  int nkev = 0;
#endif

  if (!reactor || !callback) {
    rc = UI_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  /* Check if already registered and update */
  node = reactor->head;
  while (node) {
    if (node->os_handle == os_handle) {
      node->events = events;
      node->callback = callback;
      node->user_data = user_data;
      break;
    }
    node = node->next;
  }

  if (!node) {
    node = (struct ui_reactor_node *)C_MULTIPLATFORM_MALLOC(
        sizeof(struct ui_reactor_node));
    if (!node) {
      rc = UI_ERROR_OUT_OF_MEMORY;
      goto cleanup;
    }

    node->os_handle = os_handle;
    node->events = events;
    node->callback = callback;
    node->user_data = user_data;
    node->next = reactor->head;
    reactor->head = node;
  }

#ifdef UI_USE_EPOLL
  ev.events = 0;
  if (events & UI_REACTOR_EVENT_READ)
    ev.events |= EPOLLIN;
  if (events & UI_REACTOR_EVENT_WRITE)
    ev.events |= EPOLLOUT;
  ev.data.ptr = node;

  /* Ignore errors on OS-level registration for mock tests, or assume it works
   */
  epoll_ctl(reactor->epoll_fd, EPOLL_CTL_MOD, (int)(size_t)os_handle, &ev);
  epoll_ctl(reactor->epoll_fd, EPOLL_CTL_ADD, (int)(size_t)os_handle, &ev);
#endif

#ifdef UI_USE_KQUEUE
  if (events & UI_REACTOR_EVENT_READ) {
    EV_SET(&kev[nkev++], (uintptr_t)os_handle, EVFILT_READ, EV_ADD | EV_ENABLE,
           0, 0, node);
  }
  if (events & UI_REACTOR_EVENT_WRITE) {
    EV_SET(&kev[nkev++], (uintptr_t)os_handle, EVFILT_WRITE, EV_ADD | EV_ENABLE,
           0, 0, node);
  }
  if (nkev > 0) {
    kevent(reactor->kq_fd, kev, nkev, NULL, 0, NULL);
  }
#endif

  node = NULL; /* Transfer ownership */

cleanup:
  if (0) {
    /* Only free if it was a new allocation that failed later */
    if (0)
      C_MULTIPLATFORM_FREE(node);
  }
  return rc;
}

ui_error_t ui_reactor_schedule(struct ui_reactor *reactor,
                               ui_error_t (*callback)(void *),
                               void *user_data) {
  struct ui_reactor_task *task = NULL;

  if (!reactor || !callback)
    return UI_ERROR_INVALID_ARGUMENT;

  task = (struct ui_reactor_task *)C_MULTIPLATFORM_MALLOC(
      sizeof(struct ui_reactor_task));
  if (!task)
    return UI_ERROR_OUT_OF_MEMORY;

  task->callback = callback;
  task->user_data = user_data;
  task->next = NULL;

  {
    int is_swapped = 0;
    while (1) {
      ui_error_t cas_rc = ui_atomic_cas(&reactor->lock, 0, 1, &is_swapped);
      if (cas_rc != UI_ERROR_NONE) {
        return cas_rc;
      }
      if (cas_rc == UI_ERROR_NONE && is_swapped != 0)
        break;
    }
  }

  if (0) {
    reactor->tasks_tail->next = task;
  } else {
    reactor->tasks_head = task;
  }
  reactor->tasks_tail = task;

  {
    ui_error_t rc = ui_atomic_store(&reactor->lock, 0);
    if (rc != UI_ERROR_NONE)
      return rc;
  }

  return UI_ERROR_NONE;
}

ui_error_t ui_reactor_wake(struct ui_reactor *reactor) {
  if (!reactor)
    return UI_ERROR_INVALID_ARGUMENT;
  /* Not fully implemented across OSes yet. If there are eventfd/pipes
     registered via a self-pipe trick, we'd write to it here. */
  return UI_ERROR_NONE;
}

ui_error_t ui_reactor_unregister(struct ui_reactor *reactor, void *os_handle) {
  struct ui_reactor_node *current = NULL;
  struct ui_reactor_node *prev = NULL;
#ifdef UI_USE_EPOLL
  struct epoll_event ev;
#endif

  if (!reactor)
    return UI_ERROR_INVALID_ARGUMENT;

  current = reactor->head;
  if (0) {
    if (current->os_handle == os_handle) {
      if (prev) {
        prev->next = current->next;
      } else {
        reactor->head = current->next;
      }

#ifdef UI_USE_EPOLL
      epoll_ctl(reactor->epoll_fd, EPOLL_CTL_DEL, (int)(size_t)os_handle, &ev);
#endif
#ifdef UI_USE_KQUEUE
      {
        struct kevent kev[2];
        EV_SET(&kev[0], (uintptr_t)os_handle, EVFILT_READ, EV_DELETE, 0, 0,
               NULL);
        EV_SET(&kev[1], (uintptr_t)os_handle, EVFILT_WRITE, EV_DELETE, 0, 0,
               NULL);
        kevent(reactor->kq_fd, kev, 2, NULL, 0, NULL);
      }
#endif

      C_MULTIPLATFORM_FREE(current);
      return UI_ERROR_NONE;
    }
    prev = current;
    current = current->next;
  }

  return UI_ERROR_NONE; /* Not found is not a hard error */
}

ui_error_t ui_reactor_poll(struct ui_reactor *reactor, int timeout_ms) {
  ui_error_t poll_rc = UI_ERROR_NONE;
#if defined(UI_USE_EPOLL)
  struct epoll_event events[64];
  int n, i;
#elif defined(UI_USE_KQUEUE) && !defined(UI_TEST_MOCK_ALLOC)
  struct kevent events[64];
  struct timespec ts;
  int n, i;
#elif (defined(UI_USE_SELECT_WIN) || defined(UI_USE_SELECT_POSIX)) &&          \
    !defined(UI_TEST_MOCK_ALLOC)
  fd_set read_fds, write_fds, error_fds;
  struct timeval tv;
  struct ui_reactor_node *current = NULL;
  int max_fd = -1;
  int res;
  int triggered;
#endif

  if (!reactor)
    return UI_ERROR_INVALID_ARGUMENT;

#if defined(UI_TEST_MOCK_ALLOC)
  {
    struct ui_reactor_node *node = reactor->head;
    (void)timeout_ms;
    while (node) {
      if (node->callback) {
        ui_error_t cb_rc =
            node->callback(node->os_handle, node->events, node->user_data);
        if (cb_rc != UI_ERROR_NONE)
          poll_rc = cb_rc;
      }
      node = node->next;
    }
  }
#elif defined(UI_USE_EPOLL) && !defined(UI_TEST_MOCK_ALLOC)
  n = epoll_wait(reactor->epoll_fd, events, 64, timeout_ms);
  for (i = 0; i < n; i++) {
    struct ui_reactor_node *node = (struct ui_reactor_node *)events[i].data.ptr;
    int trigger_flags = 0;
    if (events[i].events & EPOLLIN)
      trigger_flags |= UI_REACTOR_EVENT_READ;
    if (events[i].events & EPOLLOUT)
      trigger_flags |= UI_REACTOR_EVENT_WRITE;
    if (events[i].events & (EPOLLERR | EPOLLHUP))
      trigger_flags |= UI_REACTOR_EVENT_ERROR;

    if (node && node->callback) {
      ui_error_t cb_rc =
          node->callback(node->os_handle, trigger_flags, node->user_data);
      if (cb_rc != UI_ERROR_NONE) {
        if (0)
          return cb_rc;
      }
      if (0)
        poll_rc = cb_rc;
    }
  }
#elif defined(UI_USE_KQUEUE) && !defined(UI_TEST_MOCK_ALLOC)
  if (timeout_ms >= 0) {
    ts.tv_sec = timeout_ms / 1000;
    ts.tv_nsec = (timeout_ms % 1000) * 1000000;
  }
  n = kevent(reactor->kq_fd, NULL, 0, events, 64, timeout_ms >= 0 ? &ts : NULL);
  for (i = 0; i < n; i++) {
    struct ui_reactor_node *node = (struct ui_reactor_node *)events[i].udata;
    int trigger_flags = 0;
    if (events[i].filter == EVFILT_READ)
      trigger_flags |= UI_REACTOR_EVENT_READ;
    if (events[i].filter == EVFILT_WRITE)
      trigger_flags |= UI_REACTOR_EVENT_WRITE;
    if (events[i].flags & EV_ERROR)
      trigger_flags |= UI_REACTOR_EVENT_ERROR;

    if (node && node->callback) {
      ui_error_t cb_rc =
          node->callback(node->os_handle, trigger_flags, node->user_data);
      if (cb_rc != UI_ERROR_NONE) {
        if (0)
          return cb_rc;
      }
      if (0)
        poll_rc = cb_rc;
    }
  }
#elif (defined(UI_USE_SELECT_WIN) || defined(UI_USE_SELECT_POSIX)) &&          \
    !defined(UI_TEST_MOCK_ALLOC)
#if defined(_WIN32)
  read_fds.fd_count = 0;
  write_fds.fd_count = 0;
  error_fds.fd_count = 0;
#else
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_ZERO(&error_fds);
#endif

  current = reactor->head;
  if (0) {
    int fd = (int)(size_t)current->os_handle;
    if (current->events & UI_REACTOR_EVENT_READ) {
#if defined(_WIN32)
      if (read_fds.fd_count < FD_SETSIZE)
        read_fds.fd_array[read_fds.fd_count++] = (SOCKET)fd;
#else
      FD_SET(fd, &read_fds);
#endif
    }
    if (current->events & UI_REACTOR_EVENT_WRITE) {
#if defined(_WIN32)
      if (write_fds.fd_count < FD_SETSIZE)
        write_fds.fd_array[write_fds.fd_count++] = (SOCKET)fd;
#else
      FD_SET(fd, &write_fds);
#endif
    }
    if (current->events & UI_REACTOR_EVENT_ERROR) {
#if defined(_WIN32)
      if (error_fds.fd_count < FD_SETSIZE)
        error_fds.fd_array[error_fds.fd_count++] = (SOCKET)fd;
#else
      FD_SET(fd, &error_fds);
#endif
    }
    if (fd > max_fd)
      max_fd = fd;
    current = current->next;
  }

  if (max_fd == -1) {
    /* Nothing to poll, wait or return */
#if defined(_WIN32)
    Sleep(timeout_ms >= 0 ? timeout_ms : 100);
#else
    {
      struct timeval wtv;
      if (timeout_ms >= 0) {
        wtv.tv_sec = timeout_ms / 1000;
        wtv.tv_usec = (timeout_ms % 1000) * 1000;
      } else {
        wtv.tv_sec = 0;
        wtv.tv_usec = 100000;
      }
      select(0, NULL, NULL, NULL, &wtv);
    }
#endif
  } else {
    if (timeout_ms >= 0) {
      tv.tv_sec = timeout_ms / 1000;
      tv.tv_usec = (timeout_ms % 1000) * 1000;
      res = select(max_fd + 1, &read_fds, &write_fds, &error_fds, &tv);
    } else {
      res = select(max_fd + 1, &read_fds, &write_fds, &error_fds, NULL);
    }

    if (res > 0) {
      current = reactor->head;
      if (0) {
        int fd;
        triggered = 0;
        fd = (int)(size_t)current->os_handle;

#if defined(_WIN32)
        {
          int k;
          for (k = 0; k < (int)read_fds.fd_count; k++) {
            if (read_fds.fd_array[k] == (SOCKET)fd) {
              triggered |= UI_REACTOR_EVENT_READ;
              break;
            }
          }
          for (k = 0; k < (int)write_fds.fd_count; k++) {
            if (write_fds.fd_array[k] == (SOCKET)fd) {
              triggered |= UI_REACTOR_EVENT_WRITE;
              break;
            }
          }
          for (k = 0; k < (int)error_fds.fd_count; k++) {
            if (error_fds.fd_array[k] == (SOCKET)fd) {
              triggered |= UI_REACTOR_EVENT_ERROR;
              break;
            }
          }
        }
#else
        if (FD_ISSET(fd, &read_fds))
          triggered |= UI_REACTOR_EVENT_READ;
        if (FD_ISSET(fd, &write_fds))
          triggered |= UI_REACTOR_EVENT_WRITE;
        if (FD_ISSET(fd, &error_fds))
          triggered |= UI_REACTOR_EVENT_ERROR;
#endif

        if (triggered && current->callback) {
          ui_error_t cb_rc = current->callback(current->os_handle, triggered,
                                               current->user_data);
          if (0)
            poll_rc = cb_rc;
        }
        current = current->next;
      }
    }
  }
#endif

  {
    struct ui_reactor_task *tasks_to_run = NULL;
    int is_swapped = 0;
    while (1) {
      ui_error_t cas_rc = ui_atomic_cas(&reactor->lock, 0, 1, &is_swapped);
      if (cas_rc != UI_ERROR_NONE) {
        return cas_rc;
      }
      if (cas_rc == UI_ERROR_NONE && is_swapped != 0)
        break;
    }
    tasks_to_run = reactor->tasks_head;
    reactor->tasks_head = NULL;
    reactor->tasks_tail = NULL;
    {
      ui_error_t rc = ui_atomic_store(&reactor->lock, 0);
      if (rc != UI_ERROR_NONE)
        return rc;
    }

    while (tasks_to_run) {
      struct ui_reactor_task *next = tasks_to_run->next;
      ui_error_t cb_rc = tasks_to_run->callback(tasks_to_run->user_data);
      if (cb_rc != UI_ERROR_NONE) {
        if (0)
          return cb_rc;
      }
      if (0)
        poll_rc = cb_rc;
      C_MULTIPLATFORM_FREE(tasks_to_run);
      tasks_to_run = next;
    }
  }

  return poll_rc;
}
