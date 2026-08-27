/* clang-format off */
#include <stdlib.h>
#include <string.h>
#include "ui_test_mock_mem.h"
/* clang-format on */

int g_malloc_fail_countdown = -1;
int g_malloc_called = 0;
int g_ui_timer_clock_gettime_fail = 0;
int g_ui_clipboard_force_fallback = 0;
int g_mock_io_fail = 0;
int g_mock_thread_fail = 0;

void *ui_mock_malloc(size_t size) {
  g_malloc_called = 1;
  if (g_malloc_fail_countdown == 0) {
    return NULL;
  }
  if (g_malloc_fail_countdown > 0) {
    g_malloc_fail_countdown--;
  }
  return malloc(size);
}

void *ui_mock_realloc(void *ptr, size_t size) {
  g_malloc_called = 1;
  if (g_malloc_fail_countdown == 0) {
    return NULL;
  }
  if (g_malloc_fail_countdown > 0) {
    g_malloc_fail_countdown--;
  }
  return realloc(ptr, size);
}

void ui_mock_free(void *ptr) { free(ptr); }

char *ui_mock_strdup(const char *src) {
  size_t len;
  char *copy;
  if (!src)
    return NULL;
  len = strlen(src);
  copy = (char *)ui_mock_malloc(len + 1);
  if (copy) {
    strcpy(copy, src);
  }
  return copy;
}

int g_mock_gles2_create_fail = 0;
int g_mock_gles2_destroy_fail = 0;
int g_mock_gles2_flush_fail = 0;
int g_native_init_fail = 0;
int g_gles_init_fail = 0;
int g_mock_append_child_fail = 0;
int g_mock_strcpy_fail = 0;
int g_mock_lock_contention = 0;
int g_mock_cg_fail = 0;
