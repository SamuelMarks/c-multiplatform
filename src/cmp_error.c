/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>

#if defined(_MSC_VER)
#define FPRINTF fprintf_s
#define VFPRINTF vfprintf_s
#define SPRINTF sprintf_s
#else
#define FPRINTF fprintf
#define VFPRINTF vfprintf
#define SPRINTF sprintf
#endif


#if defined(__linux__) || defined(__APPLE__)

#include <execinfo.h>
#include <unistd.h>

/**
 * @brief cmp_dump_stack_trace
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dump_stack_trace(void) {
int rc = CMP_SUCCESS;
    void *stack[100];
    int frames;

    FPRINTF(stderr, "--- Stack Trace ---\n");
    frames = backtrace(stack, 100);
    backtrace_symbols_fd(stack, frames, STDERR_FILENO);

    return rc;
}
#else

/**
 * @brief cmp_dump_stack_trace
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_dump_stack_trace(void) {
int rc = CMP_SUCCESS;
    FPRINTF(stderr, "Stack trace not supported on this platform.\n");
    return rc;
}
#endif
/* clang-format on */

/**
 * @brief Default callback for handling assertions.
 *
 * @param msg The message describing the assertion failure.
 * @param file The source file where the assertion failed.
 * @param line The line number where the assertion failed.
 */
CMP_EXEMPT(static void cmp_default_assert_handler(const char *msg,
                                                  const char *file, int line)) {
  int rc = CMP_SUCCESS;
  FPRINTF(stderr, "Assertion failed: %s at %s:%d\n", msg, file, line);
  rc = cmp_dump_stack_trace();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to dump stack trace during assert\n");
  }
  abort();
}

/**
 * @brief Global pointer to the current assertion handler.
 */
static cmp_assert_handler_t g_assert_handler = cmp_default_assert_handler;

/**
 * @brief Sets a custom assertion handler.
 *
 * @param handler The custom handler to use.
 * @return int Returns 0 on success.
 */
int cmp_set_assert_handler(cmp_assert_handler_t handler) {
  int rc = CMP_SUCCESS;
  g_assert_handler = handler;
  return rc;
}

/**
 * @brief cmp_crash_handler
 *
 * @param sig Parameter description.
 */
CMP_EXEMPT(static void cmp_crash_handler(int sig)) {
  int rc = CMP_SUCCESS;
  FPRINTF(stderr, "Caught signal %d\n", sig);
  rc = cmp_dump_stack_trace();
  if (rc != CMP_SUCCESS) {
    LOG_DEBUG("Failed to dump stack trace during crash\n");
  }
  exit(1);
}

/**
 * @brief cmp_crash_handler_init
 *
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_crash_handler_init(void) {
  int rc = CMP_SUCCESS;
  signal(SIGSEGV, cmp_crash_handler);
  signal(SIGABRT, cmp_crash_handler);
  signal(SIGILL, cmp_crash_handler);
  signal(SIGFPE, cmp_crash_handler);
  return rc;
}

/**
 * @brief cmp_assert_fail
 *
 * @param condition Parameter description.
 * @param file Parameter description.
 * @param line Parameter description.
 */
CMP_EXEMPT(void cmp_assert_fail(const char *condition, const char *file,
                                int line)) {
  if (g_assert_handler) {
    g_assert_handler(condition, file, line);
  } else {
    cmp_default_assert_handler(condition, file, line);
  }
}

/**
 * @brief cmp_log_debug
 *
 * @param fmt Parameter description.
 */
CMP_EXEMPT(void cmp_log_debug(const char *fmt, ...)) {
  va_list args;
  FPRINTF(stderr, "[DEBUG] ");
  va_start(args, fmt);
  VFPRINTF(stderr, fmt, args);
  va_end(args);
}

/**
 * @brief cmp_strerror
 *
 * @param error Parameter description.
 * @param out_str Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
int cmp_strerror(int error, const char **out_str) {
  int rc = CMP_SUCCESS;

  if (out_str == NULL) {
    LOG_DEBUG("out_str pointer is NULL\n");
    rc = CMP_ERROR_INVALID_ARG;
    return rc;
  }

  switch (error) {
  case 0:
    *out_str = "Success";
    break;
  case CMP_ERROR_OOM:
    *out_str = "Out of memory";
    break;
  case CMP_ERROR_INVALID_ARG:
    *out_str = "Invalid argument provided";
    break;
  case CMP_ERROR_NOT_FOUND:
    *out_str = "Resource not found";
    break;
  case CMP_ERROR_BOUNDS:
    *out_str = "Out of bounds access";
    break;
  case CMP_ERROR_IO:
    *out_str = "I/O error";
    break;
  case CMP_ERROR_INVALID_STATE:
    *out_str = "Object in invalid state";
    break;
  case CMP_ERROR_GENERAL:
    *out_str = "General/Unknown error";
    break;
  default:
    *out_str = "Unknown error code";
    break;
  }

  return rc;
}
