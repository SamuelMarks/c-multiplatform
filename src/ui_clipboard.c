/**
 * @file ui_clipboard.c
 * @brief Implementation of ui_clipboard.c.
 */
/* clang-format off */
#include "ui_clipboard.h"
#include "ui_internal_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clipboard_posix.h"
/* clang-format on */

#ifdef UI_TEST_MOCK_ALLOC
extern int (*g_mock_system_fn)(const char *);
extern FILE *(*g_mock_popen_fn)(const char *, const char *);
extern int (*g_mock_pclose_fn)(FILE *);
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
/* Win32 API Forward Declarations */
__declspec(dllimport) int __stdcall OpenClipboard(void *);
__declspec(dllimport) int __stdcall EmptyClipboard(void);
__declspec(dllimport) void *__stdcall SetClipboardData(unsigned int, void *);
__declspec(dllimport) void *__stdcall GetClipboardData(unsigned int);
__declspec(dllimport) int __stdcall CloseClipboard(void);
__declspec(dllimport) int __stdcall IsClipboardFormatAvailable(unsigned int);
__declspec(dllimport) void *__stdcall GlobalAlloc(unsigned int, size_t);
__declspec(dllimport) void *__stdcall GlobalLock(void *);
__declspec(dllimport) int __stdcall GlobalUnlock(void *);
__declspec(dllimport) void *__stdcall GlobalFree(void *);

/** @cond */
#define UI_CF_TEXT 1
/** @endcond */
/** @cond */
#define UI_GMEM_MOVEABLE 0x0002
/** @endcond */

#else
#endif

/** @brief Global fallback clipboard buffer for environments lacking native
 * support */
static char *s_fallback_clipboard = NULL;

#ifdef UI_TEST_MOCK_ALLOC
extern int g_ui_clipboard_force_fallback;
/* We use simple function pointer overrides instead of ternaries to avoid branch
 * coverage holes. */
extern int (*g_mock_system_fn)(const char *);
extern FILE *(*g_mock_popen_fn)(const char *, const char *);
extern int (*g_mock_pclose_fn)(FILE *);

/** @brief internal */
#define SYSTEM_CMD(cmd) (g_mock_system_fn(cmd))
/** @brief internal */
#define POPEN_CMD(cmd, mode) (g_mock_popen_fn(cmd, mode))
/** @brief internal */
#define PCLOSE_CMD(stream) (g_mock_pclose_fn(stream))

#else
/** @brief internal */
#define SYSTEM_CMD(cmd) system(cmd)
/** @brief internal */
#define POPEN_CMD(cmd, mode) popen(cmd, mode)
/** @brief internal */
#define PCLOSE_CMD(stream) pclose(stream)
#endif

/**
 * @brief ui_clipboard_set_text.
 * @param text Parameter text.
 * @return Return value.
 */
ui_error_t ui_clipboard_set_text(const char *text) {
  size_t len;
  if (!text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }
  len = strlen(text) + 1;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_ui_clipboard_force_fallback) {
    goto fallback;
  }
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)

  {
    void *hMem = GlobalAlloc(UI_GMEM_MOVEABLE, len);
    void *locked;
    if (!hMem) {
      return UI_ERROR_OUT_OF_MEMORY;
    }

    locked = GlobalLock(hMem);
    if (!locked) {
      GlobalFree(hMem);
      return UI_ERROR_UNKNOWN;
    }

    memcpy(locked, text, len);
    GlobalUnlock(hMem);

    if (!OpenClipboard(NULL)) {
      GlobalFree(hMem);
      /* Fallback to process local if native fails */
      goto fallback;
    }

    EmptyClipboard();
    SetClipboardData(UI_CF_TEXT, hMem);
    CloseClipboard();
    return UI_ERROR_NONE;
  }
#elif defined(__APPLE__)
  {
    FILE *p = POPEN_CMD("pbcopy", "w");
    if (p) {
      fputs(text, p);
      PCLOSE_CMD(p);
      return UI_ERROR_NONE;
    }
  }
#elif defined(__linux__) || defined(__unix__)
  {
    FILE *p = NULL;
    if (SYSTEM_CMD("which wl-copy >/dev/null 2>&1") == 0) {
      p = POPEN_CMD("wl-copy", "w");
    } else if (SYSTEM_CMD("which xclip >/dev/null 2>&1") == 0) {
      p = POPEN_CMD("xclip -selection clipboard", "w");
    } else if (SYSTEM_CMD("which xsel >/dev/null 2>&1") == 0) {
      p = POPEN_CMD("xsel --clipboard --input", "w");
    }
    if (p) {
      fputs(text, p);
      PCLOSE_CMD(p);
      return UI_ERROR_NONE;
    }
  }
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) ||          \
    defined(UI_TEST_MOCK_ALLOC)
fallback:
#endif

  /* Fallback in-memory clipboard */
  if (s_fallback_clipboard) {
    C_MULTIPLATFORM_FREE(s_fallback_clipboard);
  }
  s_fallback_clipboard = (char *)C_MULTIPLATFORM_MALLOC(len);
  if (!s_fallback_clipboard) {
    return UI_ERROR_OUT_OF_MEMORY;
  }
  memcpy(s_fallback_clipboard, text, len);

  return UI_ERROR_NONE;
}

