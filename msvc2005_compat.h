/* clang-format off */
#ifndef MSVC2005_COMPAT_H
#define MSVC2005_COMPAT_H

#if defined(_MSC_VER) && _MSC_VER < 1500
#include <winsock2.h>
#ifndef ENOTSUP
#define ENOTSUP 129
#endif
#ifndef ENODATA
#define ENODATA 120
#endif
typedef struct _CONDITION_VARIABLE { HANDLE events[2]; int waiters; CRITICAL_SECTION lock; } CONDITION_VARIABLE, *PCONDITION_VARIABLE;
static __inline void InitializeConditionVariable(PCONDITION_VARIABLE cv) { cv->waiters = 0; InitializeCriticalSection(&cv->lock); cv->events[0] = CreateEventA(NULL, FALSE, FALSE, NULL); cv->events[1] = CreateEventA(NULL, TRUE, FALSE, NULL); }
static __inline BOOL SleepConditionVariableCS(PCONDITION_VARIABLE cv, PCRITICAL_SECTION cs, DWORD dwMilli) { int res, last; EnterCriticalSection(&cv->lock); cv->waiters++; LeaveCriticalSection(&cv->lock); LeaveCriticalSection(cs); res = WaitForMultipleObjects(2, cv->events, FALSE, dwMilli); EnterCriticalSection(&cv->lock); cv->waiters--; last = (res == WAIT_OBJECT_0 + 1) && (cv->waiters == 0); LeaveCriticalSection(&cv->lock); if (last) ResetEvent(cv->events[1]); EnterCriticalSection(cs); return (res != WAIT_TIMEOUT); }
static __inline void WakeConditionVariable(PCONDITION_VARIABLE cv) { EnterCriticalSection(&cv->lock); if (cv->waiters > 0) SetEvent(cv->events[0]); LeaveCriticalSection(&cv->lock); }
static __inline void WakeAllConditionVariable(PCONDITION_VARIABLE cv) { EnterCriticalSection(&cv->lock); if (cv->waiters > 0) SetEvent(cv->events[1]); LeaveCriticalSection(&cv->lock); }
#endif
#endif
/* clang-format on */
