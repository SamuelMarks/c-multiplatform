/* clang-format off */
#include "ui_clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <signal.h>
/* clang-format on */
#endif

extern int g_malloc_fail_countdown;
extern int g_ui_clipboard_force_fallback;

int (*g_mock_system_fn)(const char *) = NULL;
FILE *(*g_mock_popen_fn)(const char *, const char *) = NULL;
int (*g_mock_pclose_fn)(FILE *) = NULL;

static int mock_system_wl(const char *cmd) {
  if (strstr(cmd, "wl-copy") || strstr(cmd, "wl-paste"))
    return 0;
  return 1;
}

static int mock_system_xclip(const char *cmd) {
  if (strstr(cmd, "xclip"))
    return 0;
  return 1;
}

static int mock_system_xsel(const char *cmd) {
  if (strstr(cmd, "xsel"))
    return 0;
  return 1;
}

static FILE *mock_popen_dummy(const char *cmd, const char *mode) {
  (void)cmd;
  (void)mode;
  return NULL; /* Just pretend it failed to run */
}

static FILE *mock_popen_devnull(const char *cmd, const char *mode) {
  (void)cmd;
  (void)mode;
  return fopen("/dev/null", "w");
}

static FILE *mock_popen_empty(const char *cmd, const char *mode) {
  (void)cmd;
  (void)mode;
  return fopen("/dev/null", "r");
}

static FILE *mock_popen_large(const char *cmd, const char *mode) {
  FILE *f;
  (void)cmd;
  (void)mode;
  /* Create a temporary file with 1500 bytes to simulate large output */
  f = tmpfile();
  if (f) {
    char buf[1500];
    memset(buf, 'A', sizeof(buf) - 1);
    buf[1499] = '\0';
    fputs(buf, f);
    rewind(f);
  }
  return f;
}

static int mock_pclose_dummy(FILE *stream) {
  if (stream)
    fclose(stream);
  return 0;
}

static int mock_system_always_fail(const char *cmd) {
  (void)cmd;
  return 1;
}