/**
 * @brief ui_clipboard_get_text.
 * @param out_text Parameter out_text.
 * @return Return value.
 */
ui_error_t ui_clipboard_get_text(char **out_text) {
  if (!out_text) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  *out_text = NULL;

#ifdef UI_TEST_MOCK_ALLOC
  if (g_ui_clipboard_force_fallback) {
    goto fallback;
  }
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
  if (IsClipboardFormatAvailable(UI_CF_TEXT)) {
    if (OpenClipboard(NULL)) {
      void *hMem = GetClipboardData(UI_CF_TEXT);
      if (hMem) {
        const char *locked = (const char *)GlobalLock(hMem);
        if (locked) {
          size_t len = strlen(locked) + 1;
          char *copy = (char *)C_MULTIPLATFORM_MALLOC(len);
          if (copy) {
            memcpy(copy, locked, len);
            *out_text = copy;
          }
          GlobalUnlock(hMem);
        }
      }
      CloseClipboard();
      if (*out_text) {
        return UI_ERROR_NONE;
      }
    }
  }
  goto fallback;
#elif defined(__APPLE__)
  {
    FILE *p = POPEN_CMD("pbpaste", "r");
    if (p) {
      char buffer[1024]; /**< buffer */
      size_t total = 0;
      size_t cap = 1024;
      char *text = (char *)C_MULTIPLATFORM_MALLOC(cap);
      if (!text) {
        PCLOSE_CMD(p);
        return UI_ERROR_OUT_OF_MEMORY;
      }
      text[0] = '\0';
      while (fgets(buffer, sizeof(buffer), p)) {
        size_t len = strlen(buffer);
        if (total + len + 1 > cap) {
          char *new_text;
          cap *= 2;
          new_text = (char *)C_MULTIPLATFORM_REALLOC(text, cap);
          if (!new_text) {
            C_MULTIPLATFORM_FREE(text);
            PCLOSE_CMD(p);
            return UI_ERROR_OUT_OF_MEMORY;
          }
          text = new_text;
        }
        memcpy(text + total, buffer, len + 1);
        total += len;
      }
      PCLOSE_CMD(p);
      if (total > 0) {
        *out_text = text;
        return UI_ERROR_NONE;
      } else {
        C_MULTIPLATFORM_FREE(text);
      }
    }
  }
#elif defined(__linux__) || defined(__unix__)
  {
    FILE *p = NULL;
    if (SYSTEM_CMD("which wl-paste >/dev/null 2>&1") == 0) {
      p = POPEN_CMD("wl-paste", "r");
    } else if (SYSTEM_CMD("which xclip >/dev/null 2>&1") == 0) {
      p = POPEN_CMD("xclip -selection clipboard -o", "r");
    } else if (SYSTEM_CMD("which xsel >/dev/null 2>&1") == 0) {
      p = POPEN_CMD("xsel --clipboard --output", "r");
    }
    if (p) {
      char buffer[1024]; /**< buffer */
      size_t total = 0;
      size_t cap = 1024;
      char *text = (char *)C_MULTIPLATFORM_MALLOC(cap);
      if (!text) {
        PCLOSE_CMD(p);
        return UI_ERROR_OUT_OF_MEMORY;
      }
      text[0] = '\0';
      while (fgets(buffer, sizeof(buffer), p)) {
        size_t len = strlen(buffer);
        if (total + len + 1 > cap) {
          char *new_text;
          cap *= 2;
          new_text = (char *)C_MULTIPLATFORM_REALLOC(text, cap);
          if (!new_text) {
            C_MULTIPLATFORM_FREE(text);
            PCLOSE_CMD(p);
            return UI_ERROR_OUT_OF_MEMORY;
          }
          text = new_text;
        }
        memcpy(text + total, buffer, len + 1);
        total += len;
      }
      PCLOSE_CMD(p);
      if (total > 0) {
        *out_text = text;
        return UI_ERROR_NONE;
      } else {
        C_MULTIPLATFORM_FREE(text);
      }
    }
  }
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) ||          \
    defined(UI_TEST_MOCK_ALLOC)
fallback:
#endif

  /* Fallback in-memory clipboard */
  if (s_fallback_clipboard) {
    size_t len = strlen(s_fallback_clipboard) + 1;
    *out_text = (char *)C_MULTIPLATFORM_MALLOC(len);
    if (!*out_text) {
      return UI_ERROR_OUT_OF_MEMORY;
    }
    memcpy(*out_text, s_fallback_clipboard, len);
    return UI_ERROR_NONE;
  }

  return UI_ERROR_UNSUPPORTED;
}

/**
 * @brief ui_clipboard_free_text.
 * @param text Parameter text.
 * @return Return value.
 */
ui_error_t ui_clipboard_free_text(char *text) {
  if (text) {
    C_MULTIPLATFORM_FREE(text);
  }
  return UI_ERROR_NONE;
}

/**
 * @brief ui_clipboard_cleanup.
 * @return Return value.
 */
ui_error_t ui_clipboard_cleanup(void) {
  if (s_fallback_clipboard) {
    C_MULTIPLATFORM_FREE(s_fallback_clipboard);
    s_fallback_clipboard = NULL;
  }
  return UI_ERROR_NONE;
}
