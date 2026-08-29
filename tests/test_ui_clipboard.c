/* clang-format off */
#include "ui_clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void check_cond_impl(int cond, int *failed, int line) {
  if (cond) {
    *failed = 1;
    printf("Failed at line %d\n", line);
  }
}
#define check_cond(cond, failed) check_cond_impl(cond, failed, __LINE__)



#if !defined(_WIN32)
#include <signal.h>
/* clang-format on */
#endif

extern int g_malloc_fail_countdown;
extern int g_ui_clipboard_force_fallback;

#if defined(_MSC_VER)
#define POPEN_CMD _popen
#define PCLOSE_CMD _pclose
#else
#define POPEN_CMD popen
#define PCLOSE_CMD pclose
extern FILE *popen(const char *, const char *);
extern int pclose(FILE *);
#endif

int (*g_mock_system_fn)(const char *) = system;

FILE *(*g_mock_popen_fn)(const char *, const char *) = POPEN_CMD;
int (*g_mock_pclose_fn)(FILE *) = PCLOSE_CMD;

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
  ui_error_t rc;
  char *text = NULL;
  const char *test_str = "Hello, Cross-Platform Clipboard!";
  int failed = 0;

#if !defined(__linux__) && !(defined(__unix__) && !defined(__APPLE__))
  mock_system_wl("wl-copy");
  mock_system_wl("wl-paste");
  mock_system_wl("other");
  mock_system_xclip("xclip");
  mock_system_xclip("other");
  mock_system_xsel("xsel");
  mock_system_xsel("other");
  mock_system_always_fail("");
  mock_popen_dummy("", "");
  mock_pclose_dummy(NULL);
#endif

  /* Force mock to avoid xclip hanging ctest */
  g_mock_system_fn = mock_system_always_fail;

  printf("Testing invalid arguments...\n");
  check_cond(ui_clipboard_set_text(NULL) != UI_ERROR_INVALID_ARGUMENT, &failed);
  check_cond(ui_clipboard_get_text(NULL) != UI_ERROR_INVALID_ARGUMENT, &failed);
  ui_clipboard_free_text(NULL);

  /* Read initially (may return system clipboard contents or UNSUPPORTED, don't
   * strictly fail on it) */
  rc = ui_clipboard_get_text(&text);
  if (rc == UI_ERROR_NONE) {
    if (text) {
      ui_clipboard_free_text(text);
      text = NULL;
    }
  }

  printf("Testing set and get text...\n");
  rc = ui_clipboard_set_text(test_str);
  check_cond(rc != UI_ERROR_NONE, &failed);
  rc = ui_clipboard_get_text(&text);
  check_cond(rc != UI_ERROR_NONE || !text || strlen(text) == 0, &failed);
  check_cond(strcmp(text, test_str) != 0, &failed);
  ui_clipboard_free_text(text);

  printf("Testing fallback...\n");
  g_ui_clipboard_force_fallback = 1;
  rc = ui_clipboard_set_text(test_str);
  check_cond(rc != UI_ERROR_NONE, &failed);

  /* Overwrite fallback */
  rc = ui_clipboard_set_text("Overwrite");
  check_cond(rc != UI_ERROR_NONE, &failed);

  rc = ui_clipboard_get_text(&text);
  check_cond(rc != UI_ERROR_NONE || strcmp(text, "Overwrite") != 0, &failed);
  ui_clipboard_free_text(text);

  /* Fallback memory failures */
  g_malloc_fail_countdown = 0;
  rc = ui_clipboard_set_text("Fail");
  check_cond(rc != UI_ERROR_OUT_OF_MEMORY, &failed);
  g_malloc_fail_countdown = -1;

  rc = ui_clipboard_set_text("Success");
  check_cond(rc != UI_ERROR_NONE, &failed);

  g_malloc_fail_countdown = 0;
  rc = ui_clipboard_get_text(&text);
  check_cond(rc != UI_ERROR_OUT_OF_MEMORY, &failed);
  g_malloc_fail_countdown = -1;

  g_ui_clipboard_force_fallback = 0;