static int run_normal_tests(void) {
  enum ui_error rc;
  char *text = NULL;
  const char *test_str = "Hello, Cross-Platform Clipboard!";

  /* Force mock to avoid xclip hanging ctest */
  g_mock_system_fn = mock_system_always_fail;

  printf("Testing invalid arguments...\n");
  if (ui_clipboard_set_text(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  if (ui_clipboard_get_text(NULL) != UI_ERROR_INVALID_ARGUMENT)
    return 1;
  ui_clipboard_free_text(NULL);

  /* Read initially (may return system clipboard contents or UNSUPPORTED, don't
   * strictly fail on it) */
  rc = ui_clipboard_get_text(&text);
  if (rc == UI_ERROR_NONE && text) {
    ui_clipboard_free_text(text);
    text = NULL;
  }

  printf("Testing set and get text...\n");
  rc = ui_clipboard_set_text(test_str);
  if (rc != UI_ERROR_NONE) {
    printf("Failed to set clipboard text (likely headless CI). Skipping.\n");
  } else {
    rc = ui_clipboard_get_text(&text);
    if (rc != UI_ERROR_NONE || !text || strlen(text) == 0) {
      printf("Failed to get clipboard text (likely headless CI). Skipping.\n");
    } else {
      if (strcmp(text, test_str) != 0) {
        printf("Clipboard text mismatch. Expected '%s', got '%s'\n", test_str,
               text);
        ui_clipboard_free_text(text);
        return 1;
      }
      ui_clipboard_free_text(text);
    }
  }

  printf("Testing fallback...\n");
  g_ui_clipboard_force_fallback = 1;
  rc = ui_clipboard_set_text(test_str);
  if (rc != UI_ERROR_NONE)
    return 1;

  /* Overwrite fallback */
  rc = ui_clipboard_set_text("Overwrite");
  if (rc != UI_ERROR_NONE)
    return 1;

  rc = ui_clipboard_get_text(&text);
  if (rc != UI_ERROR_NONE || strcmp(text, "Overwrite") != 0)
    return 1;
  ui_clipboard_free_text(text);

  /* Fallback memory failures */
  g_malloc_fail_countdown = 0;
  rc = ui_clipboard_set_text("Fail");
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  rc = ui_clipboard_set_text("Success");
  if (rc != UI_ERROR_NONE)
    return 1;

  g_malloc_fail_countdown = 0;
  rc = ui_clipboard_get_text(&text);
  if (rc != UI_ERROR_OUT_OF_MEMORY)
    return 1;
  g_malloc_fail_countdown = -1;

  g_ui_clipboard_force_fallback = 0;

  printf("Testing large text for real clipboard realloc...\n");
  {
    char large_text[1500];
    memset(large_text, 'A', 1499);
    large_text[1499] = '\0';

    g_mock_system_fn = mock_system_wl;
    g_mock_popen_fn = mock_popen_large;
    g_mock_pclose_fn = mock_pclose_dummy;

    rc = ui_clipboard_get_text(&text);
    if (rc == UI_ERROR_NONE && text) {
      if (strcmp(text, large_text) != 0)
        return 1;
      ui_clipboard_free_text(text);
    }

    /* Test real clipboard realloc failure (requires popen to return text, so we
     * do it after successful set) */
    g_malloc_fail_countdown = 1; /* 0 is first malloc, 1 is the first realloc */
    rc = ui_clipboard_get_text(&text);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      return 1;
    }

    g_mock_system_fn = mock_system_always_fail;
    g_mock_popen_fn = NULL;
    g_mock_pclose_fn = NULL;
  }

  /* Test real clipboard first malloc failure */
  g_mock_system_fn = mock_system_wl;
  g_mock_popen_fn = mock_popen_large;
  g_mock_pclose_fn = mock_pclose_dummy;
  g_malloc_fail_countdown = 0;
  rc = ui_clipboard_get_text(&text);
  g_malloc_fail_countdown = -1;
  g_mock_system_fn = mock_system_always_fail;
  g_mock_popen_fn = NULL;
  g_mock_pclose_fn = NULL;

  printf("Testing empty real clipboard...\n");
  g_mock_system_fn = mock_system_wl;
  g_mock_popen_fn = mock_popen_empty;
  g_mock_pclose_fn = mock_pclose_dummy;
  rc = ui_clipboard_get_text(&text);
  if (rc == UI_ERROR_NONE && text) {
    ui_clipboard_free_text(text);
  }
  g_mock_system_fn = mock_system_always_fail;
  g_mock_popen_fn = NULL;
  g_mock_pclose_fn = NULL;

  printf("Testing mock wl-copy...\n");
  g_mock_system_fn = mock_system_wl;
  g_mock_popen_fn = mock_popen_devnull;
  g_mock_pclose_fn = mock_pclose_dummy;
  ui_clipboard_set_text("mock wl");

  printf("Testing mock xsel...\n");
  g_mock_system_fn = mock_system_xsel;
  g_mock_popen_fn = mock_popen_devnull;
  g_mock_pclose_fn = mock_pclose_dummy;
  ui_clipboard_set_text("mock xsel");
  ui_clipboard_get_text(&text);
  if (text)
    ui_clipboard_free_text(text);

  printf("Testing mock xsel empty...\n");
  g_mock_system_fn = mock_system_xsel;
  g_mock_popen_fn = mock_popen_empty;
  g_mock_pclose_fn = mock_pclose_dummy;
  ui_clipboard_get_text(&text);
  if (text)
    ui_clipboard_free_text(text);

  printf("Testing mock xclip...\n");
  g_mock_system_fn = mock_system_xclip;
  g_mock_popen_fn = mock_popen_devnull;
  g_mock_pclose_fn = mock_pclose_dummy;
  ui_clipboard_set_text("mock xclip");
  ui_clipboard_get_text(&text);
  if (text)
    ui_clipboard_free_text(text);

  g_mock_system_fn = NULL;
  g_mock_popen_fn = NULL;
  g_mock_pclose_fn = NULL;

  printf("Testing cleanup...\n");
  ui_clipboard_cleanup();
  /* Repeated cleanup should be safe */
  ui_clipboard_cleanup();

  /* Get text after cleanup on fallback */
  g_ui_clipboard_force_fallback = 1;
  rc = ui_clipboard_get_text(&text);
  if (rc != UI_ERROR_UNSUPPORTED)
    return 1;
  g_ui_clipboard_force_fallback = 0;

  return 0;
}

int main(void) {
#if !defined(_WIN32)
  signal(SIGPIPE, SIG_IGN);
#endif

#ifdef _WIN32
  if (getenv("WINELOADER") != NULL) {
    printf("Skipping test_ui_clipboard under Wine CI to avoid pipe hangs.\n");
    return 0;
  }
#endif

  if (run_normal_tests() != 0) {
    return 1;
  }

  printf("test_ui_clipboard passed.\n");
  return 0;
}
