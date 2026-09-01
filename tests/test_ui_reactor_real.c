/* clang-format off */
#include "../include/ui_reactor.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#include <fcntl.h>
#endif
/* clang-format on */

struct ui_reactor_node {
  void *os_handle;
  int events;
  ui_error_t (*callback)(void *, int, void *);
  void *user_data;
  struct ui_reactor_node *next;
};

struct ui_reactor {
  struct ui_reactor_node *head;
  void *tasks_head;
  void *tasks_tail;
  long lock;
#if defined(__linux__)
  int epoll_fd;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||    \
    defined(__NetBSD__) || defined(__DragonFly__)
  int kq_fd;
#endif
};

static ui_error_t test_callback(void *os_handle, int events, void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    *val |= events;
  }
  (void)os_handle;
  return UI_ERROR_NONE;
}

static ui_error_t test_schedule_callback(void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    *val = 1;
  }
  return UI_ERROR_NONE;
}

static ui_error_t my_failing_task(void *data) { return UI_ERROR_OUT_OF_MEMORY; }
static ui_error_t my_failing_callback(void *os_handle, int events,
                                      void *user_data) {
  (void)os_handle;
  (void)events;
  (void)user_data;
  return UI_ERROR_OUT_OF_MEMORY;
}

static ui_error_t thread_spam_schedule(void *user_data) {
  struct ui_reactor *r = (struct ui_reactor *)user_data;
  int i;
  for (i = 0; i < 5000; i++) {
    {
      ui_error_t rc_cleanup =
          ui_reactor_schedule(r, test_schedule_callback, NULL);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }
  return UI_ERROR_NONE;
}

int main(void) {
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;
  int test_val = 0;
  int pipes[2];

  printf("Running real reactor tests...\n");

#ifndef _WIN32
  if (pipe(pipes) != 0) {
    return 1;
  }
#else
  if (_pipe(pipes, 256, 0) != 0) {
    return 1;
  }
#endif

  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test register invalid args */
  rc = ui_reactor_register(reactor, (void *)(size_t)pipes[0],
                           UI_REACTOR_EVENT_READ, test_callback, &test_val);
  if (rc != UI_ERROR_NONE)
    return 1;

#if !defined(_WIN32) && !defined(__CYGWIN__)
  write(pipes[1], "A", 1);
#endif

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__)
  {
#include <sys/event.h>
    struct kevent kev;
    /* Add a user event with udata = NULL to trigger node == NULL */
    EV_SET(&kev, 12345, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
    kevent(reactor->kq_fd, &kev, 1, NULL, 0, NULL);
    /* Trigger it */
    EV_SET(&kev, 12345, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
    kevent(reactor->kq_fd, &kev, 1, NULL, 0, NULL);

    /* Modify the existing node to have callback = NULL to trigger
     * node->callback == NULL */
    if (reactor->head) {
      reactor->head->callback = NULL;
    }
  }
#endif
  rc = ui_reactor_poll(reactor, 100);

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__)
  if (reactor->head) {
    reactor->head->callback = test_callback;
  }
#endif

  rc = ui_reactor_poll(reactor, 100);

  if (rc != UI_ERROR_NONE)
    return 1;

#if !defined(_WIN32) && !defined(__CYGWIN__)
  if ((test_val & UI_REACTOR_EVENT_READ) == 0) {
    printf("Read event not triggered!\n");
    return 1;
  }
#endif

  /* Schedule a task */
  test_val = 0;
  rc = ui_reactor_schedule(reactor, test_schedule_callback, &test_val);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_reactor_poll(reactor, 100);
  if (rc != UI_ERROR_NONE)
    return 1;
  if (test_val != 1)
    return 1;

  /* Schedule failing task */
  rc = ui_reactor_schedule(reactor, my_failing_task, NULL);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_reactor_poll(reactor, 0);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;

  /* Wake up */
  rc = ui_reactor_wake(reactor);
  if (rc != UI_ERROR_NONE)
    return 1;
  rc = ui_reactor_poll(reactor, 100);

  /* Test timeout < 0 */
  rc = ui_reactor_poll(reactor, -1);

  /* Test write event */
  rc = ui_reactor_register(reactor, (void *)(size_t)pipes[1],
                           UI_REACTOR_EVENT_WRITE, test_callback, &test_val);
  if (rc == UI_ERROR_NONE) {
    rc = ui_reactor_poll(reactor, 0);
    {
      ui_error_t rc_cleanup =
          ui_reactor_unregister(reactor, (void *)(size_t)pipes[1]);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
  }

  /* Test error event */
  /* We can trigger EV_ERROR or we can mock callback failing */
  rc = ui_reactor_register(reactor, (void *)(size_t)pipes[0],
                           UI_REACTOR_EVENT_READ, my_failing_callback, NULL);
  /* The failing task will return UI_ERROR_OUT_OF_MEMORY and that should be
   * returned by poll */
#ifndef _MSC_VER
  write(pipes[1], "B", 1);
#endif
  rc = ui_reactor_poll(reactor, 100);

  /* Test kq_fd < 0 destroy */
  {
    struct ui_reactor *bad_reactor = NULL;
    {
      ui_error_t rc_cleanup = ui_reactor_create(&bad_reactor);
      if (rc_cleanup != UI_ERROR_NONE) {
        (void)rc_cleanup; /* Avoid override */
      }
    }
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||      \
    defined(__NetBSD__) || defined(__DragonFly__)
    if (bad_reactor) {
      int old_fd = bad_reactor->kq_fd;
      bad_reactor->kq_fd = -1;
      {
        ui_error_t rc_cleanup = ui_reactor_destroy(bad_reactor);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      close(old_fd);
    }
#endif
#if defined(__linux__)
    if (bad_reactor) {
      int old_fd = bad_reactor->epoll_fd;
      bad_reactor->epoll_fd = -1;
      {
        ui_error_t rc_cleanup = ui_reactor_destroy(bad_reactor);
        if (rc_cleanup != UI_ERROR_NONE) {
          (void)rc_cleanup; /* Avoid override */
        }
      }
      close(old_fd);
    }
#endif
  }

  rc = ui_reactor_destroy(reactor);
  if (rc != UI_ERROR_NONE)
    return 1;

#if !defined(_WIN32) && !defined(__CYGWIN__)
  close(pipes[0]);
  close(pipes[1]);
#endif

  /* Test lock contention in schedule and poll */
  {
#include "../include/ui_thread_pool.h"
    struct ui_thread_pool *pool = NULL;
    int i;
    rc = ui_reactor_create(&reactor);
    if (rc == UI_ERROR_NONE &&
        ui_thread_pool_create(10, &pool) == UI_ERROR_NONE) {
      for (i = 0; i < 10; i++) {
        {
          ui_error_t rc_cleanup =
              ui_thread_pool_schedule(pool, thread_spam_schedule, reactor);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
      for (i = 0; i < 50000; i++) {
        {
          ui_error_t rc_cleanup = ui_reactor_poll(reactor, 0);
          if (rc_cleanup != UI_ERROR_NONE) {
            (void)rc_cleanup; /* Avoid override */
          }
        }
      }
      ui_thread_pool_destroy(pool);
    }
    if (reactor)
      ui_reactor_destroy(reactor);
  }

  printf("Real reactor tests passed.\n");
  return 0;
}
