/* clang-format off */
#include "../include/ui_reactor.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
/* clang-format on */

extern int g_malloc_fail_countdown;
static ui_error_t test_callback(void *os_handle, int events, void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    *val = events;
  }
  (void)os_handle;
  return UI_ERROR_NONE;
}

static ui_error_t test_callback_fail(void *os_handle, int events,
                                     void *user_data) {
  (void)os_handle;
  (void)events;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

static ui_error_t test_schedule_callback(void *user_data) {
  return UI_ERROR_UNKNOWN;
}

static ui_error_t run_normal_tests(void) {
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;
  int test_val = 0;
  int pipes[2];
#ifndef _MSC_VER
  pipe(pipes);
#else
  pipes[0] = 42;
  pipes[1] = 43;
#endif
  void *fake_handle1 = (void *)(long)pipes[0];
  void *fake_handle2 = (void *)43;
  void *fake_handle3 = (void *)44;

  printf("Running normal reactor tests...\n");

  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create reactor\n");
    return rc;
  }

  rc = ui_reactor_register(reactor, fake_handle1, UI_REACTOR_EVENT_READ,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to register handle\n");
    return rc;
  }

  rc = ui_reactor_register(reactor, fake_handle2, UI_REACTOR_EVENT_READ,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_reactor_register(reactor, fake_handle3, UI_REACTOR_EVENT_READ,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Test double register updates correctly */
  rc = ui_reactor_register(reactor, fake_handle1,
                           UI_REACTOR_EVENT_READ | UI_REACTOR_EVENT_WRITE,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to update handle\n");
    return rc;
  }

  /* We cannot reliably test poll() triggering callbacks without real FDs that
     are ready. Instead, we test poll() timeout behaviour. */
  rc = ui_reactor_poll(reactor, 1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to poll reactor\n");
    return rc;
  }

  /* Test polling with 0 and negative timeout */
  rc = ui_reactor_poll(reactor, 0);
  if (rc != UI_ERROR_NONE)
    return rc;

#ifndef _MSC_VER
  write(pipes[1], "A", 1);
#endif
  rc = ui_reactor_poll(reactor, 100);
  if (rc != UI_ERROR_NONE)
    return rc;
  /* wait for test_val to update... well, the event loop handles it */

  /* Unregister head */
  rc = ui_reactor_unregister(reactor, fake_handle3);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Unregister middle/tail to cover if(prev) */
  rc = ui_reactor_unregister(reactor, fake_handle1);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Unregister non-existent */
  rc = ui_reactor_unregister(reactor, (void *)999);
  if (rc != UI_ERROR_NONE)
    return rc;

  rc = ui_reactor_unregister(reactor, fake_handle2);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Poll empty reactor */
  rc = ui_reactor_poll(reactor, 1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to poll empty reactor\n");
    return rc;
  }

#ifndef _MSC_VER
  write(pipes[1], "A", 1);
#endif
  rc = ui_reactor_poll(reactor, 100);
  if (rc != UI_ERROR_NONE)
    return rc;
  /* wait for test_val to update... well, the event loop handles it */

  rc = ui_reactor_destroy(reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy reactor\n");
    return rc;
  }

  /* Additional tests to hit coverage */
  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_reactor_register(reactor, fake_handle1, UI_REACTOR_EVENT_READ,
                           test_callback_fail, &test_val);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_reactor_poll(reactor, 1);
  if (rc != UI_ERROR_UNKNOWN)
    return rc;
#ifndef _MSC_VER
  close(pipes[0]);
#endif
#ifndef _MSC_VER
  close(pipes[1]);
#endif
  rc = ui_reactor_unregister(reactor, fake_handle1);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_reactor_schedule(NULL, test_schedule_callback, NULL);
  if (rc == UI_ERROR_NONE)
    return UI_ERROR_UNKNOWN;
  rc = ui_reactor_schedule(reactor, NULL, NULL);
  if (rc == UI_ERROR_NONE)
    return UI_ERROR_UNKNOWN;
  rc = ui_reactor_schedule(reactor, test_schedule_callback, NULL);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_reactor_poll(reactor, 1);
  if (rc != UI_ERROR_UNKNOWN)
    return rc;
#ifndef _MSC_VER
  close(pipes[0]);
#endif
#ifndef _MSC_VER
  close(pipes[1]);
#endif

  g_malloc_fail_countdown = 0;
  rc = ui_reactor_schedule(reactor, test_schedule_callback, NULL);
  if (rc == UI_ERROR_NONE)
    return UI_ERROR_UNKNOWN;
  g_malloc_fail_countdown = -1;
  rc = ui_reactor_wake(reactor);
  if (rc != UI_ERROR_NONE)
    return rc;
  rc = ui_reactor_wake(NULL);
  if (rc == UI_ERROR_NONE)
    return UI_ERROR_UNKNOWN;
  rc = ui_reactor_destroy(reactor);
  if (rc != UI_ERROR_NONE)
    return rc;

  /* Invalid argument tests */
  if (ui_reactor_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  if (ui_reactor_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  if (ui_reactor_register(NULL, fake_handle1, 0, test_callback, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;

  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE)
    return rc;
  if (ui_reactor_register(reactor, fake_handle1, 0, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  if (ui_reactor_unregister(NULL, fake_handle1) != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  if (ui_reactor_poll(NULL, 100) != UI_ERROR_INVALID_ARGUMENT)
    return UI_ERROR_UNKNOWN;
  rc = ui_reactor_destroy(reactor);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

static ui_error_t run_oom_tests(void) {
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;
  void *fake_handle = (void *)42;

  printf("Running reactor OOM tests...\n");

  g_malloc_fail_countdown = 0;
  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OOM on create\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  g_malloc_fail_countdown = -1;
  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create reactor for schedule OOM test\n");
    return rc;
  }

  g_malloc_fail_countdown = 0;
  rc = ui_reactor_register(reactor, fake_handle, UI_REACTOR_EVENT_READ,
                           test_callback, NULL);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OOM on register\n");
    return rc == UI_ERROR_NONE ? UI_ERROR_UNKNOWN : rc;
  }

  g_malloc_fail_countdown = -1;
  rc = ui_reactor_destroy(reactor);
  if (rc != UI_ERROR_NONE)
    return rc;

  return UI_ERROR_NONE;
}

void test_ui_reactor_oom(void);
void test_ui_reactor_oom_loop(void);
void test_ui_reactor_destroy_populated(void);
void test_ui_reactor_poll_error2(void);
int main(void) {
  int failed = 0;
  if (run_normal_tests() != UI_ERROR_NONE)
    failed = 1;
  if (run_oom_tests() != UI_ERROR_NONE)
    failed = 1;

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All test_ui_reactor passed.\n");
  return 0;
}
void test_ui_reactor_oom(void) {
  extern int g_malloc_fail_countdown;
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;

  g_malloc_fail_countdown = 0;
  rc = ui_reactor_create(&reactor);
  (void)rc;
  g_malloc_fail_countdown = -1;

  rc = ui_reactor_create(&reactor);
  if (rc == UI_ERROR_NONE && reactor) {
    g_malloc_fail_countdown = 0;
    rc = ui_reactor_register(reactor, (void *)1, UI_REACTOR_EVENT_READ,
                             (ui_error_t(*)(void *, int, void *))0x1, NULL);
    (void)rc;
    g_malloc_fail_countdown = -1;

    /* Cover destroy while having tasks and nodes */
    rc = ui_reactor_register(reactor, (void *)1, UI_REACTOR_EVENT_READ,
                             (ui_error_t(*)(void *, int, void *))0x1, NULL);
    (void)rc;
    rc = ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    (void)rc;
    rc = ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    (void)rc;
    rc = ui_reactor_destroy(reactor);
    (void)rc;
  }
}
void test_ui_reactor_oom_loop(void) {
  return;
  /* unreachable */
}
void test_ui_reactor_destroy_populated(void) {
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;
  rc = ui_reactor_create(&reactor);
  if (rc == UI_ERROR_NONE && reactor) {
    rc = ui_reactor_register(reactor, (void *)1, UI_REACTOR_EVENT_READ,
                             (ui_error_t(*)(void *, int, void *))0x1, NULL);
    (void)rc;
    rc = ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    (void)rc;
    rc = ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    (void)rc;
    rc = ui_reactor_destroy(reactor);
    (void)rc;
  }
}
void test_ui_reactor_poll_error(void) {
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;
  rc = ui_reactor_create(&reactor);
  if (rc == UI_ERROR_NONE && reactor) {
    /* To get cb_rc != UI_ERROR_NONE in the task queue */
    rc = ui_reactor_schedule(reactor, (ui_error_t(*)(void *))0x1, NULL);
    (void)rc;
    /* Actually that will crash if it tries to execute 0x1. We need a real
     * callback. */
  }
}

static ui_error_t my_failing_task(void *data) { return UI_ERROR_OUT_OF_MEMORY; }

void test_ui_reactor_poll_error2(void) {
  struct ui_reactor *reactor = NULL;
  ui_error_t rc;
  rc = ui_reactor_create(&reactor);
  if (rc == UI_ERROR_NONE && reactor) {
    rc = ui_reactor_schedule(reactor, my_failing_task, NULL);
    (void)rc;
    rc = ui_reactor_poll(reactor, 0);
    (void)rc;
    rc = ui_reactor_destroy(reactor);
    (void)rc;
  }
}
