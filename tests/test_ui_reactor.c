/* clang-format off */
#include "../include/ui_reactor.h"
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
/* clang-format on */

extern int g_malloc_fail_countdown;
static enum ui_error test_callback(void *os_handle, int events,
                                   void *user_data) {
  if (user_data) {
    int *val = (int *)user_data;
    *val = events;
  }
  (void)os_handle;
  return UI_ERROR_NONE;
}

static enum ui_error test_callback_fail(void *os_handle, int events,
                                        void *user_data) {
  (void)os_handle;
  (void)events;
  (void)user_data;
  return UI_ERROR_UNKNOWN;
}

static enum ui_error test_schedule_callback(void *user_data) {
  return UI_ERROR_UNKNOWN;
}

static int run_normal_tests(void) {
  struct ui_reactor *reactor = NULL;
  enum ui_error rc;
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
    return 1;
  }

  rc = ui_reactor_register(reactor, fake_handle1, UI_REACTOR_EVENT_READ,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to register handle\n");
    return 1;
  }

  rc = ui_reactor_register(reactor, fake_handle2, UI_REACTOR_EVENT_READ,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_reactor_register(reactor, fake_handle3, UI_REACTOR_EVENT_READ,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Test double register updates correctly */
  rc = ui_reactor_register(reactor, fake_handle1,
                           UI_REACTOR_EVENT_READ | UI_REACTOR_EVENT_WRITE,
                           test_callback, &test_val);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to update handle\n");
    return 1;
  }

  /* We cannot reliably test poll() triggering callbacks without real FDs that
     are ready. Instead, we test poll() timeout behaviour. */
  rc = ui_reactor_poll(reactor, 1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to poll reactor\n");
    return 1;
  }

  /* Test polling with 0 and negative timeout */
  rc = ui_reactor_poll(reactor, 0);
  if (rc != UI_ERROR_NONE)
    return 1;

#ifndef _MSC_VER
  write(pipes[1], "A", 1);
#endif
  rc = ui_reactor_poll(reactor, 100);
  if (rc != UI_ERROR_NONE)
    return 1;
  /* wait for test_val to update... well, the event loop handles it */

  /* Unregister head */
  rc = ui_reactor_unregister(reactor, fake_handle3);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Unregister middle/tail to cover if(prev) */
  rc = ui_reactor_unregister(reactor, fake_handle1);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Unregister non-existent */
  rc = ui_reactor_unregister(reactor, (void *)999);
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_reactor_unregister(reactor, fake_handle2);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Poll empty reactor */
  rc = ui_reactor_poll(reactor, 1);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to poll empty reactor\n");
    return 1;
  }

#ifndef _MSC_VER
  write(pipes[1], "A", 1);
#endif
  rc = ui_reactor_poll(reactor, 100);
  if (rc != UI_ERROR_NONE)
    return 1;
  /* wait for test_val to update... well, the event loop handles it */

  rc = ui_reactor_destroy(reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to destroy reactor\n");
    return 1;
  }

  /* Additional tests to hit coverage */
  ui_reactor_create(&reactor);
  ui_reactor_register(reactor, fake_handle1, UI_REACTOR_EVENT_READ,
                      test_callback_fail, &test_val);
  ui_reactor_poll(reactor, 1);
#ifndef _MSC_VER
  close(pipes[0]);
#endif
#ifndef _MSC_VER
  close(pipes[1]);
#endif
  ui_reactor_unregister(reactor, fake_handle1);
  ui_reactor_schedule(NULL, test_schedule_callback, NULL);
  ui_reactor_schedule(reactor, NULL, NULL);
  ui_reactor_schedule(reactor, test_schedule_callback, NULL);
  ui_reactor_poll(reactor, 1);
#ifndef _MSC_VER
  close(pipes[0]);
#endif
#ifndef _MSC_VER
  close(pipes[1]);
#endif

  g_malloc_fail_countdown = 0;
  ui_reactor_schedule(reactor, test_schedule_callback, NULL);
  g_malloc_fail_countdown = -1;
  ui_reactor_wake(reactor);
  ui_reactor_wake(NULL);
  ui_reactor_destroy(reactor);

  /* Invalid argument tests */
  if (ui_reactor_create(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_reactor_destroy(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_reactor_register(NULL, fake_handle1, 0, test_callback, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;

  ui_reactor_create(&reactor);
  if (ui_reactor_register(reactor, fake_handle1, 0, NULL, NULL) !=
      UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_reactor_unregister(NULL, fake_handle1) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_reactor_poll(NULL, 100) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_reactor_destroy(reactor);

  return 0;
}

static int run_oom_tests(void) {
  struct ui_reactor *reactor = NULL;
  enum ui_error rc;
  void *fake_handle = (void *)42;

  printf("Running reactor OOM tests...\n");

  g_malloc_fail_countdown = 0;
  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OOM on create\n");
    return 1;
  }

  g_malloc_fail_countdown = -1;
  rc = ui_reactor_create(&reactor);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to create reactor for schedule OOM test\n");
    return 1;
  }

  g_malloc_fail_countdown = 0;
  rc = ui_reactor_register(reactor, fake_handle, UI_REACTOR_EVENT_READ,
                           test_callback, NULL);
  if (rc != UI_ERROR_OUT_OF_MEMORY) {
    printf("Expected OOM on register\n");
    return 1;
  }

  g_malloc_fail_countdown = -1;
  ui_reactor_destroy(reactor);

  return 0;
}

int main(void) {
  int failed = 0;
  failed |= run_normal_tests();
  failed |= run_oom_tests();

  if (failed) {
    printf("Tests failed.\n");
    return 1;
  }
  printf("All test_ui_reactor passed.\n");
  return 0;
}
