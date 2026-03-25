/* clang-format off */
#include "cmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#if defined(_WIN32)

/* Forward declare Windows APIs to avoid <windows.h> */
__declspec(dllimport) void *__stdcall LoadLibraryA(const char *lpLibFileName);
__declspec(dllimport) void *__stdcall GetProcAddress(void *hModule, const char *lpProcName);
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

void cmp_dump_stack_trace(void) {
    void *stack[100];
    int frames;

    fprintf(stderr, "--- Stack Trace ---\n");
    frames = backtrace(stack, 100);
    backtrace_symbols_fd(stack, frames, STDERR_FILENO);
}

#else

void cmp_dump_stack_trace(void) {
    fprintf(stderr, "Stack trace not supported on this platform.\n");
}

#endif
/* clang-format on */

static void cmp_crash_handler(int sig) {
  fprintf(stderr, "Caught signal %d\n", sig);
  cmp_dump_stack_trace();
  exit(1);
}

int cmp_crash_handler_init(void) {
  signal(SIGSEGV, cmp_crash_handler);
  signal(SIGABRT, cmp_crash_handler);
  signal(SIGILL, cmp_crash_handler);
  signal(SIGFPE, cmp_crash_handler);
  return CMP_SUCCESS;
}

void cmp_assert_fail(const char *condition, const char *file, int line) {
  fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", condition, file,
          line);
  cmp_dump_stack_trace();
  exit(1);
}