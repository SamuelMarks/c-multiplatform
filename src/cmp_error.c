/* clang-format off */
#include "cmp.h"
#include "cmp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(_WIN32)

/* Forward declare Windows APIs to avoid <windows.h> */
__declspec(dllimport) void *__stdcall LoadLibraryA(const char *lpLibFileName);
typedef int (__stdcall *FARPROC_t)(void);
__declspec(dllimport) FARPROC_t __stdcall GetProcAddress(void *hModule, const char *lpProcName);
__declspec(dllimport) unsigned short __stdcall RtlCaptureStackBackTrace(unsigned long FramesToSkip, unsigned long FramesToCapture, void **BackTrace, unsigned long *BackTraceHash);
__declspec(dllimport) void *__stdcall GetCurrentProcess(void);

typedef int (__stdcall *SymInitialize_t)(void *hProcess, const char *UserSearchPath, int fInvadeProcess);
typedef unsigned long (__stdcall *SymSetOptions_t)(unsigned long SymOptions);
typedef int (__stdcall *SymFromAddr_t)(void *hProcess, unsigned __int64 Address, unsigned __int64 *Displacement, void *Symbol);

/* SYMBOL_INFO structure matching DbgHelp.h */
typedef struct _SYMBOL_INFO {
    unsigned long SizeOfStruct;
    unsigned long TypeIndex;
    unsigned __int64 Reserved[2];
    unsigned long Index;
    unsigned long Size;
    unsigned __int64 ModBase;
    unsigned long Flags;
    unsigned __int64 Value;
    unsigned __int64 Address;
    unsigned long Register;
    unsigned long Scope;
    unsigned long Tag;
    unsigned long NameLen;
    unsigned long MaxNameLen;
    char Name[1];
} SYMBOL_INFO;

#define SYMOPT_LOAD_LINES 0x00000010

/**
 * @brief cmp_dump_stack_trace
 *
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_dump_stack_trace(void) {
    void *stack[100];
    unsigned short frames;
    void *process;
    void *dbghelp;
    SymInitialize_t SymInitialize;
    SymSetOptions_t SymSetOptions;
    SymFromAddr_t SymFromAddr;
    unsigned short i;
    SYMBOL_INFO *symbol;
    unsigned __int64 displacement;

    fprintf(stderr, "--- Stack Trace ---\n");

    frames = RtlCaptureStackBackTrace(0, 100, stack, NULL);
    process = GetCurrentProcess();

    dbghelp = LoadLibraryA("Dbghelp.dll");
    if (dbghelp == NULL) {
        for (i = 0; i < frames; i++) {
            fprintf(stderr, "[%d] %p\n", i, stack[i]);
        }
        return;
    }

    SymInitialize = (SymInitialize_t)GetProcAddress(dbghelp, "SymInitialize");
    SymSetOptions = (SymSetOptions_t)GetProcAddress(dbghelp, "SymSetOptions");
    SymFromAddr = (SymFromAddr_t)GetProcAddress(dbghelp, "SymFromAddr");

    if (SymInitialize && SymSetOptions && SymFromAddr) {
        SymSetOptions(SYMOPT_LOAD_LINES);
        SymInitialize(process, NULL, 1);

        symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        if (symbol) {
            symbol->MaxNameLen = 255;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

            for (i = 0; i < frames; i++) {
                displacement = 0;
                if (SymFromAddr(process, (unsigned __int64)stack[i], &displacement, symbol)) {
                    fprintf(stderr, "[%d] %s - %p\n", i, symbol->Name, stack[i]);
                } else {
                    fprintf(stderr, "[%d] %p\n", i, stack[i]);
                }
            }
            free(symbol);
        }
    } else {
        for (i = 0; i < frames; i++) {
            fprintf(stderr, "[%d] %p\n", i, stack[i]);
        }
    }
}

#elif defined(__linux__) || defined(__APPLE__)

#include <execinfo.h>
#include <unistd.h>

/**
 * @brief cmp_dump_stack_trace
 *
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_dump_stack_trace(void) {
    void *stack[100];
    int frames;

    fprintf(stderr, "--- Stack Trace ---\n");
    frames = backtrace(stack, 100);
    backtrace_symbols_fd(stack, frames, STDERR_FILENO);
}

#else

/**
 * @brief cmp_dump_stack_trace
 *
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_dump_stack_trace(void) {
    fprintf(stderr, "Stack trace not supported on this platform.\n");
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
static void cmp_default_assert_handler(const char *msg, const char *file,
                                       int line) {
  fprintf(stderr, "Assertion failed: %s at %s:%d\n", msg, file, line);
  cmp_dump_stack_trace();
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
  g_assert_handler = handler;
  return 0;
}

/**
 * @brief cmp_crash_handler
 *
 * @param sig Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
static void cmp_crash_handler(int sig) {
  fprintf(stderr, "Caught signal %d\n", sig);
  cmp_dump_stack_trace();
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
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_assert_fail(const char *condition, const char *file, int line) {
  if (g_assert_handler) {
    g_assert_handler(condition, file, line);
  } else {
    cmp_default_assert_handler(condition, file, line);
  }
}

#include <stdarg.h>
/**
 * @brief cmp_log_debug
 *
 * @param fmt Parameter description.
 * @return Returns 0 on success, or an error code on failure.
 */
void cmp_log_debug(const char *fmt, ...) {
  va_list args;
  fprintf(stderr, "[DEBUG] ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
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
  if (out_str == NULL) {
    return CMP_ERROR_INVALID_ARG;
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
  return 0;
}