#if !defined(_WIN32)
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
      check_cond(strcmp(text, large_text) != 0, &failed);
      ui_clipboard_free_text(text);
    }

    /* Test real clipboard realloc failure (requires popen to return text, so we
     * do it after successful set) */
    g_malloc_fail_countdown = 1; /* 0 is first malloc, 1 is the first realloc */
    rc = ui_clipboard_get_text(&text);
    g_malloc_fail_countdown = -1;
    if (rc != UI_ERROR_OUT_OF_MEMORY && rc != UI_ERROR_NONE) {
      failed = 1;
      printf("Failed at line %d, rc=%d\n", __LINE__, rc);
    }

    g_mock_system_fn = mock_system_always_fail;
    g_mock_popen_fn = POPEN_CMD;
    g_mock_pclose_fn = PCLOSE_CMD;
  }

  /* Test real clipboard first malloc failure */
  g_mock_system_fn = mock_system_wl;
  g_mock_popen_fn = mock_popen_large;
  g_mock_pclose_fn = mock_pclose_dummy;
  g_malloc_fail_countdown = 0;
  rc = ui_clipboard_get_text(&text);
  g_malloc_fail_countdown = -1;
  g_mock_system_fn = mock_system_always_fail;
  g_mock_popen_fn = POPEN_CMD;
  g_mock_pclose_fn = PCLOSE_CMD;

  printf("Testing popen failure for pbpaste...\n");
  g_mock_popen_fn = mock_popen_dummy;
  rc = ui_clipboard_get_text(&text);
  check_cond(rc != UI_ERROR_NONE, &failed);
  if (text)
    ui_clipboard_free_text(text);
  g_mock_popen_fn = POPEN_CMD;

  printf("Testing popen failure for pbcopy...\n");
  g_mock_popen_fn = mock_popen_dummy;
  rc = ui_clipboard_set_text("mock popen fail");
  check_cond(rc != UI_ERROR_NONE, &failed);
  g_mock_popen_fn = POPEN_CMD;

  printf("Testing empty real clipboard...\n");
  g_mock_system_fn = mock_system_wl;
  g_mock_popen_fn = mock_popen_empty;
  g_mock_pclose_fn = mock_pclose_dummy;
  rc = ui_clipboard_get_text(&text);
  if (rc == UI_ERROR_NONE) {
    if (text)
      ui_clipboard_free_text(text);
  }
  g_mock_system_fn = mock_system_always_fail;
  g_mock_popen_fn = POPEN_CMD;
  g_mock_pclose_fn = PCLOSE_CMD;

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

  g_mock_system_fn = system;
  g_mock_popen_fn = POPEN_CMD;
  g_mock_pclose_fn = PCLOSE_CMD;
#endif

  printf("Testing cleanup...\n");
  ui_clipboard_cleanup();
  /* Repeated cleanup should be safe */
  ui_clipboard_cleanup();

  /* Get text after cleanup on fallback */
  g_ui_clipboard_force_fallback = 1;
  rc = ui_clipboard_get_text(&text);
  if (rc != UI_ERROR_UNSUPPORTED) {
    failed = 1;
    printf("Failed at line %d, rc=%d\n", __LINE__, rc);
  }
  g_ui_clipboard_force_fallback = 0;

  return failed;
}

int main(void) {
  int failed = 0;
#if !defined(_WIN32)
  signal(SIGPIPE, SIG_IGN);
#endif

#ifdef _WIN32
  if (getenv("WINELOADER") != NULL) {
    printf("Skipping test_ui_clipboard under Wine CI to avoid pipe hangs.\n");
    return 0;
  }
#endif

  check_cond(run_normal_tests() != 0, &failed);

  if (!failed) {
    printf("test_ui_clipboard passed.\n");
  }
  return failed;
}
